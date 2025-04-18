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

#include "hal.h"
#include "hal_platform.h"
#include "hal_msdc.h"
#include "hal_sdio_internal.h"
#include "hal_sdio.h"

#ifdef HAL_SDIO_MODULE_ENABLED
#include "hal_gpt.h"
#include "hal_log.h"

sdio_information_t sdio_information;
static uint8_t sdio_cccr_register[SDIO_CCCR_SIZE];
static uint8_t sdio_fbr_register[SDIO_FBR_SIZE];
static uint8_t sdio_tuple[HAL_SDIO_FUNCTION_1 + 1][SDIO_TUPLE_SIZE];
static uint32_t sdio_dma_interrupt_gpt_timer;
#ifdef HAL_SLEEP_MANAGER_ENABLED
static volatile sdio_backup_parameter_t sdio_backup_parameter;
static volatile MSDC_REGISTER_T sdio_backup_register;
#endif
extern msdc_config_t msdc_config[];

sdio_internal_status_t sdio_wait_command_ready(hal_sdio_port_t sdio_port)
{
    uint32_t interrupt_status;
    uint32_t start_count = 0;
    uint32_t end_count = 0;
    uint32_t count = 0;
    uint32_t loop_count = 0;

    MSDC_REGISTER_T *msdc_register_base;

    msdc_register_base = MSDC0_REG;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &start_count);
    do {
        interrupt_status = msdc_register_base->MSDC_INT;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &end_count);
        hal_gpt_get_duration_count(start_count, (end_count + 10), &count);
        loop_count++;
    } while ((!(interrupt_status & MSDC_CMD_INTS)) && (count < MSDC_TIMEOUT_PERIOD_COMMAND));

    while (msdc_register_base->SDC_STS & SDC_STS_CMDBUSY_MASK);

    if (count >= MSDC_TIMEOUT_PERIOD_COMMAND) {
        //log_hal_error("wait sdio command ready timeout, MSDC_INT = %x, interrupt_status = %x, loop_count = %d.\r\n", MSDC_REG->MSDC_INT, interrupt_status, loop_count);
        return ERROR_COMMANDD_TIMEOUT;
    }

    /*clear msdc interrupt status*/
    msdc_register_base->MSDC_INT =  MSDC_CMD_INTS | MSDC_ACMD_INTS;

    if (interrupt_status & MSDC_INT_CMDRDY_MASK) {
        return NO_ERROR;
    } else if (interrupt_status & MSDC_INT_RSPCRCERR_MASK) {
        return ERROR_COMMAND_CRC_ERROR;
    } else if (interrupt_status & MSDC_INT_CMDTMO_MASK) {
        //while((*(volatile uint32_t *)0xa1030000) != 0xffff0000);
        return ERROR_COMMANDD_TIMEOUT;
    }

    return NO_ERROR;
}


sdio_internal_status_t sdio_wait_data_ready(hal_sdio_port_t sdio_port)
{
    volatile uint32_t interrupt_status;
    uint32_t start_count = 0;
    uint32_t end_count = 0;
    uint32_t count = 0;

    MSDC_REGISTER_T *msdc_register_base;

    msdc_register_base = MSDC0_REG;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &start_count);
    do {
        interrupt_status = msdc_register_base->MSDC_INT;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &end_count);
        hal_gpt_get_duration_count(start_count, (end_count + 10), &count);
    } while ((!(interrupt_status & MSDC_DAT_INTS)) && (count < MSDC_TIMEOUT_PERIOD_DATA));

    /*clear msdc interrupt status*/
    msdc_register_base->MSDC_INT =  MSDC_DAT_INTS;

    if (count >= MSDC_TIMEOUT_PERIOD_DATA) {
        log_hal_msgid_error("wait sdio data ready timeout.\r\n", 0);
        return ERROR_DATA_TIMEOUT;
    }


    if (interrupt_status & MSDC_INT_DATTMO_MASK) {
        return ERROR_DATA_TIMEOUT;
    } else if (interrupt_status & MSDC_INT_DATCRCERR_MASK) {
        log_hal_msgid_error("interrupt_status = %x\r\n", 1, (unsigned int)interrupt_status);
        return ERROR_DATA_CRC_ERROR;
    } else {
        return NO_ERROR;
    }
}


sdio_internal_status_t sdio_send_command(hal_sdio_port_t sdio_port, uint32_t msdc_command, uint32_t argument)
{
    sdio_internal_status_t status;
    MSDC_REGISTER_T *msdc_register_base;

    msdc_register_base = MSDC0_REG;

    /*only wait sdc_busy bit*/
    while (msdc_register_base->SDC_STS & SDC_STS_SDCBUSY_MASK);

    /*clear msdc command interrupt*/
    msdc_register_base->MSDC_INT = MSDC_CMD_INTS | MSDC_ACMD_INTS;
    msdc_register_base->SDC_ARG = argument;
    msdc_register_base->SDC_CMD = msdc_command;

    status = sdio_wait_command_ready(sdio_port);
    if (status != NO_ERROR) {
        log_hal_msgid_error("send command error, command = %x, arg = %x \r\n", 2, (unsigned int)msdc_command, (unsigned int)argument);
        return status;
    }

    return NO_ERROR;
}


sdio_internal_status_t sdio_send_command5(hal_sdio_port_t sdio_port, uint32_t ocr)
{
    sdio_internal_status_t status;
    uint32_t response;
    MSDC_REGISTER_T *msdc_register_base;

    msdc_register_base = MSDC0_REG;

    status = sdio_send_command(sdio_port, MSDC_COMMAND5_SDIO, ocr);
    if (status != NO_ERROR) {
        return status;
    }

    response = msdc_register_base->SDC_RESP0;

    sdio_information.ocr = response & SDIO_COMMAND5_RESPONSE_OCR_BIT_MASK;
    sdio_information.number_function = (uint8_t)((response & SDIO_COMMAND5_RESPONSE_IO_FUNCTION_BIT_MASK) >> SDIO_COMMAND5_RESPONSE_IO_FUNCTION_BIT_SHIFT);
    sdio_information.is_memory_present = (response & SDIO_COMMAND5_RESPONSE_MEMORY_PRESENT_BIT_MASK) ? true : false;
    sdio_information.is_io_ready = (response & SDIO_COMMAND5_RESPONSE_IS_READY_BIT_MASK) ? true : false;

    return NO_ERROR;
}



sdio_internal_status_t sdio_get_rca(hal_sdio_port_t sdio_port)
{
    sdio_internal_status_t status;
    uint32_t response = 0;
    MSDC_REGISTER_T *msdc_register_base;

    msdc_register_base = MSDC0_REG;

    status = sdio_send_command(sdio_port, MSDC_COMMAND3, COMMAND_NO_ARGUMENT);
    if (NO_ERROR != status) {
        return status;
    }

    response = msdc_register_base->SDC_RESP0;
    sdio_information.rca = response >> COMMAND_RCA_ARGUMENT_SHIFT;

    return NO_ERROR;
}

sdio_internal_status_t sdio_check_card_status(hal_sdio_port_t sdio_port)
{
    uint32_t response;
    MSDC_REGISTER_T *msdc_register_base;

    msdc_register_base = MSDC0_REG;

    response = msdc_register_base->SDC_RESP0;

    if (!(response & SDIO_CARD_STATUS_ERROR_MASK)) {
        return NO_ERROR;
    } else {
        return ERROR_STATUS;
    }
}


sdio_internal_status_t sdio_select_card(hal_sdio_port_t sdio_port, uint32_t rca)
{
    sdio_internal_status_t status;

    status = sdio_send_command(sdio_port, MSDC_COMMAND7, rca << COMMAND_RCA_ARGUMENT_SHIFT);
    if (status !=  NO_ERROR) {
        return status;
    }

    status = sdio_check_card_status(sdio_port);
    if (status !=  NO_ERROR) {
        return status;
    }

    return NO_ERROR;
}


/**************************************************************************************************************************/
/************************************************CMD52 ********************************************************************/
/**************************************************************************************************************************/
/*| 1 | 1 |       6       |  1  |           3            |  1  |  1    |         17       |   1   |      8     |  7  | 1 |*/
/*| S | D | Command index | R/W | Numberof I/O functions | RAW | stuff | register address | stuff | write data | CRC | E |*/
/**************************************************************************************************************************/
sdio_internal_status_t sdio_command52(hal_sdio_port_t sdio_port, hal_sdio_command52_config_t *command52_config)
{
    sdio_internal_status_t status;
    uint32_t argument = 0;
    uint32_t command;
    uint32_t response;
    MSDC_REGISTER_T *msdc_register_base;

    msdc_register_base = MSDC0_REG;

    msdc_nvic_set((msdc_port_t)sdio_port, false);
    /*set R/W bit*/
    argument = (command52_config->direction == HAL_SDIO_DIRECTION_WRITE) ? (argument | COMMAND52_DIRECTION_BIT_MASK) :
               (argument & (~COMMAND52_DIRECTION_BIT_MASK));
    /*set RAW bit*/
    argument =  command52_config->read_after_write ? (argument | COMMAND52_RAW_BIT_MASK) :
                (argument & (~COMMAND52_RAW_BIT_MASK));
    /*set Numberof I/O functions bit*/
    argument = (argument & (~COMMAND52_FUNCTION_NUMBER_BIT_MASK)) |
               ((uint32_t)command52_config->function << COMMAND52_FUNCTION_NUMBER_BIT_SHIFT);
    /*set register address bit*/
    argument = (argument & (~COMMAND52_REGISTER_ADDRESS_BIT_MASK)) |
               ((uint32_t)command52_config->address << COMMAND52_REGISTER_ADDRESS_BIT_SHIFT);
    /*set write data*/
    if (command52_config->direction == HAL_SDIO_DIRECTION_WRITE) {
        argument = (argument & (~COMMAND52_WRITE_DATA_BIT_MASK)) | command52_config->data;
    }

    command = MSDC_COMMAND52_SDIO;

    command = command52_config->stop ? (command | SDC_CMD_STOP_MASK) :
              (command & (~SDC_CMD_STOP_MASK));

    command = command52_config->stop ? (command | COMMAND_R1B_RESPONSE) :
              ((command & (~COMMAND_R1B_RESPONSE)) | COMMAND_R1_RESPONSE);

    status =  sdio_send_command(sdio_port, command, argument);
    if (status != NO_ERROR) {
        goto error;
    }

    response = msdc_register_base->SDC_RESP0;

    if ((command52_config->direction == HAL_SDIO_DIRECTION_READ) ||
        (command52_config->direction == HAL_SDIO_DIRECTION_WRITE && command52_config->read_after_write)) {
        command52_config->data =  response & SDIO_R5_RESPONSE_DATA_BIT_MASK;
    }

    sdio_information.state = (sdio_state_t)((response & SDIO_R5_RESPONSE_FLAG_STATE_BIT_MASK) >> SDIO_R5_RESPONSE_FLAG_STATE_BIT_SHIFT);
    sdio_information.response = (response & SDIO_R5_RESPONSE_FLAG_BIT_MASK) >> SDIO_R5_RESPONSE_FLAG_BIT_SHIFT;

    if (sdio_information.response & SDIO_R5_RESPONSE_FLAG_ERROR_BIT_MASK) {
        goto error;
    }

    msdc_nvic_set((msdc_port_t)sdio_port, true);
    return NO_ERROR;

error:
    *(volatile uint32_t *)0xA2010060 = *(volatile uint32_t *)0xA2010060 | 0x8000;

    msdc_nvic_set((msdc_port_t)sdio_port, true);

    return status;
}

sdio_internal_status_t sdio_get_cccr(hal_sdio_port_t sdio_port)
{
    sdio_internal_status_t status;
    uint32_t i = 0;
    hal_sdio_command52_config_t command52;

    command52.direction = HAL_SDIO_DIRECTION_READ;
    command52.function = HAL_SDIO_FUNCTION_0;
    command52.read_after_write = false;
    command52.stop = false;

    for (i = 0; i < SDIO_CCCR_SIZE; i++) {
        command52.address = i;
        status =  sdio_command52(sdio_port, &command52);
        if (status != NO_ERROR) {
            return status;
        }
        sdio_cccr_register[i] = command52.data;
    }

    log_hal_msgid_info("cccr_ret[13] = %d \r\n", 1, sdio_cccr_register[13]);

    return NO_ERROR;
}

void sdio_get_capacity(hal_sdio_port_t sdio_port)
{
    sdio_information.capability = sdio_cccr_register[8];
}

void sdio_get_power_control(hal_sdio_port_t sdio_port)
{
    sdio_information.power_control = sdio_cccr_register[18];
}

void sdio_get_high_speed(hal_sdio_port_t sdio_port)
{
    sdio_information.high_speed = sdio_cccr_register[19];
}

/*true -- low speed card*/
bool sdio_is_support_lsc(hal_sdio_port_t sdio_port)
{
    return ((sdio_information.capability & SDIO_CCCR_LSC_BIT_MASK) >> SDIO_CCCR_LSC_BIT_SHIFT);
}

bool sdio_is_support_s4mi(hal_sdio_port_t sdio_port)
{
    return ((sdio_information.capability & SDIO_CCCR_S4MI_BIT_MASK) >> SDIO_CCCR_S4MI_BIT_SHIFT);
}

bool sdio_is_support_shs(hal_sdio_port_t sdio_port)
{
    return ((sdio_information.high_speed & SDIO_CCCR_SHS_BIT_MASK) >> SDIO_CCCR_SHS_BIT_SHIFT);
}

bool sdio_is_support_4bls(hal_sdio_port_t sdio_port)
{
    return ((sdio_information.capability & SDIO_CCCR_4BLS_BIT_MASK) >> SDIO_CCCR_4BLS_BIT_SHIFT);
}

bool sdio_is_support_mps(hal_sdio_port_t sdio_port)
{
    return (sdio_information.power_control & 0x01);
}

sdio_internal_status_t sdio_set_mps(hal_sdio_port_t sdio_port, bool is_enable)
{
    sdio_internal_status_t status;
    hal_sdio_command52_config_t command52;

    command52.direction = HAL_SDIO_DIRECTION_WRITE;
    command52.function = HAL_SDIO_FUNCTION_0;
    command52.read_after_write = false;
    command52.stop = false;
    command52.address = 0x12;

    command52.data = is_enable ? 0x02 : 0;
    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }

    sdio_information.power_control = is_enable ? (sdio_information.power_control | 0x02) :
                                     (sdio_information.power_control & (~0x02));

    return NO_ERROR;
}


sdio_internal_status_t sdio_set_e4mi(hal_sdio_port_t sdio_port, bool is_enable)
{
    sdio_internal_status_t status;
    hal_sdio_command52_config_t command52;

    if (!(sdio_is_support_s4mi(sdio_port))) {
        return ERROR_SDIO_NOT_SUPPORT_4MI;
    }

    command52.direction = HAL_SDIO_DIRECTION_WRITE;
    command52.function = HAL_SDIO_FUNCTION_0;
    command52.read_after_write = false;
    command52.stop = false;
    command52.address = 0x08;

    command52.data = is_enable ? SDIO_CCCR_E4MI_BIT_MASK : 0;

    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }

    /*read back and compare*/
    command52.direction = HAL_SDIO_DIRECTION_READ;

    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }
    if (!(command52.data & SDIO_CCCR_E4MI_BIT_MASK)) {
        /*need add assert here*/
    }

    sdio_information.capability = is_enable ? (sdio_information.capability | SDIO_CCCR_E4MI_BIT_MASK) :
                                  (sdio_information.capability & (~SDIO_CCCR_E4MI_BIT_MASK));

    return NO_ERROR;
}

sdio_internal_status_t sdio_set_ehs(hal_sdio_port_t sdio_port, bool is_enable)
{
    sdio_internal_status_t status;
    hal_sdio_command52_config_t command52;

    if (!(sdio_is_support_shs(sdio_port))) {
        return ERROR_HIGH_SPEED_NOT_SUPPORT;
    }

    command52.direction = HAL_SDIO_DIRECTION_WRITE;
    command52.function = HAL_SDIO_FUNCTION_0;
    command52.read_after_write = false;
    command52.stop = false;
    command52.address = 0x13;

    command52.data = is_enable ? SDIO_CCCR_EHS_BIT_MASK : 0;

    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }

    /*read back and compare*/
    command52.direction = HAL_SDIO_DIRECTION_READ;

    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }
    if (!(command52.data & SDIO_CCCR_EHS_BIT_MASK)) {
        /*need add assert here*/
    }

    return NO_ERROR;
}

sdio_internal_status_t sdio_set_io(hal_sdio_port_t sdio_port, hal_sdio_function_id_t function, bool is_enable)
{
    sdio_internal_status_t status;
    hal_sdio_command52_config_t command52;

    command52.direction = HAL_SDIO_DIRECTION_READ;
    command52.function = HAL_SDIO_FUNCTION_0;
    command52.read_after_write = false;
    command52.stop = false;
    command52.address = 2;
    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }

    command52.data = is_enable ? (command52.data | (1 << function)) : (command52.data & (~(1 << function)));
    command52.direction = HAL_SDIO_DIRECTION_WRITE;
    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }

    return NO_ERROR;
}

#ifdef USE_SDIO_DTA1_INT
sdio_internal_status_t sdio_set_function_interrupt(hal_sdio_port_t sdio_port, hal_sdio_function_id_t function, bool is_enable)
{
    sdio_internal_status_t status;
    hal_sdio_command52_config_t command52;

    command52.direction = HAL_SDIO_DIRECTION_WRITE;
    command52.function = HAL_SDIO_FUNCTION_0;
    command52.read_after_write = false;
    command52.stop = false;
    command52.address = 0x04;

    command52.data = is_enable ? (0x01 | (1 << function)) : 0;

    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }

    /*read back and compare*/
    command52.direction = HAL_SDIO_DIRECTION_READ;

    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }
    if (!(command52.data & 0x03)) {
        /*need add assert here*/
    }

    return NO_ERROR;
}
#endif

sdio_internal_status_t sdio_check_io_ready(hal_sdio_port_t sdio_port, hal_sdio_function_id_t function, bool *is_ready)
{
    sdio_internal_status_t status;
    hal_sdio_command52_config_t command52;

    command52.direction = HAL_SDIO_DIRECTION_READ;
    command52.function = HAL_SDIO_FUNCTION_0;
    command52.read_after_write = false;
    command52.stop = false;
    command52.address = 3;
    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }

    *is_ready = (command52.data & (function << 1)) >> function;

    return NO_ERROR;
}


sdio_internal_status_t sdio_get_fbr(hal_sdio_port_t sdio_port, hal_sdio_function_id_t function)
{
    sdio_internal_status_t status;
    hal_sdio_command52_config_t command52;
    uint32_t i = 0;

    command52.direction = HAL_SDIO_DIRECTION_READ;
    command52.function = HAL_SDIO_FUNCTION_0;
    command52.read_after_write = false;
    command52.stop = false;

    for (i = 0; i < SDIO_FBR_SIZE; i++) {
        command52.address = 0x100 * function + i;
        status = sdio_command52(sdio_port, &command52);
        if (status != NO_ERROR) {
            return status;
        }
        sdio_fbr_register[i] = command52.data;
    }

    return status;
}


sdio_internal_status_t sdio_get_cis(hal_sdio_port_t sdio_port, hal_sdio_function_id_t function)
{
    sdio_internal_status_t status;
    hal_sdio_command52_config_t command52;
    uint32_t index = 0;
    uint32_t tuple_size = 0;
    uint32_t i = 0;
    uint32_t manfid_index = 0;

    while (1) {
        command52.direction = HAL_SDIO_DIRECTION_READ;
        command52.function = function;
        command52.read_after_write = false;
        command52.stop = false;
        command52.data = 0;

        command52.address = (function ==  HAL_SDIO_FUNCTION_0) ?
                            ((uint32_t)((((uint32_t)sdio_cccr_register[11] << 16) | ((uint32_t)sdio_cccr_register[10] << 8) | (uint32_t)sdio_cccr_register[9]) + index)) :
                            ((uint32_t)((((uint32_t)sdio_fbr_register[11] << 16) | ((uint32_t)sdio_fbr_register[10] << 8) | (uint32_t)sdio_fbr_register[9]) + index));

        status = sdio_command52(sdio_port, &command52);
        if (status != NO_ERROR) {
            return status;
        }

        sdio_tuple[function][index] = command52.data;
        index++;

        if ((command52.data == CISTPL_NULL) || (command52.data == CISTPL_END)) {
            return NO_ERROR;
        } else {
            tuple_size = 0;
            command52.direction = HAL_SDIO_DIRECTION_READ;
            command52.function = function;
            command52.read_after_write = false;
            command52.stop = false;
            command52.data = 0;
            command52.address++;

            /*read TPL_LINK*/
            status = sdio_command52(sdio_port, &command52);
            if (status != NO_ERROR) {
                return status;
            }

            sdio_tuple[function][index] = command52.data;
            tuple_size = command52.data;
            index++;

            for (i = 0; i < tuple_size; i++) {
                command52.address++;

                status = sdio_command52(sdio_port, &command52);
                if (status != NO_ERROR) {
                    return status;
                }

                sdio_tuple[function][index] = command52.data;
                index++;
            }

            if (CISTPL_MANFID == sdio_tuple[function][index - tuple_size - 2]) {
                manfid_index = index - tuple_size;
                /*TPLMID_MANF*/
                sdio_information.cis.vendor = (uint16_t)sdio_tuple[function][manfid_index] | (uint16_t)sdio_tuple[function][manfid_index + 1];
                /*TPLMID_CARD*/
                sdio_information.cis.device = (uint16_t)sdio_tuple[function][manfid_index + 2] | (uint16_t)sdio_tuple[function][manfid_index + 3];
            }
        }
    }
}



sdio_internal_status_t sdio_stop(hal_sdio_port_t sdio_port)
{
    sdio_internal_status_t status;
    hal_sdio_command52_config_t command52;

    command52.direction = HAL_SDIO_DIRECTION_WRITE;
    command52.function = HAL_SDIO_FUNCTION_0;
    command52.read_after_write = false;
    command52.stop = true;
    command52.address = 6;
    command52.data = 1;

    status = sdio_command52(sdio_port, &command52);
    if (status != NO_ERROR) {
        return status;
    }

    return NO_ERROR;
}

sdio_internal_status_t sdio_command12_stop(hal_sdio_port_t sdio_port)
{
    sdio_internal_status_t status;

    status = sdio_send_command(sdio_port, MSDC_COMMAND12, COMMAND_NO_ARGUMENT);
    if (status != NO_ERROR) {
        return status;
    }

    status = sdio_check_card_status(sdio_port);
    if (status != NO_ERROR) {
        return status;
    }

    return NO_ERROR;
}

void sdio_isr(void)
{
    /*SDIO isr handler.*/
}

static void sdio_dma_interrput_gpt_callback(void *user_data)
{
    uint32_t wints = MSDC_INTEN_XFER_COMPL_MASK |    /* To detect one GPD data transfer complete */
                     MSDC_INTEN_DATTMO_MASK |        /* To detect data transfer timdout */
                     MSDC_INTEN_DATCRCERR_MASK;        /* To detect data transfer error */
    static uint8_t count = 0;

    /*100ms gpt timeout*/
    if (count < 100) {
        count++;
    } else {
        count = 0;
        /*free gpt timer*/
        hal_gpt_sw_free_timer(sdio_dma_interrupt_gpt_timer);
        msdc_reset(MSDC_PORT_0);
        msdc_nvic_set(MSDC_PORT_0, true);

        /*call user callback*/
        msdc_config[MSDC_PORT_0].msdc_sdio_dma_interrupt_callback(HAL_SDIO_EVENT_TRANSFER_ERROR, (void *)0);
        sdio_information.is_busy = false;

        return;
    }

    if ((!(MSDC0_REG->DMA_CTRL & MSDC_DMA_CTRL_STOP_MASK)) || (!(MSDC0_REG->DMA_CFG & MSDC_DMA_CFG_STS_MASK))) {
        count = 0;
        /*free gpt timer*/
        hal_gpt_sw_free_timer(sdio_dma_interrupt_gpt_timer);

        MSDC0_REG->MSDC_INTEN &= ~wints;
        MSDC0_REG->MSDC_INT |= wints;

        msdc_reset(MSDC_PORT_0);
        msdc_nvic_set(MSDC_PORT_0, true);
        msdc_reset(MSDC_PORT_0);

        /*call user callback*/
        msdc_config[MSDC_PORT_0].msdc_sdio_dma_interrupt_callback(HAL_SDIO_EVENT_SUCCESS, (void *)0);
        sdio_information.is_busy = false;
    } else {
        hal_gpt_sw_get_timer(&sdio_dma_interrupt_gpt_timer);
        hal_gpt_sw_start_timer_ms(sdio_dma_interrupt_gpt_timer, 1, sdio_dma_interrput_gpt_callback, NULL);
    }
}

hal_sdio_status_t sdio_wait_dma_interrupt_transfer_ready(hal_sdio_port_t sdio_port)
{
    uint32_t wints = MSDC_INTEN_XFER_COMPL_MASK |    /* To detect one GPD data transfer complete */
                     MSDC_INTEN_DATTMO_MASK |        /* To detect data transfer timdout */
                     MSDC_INTEN_DATCRCERR_MASK;      /* To detect data transfer error */
    MSDC_REGISTER_T *msdc_register_base;

    msdc_register_base = MSDC0_REG;


    msdc_register_base->DMA_CTRL |= MSDC_DMA_CTRL_STOP_MASK;
    if ((!(msdc_register_base->DMA_CTRL & MSDC_DMA_CTRL_STOP_MASK)) || (!(msdc_register_base->DMA_CFG & MSDC_DMA_CFG_STS_MASK))) {
        msdc_register_base->MSDC_INTEN &= ~wints;
        msdc_register_base->MSDC_INT |= wints;
        msdc_dma_stop((msdc_port_t)sdio_port);
        msdc_reset((msdc_port_t)sdio_port);
        msdc_nvic_set((msdc_port_t)sdio_port, false);

        /*call user callback*/
        msdc_config[sdio_port].msdc_sdio_dma_interrupt_callback(HAL_SDIO_EVENT_SUCCESS, (void *)0);
        sdio_information.is_busy = false;
    } else {
        hal_gpt_sw_get_timer(&sdio_dma_interrupt_gpt_timer);
        hal_gpt_sw_start_timer_ms(sdio_dma_interrupt_gpt_timer, 1, sdio_dma_interrput_gpt_callback, NULL);
    }

    return HAL_SDIO_STATUS_OK;
}

sdio_internal_status_t sdio_check_msdc_port_valid(hal_sdio_port_t sdio_port)
{
    if ((msdc_port_t)sdio_port >= MSDC_PORT_MAX) {
        log_hal_msgid_error("SDIO:msdc port error number!\r\n", 0);
        return ERROR_STATUS;
    }

    return NO_ERROR;
}

#ifdef USE_SDIO_DTA1_INT
hal_sdio_status_t sdio_interrupt_register_callback(hal_sdio_port_t sdio_port, hal_sdio_callback_t sdio_callback, void *user_data)
{
    if (!sdio_callback) {
        return HAL_SDIO_STATUS_ERROR;
    }

    msdc_config[(msdc_port_t)sdio_port].msdc_sdio_interrupt_callback = sdio_callback;
    return HAL_SDIO_STATUS_OK;
}
#endif


#ifdef HAL_SLEEP_MANAGER_ENABLED
void sdio_backup_all_register(hal_sdio_port_t sdio_port)
{
    MSDC_REGISTER_T *msdc_register_base;

    msdc_register_base = MSDC0_REG;

    sdio_backup_register.MSDC_CFG     = msdc_register_base->MSDC_CFG;
    sdio_backup_register.MSDC_IOCON   = msdc_register_base->MSDC_IOCON;
    sdio_backup_register.MSDC_INTEN   = msdc_register_base->MSDC_INTEN;
    sdio_backup_register.SDC_CFG      = msdc_register_base->SDC_CFG;
}


void sdio_restore_all_register(hal_sdio_port_t sdio_port)
{
    MSDC_REGISTER_T *msdc_register_base;

    msdc_register_base = MSDC0_REG;

    /*must be enable SD first*/
    msdc_register_base->MSDC_CFG = msdc_register_base->MSDC_CFG | MSDC_CFG_MODE_MASK;

    msdc_register_base->MSDC_CFG     = sdio_backup_register.MSDC_CFG;
    msdc_register_base->MSDC_IOCON   = sdio_backup_register.MSDC_IOCON;
    msdc_register_base->MSDC_INTEN   = sdio_backup_register.MSDC_INTEN;
    msdc_register_base->SDC_CFG      = sdio_backup_register.SDC_CFG;
}

void sdio_backup_all(void *data, uint32_t mode)
{
    if (!sdio_information.is_initialized) {
        sdio_backup_parameter.is_initialized = false;
        return;
    } else {
        sdio_backup_parameter.is_initialized = true;
    }

    sdio_backup_all_register(sdio_backup_parameter.sdio_port);
}


void sdio_restore_all(void *data, uint32_t mode)
{
    if (!sdio_backup_parameter.is_initialized) {
        return;
    }

    sdio_restore_all_register(sdio_backup_parameter.sdio_port);
    sdio_backup_parameter.is_initialized = false;

    /*call once to make MSDC enter transfer state*/
    sdio_get_cccr(sdio_backup_parameter.sdio_port);
}
#endif
#endif /*HAL_SDIO_MODULE_ENABLED*/


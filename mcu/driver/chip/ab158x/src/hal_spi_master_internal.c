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

#include "hal_spi_master.h"

#ifdef HAL_SPI_MASTER_MODULE_ENABLED
#include <string.h>
#include "hal_spi_master_internal.h"
#include "hal_log.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_dvfs_internal.h"
#include "hal_nvic.h"
#undef HAL_DVFS_MODULE_ENABLED
SPIM_REGISTER_T *const g_spi_master_register[HAL_SPI_MASTER_MAX] = {SPI_MASTER_0, SPI_MASTER_1, SPI_MASTER_2};
#ifdef HAL_SLEEP_MANAGER_ENABLED
static uint32_t g_spi_master_ctrl_0_reg[HAL_SPI_MASTER_MAX];
static uint32_t g_spi_master_ctrl_1_reg[HAL_SPI_MASTER_MAX];
static uint32_t g_spi_master_cfg_0_reg[HAL_SPI_MASTER_MAX];
static uint32_t g_spi_master_cfg_1_reg[HAL_SPI_MASTER_MAX];
static uint32_t g_spi_master_cfg_2_reg[HAL_SPI_MASTER_MAX];
static uint32_t g_spi_master_cfg_3_reg[HAL_SPI_MASTER_MAX];
static uint8_t g_spi_master_cfg_4_reg[HAL_SPI_MASTER_MAX][10];
extern sleep_management_lock_request_t g_spi_master_sleep_handle[HAL_SPI_MASTER_MAX];
#endif

extern spi_master_direction_t g_spi_master_direction[HAL_SPI_MASTER_MAX];

void spi_master_isr_handler(hal_spi_master_port_t master_port, hal_spi_master_callback_t user_callback, void *user_data)
{
    hal_spi_master_callback_event_t event;
    uint32_t interrupt_status = 0;

    interrupt_status = g_spi_master_register[master_port]->INT;

    if (interrupt_status & SPIM_INT_FINISH_INT_MASK) {
        switch (g_spi_master_direction[master_port]) {
            case SPI_MASTER_TX:
                event = HAL_SPI_MASTER_EVENT_SEND_FINISHED;
                break;
            case SPI_MASTER_RX:
                event = HAL_SPI_MASTER_EVENT_RECEIVE_FINISHED;
                break;
            default:
                event = HAL_SPI_MASTER_EVENT_RECEIVE_FINISHED;
                break;
        }
        /* This is just for data corruption check */
        if (NULL != user_callback) {
            user_callback(event, user_data);
        }
#ifdef HAL_SLEEP_MANAGER_ENABLED
        hal_sleep_manager_unlock_sleep(g_spi_master_sleep_handle[master_port]);
#endif
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_UNLOCK);
#endif
    } else if (interrupt_status & SPIM_INT_PAUSE_INT_MASK) {
        spi_master_start_transfer_dma(master_port, true, false);
    }
}

void spi_master_init(hal_spi_master_port_t master_port, const hal_spi_master_config_t *spi_config)
{
    uint32_t sck_count;

    /* Using DVFS lock to make sure alwasy lock to max frequency */
    sck_count = SPI_MASTER_INPUT_CLOCK_FREQUENCY / (spi_config->clock_frequency * 2) - 1;
    g_spi_master_register[master_port]->CFG1_UNION.CFG1 = ((sck_count << SPIM_CFG1_SCK_LOW_COUNT_OFFSET) | sck_count);

    switch (spi_config->bit_order) {
        case HAL_SPI_MASTER_LSB_FIRST:
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 &= (~(SPIM_CTRL0_TXMSBF_MASK | SPIM_CTRL0_RXMSBF_MASK));
            break;
        case HAL_SPI_MASTER_MSB_FIRST:
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 |= (SPIM_CTRL0_TXMSBF_MASK | SPIM_CTRL0_RXMSBF_MASK);
            break;
    }

    switch (spi_config->polarity) {
        case HAL_SPI_MASTER_CLOCK_POLARITY0:
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 &= (~SPIM_CTRL0_CPOL_MASK);
            break;
        case HAL_SPI_MASTER_CLOCK_POLARITY1:
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 |= (SPIM_CTRL0_CPOL_MASK);
            break;
    }

    switch (spi_config->phase) {
        case HAL_SPI_MASTER_CLOCK_PHASE0:
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 &= (~SPIM_CTRL0_CPHA_MASK);
            break;
        case HAL_SPI_MASTER_CLOCK_PHASE1:
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 |= (SPIM_CTRL0_CPHA_MASK);
            break;
    }

    g_spi_master_register[master_port]->CFG4_UNION.CFG4 = (9 | (spi_config->slave_port << SPIM_CFG4_SEL_WDATA_OFFSET));

    /* default use non-paused mode*/
    g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.PAUSE_EN = 0;

    g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 |= HAL_SPI_MASTER_SINGLE_MODE << SPIM_CTRL0_TYPE_OFFSET;
}

#ifdef HAL_SPI_MASTER_FEATURE_ADVANCED_CONFIG
void spi_master_set_advanced_config(hal_spi_master_port_t master_port, const hal_spi_master_advanced_config_t *advanced_config)
{
    switch (advanced_config->byte_order) {
        case HAL_SPI_MASTER_LITTLE_ENDIAN:
            g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.CTRL1 &= ~(SPIM_CTRL1_ENDIAN_MASK);
            break;
        case HAL_SPI_MASTER_BIG_ENDIAN:
            g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.CTRL1 |= SPIM_CTRL1_ENDIAN_MASK;
            break;
    }

    switch (advanced_config->chip_polarity) {
        case HAL_SPI_MASTER_CHIP_SELECT_LOW:
            g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.CTRL1 &= (~SPIM_CTRL1_CS_POL_MASK);
            break;
        case HAL_SPI_MASTER_CHIP_SELECT_HIGH:
            g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.CTRL1 |= SPIM_CTRL1_CS_POL_MASK;
            break;
    }

    switch (advanced_config->sample_select) {
        case HAL_SPI_MASTER_SAMPLE_POSITIVE:
            g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.CTRL1 &= (~SPIM_CTRL1_SAMPLE_SEL_MASK);
            break;
        case HAL_SPI_MASTER_SAMPLE_NEGATIVE:
            g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.CTRL1 |= SPIM_CTRL1_SAMPLE_SEL_MASK;
            break;
    }

    g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.GET_DLY = (uint8_t)(advanced_config->get_tick);
}
#endif

uint32_t spi_master_get_status(hal_spi_master_port_t master_port)
{
    volatile uint32_t status;

    status = (g_spi_master_register[master_port]->STA);

    if (status == 1) {
        status = 0;
    } else {
        status = 1;
    }

    return status;
}

void spi_master_set_rwaddr(hal_spi_master_port_t master_port, spi_master_direction_t type, uint8_t *addr)
{
    switch (type) {
        case SPI_MASTER_TX:
            g_spi_master_register[master_port]->TX_SRC = (uint32_t)addr;
            break;
        case SPI_MASTER_RX:
            g_spi_master_register[master_port]->RX_DST = (uint32_t)addr;
            break;
    }
}

hal_spi_master_status_t spi_master_push_data(hal_spi_master_port_t master_port, const uint8_t *data, uint32_t size, uint32_t total_size)
{
    uint32_t spi_data = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    const uint8_t *temp_addr = data;
    uint8_t temp_data = 0;
    uint32_t quotient = 0;
    uint32_t remainder = 0;
    hal_spi_master_status_t status = HAL_SPI_MASTER_STATUS_OK;

    /* if byte_order setting is big_endian, return error */
    if ((g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.CTRL1) & SPIM_CTRL1_TX_ENDIAN_MASK) {
        log_hal_msgid_error("[SPIM%d][push_data]:big_endian error.\r\n", 1, master_port);
        return HAL_SPI_MASTER_STATUS_ERROR;
    }

    /* clear and configure packet length and count register */
    /* HW limitation: When using FIFO mode, need to configure transfer size before push data to Tx FIFO */
    g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LOOP_CNT = 0;
    g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LENGTH_CNT = (total_size - 1);

    if (size == 0) {
        spi_master_clear_fifo(master_port);
        return status;
    }

    quotient = size / sizeof(uint32_t);
    remainder = size % sizeof(uint32_t);

    for (i = 0; i < quotient; i++) {
        spi_data = 0;
        for (j = 0; j < 4; j++) {
            temp_data = (*temp_addr);
            spi_data |= (temp_data << (8 * j));
            temp_addr++;
        }
        g_spi_master_register[master_port]->TX_DATA = spi_data;
    }
    if (remainder > 0) {
        spi_data = 0;
        for (j = 0; j < 4; j++) {
            temp_data = (*temp_addr);
            spi_data |= (temp_data << (8 * j));
            temp_addr++;
        }
        switch (remainder) {
            case 3:
                g_spi_master_register[master_port]->TX_DATA = (spi_data & 0x00FFFFFF);
                break;
            case 2:
                g_spi_master_register[master_port]->TX_DATA = (spi_data & 0x0000FFFF);
                break;
            case 1:
                g_spi_master_register[master_port]->TX_DATA = (spi_data & 0x000000FF);
                break;
        }
    }

    return status;
}

hal_spi_master_status_t spi_master_pop_data(hal_spi_master_port_t master_port, uint8_t *buffer, uint32_t size)
{
    uint32_t spi_data = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    uint8_t *temp_addr = buffer;
    uint8_t temp_data = 0;
    uint32_t quotient = 0;
    uint32_t remainder = 0;
    hal_spi_master_status_t status = HAL_SPI_MASTER_STATUS_OK;

    /* if byte_order setting is big_endian, return error */
    if ((g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.CTRL1) & SPIM_CTRL1_RX_ENDIAN_MASK) {
        log_hal_msgid_error("[SPIM%d][pop_data]:big_endian error.\r\n", 1, master_port);
        return HAL_SPI_MASTER_STATUS_ERROR;
    }

    quotient = size / sizeof(uint32_t);
    remainder = size % sizeof(uint32_t);

    for (i = 0; i < quotient; i++) {
        spi_data = g_spi_master_register[master_port]->RX_DATA;
        for (j = 0; j < 4; j++) {
            temp_data = ((spi_data & (0xff << (8 * j))) >> (8 * j));
            *temp_addr = temp_data;
            temp_addr++;
        }
    }
    if (remainder > 0) {
        spi_data = g_spi_master_register[master_port]->RX_DATA;
        switch (remainder) {
            case 3:
                spi_data &= 0x00FFFFFF;
                break;
            case 2:
                spi_data &= 0x0000FFFF;
                break;
            case 1:
                spi_data &= 0x000000FF;
                break;
        }

        for (j = 0; j < remainder; j++) {
            *temp_addr = (spi_data >> (8 * j));
            temp_addr++;
        }
    }

    return status;
}

void spi_master_set_interrupt(hal_spi_master_port_t master_port, bool status)
{
    switch (status) {
        case false:
            g_spi_master_register[master_port]->IE = (~(SPIM_IE_FINISH_IE_MASK | SPIM_IE_PAUSE_IE_MASK));
            break;
        case true:
            g_spi_master_register[master_port]->IE = (SPIM_IE_FINISH_IE_MASK | SPIM_IE_PAUSE_IE_MASK);
            break;
    }
}

void spi_master_clear_fifo(hal_spi_master_port_t master_port)
{
    uint32_t i;

    for (i = 0; i < 8; i++) {
        g_spi_master_register[master_port]->TX_DATA = 0;
    }
}

void spi_master_set_mode(hal_spi_master_port_t master_port, spi_master_direction_t type, spi_master_mode_t mode)
{
    if (SPI_MASTER_TX == type) {
        switch (mode) {
            case SPI_MASTER_MODE_DMA:
                g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.TXDMA_EN = 1;
                break;
            case SPI_MASTER_MODE_FIFO:
                g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.TXDMA_EN = 0;
        }
    } else {
        switch (mode) {
            case SPI_MASTER_MODE_DMA:
                g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.RXDMA_EN = 1;
                break;
            case SPI_MASTER_MODE_FIFO:
                g_spi_master_register[master_port]->CTRL1_UNION.CTRL1_CELLS.RXDMA_EN = 0;
        }
    }
}

/*
 * g_full_packet_count = x, g_partial_packet_count = y, g_remainder_count = z
 *
 *   index     transfer_start               transfer_end       pause_mode            condition
 *     0             x                           x                  no               x==1,y==0,z==0
 *     1             x                           x                 yes               x>1, y==0,z==0
 *     2             x                           y                 yes               x>=1,y!=0,z==0
 *     3             x                           z                 yes               x>=1,y>=0,z!=0
 *     4             y                           y                  no               x==0,y!=0,z==0
 *     5             y                           z                 yes               x==0,y!=0,z!=0
 *     6             z                           z                  no               x==0,y==0,z!=0
 */

typedef enum {
    PAUSE_END_NONE,
    PAUSE_END_FULL,
    PAUSE_END_PARTIAL,
    PAUSE_END_REMAINDER,
} pause_end_t;
static const bool g_pause_mode_on[7] = {false, true, true, true, false, true, false};
static const pause_end_t g_pause_mode_off[7] = {PAUSE_END_NONE, PAUSE_END_FULL, PAUSE_END_PARTIAL, PAUSE_END_REMAINDER, PAUSE_END_NONE, PAUSE_END_REMAINDER, PAUSE_END_NONE};
static uint32_t g_pause_mode_index[HAL_SPI_MASTER_MAX] = {0};
static uint32_t g_full_packet_count[HAL_SPI_MASTER_MAX] = {0};
static uint32_t g_partial_packet_count[HAL_SPI_MASTER_MAX] = {0};
static uint32_t g_remainder_count[HAL_SPI_MASTER_MAX] = {0};

void spi_master_start_transfer_fifo(hal_spi_master_port_t master_port, bool is_write)
{
    uint32_t status;

    if (is_write == true) {
        g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 |= (1 << SPIM_CTRL0_RW_MODE_OFFSET);
    } else {
        g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 &= ~(1 << SPIM_CTRL0_RW_MODE_OFFSET);
    }

    /* HW limitation: When using FIFO mode, need to configure transfer size before push data to Tx FIFO */
    g_spi_master_register[master_port]->TRIG_UNION.TRIG_CELLS.CMD_ACT = 1;
    do {
        status = g_spi_master_register[master_port]->STA;
    } while ((status == SPIM_STATUS_BUSY_MASK));
    /* read clear the finish flag after transfer complete */
    status = (g_spi_master_register[master_port]->INT);
    status = status;
}

void spi_master_start_transfer_dma(hal_spi_master_port_t master_port, bool is_continue, bool is_write)
{
    bool continue_pause_mode = true;

    if (is_continue == true) {
        if (!((g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.PAUSE_EN) & 1)) {
            log_hal_msgid_error("[spi-%d]: pause status error.\r\n", 1, master_port);
            return;
        }
    }

    if (g_full_packet_count[master_port] > 0) {
        g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LOOP_CNT = (uint8_t)SPI_MASTER_MAX_PACKET_COUNT_MASK;
        g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LENGTH_CNT = SPI_MASTER_MAX_PACKET_LENGTH_MASK;
        g_full_packet_count[master_port]--;
        if ((g_pause_mode_off[g_pause_mode_index[master_port]] == PAUSE_END_FULL) &&
            (g_full_packet_count[master_port] == 0)) {
            continue_pause_mode = false;
        }
    } else if (g_partial_packet_count[master_port] > 0) {
        /* only need one transfer for g_partial_packet_count */
        g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LOOP_CNT = (uint8_t)(g_partial_packet_count[master_port] - 1);
        g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LENGTH_CNT = SPI_MASTER_MAX_PACKET_LENGTH_MASK;
        g_partial_packet_count[master_port] = 0;
        if (g_pause_mode_off[g_pause_mode_index[master_port]] == PAUSE_END_PARTIAL) {
            continue_pause_mode = false;
        }
    } else if (g_remainder_count[master_port] > 0) {
        /* packet_loop_cnt = 0 */
        g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LOOP_CNT = 0;
        g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LENGTH_CNT = g_remainder_count[master_port] - 1;
        if (g_pause_mode_off[g_pause_mode_index[master_port]] == PAUSE_END_REMAINDER) {
            continue_pause_mode = false;
        }
    } else {
        log_hal_msgid_error("[spi-%d]: machine status error.\r\n", 1, master_port);
        return;
    }

    if (is_continue == false) {
        /* set direction for dual mode and quad mode. */
        if (is_write == true) {
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 |= (1 << SPIM_CTRL0_RW_MODE_OFFSET);
        } else {
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 &= ~(1 << SPIM_CTRL0_RW_MODE_OFFSET);
        }
        if (g_pause_mode_on[g_pause_mode_index[master_port]] == true) {
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.PAUSE_EN = 1;
        }
        g_spi_master_register[master_port]->TRIG_UNION.TRIG_CELLS.CMD_ACT = 1;
    } else {
        if (continue_pause_mode == false) {
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.PAUSE_EN = 0;
            g_spi_master_register[master_port]->TRIG_UNION.TRIG_CELLS.RESUME = 1;
        } else {
            g_spi_master_register[master_port]->TRIG_UNION.TRIG_CELLS.RESUME = 1;
        }
    }
}

void spi_master_start_transfer_dma_blocking(hal_spi_master_port_t master_port, bool is_write)
{
    bool continue_pause_mode, is_continue, loop_end;
    uint32_t irq_status;

    is_continue = false;
    continue_pause_mode = true;
    do {
        if (g_full_packet_count[master_port] > 0) {
            g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LOOP_CNT = (uint8_t)SPI_MASTER_MAX_PACKET_COUNT_MASK;
            g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LENGTH_CNT = SPI_MASTER_MAX_PACKET_LENGTH_MASK;
            g_full_packet_count[master_port]--;
            if ((g_pause_mode_off[g_pause_mode_index[master_port]] == PAUSE_END_FULL) &&
                (g_full_packet_count[master_port] == 0)) {
                continue_pause_mode = false;
            }
        } else if (g_partial_packet_count[master_port] > 0) {
            /* only need one transfer for g_partial_packet_count */
            g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LOOP_CNT = (uint8_t)(g_partial_packet_count[master_port] - 1);
            g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LENGTH_CNT = SPI_MASTER_MAX_PACKET_LENGTH_MASK;
            g_partial_packet_count[master_port] = 0;
            if (g_pause_mode_off[g_pause_mode_index[master_port]] == PAUSE_END_PARTIAL) {
                continue_pause_mode = false;
            }
        } else if (g_remainder_count[master_port] > 0) {
            /* packet_loop_cnt = 0 */
            g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LOOP_CNT = 0;
            g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.PACKET_LENGTH_CNT = g_remainder_count[master_port] - 1;
            if (g_pause_mode_off[g_pause_mode_index[master_port]] == PAUSE_END_REMAINDER) {
                continue_pause_mode = false;
            }
        } else {
            log_hal_msgid_error("[spi-%d]: machine status error.\r\n", 1, master_port);
            return;
        }

        if (is_continue == false) {
            /* set direction for dual mode and quad mode. */
            if (is_write == true) {
                g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 |= (1 << SPIM_CTRL0_RW_MODE_OFFSET);
            } else {
                g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 &= ~(1 << SPIM_CTRL0_RW_MODE_OFFSET);
            }
            loop_end = true;
            if (g_pause_mode_on[g_pause_mode_index[master_port]] == true) {
                g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.PAUSE_EN = 1;
                loop_end = false;
            }
            g_spi_master_register[master_port]->TRIG_UNION.TRIG_CELLS.CMD_ACT = 1;
            is_continue = true;
        } else {
            if (continue_pause_mode == false) {
                g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.PAUSE_EN = 0;
                g_spi_master_register[master_port]->TRIG_UNION.TRIG_CELLS.RESUME = 1;
                loop_end = true;
            } else {
                g_spi_master_register[master_port]->TRIG_UNION.TRIG_CELLS.RESUME = 1;
                loop_end = false;
            }
        }
        /* Wait current part of transfer finish. */
        while ((g_spi_master_register[master_port]->STA) == SPIM_STATUS_BUSY_MASK);
        do {
            irq_status = g_spi_master_register[master_port]->INT;
        } while ((irq_status != SPIM_INT_FINISH_INT_MASK) && (irq_status != SPIM_INT_PAUSE_INT_MASK));
    } while (loop_end == false);
}

hal_spi_master_status_t spi_master_analyse_transfer_size(hal_spi_master_port_t master_port, uint32_t size)
{
    uint32_t remainder;

    g_full_packet_count[master_port] = (size / SPI_MASTER_MAX_SIZE_FOR_NON_PAUSE);
    remainder = size % SPI_MASTER_MAX_SIZE_FOR_NON_PAUSE;
    g_partial_packet_count[master_port] = remainder / SPI_MASTER_MAX_PACKET_LENGTH;
    g_remainder_count[master_port] = remainder % SPI_MASTER_MAX_PACKET_LENGTH;

    /*
     * 1. decide whether we need use pause mode.
     * 2. decide where should we stop pause mode.
     * 3. Refer comment above about how to decide index here.
     */
    if (g_full_packet_count[master_port] > 0) {
        if (g_remainder_count[master_port] > 0) {
            g_pause_mode_index[master_port] = 3;
        } else if (g_partial_packet_count[master_port] > 0) {
            g_pause_mode_index[master_port] = 2;
        } else if (g_full_packet_count[master_port] > 1) {
            g_pause_mode_index[master_port] = 1;
        } else {
            g_pause_mode_index[master_port] = 0;
        }
    } else {
        if (g_remainder_count[master_port] == 0) {
            g_pause_mode_index[master_port] = 4;
        } else if (g_partial_packet_count[master_port] > 0) {
            g_pause_mode_index[master_port] = 5;
        } else {
            g_pause_mode_index[master_port] = 6;
        }
    }

    /* When we need pause mode, de-assert must NOT be enabled. */
    if (g_pause_mode_on[g_pause_mode_index[master_port]] == true) {
        if ((g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.DEASSERT_EN & 1) != 0) {
            log_hal_msgid_error("[SPIM%d]:pause deassert mode error.\r\n", 1, master_port);
            return HAL_SPI_MASTER_STATUS_ERROR;
        }
    }

    return HAL_SPI_MASTER_STATUS_OK;
}

#ifdef HAL_SPI_MASTER_FEATURE_CHIP_SELECT_TIMING
void spi_master_set_chip_select_timing(hal_spi_master_port_t master_port, hal_spi_master_chip_select_timing_t chip_select_timing)
{
    g_spi_master_register[master_port]->CFG0_UNION.CFG0 = (chip_select_timing.chip_select_setup_count << 16) | chip_select_timing.chip_select_hold_count;

    g_spi_master_register[master_port]->CFG2_UNION.CFG2_CELLS.CS_IDLE_CNT = chip_select_timing.chip_select_idle_count;
}
#endif

#ifdef HAL_SPI_MASTER_FEATURE_DEASSERT_CONFIG
void spi_master_set_deassert(hal_spi_master_port_t master_port, hal_spi_master_deassert_t deassert)
{
    switch (deassert) {
        case HAL_SPI_MASTER_DEASSERT_DISABLE:
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.DEASSERT_EN = 0;
            break;
        case HAL_SPI_MASTER_DEASSERT_ENABLE:
            g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.DEASSERT_EN = 1;
            break;
    }
}
#endif

#ifdef HAL_SPI_MASTER_FEATURE_DUAL_QUAD_MODE
void spi_master_set_type(hal_spi_master_port_t master_port, hal_spi_master_mode_t mode)
{
    g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 &= ~(SPIM_CTRL0_TYPE_MASK);
    g_spi_master_register[master_port]->CTRL0_UNION.CTRL0_CELLS.CTRL0 |= mode << SPIM_CTRL0_TYPE_OFFSET;
}

void spi_master_set_dummy_bits(hal_spi_master_port_t master_port, uint8_t dummy_bits)
{
    g_spi_master_register[master_port]->CFG3_UNION.CFG3_CELLS.DUMMY_CNT = dummy_bits;
}

void spi_master_set_command_bytes(hal_spi_master_port_t master_port, uint8_t command_bytes)
{
    g_spi_master_register[master_port]->CFG3_UNION.CFG3_CELLS.COMMAND_CNT = command_bytes;
}
#endif

#ifdef HAL_SLEEP_MANAGER_ENABLED
void spi_master_backup_register_callback(void *data)
{
    uint32_t i;
    hal_spi_master_port_t master_port;

    for (master_port = HAL_SPI_MASTER_0; master_port < HAL_SPI_MASTER_MAX; master_port++) {
        /* backup related spi_master register values */
        g_spi_master_ctrl_0_reg[master_port] = g_spi_master_register[master_port]->CTRL0_UNION.CTRL0;
        g_spi_master_ctrl_1_reg[master_port] = g_spi_master_register[master_port]->CTRL1_UNION.CTRL1;
        g_spi_master_cfg_0_reg[master_port] = g_spi_master_register[master_port]->CFG0_UNION.CFG0;
        g_spi_master_cfg_1_reg[master_port] = g_spi_master_register[master_port]->CFG1_UNION.CFG1;
        g_spi_master_cfg_2_reg[master_port] = g_spi_master_register[master_port]->CFG2_UNION.CFG2;
        g_spi_master_cfg_3_reg[master_port] = g_spi_master_register[master_port]->CFG3_UNION.CFG3;
        for (i = 0; i <= 9; i++) {
            g_spi_master_register[master_port]->CFG4_UNION.CFG4_CELLS.SEL_ADDR = i;
            g_spi_master_cfg_4_reg[master_port][i] = g_spi_master_register[master_port]->CFG4_UNION.CFG4_CELLS.SEL_RDATA;
        }
    }
}

void spi_master_restore_register_callback(void *data)
{
    uint32_t i;
    hal_spi_master_port_t master_port;

    for (master_port = HAL_SPI_MASTER_0; master_port < HAL_SPI_MASTER_MAX; master_port++) {
        /* restore related spi_master register values */
        g_spi_master_register[master_port]->CTRL0_UNION.CTRL0 = g_spi_master_ctrl_0_reg[master_port];
        g_spi_master_register[master_port]->CTRL1_UNION.CTRL1 = g_spi_master_ctrl_1_reg[master_port];
        g_spi_master_register[master_port]->CFG0_UNION.CFG0 = g_spi_master_cfg_0_reg[master_port];
        g_spi_master_register[master_port]->CFG1_UNION.CFG1 = g_spi_master_cfg_1_reg[master_port];
        g_spi_master_register[master_port]->CFG2_UNION.CFG2 = g_spi_master_cfg_2_reg[master_port];
        g_spi_master_register[master_port]->CFG3_UNION.CFG3 = g_spi_master_cfg_3_reg[master_port];
        for (i = 0; i <= 9; i++) {
            g_spi_master_register[master_port]->CFG4_UNION.CFG4_CELLS.SEL_ADDR = i;
            g_spi_master_register[master_port]->CFG4_UNION.CFG4_CELLS.SEL_WDATA = g_spi_master_cfg_4_reg[master_port][i];
        }
    }
}
#endif

void spi_master_reset_default_value(hal_spi_master_port_t master_port)
{
    uint32_t i, REG_INT;

    g_spi_master_register[master_port]->IE = 0;

    g_spi_master_register[master_port]->CTRL0_UNION.CTRL0 = 0;
    g_spi_master_register[master_port]->CTRL1_UNION.CTRL1 = 0;
    g_spi_master_register[master_port]->TX_SRC = 0;
    g_spi_master_register[master_port]->RX_DST = 0;
    g_spi_master_register[master_port]->CFG0_UNION.CFG0 = 0;
    g_spi_master_register[master_port]->CFG1_UNION.CFG1 = 0;
    g_spi_master_register[master_port]->CFG2_UNION.CFG2 = 0;
    g_spi_master_register[master_port]->CFG3_UNION.CFG3 = 0;
    for (i = 0; i <= 9; i++) {
        g_spi_master_register[master_port]->CFG4_UNION.CFG4_CELLS.SEL_ADDR = i;
        g_spi_master_register[master_port]->CFG4_UNION.CFG4_CELLS.SEL_WDATA = 0;
    }

    g_spi_master_register[master_port]->TRIG_UNION.TRIG_CELLS.RST = 1;

    REG_INT = g_spi_master_register[master_port]->INT;
    REG_INT = REG_INT;
}


#if defined(HAL_SPI_MASTER_FEATURE_NO_BUSY) && defined (HAL_SPI_MASTER_FEATURE_DMA_MODE)
hal_spi_sw_fifo_t spi_sw_fifo[HAL_SPI_MASTER_MAX];

void spi_sw_fifo_init(uint32_t spi_port)
{
    static hal_spi_sw_fifo_base spi_sw_fifo_space[HAL_SPI_MASTER_MAX][HAL_SPI_SW_FIFO_LEN];

    spi_sw_fifo[spi_port].spi_sw_fifo_len = HAL_SPI_SW_FIFO_LEN;
    spi_sw_fifo[spi_port].spi_sw_fifo_start = &spi_sw_fifo_space[spi_port][0];
    spi_sw_fifo[spi_port].spi_sw_fifo_r_index = 0;
    spi_sw_fifo[spi_port].spi_sw_fifo_w_index = 0;
}


uint32_t spi_push_sw_fifo(uint32_t spi_port, hal_spi_sw_fifo_node_t *tansfer_config, uint32_t transfer_cnt)
{
    uint32_t temp_cnt0 = 0;
    uint32_t temp_cnt1 = 0;
    uint32_t temp_cnt2 = 0;
    uint32_t saved_mask;
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    hal_spi_sw_fifo_node_t *spi_sw_fifo_p =  spi_sw_fifo[spi_port].spi_sw_fifo_start +  spi_sw_fifo[spi_port].spi_sw_fifo_w_index;

    /* Check if fifo is full */
    if ((spi_sw_fifo[spi_port].spi_sw_fifo_w_index + 1) % HAL_SPI_SW_FIFO_LEN == (spi_sw_fifo[spi_port].spi_sw_fifo_r_index % HAL_SPI_SW_FIFO_LEN)) {
        //log_hal_msgid_warning("[SPI[%d]:]spi sw fifo is full,please pop data\r\n", 2, spi_port);
        hal_nvic_restore_interrupt_mask(saved_mask);
        return 0;
    }

    /* read index > write index */
    if (spi_sw_fifo[spi_port].spi_sw_fifo_r_index > spi_sw_fifo[spi_port].spi_sw_fifo_w_index) {
        /* Get min of frame cnt remained in fifo and the frame cnt of user wanted to push in fifo */
        temp_cnt0 = MIN(spi_sw_fifo[spi_port].spi_sw_fifo_r_index - spi_sw_fifo[spi_port].spi_sw_fifo_w_index - 1, transfer_cnt);

        /* push frame in fifo */
        memcpy(spi_sw_fifo_p, tansfer_config, temp_cnt0 * sizeof(hal_spi_sw_fifo_node_t));

        spi_sw_fifo[spi_port].spi_sw_fifo_w_index += temp_cnt0;

    } else if (spi_sw_fifo[spi_port].spi_sw_fifo_r_index == 0) {
        /* Get min of frame cnt remained in fifo and the frame cnt of user wanted to push in fifo */
        temp_cnt0 = MIN(spi_sw_fifo[spi_port].spi_sw_fifo_len - spi_sw_fifo[spi_port].spi_sw_fifo_w_index - 1, transfer_cnt);

        /* push frame in fifo */
        memcpy(spi_sw_fifo_p, tansfer_config, temp_cnt0 * sizeof(hal_spi_sw_fifo_node_t));

        spi_sw_fifo[spi_port].spi_sw_fifo_w_index += temp_cnt0;
    } else {
        temp_cnt1 = MIN(spi_sw_fifo[spi_port].spi_sw_fifo_len - spi_sw_fifo[spi_port].spi_sw_fifo_w_index, transfer_cnt);
        memcpy(spi_sw_fifo_p, tansfer_config, temp_cnt1 * sizeof(hal_spi_sw_fifo_node_t));
        if (temp_cnt1 < transfer_cnt) {
            temp_cnt2 = MIN(spi_sw_fifo[spi_port].spi_sw_fifo_r_index, transfer_cnt - temp_cnt1);
            memcpy(spi_sw_fifo[spi_port].spi_sw_fifo_start, &tansfer_config[temp_cnt1], temp_cnt2 * sizeof(hal_spi_sw_fifo_node_t));
            temp_cnt0 =  temp_cnt1 + temp_cnt2;
        } else {
            temp_cnt0 =  temp_cnt1;
        }
        spi_sw_fifo[spi_port].spi_sw_fifo_w_index = (spi_sw_fifo[spi_port].spi_sw_fifo_w_index + temp_cnt0) % spi_sw_fifo[spi_port].spi_sw_fifo_len ;
    }

    hal_nvic_restore_interrupt_mask(saved_mask);
    return temp_cnt0;
}

uint32_t  spi_pop_sw_fifo(uint32_t spi_port, hal_spi_sw_fifo_node_t *transfer_config, uint32_t transfer_cnt)
{
    uint32_t temp_cnt0 = 0;
    uint32_t temp_cnt1 = 0;
    uint32_t temp_cnt2 = 0;
    uint32_t saved_mask;
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    hal_spi_sw_fifo_node_t *spi_sw_fifo_p =   spi_sw_fifo[spi_port].spi_sw_fifo_start +  spi_sw_fifo[spi_port].spi_sw_fifo_r_index;

    /* fifo empty */
    if (spi_sw_fifo[spi_port].spi_sw_fifo_r_index ==  spi_sw_fifo[spi_port].spi_sw_fifo_w_index) {
        hal_nvic_restore_interrupt_mask(saved_mask);
        return 0;
    } else if (spi_sw_fifo[spi_port].spi_sw_fifo_w_index >  spi_sw_fifo[spi_port].spi_sw_fifo_r_index) {
        temp_cnt0 = MIN(spi_sw_fifo[spi_port].spi_sw_fifo_w_index - spi_sw_fifo[spi_port].spi_sw_fifo_r_index, transfer_cnt);

        memcpy(transfer_config, spi_sw_fifo_p, temp_cnt0 * sizeof(hal_spi_sw_fifo_node_t));

        spi_sw_fifo[spi_port].spi_sw_fifo_r_index += temp_cnt0;
    } else {
        temp_cnt1 = MIN(spi_sw_fifo[spi_port].spi_sw_fifo_len - spi_sw_fifo[spi_port].spi_sw_fifo_r_index, transfer_cnt);
        memcpy(transfer_config, spi_sw_fifo_p, temp_cnt1 * sizeof(hal_spi_sw_fifo_node_t));
        if (temp_cnt1 < transfer_cnt) {
            temp_cnt2 = MIN(spi_sw_fifo[spi_port].spi_sw_fifo_w_index, transfer_cnt - temp_cnt1);
            memcpy(transfer_config + temp_cnt1, spi_sw_fifo[spi_port].spi_sw_fifo_start, temp_cnt2 * sizeof(hal_spi_sw_fifo_node_t));
            temp_cnt0 =  temp_cnt1 + temp_cnt2;
        } else {
            temp_cnt0 =  temp_cnt1;
        }
        spi_sw_fifo[spi_port].spi_sw_fifo_r_index = (spi_sw_fifo[spi_port].spi_sw_fifo_r_index + temp_cnt0) %  spi_sw_fifo[spi_port].spi_sw_fifo_len ;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    return temp_cnt0;
}

int32_t spi_get_sw_fifo_remain_transfer_cnt(uint32_t spi_port)
{
    uint32_t saved_mask;
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    int32_t cnt =  spi_sw_fifo[spi_port].spi_sw_fifo_r_index -  spi_sw_fifo[spi_port].spi_sw_fifo_w_index - 1;
    if (cnt < 0) {
        cnt +=  spi_sw_fifo[spi_port].spi_sw_fifo_len;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    return cnt;
}

int32_t spi_get_sw_fifo_available_transfer_cnt(uint32_t spi_port)
{
    uint32_t saved_mask;
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    int32_t cnt =  spi_sw_fifo[spi_port].spi_sw_fifo_w_index -  spi_sw_fifo[spi_port].spi_sw_fifo_r_index ;
    if (cnt < 0) {
        cnt +=  spi_sw_fifo[spi_port].spi_sw_fifo_len ;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    return cnt;
}


void spi_sw_fifo_deinit(uint32_t spi_port)
{
    spi_sw_fifo[spi_port].spi_sw_fifo_len = 0;

    if (0 != spi_get_sw_fifo_available_transfer_cnt(spi_port)) {
        log_hal_msgid_error("[SPI%d][deinit]:spi_sw_fifo_deinit failed!\r\n", 1, spi_port);
    } else {
        if (NULL != spi_sw_fifo[spi_port].spi_sw_fifo_start) {
            spi_sw_fifo[spi_port].spi_sw_fifo_r_index = 0;
            spi_sw_fifo[spi_port].spi_sw_fifo_w_index = 0;
        } else {
            log_hal_msgid_error("[SPI%d][deinit]:spi_sw_fifo_deinit failed!\r\n", 1, spi_port);
        }
    }
}

#endif

#endif /* HAL_SPI_MASTER_MODULE_ENABLED */


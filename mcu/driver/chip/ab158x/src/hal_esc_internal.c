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

#include "hal_platform.h"

#ifdef HAL_ESC_MODULE_ENABLED

#include <string.h>
#include <memory_attribute.h>
#include <assert.h>
#include "hal.h"
#include "hal_esc_internal.h"
#include "hal_nvic_internal.h"

extern uint32_t hal_clock_get_freq_meter(hal_src_clock SRC_CLK, uint32_t winset);
extern uint8_t clock_mux_cur_sel(clock_mux_sel_id mux_id);

#ifndef min
#define min(_a, _b)   (((_a)<(_b))?(_a):(_b))
#endif

#ifndef max
#define max(_a, _b)   (((_a)>(_b))?(_a):(_b))
#endif

#define ESC_FAKE_ADDRESS    (0x1000)

//it should be on AO domain
ATTR_RWDATA_IN_TCM ESC_REGISTER_T *esc_register = (ESC_REGISTER_T *)ESC_BASE;

#ifdef ESC_INTERNAL_DEBUG
void check_mac_en(const char *called_func, uint32_t error_value);
#endif

/* common function declaration */
#if defined(ESC_PSRAM_ENABLE) || defined(ESC_FLASH_ENABLE)

#ifdef HAL_SLEEP_MANAGER_ENABLED
ATTR_RWDATA_IN_TCM  uint32_t g_backup_esc_reg[ESC_REG_MAX_IDX];
sleep_management_lock_request_t g_esc_sleep_handle = SLEEP_LOCK_ESC;
sleep_management_backup_restore_module_t g_esc_sleep_backup_restore_handle = SLEEP_BACKUP_RESTORE_ESC;

void esc_backup_register_callback(void *data);
void esc_restore_register_callback(void *data);
#endif /* HAL_SLEEP_MANAGER_ENABLED */

void ESC_MacEnable(void);
void ESC_MacLeave(void);
void ESC_MacTrigger(void);
void ESC_MacWaitReady(void);
void esc_isr(hal_nvic_irq_t irq);
void ESC_Dev_Command(const uint16_t CS, const uint32_t cmd);
void ESC_Dev_Command_Ext(const uint16_t CS, const uint8_t *cmd, uint8_t *data, const uint16_t outl, const uint16_t inl);

/* common function definition */
void reset_esc_register(void)
{
    esc_register->ESC_MAC_CTL    = 0x00000000;
    esc_register->ESC_DIRECT_CTL = 0x03027000;
    esc_register->ESC_MISC_CTL1  = 0x00020103;
    esc_register->ESC_MISC_CTL2  = 0x00000000;
    esc_register->ESC_MAC_OUTL   = 0x00000000;
    esc_register->ESC_MAC_INL    = 0x00000000;
    esc_register->ESC_STA1_CTL   = 0x00000000;
    esc_register->ESC_STA2_CTL   = 0x00000000;
    esc_register->ESC_DLY_CTL1   = 0x00000000;
    esc_register->ESC_DLY_CTL2   = 0x00000000;
    esc_register->ESC_DLY_CTL3   = 0x00000003;
    esc_register->ESC_DLY_CTL4   = 0x00000100;
    esc_register->ESC_QIO_CTRL   = 0x000000F0;
    esc_register->ESC_STA3       = 0x00000000;
    esc_register->ESC_DEBUG_1    = 0x00000000;
    esc_register->ESC_DEBUG_2    = 0x00000000;
    esc_register->ESC_DEBUG_MUX  = 0x00000000;
    esc_register->ESC_MISC_CTL3  = 0xFFFFFFFF;
    esc_register->ESC_MAC_IRQ    = 0x00000000;
    hal_nvic_register_isr_handler(ESC_IRQn, esc_isr);
    hal_nvic_enable_irq(ESC_IRQn);
}


/* @brief: Enter macro mode */
ATTR_TEXT_IN_TCM void ESC_MacEnable(void)
{
    esc_register->ESC_MAC_CTL |= ESC_MAC_EN;
}


/* @brief: Leaves macro mode, and returns to direct read mode */
ATTR_TEXT_IN_TCM void ESC_MacLeave(void)
{
    uint32_t val;

    // clear ESC_MAC_TRIG and leave MACRO mode
    val = esc_register->ESC_MAC_CTL;
    val &= ~(ESC_MAC_TRIG | ESC_MAC_EN);
    esc_register->ESC_MAC_CTL = val;
    while (esc_register->ESC_MAC_CTL & ESC_MAC_WAIT_DONE);
    while (esc_register->ESC_MAC_CTL & ESC_MAC_EN);
}


/* @brief: Send commands placed in GPRAM (in macro mode) */
ATTR_TEXT_IN_TCM void ESC_MacTrigger(void)
{
    uint32_t  val = 0;

    val = esc_register->ESC_MAC_CTL;
    // trigger ESC
    val |= (ESC_MAC_TRIG | ESC_MAC_EN);
    esc_register->ESC_MAC_CTL = val;
    // wait for ESC ready
    while (esc_register->ESC_MAC_CTL & ESC_MAC_WAIT_DONE);
}

/*!
    @brief
    Set trigger and send GPRAM data toward extern PSRAM
    Leave macro mode when done */
ATTR_TEXT_IN_TCM void ESC_MacWaitReady(void)
{
    ESC_MacTrigger();
    ESC_MacLeave();
}


/*!
  @brief
    Issue generic command to serial Flash (max: 4 bytes)

  @param[in] cmd The command to be sent, the first shift out byte is MSB.
  @remarks This function is only called by bootloader.
           Other callers must be aware of interrupts during the MAC mode
*/
ATTR_TEXT_IN_TCM void ESC_Dev_Command(const uint16_t CS, const uint32_t cmd)
{
    uint32_t cmdi, cmdo;
    uint8_t  len;

    for (cmdi = cmd, len = 0, cmdo = 0; cmdi != 0; cmdi = cmdi >> ESC_GENERIC_8_BIT_OFFSET, len++) {
        cmdo = (cmdo << ESC_GENERIC_8_BIT_OFFSET) | (cmdi & ESC_GENERIC_0xFF_MASK);
    }

    if (len == 0) {
        return;
    }

    *(volatile uint32_t *)ESC_GPRAM_BASE = cmdo;
    esc_register->ESC_MAC_OUTL = len;
    esc_register->ESC_MAC_INL = 0;
    ESC_MacEnable();
    ESC_MacWaitReady();
}

//-----------------------------------------------------------------------------
/*!
  @brief
    Issue generic command to device, and read results.

  @param[in] cmd Pointer to the commands that to be sent
  @param[out] data Pointer to the data buffer that to be stored
  @param[in] outl Length of commands (in bytes)
  @param[in] intl Length of read data

  @remarks This function shall only be invoked in bootloader.
           Other callers must be aware of interrupts during the MAC mode
*/
ATTR_TEXT_IN_TCM void ESC_Dev_Command_Ext(const uint16_t CS, const uint8_t *cmd, uint8_t *data, const uint16_t outl, const uint16_t inl)
{
    uint32_t tmp;
    uint32_t i, j;
    uint8_t *p_data, *p_tmp;

    p_tmp = (uint8_t *)(&tmp);

    for (i = 0, p_data = ((uint8_t *)ESC_GPRAM_DATA); i < outl; p_data += 4) {
        for (j = 0, tmp = 0; i < outl && j < 4; i++, j++) {
            p_tmp[j] = cmd[i];
        }
        ESC_WriteReg32(p_data, tmp);
    }
    esc_register->ESC_MAC_OUTL = outl;
    esc_register->ESC_MAC_INL = inl;
    ESC_MacEnable();
    ESC_MacWaitReady();

    for (i = 0, p_data = ((uint8_t *)ESC_GPRAM_DATA + outl); i < inl; ++i, ++data, ++p_data) {
        *data = ESC_ReadReg8(p_data);
    }
}
#endif

#ifdef ESC_PSRAM_ENABLE

const esc_psram_setting g_setting = {
    /*      MODE,    MAC_CTL, DIRECT_CTL,  MISC_CTL1,  MISC_CTL2,   DLY_CTL1,   DLY_CTL2,   DLY_CTL3,   DLY_CTL4( 52MHz ) DLY_CTL1,   DLY_CTL2,   DLY_CTL3,   DLY_CTL4( 26MHz )*/
#if defined(MICRO_CHIP_23A1024)
    ESC_QPI_MODE,        //device mode
    0x00000000,          //ESC_MAC_CTL
    0x03021052,          //ESC_DIRECT_CTL
    0x00020103,          //ESC_MISC_CTL1
    0x00000000,          //ESC_MISC_CTL2
    { 0x00000000, 0x00000000, 0x00000003, 0x00000100 },    //ESC_DELAY_CTL_52MHZ
    { 0x00000000, 0x00000000, 0x00000003, 0x00000100 },    //ESC_DELAY_CTL_26MHZ
    { 0x00000000, 0x00000000, 0x00000003, 0x00000100 },    //ESC_DELAY_CTL_104MHZ
#elif defined(ASP_CHIP_1604M_SQ_PSRAM)
    ESC_QPI_MODE,        //device mode
    0x00000000,          //ESC_MAC_CTL
    0xEB385052,          //ESC_DIRECT_CTL
    0x00020103,          //ESC_MISC_CTL1
    0x00000000,          //ESC_MISC_CTL2
    { 0x00000000, 0x01010101, 0x00000004, 0x00000000 },    //ESC_DELAY_CTL_52MHZ
    { 0x00000000, 0x02020202, 0x00000003, 0x00000004 },    //ESC_DELAY_CTL_26MHZ
    { 0x00000000, 0x00000000, 0x00000004, 0x00000003 },    //ESC_DELAY_CTL_104MHZ
    //ESC_DLY_CTL1    ESC_DLY_CTL2    ESC_DLY_CTL3   ESC_DLY_CTL4
#elif defined(ESC_ASP6404L_SQ_PSRAM)
    ESC_SPIQ_MODE,       //device mode
    0x00000000,          //ESC_MAC_CTL
    0xEB385042,          //ESC_DIRECT_CTL
    0x00020103,          //ESC_MISC_CTL1
    0x00000000,          //ESC_MISC_CTL2
    { 0x00000000, 0x00000000, 0x00000003, 0x00000100 },    //ESC_DELAY_CTL_52MHZ
    { 0x00000000, 0x00000000, 0x00000003, 0x00000100 },    //ESC_DELAY_CTL_26MHZ
    { 0x00000000, 0x00000000, 0x00000003, 0x00000100 },    //ESC_DELAY_CTL_104MHZ
#else
    "No SRAM device model specified"
#endif
};

void esc_device_reset(void)
{
#if defined(MICRO_CHIP_23A1024)
    ESC_Dev_Command(0, ESC_CMD_EXIT_QUAD_MODE);
#elif defined(ASP_CHIP_1604M_SQ_PSRAM)
    ESC_Dev_Command(0, ESC_CMD_RESET_ENABLE);
    ESC_Dev_Command(0, ESC_CMD_RESET);
#elif defined(ESC_ASP6404L_SQ_PSRAM)
    ESC_Dev_Command(0, ESC_CMD_RESET_ENABLE);
    ESC_Dev_Command(0, ESC_CMD_RESET);
#endif
}

void esc_internal_init(void)
{
    esc_clock_t clock = ESC_CLOCK_INVALID;
    /* modify registers of ESC controller */
    reset_esc_register();

    /* AHB request( direct read ) pending interrupt
     * when ESC_MAC_EN = 1 and AHB_REQ_MASK = 0.
     * Setting for DVFS mask channel.
     */
    esc_register->ESC_MAC_IRQ    = 0x00000002;

    esc_register->ESC_MAC_CTL = g_setting.ESC_MAC_CTL;
    esc_register->ESC_DIRECT_CTL = g_setting.ESC_DIRECT_CTL;
    esc_register->ESC_MISC_CTL1  = g_setting.ESC_MISC_CTL1;
    esc_register->ESC_MISC_CTL2  = g_setting.ESC_MISC_CTL2;

    clock = clock_mux_cur_sel(CLK_ESC_SEL);
    if (clock == ESC_CLOCK_26MHz) {
        esc_register->ESC_DLY_CTL1 = g_setting.ESC_DLY_CTL_26MHz[0];
        esc_register->ESC_DLY_CTL2 = g_setting.ESC_DLY_CTL_26MHz[1];
        esc_register->ESC_DLY_CTL3 = g_setting.ESC_DLY_CTL_26MHz[2];
        esc_register->ESC_DLY_CTL4 = g_setting.ESC_DLY_CTL_26MHz[3];
    } else if (clock == ESC_CLOCK_52MHz) {
        esc_register->ESC_DLY_CTL1 = g_setting.ESC_DLY_CTL_52MHz[0];
        esc_register->ESC_DLY_CTL2 = g_setting.ESC_DLY_CTL_52MHz[1];
        esc_register->ESC_DLY_CTL3 = g_setting.ESC_DLY_CTL_52MHz[2];
        esc_register->ESC_DLY_CTL4 = g_setting.ESC_DLY_CTL_52MHz[3];
    } else if (clock == ESC_CLOCK_OSC1_104MHz || clock == ESC_CLOCK_104MHz) {
        esc_register->ESC_DLY_CTL1 = g_setting.ESC_DLY_CTL_104MHz[0];
        esc_register->ESC_DLY_CTL2 = g_setting.ESC_DLY_CTL_104MHz[1];
        esc_register->ESC_DLY_CTL3 = g_setting.ESC_DLY_CTL_104MHz[2];
        esc_register->ESC_DLY_CTL4 = g_setting.ESC_DLY_CTL_104MHz[3];
    } else {
        log_hal_msgid_warning("Please check ESC clock setting!\r\n", 0);
    }

    log_hal_msgid_info("esc_internal_init: ESC freq = %d Hz, clock is %d\r\n", 2, hal_clock_get_freq_meter(hf_fesc_ck, 100), clock);

    esc_device_reset();

    if (g_setting.DEVICE_MODE == ESC_QPI_MODE) {
        ESC_Dev_Command(0, ESC_CMD_ENTER_QUAD_MODE);
    }

#ifdef HAL_SLEEP_MANAGER_ENABLED
    sleep_management_register_all_secure_suspend_callback(SLEEP_BACKUP_RESTORE_ESC, (sleep_management_suspend_callback_t)esc_backup_register_callback, NULL);
    sleep_management_register_all_secure_resume_callback(SLEEP_BACKUP_RESTORE_ESC, (sleep_management_suspend_callback_t)esc_restore_register_callback, NULL);
#endif
}


#elif defined(ESC_FLASH_ENABLE)
ATTR_RWDATA_IN_TCM esc_ctl_block_t g_esc_ctl = { ESC_SPI_MODE, NULL, ESC_CLOCK_26MHz, false, false, false, false, false };
const esc_device_info_t g_esc_support_flash_list[] = {
    {
        "W25Q32JW", 0x00EF6016, 256, 16384, 0x0B, 0xEB, 0x02, 0x7A, 0x75, 1, 0, 7,
        { 0x20, 0x52, 0xD8, 0xC7 },    //erase command
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_52MHZ
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_26MHZ
        { 0x00000000, 0x00000000, 0x00000004, 0x00000003 }     //ESC_DELAY_CTL_104MHZ
    },
    {
        "W25Q64JW", 0x00EF6017, 256, 32768, 0x0B, 0xEB, 0x02, 0x7A, 0x75, 1, 0, 7,
        { 0x20, 0x52, 0xD8, 0xC7 },    //erase command
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_52MHZ
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_26MHZ
        { 0x00000000, 0x00000000, 0x00000004, 0x00000003 }     //ESC_DELAY_CTL_104MHZ
    },
    {
        "W25Q128JW_DTR", 0x00EF8018, 256, 65536, 0x0B, 0xEB, 0x02, 0x7A, 0x75, 1, 0, 7,
        { 0x20, 0x52, 0xD8, 0xC7 },    //erase command
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_52MHZ
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_26MHZ
        { 0x00000000, 0x00000000, 0x00000004, 0x00000003 }     //ESC_DELAY_CTL_104MHZ
    },
    {
        "W25Q128JW", 0x00EF6018, 256, 65536, 0x0B, 0xEB, 0x02, 0x7A, 0x75, 1, 0, 7,
        { 0x20, 0x52, 0xD8, 0xC7 },    //erase command
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_52MHZ
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_26MHZ
        { 0x00000000, 0x00000000, 0x00000004, 0x00000003 }     //ESC_DELAY_CTL_104MHZ
    },
    {
        "W25Q256JW", 0x00EF6019, 256, 131072, 0x0B, 0xEB, 0x02, 0x7A, 0x75, 1, 0, 7,
        { 0x20, 0x52, 0xD8, 0xC7 },    //erase command
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_52MHZ
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_26MHZ
        { 0x00000000, 0x00000000, 0x00000004, 0x00000003 }     //ESC_DELAY_CTL_104MHZ
    },
    {
        "G25Q128JW", 0x00C86018, 256, 65536, 0x0B, 0xEB, 0x02, 0x7A, 0x75, 1, 0, 7,
        { 0x20, 0x52, 0xD8, 0xC7 },    //erase command
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_52MHZ
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_26MHZ
        { 0x00000000, 0x00000000, 0x00000004, 0x00000003 }     //ESC_DELAY_CTL_104MHZ
    },
    {
        "G25Q32JW", 0x00C86016, 256, 16384, 0x0B, 0xEB, 0x02, 0x7A, 0x75, 1, 0, 7,
        { 0x20, 0x52, 0xD8, 0xC7 },    //erase command
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_52MHZ
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_26MHZ
        { 0x00000000, 0x00000000, 0x00000004, 0x00000003 }     //ESC_DELAY_CTL_104MHZ
    },
    {
        "G25Q64JW", 0x00C86017, 256, 32768, 0x0B, 0xEB, 0x02, 0x7A, 0x75, 1, 0, 7,
        { 0x20, 0x52, 0xD8, 0xC7 },    //erase command
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_52MHZ
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_26MHZ
        { 0x00000000, 0x00000000, 0x00000004, 0x00000003 }     //ESC_DELAY_CTL_104MHZ
    },
    {
        "G25Q128JW", 0x00C86018, 256, 65536, 0x0B, 0xEB, 0x02, 0x7A, 0x75, 1, 0, 7,
        { 0x20, 0x52, 0xD8, 0xC7 },    //erase command
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_52MHZ
        { 0x00000000, 0x00000000, 0x00000003, 0x00000006 },    //ESC_DELAY_CTL_26MHZ
        { 0x00000000, 0x00000000, 0x00000004, 0x00000003 }     //ESC_DELAY_CTL_104MHZ
    }
};
#define SUPPORT_LIST_LENGTH  ((sizeof(g_esc_support_flash_list))/(sizeof(esc_device_info_t)))

#define ESC_IS_4_BYTE_ADDRESS_MODE   ((esc_register->ESC_MISC_CTL2 >> ESC_ADDRESS_MODE_BIT_OFFSET) & 0x1)

ATTR_RWDATA_IN_TCM uint32_t sw_esc_irq;


bool esc_judge_device(void);
bool esc_is_4_byte_mode(void);

void flash_write_en(void);

bool write_qe_bit(uint8_t bit_value);

bool device_is_busy(uint8_t status);

uint8_t read_device_reg(esc_device_reg_type_t type);

hal_esc_flash_status_t esc_flash_parameter_check(uint32_t addr, void *ptr, uint32_t len);

void ESC_Write_C1A3(uint8_t cmd, uint32_t address, const uint8_t *buff, uint32_t length);

void ESC_Write_C1A4(uint8_t cmd, uint32_t address, const uint8_t *buff, uint32_t length);

void esc_wait_device_ready(uint32_t address);



void set_initial_state(bool state)
{
    g_esc_ctl.initialized = state;
}


bool get_initial_state(void)
{
    return g_esc_ctl.initialized;
}


hal_esc_flash_status_t esc_flash_parameter_check(uint32_t addr, void *ptr, uint32_t len)
{
    uint32_t esc_addr_range[2] = { ESC_GENERIC_SRAM_BANK_MASK, ESC_GENERIC_SRAM_BANK_MASK };
    uint32_t end_addr = addr + len;
    uint32_t capacity = (g_esc_ctl.device->page_size * g_esc_ctl.device->page_number);

    if (get_initial_state() == false) {
        log_hal_msgid_error("ESC flash not initial !!!", 0);
        return HAL_ESC_FLASH_STATUS_ERROR_NO_INIT;
    }

    if ((NULL == ptr) || (0 == len) || (len > capacity) || (addr > capacity) || ((addr + len) > capacity)) {
        log_hal_msgid_error("ESC flash api was called with ptr(0x%08X) and len(0x%08X)", 2, ptr, len);
        return HAL_ESC_FLASH_WRONG_PARAMETER;
    }
    esc_addr_range[0] = 0x1000; 
    esc_addr_range[1] = (g_esc_ctl.device->page_size * g_esc_ctl.device->page_number);

    if ((addr >= esc_addr_range[0])  && (addr < esc_addr_range[1]) && (addr < end_addr) && (end_addr < esc_addr_range[1])) {
        return HAL_ESC_FLASH_STATUS_OK;
    } else {
        log_hal_msgid_error("ESC flash api was called with invalid address.", 0);
        log_hal_msgid_error("input address range: 0x%08X, 0x%08X", 2, addr, end_addr);
        log_hal_msgid_error("ESC allow address range: 0x%08X, 0x%08X", 2, esc_addr_range[0], esc_addr_range[1]);
        return HAL_ESC_FLASH_STATUS_ERROR_WRONG_ADDRESS;
    }
}


/*!
  @brief
    GPRAM access: Write data to GPRAM

  @param[in] gpram_offset The offset of the GPRAM in terms of byte, it must be
             multiple of four.
  @param[in] buff The pointer to the data buffer
  @param[in] length The length of the data buffer.
  @remarks The given gpram offset must be 4 bytes aligned, and the function will
           copy the data buffer to the GPRAM WORD/HALF-WORD/BYTE wise
           according to the starting address of the buffer.
*/

ATTR_TEXT_IN_TCM void ESC_GPRAM_Write(uint32_t gpram_offset, const uint8_t *buff, int32_t length)
{
    uint32_t i;
    uint8_t *p_data = (uint8_t *)buff;
    esc_uint tmp;

    gpram_offset += ESC_GPRAM_DATA;

    /* !!! aussme that gpram_offset always 4 byte align !!! */

    if (0 == ((uint32_t)buff & 0x3)) {
        /* source address is 4-byte aligned */
        for (; length > 0; gpram_offset += 4, p_data += 4, length -= 4) {
            ESC_WriteReg32(gpram_offset, *(uint32_t *)p_data);
        }
    } else if (0 == ((uint32_t)p_data & 0x1)) {
        /* source address is 2-byte aligned */
        for (; length > 0; gpram_offset += 4, p_data += 4, length -= 4) {
            tmp.u16[0] = *(uint16_t *)p_data;
            tmp.u16[1] = *((uint16_t *)p_data + 1);
            /* save local_data to ESC GPRAM */
            ESC_WriteReg32(gpram_offset, tmp.u32);
        }
    } else {
        /* source data is NOT 2-byte aligned */
        for (; length > 0; gpram_offset += 4, length -= 4) {
            for (i = 0; i < 4 && i < length; i++) {
                tmp.u8[i] = *p_data++;
            }
            /* save local_data to ESC GPRAM */
            ESC_WriteReg32(gpram_offset, tmp.u32);
        }
    }
}


void esc_internal_init(void)
{
    uint32_t jedec_id = g_esc_ctl.device->jedec_id;
    uint32_t direct_ctl = 0x0;
    uint8_t cmd_enter_4_byte_address_mode;

    /* AHB request( direct read ) pending interrupt
     * when ESC_MAC_EN = 1 and AHB_REQ_MASK = 0.
     * Setting for ESC Flash XIP.
     */
    esc_register->ESC_MAC_IRQ = 0x00000002;

#ifndef FPGA_ENV
    g_esc_ctl.clock = clock_mux_cur_sel(CLK_ESC_SEL);
    log_hal_msgid_info("esc_internal_init: ESC freq = %d Hz, g_esc_ctl.clock is %d\r\n", 2, hal_clock_get_freq_meter(hf_fesc_ck, 100), g_esc_ctl.clock);
#else
    g_esc_ctl.clock = ESC_CLOCK_26MHz;
#endif

    if (g_esc_ctl.clock == ESC_CLOCK_26MHz) {
        esc_register->ESC_DLY_CTL1 = g_esc_ctl.device->delay_ctl_26MHz[0];
        esc_register->ESC_DLY_CTL2 = g_esc_ctl.device->delay_ctl_26MHz[1];
        esc_register->ESC_DLY_CTL3 = g_esc_ctl.device->delay_ctl_26MHz[2];
        esc_register->ESC_DLY_CTL4 = g_esc_ctl.device->delay_ctl_26MHz[3];
    } else if (g_esc_ctl.clock == ESC_CLOCK_52MHz) {
        esc_register->ESC_DLY_CTL1 = g_esc_ctl.device->delay_ctl_52MHz[0];
        esc_register->ESC_DLY_CTL2 = g_esc_ctl.device->delay_ctl_52MHz[1];
        esc_register->ESC_DLY_CTL3 = g_esc_ctl.device->delay_ctl_52MHz[2];
        esc_register->ESC_DLY_CTL4 = g_esc_ctl.device->delay_ctl_52MHz[3];
    } else if (g_esc_ctl.clock == ESC_CLOCK_104MHz) {
        esc_register->ESC_DLY_CTL1 = g_esc_ctl.device->delay_ctl_104MHz[0];
        esc_register->ESC_DLY_CTL2 = g_esc_ctl.device->delay_ctl_104MHz[1];
        esc_register->ESC_DLY_CTL3 = g_esc_ctl.device->delay_ctl_104MHz[2];
        esc_register->ESC_DLY_CTL4 = g_esc_ctl.device->delay_ctl_104MHz[3];
    }  else {
        log_hal_msgid_warning("Please check ESC clock setting!\r\n", 0);
    }

    if (ESC_PARSE_MANU_ID(jedec_id) == WB_JEDEC_ID || ESC_PARSE_MANU_ID(jedec_id) == GD_JEDEC_ID) {
        if (0xEF6018 == jedec_id || 0xEF6017 == jedec_id || 0xEF6016 == jedec_id ||
            0xC86018 == jedec_id || 0xC86017 == jedec_id || 0xC86016 == jedec_id) {
            /* device is W25Q128JW, direct read: SPIQ mode, MAC write: SPI mode */
            /* config direct register and command */

            direct_ctl |= ((uint32_t)g_esc_ctl.device->spiq_read_cmd) << (ESC_DIRECT_READ_CMD_BIT_OFFSET);
            direct_ctl |= ((uint32_t)g_esc_ctl.device->write_cmd) << ESC_DIRECT_WRITE_CMD_BIT_OFFSET;
            direct_ctl |= ((uint32_t)5) << (ESC_RD_DUMMY_BIT_OFFSET);    /* 6 dummy cycle */
            direct_ctl |= ((uint32_t)4) << (ESC_W_R_MODE_BIT_OFFSET);    /* b100 <==> 4 I/O mode */
            direct_ctl |= ((uint32_t)1) << (ESC_RD_DUMMY_EN_BIT_OFFSET); /* enable direct read dummy */
            log_hal_msgid_info("esc_internal_init direct_ctl is 0x%08X\r\n", 1, direct_ctl);
            write_qe_bit(1);
        } else {
            /* device is W25Q256JW, direct read: SPI mode, MAC write: SPI mode for FPGA DVT */
            direct_ctl |= ((uint32_t)g_esc_ctl.device->read_cmd) << (ESC_DIRECT_READ_CMD_BIT_OFFSET);
            direct_ctl |= ((uint32_t)7) << (ESC_RD_DUMMY_BIT_OFFSET);    /* 8 dummy cycle */
            direct_ctl |= ((uint32_t)0) << (ESC_W_R_MODE_BIT_OFFSET);    /* b000 <==> 1 I/O mode */
            direct_ctl |= ((uint32_t)1) << (ESC_RD_DUMMY_EN_BIT_OFFSET); /* enable direct read dummy */

            log_hal_msgid_info("function %s line %d\r\n", 2, __FUNCTION__, __LINE__);
            write_qe_bit(1);
            /* Issue Enter 4 byte address type mode command */
            cmd_enter_4_byte_address_mode = WB_ENTER_4_BYTE_ADDRESS_MODE;
            ESC_Dev_Command_Ext(0, &cmd_enter_4_byte_address_mode, NULL, 1, 0);
            esc_register->ESC_MISC_CTL2 |= ((uint32_t)0x1 << ESC_ADDRESS_MODE_BIT_OFFSET);
            log_hal_msgid_info("function %s line %d\r\n", 2, __FUNCTION__, __LINE__);
        }
        esc_register->ESC_DIRECT_CTL = direct_ctl;
    }

#ifdef HAL_SLEEP_MANAGER_ENABLED
    sleep_management_register_all_secure_suspend_callback(SLEEP_BACKUP_RESTORE_ESC, (sleep_management_suspend_callback_t)esc_backup_register_callback, NULL);
    sleep_management_register_all_secure_resume_callback(SLEEP_BACKUP_RESTORE_ESC, (sleep_management_suspend_callback_t)esc_restore_register_callback, NULL);
#endif
}


uint8_t read_device_reg(esc_device_reg_type_t type)
{
    uint32_t jedec_id = g_esc_ctl.device->jedec_id;
    uint8_t read_cmd = 0xFF, reg_data = 0xFF;

    if (ESC_PARSE_MANU_ID(jedec_id) == WB_JEDEC_ID || ESC_PARSE_MANU_ID(jedec_id) == GD_JEDEC_ID) {
        switch (type) {
            case REG_QE:
                read_cmd = WB_READ_QE_CMD;
                break;
            case REG_WEL:
                read_cmd = WB_READ_WEL_CMD;
                break;
            case REG_BUSY:
                read_cmd = WB_READ_BUSY_CMD;
                break;
            case REG_SUSPEND:
                read_cmd = WB_READ_SUSPEND_CMD;
                break;
        }
        ESC_Dev_Command_Ext(0, &read_cmd, &reg_data, 1, 1);
    }
    return reg_data;
}


bool write_qe_bit(uint8_t bit_value)
{
    uint32_t jedec_id = g_esc_ctl.device->jedec_id;
    bool result = false;
    uint8_t cmd[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

    if (ESC_PARSE_MANU_ID(jedec_id) == WB_JEDEC_ID) {
        /* device is W25Q128JW  W25Q64JW*/
        uint8_t origin = read_device_reg(REG_QE);
        uint8_t curr = origin, read_back = 0xFF;
        uint8_t busy_reg = 0xFF;
        flash_write_en();
        if (bit_value == 1) {
            curr |= (1 << (g_esc_ctl.device->qe_bit));
        } else {
            curr &= ~(1 << (g_esc_ctl.device->qe_bit));
        }

        /* write status register 2 command */
        cmd[0] = WB_WRITE_QE_CMD;
        cmd[1] = curr;
        ESC_Dev_Command_Ext(0, &cmd[0], NULL, 2, 0);

        /* Wait for device ready */
        do {
            busy_reg = read_device_reg(REG_BUSY);
        } while (device_is_busy(busy_reg));

        result = true;

        read_back = read_device_reg(REG_QE);
        log_hal_msgid_info("origin: 0x%02X, curr: 0x%02X, read_back: 0x%02X\r\n", 3, origin, curr, read_back);
    }

    if (ESC_PARSE_MANU_ID(jedec_id) == GD_JEDEC_ID) {
        uint8_t origin = read_device_reg(REG_QE);
        uint8_t curr = origin, read_back = 0xFF;
        uint8_t busy_reg = 0xFF;
        flash_write_en();
        if (bit_value == 1) {
            curr |= (1 << (g_esc_ctl.device->qe_bit));
        } else {
            curr &= ~(1 << (g_esc_ctl.device->qe_bit));
        }

        /* write status register 2 command */
        cmd[0] = 0x1;
        cmd[1] = 0x2;
        cmd[2] = curr;  //GD 0x2
        ESC_Dev_Command_Ext(0, &cmd[0], NULL, 3, 0);

        /* Wait for device ready */
        do {
            busy_reg = read_device_reg(REG_BUSY);
        } while (device_is_busy(busy_reg));

        result = true;

        read_back = read_device_reg(REG_QE);
        log_hal_msgid_info("[write_qe_bit] origin: 0x%02X, curr: 0x%02X, read_back: 0x%02X\r\n", 3, origin, curr, read_back);
    }
    return result;
}


ATTR_TEXT_IN_TCM bool device_is_busy(uint8_t status)
{
    uint8_t busy_bit = g_esc_ctl.device->busy_bit;
    bool result = false;

    if ((status & (0x1 << (busy_bit)))) {
        result = true;
    }
    return result;
}

uint8_t esc_flash_id[3];
ATTR_TEXT_IN_TCM bool esc_judge_device(void)
{
    uint8_t id[3] = { 0xFF, 0xFF, 0xFF };

    uint8_t read_id_cmd = READ_JEDEC_ID_CMD;  /* always 0x9F for flash */
    uint32_t support_list_idx = 0;
    uint32_t jedec_id;
    bool device_is_support = false;

    ESC_Dev_Command_Ext(0, &read_id_cmd, id, 1, 3);
    jedec_id = ((uint32_t)(id[0]) << 16) | ((uint32_t)(id[1]) << 8) | (id[2]);
    log_hal_msgid_info("ESC flash id: 0x%02X, 0x%02X, 0x%02X, and JEDEC id: 0x%08X\r\n", 4, id[0], id[1], id[2], jedec_id);

    esc_flash_id[0] = id[0];
    esc_flash_id[1] = id[1];
    esc_flash_id[2] = id[2];

    for (support_list_idx = 0; support_list_idx < SUPPORT_LIST_LENGTH; support_list_idx++) {
        if (jedec_id == g_esc_support_flash_list[support_list_idx].jedec_id) {
            g_esc_ctl.device = &(g_esc_support_flash_list[support_list_idx]);
            device_is_support = true;
            break;
        }
    }
    return device_is_support;
}


ATTR_TEXT_IN_TCM void esc_flash_return_ready(void)
{
    if(g_esc_ctl.busy == false) {
        //no flash P/E,don't need to check busy 
        return;
    } else {
        if ((g_esc_ctl.suspended == false) && (g_esc_ctl.busy == true)) {
            if (sw_esc_irq == 0) {
                sw_esc_irq = 1;
                hal_nvic_set_pending_irq(ESC_IRQn);
            }
        }
    }
}

//this is for exception
ATTR_TEXT_IN_TCM void esc_flash_suspend(void)
{
    uint32_t irq_mask;
    uint8_t device_status = 0xFF;
    uint8_t suspend_cmd = 0xFF;

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);

    if ((g_esc_ctl.suspended == false) && (g_esc_ctl.busy == true)) {
        device_status = read_device_reg(REG_BUSY);
        if (device_is_busy(device_status)) {
            /* device still busy */

            suspend_cmd = g_esc_ctl.device->suspend_cmd;
            /* 1. Issue suspend command */
            ESC_Dev_Command_Ext(0, &suspend_cmd, NULL, 1, 0);

            /* 2. Wait for device ready */
            do {
                device_status = read_device_reg(REG_BUSY);
            } while (device_is_busy(device_status));

            g_esc_ctl.suspended = true;
        } else {
            /* device is idle now */
            g_esc_ctl.busy = false;
        }
        ESC_MacLeave();
    }

    hal_nvic_restore_interrupt_mask(irq_mask);
}


ATTR_TEXT_IN_TCM int32_t esc_check_ready_and_resume(void)
{
    uint8_t status;
    uint32_t irq_mask;
    int32_t result = -1;
    uint8_t resume_cmd = 0xFF;
    bool ahb_req_flag = false;
    uint32_t gpt_tick;

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);

    if (g_esc_ctl.ahb_req_while_mac == true) {
        ahb_req_flag = true;
        result = -1;
    } else {
        status = read_device_reg(REG_BUSY);

        if ((status & FLASH_BUSY_STATUS) == false) {
            /* current not busy */
            if (g_esc_ctl.suspended == true) {
                /* check device suspend flag or
                 * driver recorded suspend flag for devices not has SUSPEND flag
                 */
                resume_cmd = g_esc_ctl.device->resume_cmd;
                /* issue resume command */
                ESC_Dev_Command_Ext(0, &resume_cmd, NULL, 1, 0);
                g_esc_ctl.suspended = false;
                result = -1;
            } else {
                /* device is neither busy nor suspeded */
                g_esc_ctl.suspended = false;
                g_esc_ctl.busy = false;
                result = 0;
            }
        } else {
            result = -1;    /* still busy */
        }
    }

    hal_nvic_restore_interrupt_mask(irq_mask);

    if (ahb_req_flag == true) {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_tick);
        //log_hal_msgid_warning("DMA or DSP direct read during ESC in MAC mode at %d.\r\n", 1, gpt_tick);
        hal_gpt_delay_us(50);
        hal_nvic_save_and_set_interrupt_mask(&irq_mask);
        g_esc_ctl.ahb_req_while_mac = false;
        ahb_req_flag = false;
        hal_nvic_restore_interrupt_mask(irq_mask);
    }

    if (result == -1) {
        //flash is busy, wait a while to avoid continues disable/enable irq
        hal_gpt_delay_us(223);
    }
    return result;
}

ATTR_TEXT_IN_TCM void flash_write_en(void)
{
    uint8_t write_en_cmd = 0xFF;
    uint32_t jedec_id = g_esc_ctl.device->jedec_id;

    if (ESC_PARSE_MANU_ID(jedec_id) == WB_JEDEC_ID || ESC_PARSE_MANU_ID(jedec_id) == GD_JEDEC_ID) {
        write_en_cmd = WB_WRITE_WEL_CMD;
    }
    ESC_Dev_Command(0, write_en_cmd);
}


ATTR_TEXT_IN_TCM bool esc_is_4_byte_mode(void)
{
    bool res = false;
    if (ESC_IS_4_BYTE_ADDRESS_MODE) {
        res = true;
    }
    return res;
}

ATTR_TEXT_IN_TCM void ESC_Write_C1A3(uint8_t cmd, uint32_t address, const uint8_t *buff, uint32_t length)
{
    uint8_t cmd_address[4];
    cmd_address[0] = cmd;
    cmd_address[1] = (uint8_t)(address >> 16);
    cmd_address[2] = (uint8_t)(address >> 8);
    cmd_address[3] = (uint8_t)(address);
    ESC_GPRAM_Write(0, cmd_address, 4);
    if (buff) {
        ESC_GPRAM_Write(4, buff, length);
    }
}

ATTR_TEXT_IN_TCM void ESC_Write_C1A4(uint8_t cmd, uint32_t address, const uint8_t *buff, uint32_t length)
{
    uint8_t data[8];
    int idx;
    data[0] = cmd;
    data[1] = (uint8_t)(address >> 24);
    data[2] = (uint8_t)(address >> 16);
    data[3] = (uint8_t)(address >> 8);
    data[4] = (uint8_t)(address);
    for (idx = 0; idx < 3; idx++) {
        if (idx < length) {
            data[5 + idx] = buff[idx];
        } else {
            break;
        }
    }
    ESC_GPRAM_Write(0, data, 8);
    if (length > 3) {
        length -= 3;
        ESC_GPRAM_Write(8, buff + 3, length);
    }
}

#else
    //error condition
#endif /* ESC_FLASH_ENABLE */


#ifdef HAL_ESC_SUPPORT_FLASH
hal_esc_flash_status_t hal_esc_read_flash_data(uint32_t address, uint8_t *buffer, uint32_t len)
{
#ifdef ESC_FLASH_ENABLE
    hal_esc_flash_status_t result = HAL_ESC_FLASH_STATUS_OK;
    result = esc_flash_parameter_check(address, (void *)buffer, len);
    if (result != HAL_ESC_FLASH_STATUS_OK) {
        return result;
    }
    memcpy(buffer, (void *)(address | ESC_GENERIC_SRAM_BANK_MASK), len);
    return HAL_ESC_FLASH_STATUS_OK;

#else /* ESC_FLASH_ENABLE */
    return HAL_ESC_FLASH_NOT_ENABLE;
#endif
}


/* if pointer data point to ESC flash region, read data first and then write to dest */
//ATTR_ZIDATA_IN_TCM uint8_t esc_write_buffer[ESC_GPRAM_SIZE];
hal_esc_flash_status_t hal_esc_write_flash_data(uint32_t address, const uint8_t *data, uint32_t len)
{
#ifdef ESC_FLASH_ENABLE
    uint32_t page_offset, /*max_size,*/ page_size;
    int32_t busy_status = 0;
    uint32_t mask;
    uint32_t To = address;
    const uint8_t *Buf = data;
    uint8_t esc_write_buffer[ESC_GPRAM_SIZE];

    hal_esc_flash_status_t result = HAL_ESC_FLASH_STATUS_OK;
    result = esc_flash_parameter_check(address, (void *)data, len);
    if (result != HAL_ESC_FLASH_STATUS_OK) {
        return result;
    }

    /* what page does it start with */
    page_offset = address % ESC_GPRAM_SIZE;


    /* write everything in PAGESIZE chunks */
    while (len > 0) {
        page_size = min(len, (ESC_GPRAM_SIZE - page_offset));
        page_offset = 0;

write_retry:
        do {
            busy_status = esc_check_ready_and_resume();
        } while (busy_status != 0);

        if (ESC_GENERIC_SRAM_BANK_MASK == ((uint32_t)data & ESC_GENERIC_0xFF000000_MASK)) {
            memcpy(esc_write_buffer, Buf, page_size);    /* direct read */
            Buf = esc_write_buffer;
        }

        hal_nvic_save_and_set_interrupt_mask(&mask);
        if (g_esc_ctl.suspended == true) {
            hal_nvic_restore_interrupt_mask_special(mask);
            goto write_retry;
        }
        flash_write_en();
        if (ESC_IS_4_BYTE_ADDRESS_MODE) {
            /* 1 byte cmd and 4 bytes address */
            ESC_Write_C1A4(g_esc_ctl.device->write_cmd, To, Buf, page_size);
            esc_register->ESC_MAC_OUTL = page_size + 5;
        } else {
            /* 1 byte cmd and 3 bytes address */
            esc_register->ESC_MAC_OUTL = page_size + 4;
            ESC_Write_C1A3(g_esc_ctl.device->write_cmd, To, Buf, page_size);
        }
        esc_register->ESC_MAC_INL = 0;
        g_esc_ctl.busy = true;
        ESC_MacEnable();
        ESC_MacWaitReady();
        len -= page_size;
        To += page_size;
        Buf += page_size;
        hal_nvic_restore_interrupt_mask(mask);

        for (;;) {
            busy_status = esc_check_ready_and_resume();
            if (busy_status == 0) {
                break;
            }
        }
    }

    return HAL_ESC_FLASH_STATUS_OK;

#else /* ESC_FLASH_ENABLE */
    return HAL_ESC_FLASH_NOT_ENABLE;
#endif
}


hal_esc_flash_status_t hal_esc_erase_flash_block(uint32_t address, hal_esc_erase_type_t type)
{
#ifdef ESC_FLASH_ENABLE
    uint32_t mask;
    int32_t busy_status = 0;
    uint32_t len = 0;

    hal_esc_flash_status_t result = HAL_ESC_FLASH_STATUS_OK;
    if (type == HAL_ESC_FLASH_ERASE_4K) {
        len = ESC_ERASE_4K_LENGTH;
    } else if (type == HAL_ESC_FLASH_ERASE_32K) {
        len = ESC_ERASE_32K_LENGTH;
    } else if (type == HAL_ESC_FLASH_ERASE_64K) {
        len = ESC_ERASE_64K_LENGTH;
    } else if (type == HAL_ESC_FLASH_ERASE_CHIP) {
        len = (g_esc_ctl.device->page_size) * (g_esc_ctl.device->page_number);
    } else {
        log_hal_msgid_error("hal_esc_erase_flash_block was called with invalid type %d\r\n", 1, type);
        return HAL_ESC_FLASH_WRONG_PARAMETER;
    }

    result = esc_flash_parameter_check(address, (void *)(ESC_GENERIC_SRAM_BANK_MASK + ESC_FAKE_ADDRESS), len);
    if (result != HAL_ESC_FLASH_STATUS_OK) {
        return result;
    }

    /* erase address should align with erase length */
    if ((type != HAL_ESC_FLASH_ERASE_CHIP) && (address & (len - 1))) {
        log_hal_msgid_error("hal_esc_erase_flash_block was called with invalid address 0x%08X and type %d\r\n", 2, address, type);
        return HAL_ESC_FLASH_STATUS_ERROR_WRONG_ADDRESS;
    }

erase_retry:
    do {
        busy_status = esc_check_ready_and_resume();
    } while (busy_status != 0);

    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (g_esc_ctl.suspended == true) {
        hal_nvic_restore_interrupt_mask(mask);
        goto erase_retry;
    }
    
    g_esc_ctl.busy = true;
    flash_write_en();
    if (ESC_IS_4_BYTE_ADDRESS_MODE) {
        /* 1 byte cmd and 4 bytes address */
        ESC_Write_C1A4(g_esc_ctl.device->erase_cmd[type], address, NULL, 0);
        esc_register->ESC_MAC_OUTL = 5;
    } else {
        /* 1 byte cmd and 3 bytes address */
        esc_register->ESC_MAC_OUTL = 4;
        ESC_Write_C1A3(g_esc_ctl.device->erase_cmd[type], address, NULL, 0);
    }
    esc_register->ESC_MAC_INL = 0;
    ESC_MacEnable();
    ESC_MacWaitReady();

     
    hal_nvic_restore_interrupt_mask(mask);

    for (;;) {
        busy_status = esc_check_ready_and_resume();
        if (busy_status == 0) {
            break;
        }
    }

    return HAL_ESC_FLASH_STATUS_OK;
#else /* ESC_FLASH_ENABLE */
    return HAL_ESC_FLASH_NOT_ENABLE;
#endif
}
#endif /* HAL_ESC_SUPPORT_FLASH */


#if defined(ESC_PSRAM_ENABLE) || defined(ESC_FLASH_ENABLE)
/*!
  @brief
    ESC abort ISR
*/
ATTR_TEXT_IN_TCM void esc_isr(hal_nvic_irq_t irq)
{
    (void)irq;

    if(sw_esc_irq) {
        uint8_t device_status = 0xFF;
        uint8_t suspend_cmd = 0xFF;
        if ((g_esc_ctl.suspended == false) && (g_esc_ctl.busy == true)) {
            device_status = read_device_reg(REG_BUSY);
            if (device_status & FLASH_BUSY_STATUS) {
                /* device still busy */
                suspend_cmd = g_esc_ctl.device->suspend_cmd;
                /* 1. Issue suspend command */
                ESC_Dev_Command_Ext(0, &suspend_cmd, NULL, 1, 0);
                /* 2. Wait for device ready */
                do {
                    device_status = read_device_reg(REG_BUSY);
                } while (device_is_busy(device_status));
                g_esc_ctl.suspended = true;
            } else {
                /* device is idle now */
                g_esc_ctl.busy = false;
            }
            ESC_MacLeave();
        }
        sw_esc_irq = 0;
        return;
    }
    
#ifdef ESC_FLASH_ENABLE
    /* DMA or DSP direct read ESC Flash while controller is in MACRO mode */
    g_esc_ctl.ahb_req_while_mac = true;
#endif

    //ack IRQ bit 16
    if ((esc_register->ESC_MAC_IRQ & 0x00000100) == 0x00000100) {
        esc_register->ESC_MAC_IRQ = esc_register->ESC_MAC_IRQ | 0x00010000;
        esc_register->ESC_MAC_IRQ = esc_register->ESC_MAC_IRQ & 0xFFFEFFFF;
    }

    //ack IRQ bit 17
    if ((esc_register->ESC_MAC_IRQ & 0x00000200) == 0x00000200) {
        esc_register->ESC_MAC_IRQ = esc_register->ESC_MAC_IRQ | 0x00020000;
        esc_register->ESC_MAC_IRQ = esc_register->ESC_MAC_IRQ & 0xFFFDFFFF;
    }

    //ack abort bit28
    if ((esc_register->ESC_MAC_IRQ & 0x02000000) == 0x02000000) {
        esc_register->ESC_MAC_IRQ = esc_register->ESC_MAC_IRQ | 0x10000000;
        esc_register->ESC_MAC_IRQ = esc_register->ESC_MAC_IRQ & 0xEFFFFFFF;
    }
}

#ifdef HAL_SLEEP_MANAGER_ENABLED
ATTR_TEXT_IN_TCM void esc_backup_register_callback(void *data)
{
    (void)data; /* unused variable */

    /* backup ESC controller registers */
    g_backup_esc_reg[ESC_REG_MAC_IRQ] = esc_register->ESC_MAC_IRQ;
    g_backup_esc_reg[ESC_REG_DIRECT_CTL] = esc_register->ESC_DIRECT_CTL;
    g_backup_esc_reg[ESC_REG_DLY_CTL2] = esc_register->ESC_DLY_CTL2;
    g_backup_esc_reg[ESC_REG_DLY_CTL3] = esc_register->ESC_DLY_CTL3;
    g_backup_esc_reg[ESC_REG_DLY_CTL4] = esc_register->ESC_DLY_CTL4;
}

ATTR_TEXT_IN_TCM void esc_restore_register_callback(void *data)
{
    (void)data; /* unused variable */

    /* restore ESC controller registers */
    esc_register->ESC_MAC_IRQ = g_backup_esc_reg[ESC_REG_MAC_IRQ];
    esc_register->ESC_DIRECT_CTL = g_backup_esc_reg[ESC_REG_DIRECT_CTL];
    esc_register->ESC_DLY_CTL2 = g_backup_esc_reg[ESC_REG_DLY_CTL2];
    esc_register->ESC_DLY_CTL3 = g_backup_esc_reg[ESC_REG_DLY_CTL3];
    esc_register->ESC_DLY_CTL4 = g_backup_esc_reg[ESC_REG_DLY_CTL4];

}
#endif /* HAL_SLEEP_MANAGER_ENABLED */


extern uint32_t hal_clock_get_freq_meter(hal_src_clock SRC_CLK, uint32_t winset);
bool esc_set_freq_related_parameter(esc_clock_t freq)
{
    if ((freq != ESC_CLOCK_26MHz) && (freq != ESC_CLOCK_52MHz)) {
        return false;
    }
#ifdef ESC_FLASH_ENABLE
    g_esc_ctl.clock = freq;

    /* In the ESC driver initialization phase,
     * because the device has not been recognized,
     * the value of this variable is NULL.
     */
    if (g_esc_ctl.device != NULL) {
        /* The initialization phase has been completed,
         * and the parameters in the ESC driver change with the DVFS.
         */
        if (g_esc_ctl.clock == ESC_CLOCK_26MHz) {
            esc_register->ESC_DLY_CTL1 = g_esc_ctl.device->delay_ctl_26MHz[0];
            esc_register->ESC_DLY_CTL2 = g_esc_ctl.device->delay_ctl_26MHz[1];
            esc_register->ESC_DLY_CTL3 = g_esc_ctl.device->delay_ctl_26MHz[2];
            esc_register->ESC_DLY_CTL4 = g_esc_ctl.device->delay_ctl_26MHz[3];
        } else {
            esc_register->ESC_DLY_CTL1 = g_esc_ctl.device->delay_ctl_52MHz[0];
            esc_register->ESC_DLY_CTL2 = g_esc_ctl.device->delay_ctl_52MHz[1];
            esc_register->ESC_DLY_CTL3 = g_esc_ctl.device->delay_ctl_52MHz[2];
            esc_register->ESC_DLY_CTL4 = g_esc_ctl.device->delay_ctl_52MHz[3];
        }
    }
#endif

#ifdef ESC_PSRAM_ENABLE
    if (freq == ESC_CLOCK_26MHz) {
        esc_register->ESC_DLY_CTL1 = g_setting.ESC_DLY_CTL_26MHz[0];
        esc_register->ESC_DLY_CTL2 = g_setting.ESC_DLY_CTL_26MHz[1];
        esc_register->ESC_DLY_CTL3 = g_setting.ESC_DLY_CTL_26MHz[2];
        esc_register->ESC_DLY_CTL4 = g_setting.ESC_DLY_CTL_26MHz[3];
    } else {
        esc_register->ESC_DLY_CTL1 = g_setting.ESC_DLY_CTL_52MHz[0];
        esc_register->ESC_DLY_CTL2 = g_setting.ESC_DLY_CTL_52MHz[1];
        esc_register->ESC_DLY_CTL3 = g_setting.ESC_DLY_CTL_52MHz[2];
        esc_register->ESC_DLY_CTL4 = g_setting.ESC_DLY_CTL_52MHz[3];
    }
#endif

    return true;
}

#ifdef ESC_INTERNAL_DEBUG
ATTR_TEXT_IN_TCM void check_mac_en(const char *called_func, uint32_t error_value)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    uint32_t mac_ctl = esc_register->ESC_MAC_CTL;
    uint32_t mac_en = mac_ctl & ESC_MAC_EN;
    if (mac_en == error_value) {
        hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
        for (;;) {
            log_hal_msgid_error("%s mac_en is %d, busy is %d, suspend is %d\r\n", 4, called_func, error_value, g_esc_ctl.busy, g_esc_ctl.suspended);
            //hal_gpt_delay_ms(1000);
        }
    }
    hal_nvic_restore_interrupt_mask(mask);
}
#endif

ATTR_TEXT_IN_TCM void esc_mask_channel(void)
{
#ifdef ESC_INTERNAL_DEBUG
    check_mac_en("esc_mask_channel", 0x100);
#endif
    ESC_MacEnable();
    while ((esc_register->ESC_DEBUG_MUX & (ESC_DIRECT_BUSY)) == ESC_DIRECT_BUSY);
}


ATTR_TEXT_IN_TCM void esc_unmask_channel(void)
{
    ESC_MacLeave();
}

uint32_t esc_base_address(void)
{
    return ESC_GENERIC_SRAM_BANK_MASK + 0x1000;
}

uint32_t esc_flash_length(void)
{
#ifdef ESC_FLASH_ENABLE
    if (g_esc_ctl.device == NULL) {
        return 0xFFFFFFFF;
    }
    uint32_t flash_length = g_esc_ctl.device->page_size * g_esc_ctl.device->page_number;
    return flash_length;
#else
    return 0xFFFFFFFF;
#endif
}


uint32_t esc_flash_jedec_id(void)
{
#ifdef ESC_FLASH_ENABLE
    if (g_esc_ctl.device == NULL) {
        return 0xFFFFFFFF;
    }
    return g_esc_ctl.device->jedec_id;
#else
    return 0xFFFFFFFF;
#endif
}

void esc_memory_access_enable(void)
{
    /* CMCFG_BOOT_FROM_SLV:
          0     access ESC memory
          1     boot slaver
    */
    MCU_CFG_PRI->CMCFG_BOOT_FROM_SLV = 0x0;
}
#endif

#endif /* HAL_ESC_MODULE_ENABLED */

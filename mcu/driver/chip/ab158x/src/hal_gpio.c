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

#include "hal_gpio.h"

#ifdef HAL_GPIO_MODULE_ENABLED
#include "hal_gpio_internal.h"
#include "hal_nvic.h"
#include "hal_clock_internal.h"
#include "hal_log.h"
#include <assert.h>

#if !defined(__UBL__) && !defined(__EXT_DA__)
#include "ept_gpio_drv.h"
#include "exception_handler.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

GPIO_BASE_REGISTER_T *gpio_base = (GPIO_BASE_REGISTER_T *)(GPIO_BASE);
GPIO_CFG0_REGISTER_T *gpio_cfg0 = (GPIO_CFG0_REGISTER_T *)(IO_CFG_0_BASE);


hal_gpio_status_t hal_gpio_init(hal_gpio_pin_t gpio_pin)
{
    (void)gpio_pin;
    return HAL_GPIO_STATUS_OK;
}


hal_gpio_status_t hal_gpio_deinit(hal_gpio_pin_t gpio_pin)
{
    (void)gpio_pin;
    return HAL_GPIO_STATUS_OK;
}

hal_gpio_status_t hal_gpio_set_direction(hal_gpio_pin_t gpio_pin, hal_gpio_direction_t gpio_direction)
{
    uint32_t reg_num;
    uint32_t index_num;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    reg_num   = gpio_pin / GPIO_DIR_REG_CTRL_PIN_NUM;
    index_num = gpio_pin % GPIO_DIR_REG_CTRL_PIN_NUM;

    if (gpio_direction == HAL_GPIO_DIRECTION_INPUT) {
        gpio_base->GPIO_DIR.CLR[reg_num] = (GPIO_REG_ONE_BIT_SET_CLR << index_num);
    } else {
        gpio_base->GPIO_DIR.SET[reg_num] = (GPIO_REG_ONE_BIT_SET_CLR << index_num);
    }

    return HAL_GPIO_STATUS_OK;

}

hal_gpio_status_t hal_gpio_get_direction(hal_gpio_pin_t gpio_pin, hal_gpio_direction_t *gpio_direction)
{
    uint32_t reg_num;
    uint32_t index_num;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (NULL == gpio_direction) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    reg_num   = gpio_pin / GPIO_DIR_REG_CTRL_PIN_NUM;
    index_num = gpio_pin % GPIO_DIR_REG_CTRL_PIN_NUM;

    if (gpio_base->GPIO_DIR.RW[reg_num] & (GPIO_REG_ONE_BIT_SET_CLR << index_num)) {
        *gpio_direction = HAL_GPIO_DIRECTION_OUTPUT;
    } else {
        *gpio_direction = HAL_GPIO_DIRECTION_INPUT;
    }

    return HAL_GPIO_STATUS_OK;

}

hal_pinmux_status_t hal_pinmux_set_function(hal_gpio_pin_t gpio_pin, uint8_t function_index)
{
    uint32_t no;
    uint32_t remainder;
    uint32_t mask;
    uint32_t temp;
    uint32_t shift;
#if !defined(__UBL__) && !defined(__EXT_DA__)
    uint32_t log_port;
    bool one_wire_log;
#endif

    log_hal_msgid_info("[pinmux] pin=%d mode=%d caller=0x%x\r\n", 3, gpio_pin, function_index, (uint32_t)__builtin_return_address(0));

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_PINMUX_STATUS_ERROR_PORT;
    }

    /* check whether the function index is right as one function is corresponding to 4 bits of oen pin */
    if (function_index >= GPIO_MODE_MAX_NUMBER) {
        return HAL_PINMUX_STATUS_INVALID_FUNCTION;
    }

/* Dynamic switching of log port GPIO pinmux is not allowed */
#if !defined(__UBL__) && !defined(__EXT_DA__)
    log_port = query_syslog_port();
    one_wire_log = syslog_query_one_wire_log_active();
    if((log_port < HAL_UART_MAX) && (one_wire_log == false)) {
        switch (log_port) {
            case HAL_UART_0:
                if((gpio_pin == HAL_UART0_TXD_PIN) || (gpio_pin == HAL_UART0_RXD_PIN)) {
                    assert(0);
                }
                break;
            case HAL_UART_1:
                if((gpio_pin == HAL_UART1_TXD_PIN) || (gpio_pin == HAL_UART1_RXD_PIN)) {
                   assert(0);
                }
                break;
            case HAL_UART_2:
                if((gpio_pin == HAL_UART2_TXD_PIN) || (gpio_pin == HAL_UART2_RXD_PIN)) {
                    assert(0);
                }
                break;
            default:
                break;
        }
    }
#endif

    /* protect the configuration to prevent possible interrupt */
    hal_nvic_save_and_set_interrupt_mask(&mask);

    /* get the register number corresponding to the pin as one register can control 8 pins*/
    no = gpio_pin / GPIO_MODE_REG_CTRL_PIN_NUM;

    /* get the bit offset within the register as one register can control 8 pins*/
    remainder = gpio_pin % GPIO_MODE_REG_CTRL_PIN_NUM;

    temp = gpio_base->GPIO_MODE.RW[no];
    temp &= ~(GPIO_REG_FOUR_BIT_SET_CLR << (remainder * GPIO_MODE_FUNCTION_CTRL_BITS));
    temp |= (function_index << (remainder * GPIO_MODE_FUNCTION_CTRL_BITS));
    gpio_base->GPIO_MODE.RW[no] = temp;

    shift = gpio_cfg_table[gpio_pin].g_cfg_shift;
    /* switch to digital mode for leakage, ADC driver change it to analog mode. */
    if (shift != 0xFF) {
        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].g_cfg_reg + GPIO_SET_ADDR) = 1 << shift;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].g_cfg_reg + GPIO_SET_ADDR) = 1 << shift;
        }
    }
#if 0
    shift = gpio_cfg_table[gpio_pin].eh_shift;
    if (shift != 0xFF) {
        if (((function_index == 1) && (gpio_pin >= 13) && (gpio_pin <= 18))
            || ((function_index == 3) && (gpio_pin >= 47) && (gpio_pin <= 50))) { //I2C mode
            //enable I2C mode
            GPIO_REG32(gpio_cfg_table[gpio_pin].eh_reg + GPIO_SET_ADDR) = 0x1 << shift;
        } else {
            //disable I2C mode
            GPIO_REG32(gpio_cfg_table[gpio_pin].eh_reg + GPIO_CLR_ADDR) = 0x1 << shift;
        }
    }
#endif
    hal_nvic_restore_interrupt_mask(mask);

    return HAL_PINMUX_STATUS_OK;

}

uint32_t hal_pinmux_get_function(hal_gpio_pin_t gpio_pin)
{
    uint32_t reg_index;
    uint32_t bit_index;
    uint32_t mode;

    reg_index = gpio_pin / 8;
    bit_index = (gpio_pin % 8) * 4;
    mode = (gpio_base->GPIO_MODE.RW[reg_index] >> (bit_index) & 0xf);

    return mode;
}

#if !defined(__UBL__) && !defined(__EXT_DA__)
hal_gpio_status_t hal_gpio_mode_ept_compare_hw_config(void)
{
    uint32_t i, j;
    uint32_t count = 0;
    uint32_t hw_gpio_mode[HAL_GPIO_MAX]  = {0};
    uint32_t ept_gpio_mode[] = {GPIO_PORT0_MODE,  GPIO_PORT1_MODE,  GPIO_PORT2_MODE,  GPIO_PORT3_MODE,  GPIO_PORT4_MODE,  GPIO_PORT5_MODE,  GPIO_PORT6_MODE,  GPIO_PORT7_MODE,  GPIO_PORT8_MODE,  GPIO_PORT9_MODE,\
                                GPIO_PORT10_MODE, GPIO_PORT11_MODE, GPIO_PORT12_MODE, GPIO_PORT13_MODE, GPIO_PORT14_MODE, GPIO_PORT15_MODE, GPIO_PORT16_MODE, GPIO_PORT17_MODE, GPIO_PORT18_MODE, GPIO_PORT19_MODE,\
                                GPIO_PORT20_MODE, GPIO_PORT21_MODE, GPIO_PORT22_MODE, GPIO_PORT23_MODE, GPIO_PORT24_MODE, GPIO_PORT25_MODE, GPIO_PORT26_MODE, GPIO_PORT27_MODE, GPIO_PORT28_MODE, GPIO_PORT29_MODE,\
                                GPIO_PORT30_MODE, GPIO_PORT31_MODE, GPIO_PORT32_MODE, GPIO_PORT33_MODE, GPIO_PORT34_MODE, GPIO_PORT35_MODE, GPIO_PORT36_MODE, GPIO_PORT37_MODE, GPIO_PORT38_MODE, GPIO_PORT39_MODE,\
                                GPIO_PORT40_MODE, GPIO_PORT41_MODE, GPIO_PORT42_MODE, GPIO_PORT43_MODE, GPIO_PORT44_MODE, GPIO_PORT45_MODE, GPIO_PORT46_MODE, GPIO_PORT47_MODE, GPIO_PORT48_MODE, GPIO_PORT49_MODE,\
                                GPIO_PORT50_MODE, GPIO_PORT51_MODE, GPIO_PORT52_MODE, GPIO_PORT53_MODE, GPIO_PORT54_MODE, GPIO_PORT55_MODE, GPIO_PORT56_MODE, GPIO_PORT57_MODE};


    for (i = 0; i < HAL_GPIO_MAX; i++) {
        hw_gpio_mode[i] = hal_pinmux_get_function(i);
    }

    for (j = 0; j < HAL_GPIO_MAX; j++) {
       if(ept_gpio_mode[j] != hw_gpio_mode[j]) {
            if(hal_core_status_read(HAL_CORE_MCU) == HAL_CORE_EXCEPTION) {
                exception_printf("[pinmux] ept gpio[%d] = mode%d, hw gpio[%d] = mode%d\r\n", j, ept_gpio_mode[j], j, hw_gpio_mode[j]);
            } else {
                log_hal_msgid_error("[pinmux] ept gpio[%d] = mode%d, hw gpio[%d] = mode%d\r\n", 4, j, ept_gpio_mode[j], j, hw_gpio_mode[j]);
            }
        } else {
            count ++;
            //log_hal_msgid_info("[pinmux] ept gpio[%d] = mode%d,  hw gpio[%d] = mode%d\r\n", 4, j, ept_gpio_mode[j], j, hw_gpio_mode[j]);
        }
    }

    if (count == HAL_GPIO_MAX) {
        if(hal_core_status_read(HAL_CORE_MCU) == HAL_CORE_EXCEPTION) {
            exception_printf("[pinmux] all gpio pinmux mode match\r\n");
        } else {
            log_hal_msgid_info("[pinmux] all gpio pinmux mode match\r\n", 0);
        }
    }

    count = 0;

    return HAL_GPIO_STATUS_OK;
}
#endif

hal_gpio_status_t hal_gpio_get_input(hal_gpio_pin_t gpio_pin, hal_gpio_data_t *gpio_data)
{
    uint32_t reg_num;
    uint32_t index_num;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (NULL == gpio_data) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    reg_num   = gpio_pin / GPIO_DIN_REG_CTRL_PIN_NUM;
    index_num = gpio_pin % GPIO_DIN_REG_CTRL_PIN_NUM;

    if (gpio_base->GPIO_DIN.R[reg_num] & (GPIO_REG_ONE_BIT_SET_CLR << index_num)) {
        *gpio_data = HAL_GPIO_DATA_HIGH;
    } else {
        *gpio_data = HAL_GPIO_DATA_LOW;
    }

    return HAL_GPIO_STATUS_OK;

}


hal_gpio_status_t hal_gpio_set_output(hal_gpio_pin_t gpio_pin, hal_gpio_data_t gpio_data)
{
    uint32_t reg_num;
    uint32_t index_num;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    reg_num   = gpio_pin / GPIO_DOUT_REG_CTRL_PIN_NUM;
    index_num = gpio_pin % GPIO_DOUT_REG_CTRL_PIN_NUM;

    if (gpio_data) {
        gpio_base->GPIO_DOUT.SET[reg_num] = (GPIO_REG_ONE_BIT_SET_CLR << index_num);
    } else {
        gpio_base->GPIO_DOUT.CLR[reg_num] = (GPIO_REG_ONE_BIT_SET_CLR << index_num);
    }

    return HAL_GPIO_STATUS_OK;
}


hal_gpio_status_t hal_gpio_get_output(hal_gpio_pin_t gpio_pin, hal_gpio_data_t *gpio_data)
{
    uint32_t reg_num;
    uint32_t index_num;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (NULL == gpio_data) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    reg_num   = gpio_pin / GPIO_DOUT_REG_CTRL_PIN_NUM;
    index_num = gpio_pin % GPIO_DOUT_REG_CTRL_PIN_NUM;

    if (gpio_base->GPIO_DOUT.RW[reg_num] & (GPIO_REG_ONE_BIT_SET_CLR << index_num)) {
        *gpio_data = HAL_GPIO_DATA_HIGH;
    } else {
        *gpio_data = HAL_GPIO_DATA_LOW;
    }

    return HAL_GPIO_STATUS_OK;

}

hal_gpio_status_t hal_gpio_toggle_pin(hal_gpio_pin_t gpio_pin)
{

    uint32_t reg_num = 0;
    uint32_t index_num = 0;
    uint32_t mask;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    hal_nvic_save_and_set_interrupt_mask(&mask);
    reg_num   = gpio_pin / GPIO_DOUT_REG_CTRL_PIN_NUM;
    index_num = gpio_pin % GPIO_DOUT_REG_CTRL_PIN_NUM;

    if (gpio_base->GPIO_DOUT.RW[reg_num] & (GPIO_REG_ONE_BIT_SET_CLR << index_num)) {

        gpio_base->GPIO_DOUT.CLR[reg_num] = (GPIO_REG_ONE_BIT_SET_CLR << index_num);
    } else {
        gpio_base->GPIO_DOUT.SET[reg_num] = (GPIO_REG_ONE_BIT_SET_CLR << index_num);
    }
    hal_nvic_restore_interrupt_mask(mask);

    return HAL_GPIO_STATUS_OK;
}

hal_gpio_status_t hal_gpio_set_rsel_register(hal_gpio_pin_t gpio_pin, uint8_t gpio_pu, uint8_t gpio_pd, uint8_t gpio_rsel0, uint8_t gpio_rsel1)
{
    uint32_t rsel_shift;
    uint32_t pupd_shift;
    uint8_t  shift_addr[2] = {GPIO_CLR_ADDR, GPIO_SET_ADDR};

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    rsel_shift = 1 << gpio_cfg_table[gpio_pin].rsel_shift;
    pupd_shift = 1 << gpio_cfg_table[gpio_pin].pupd_shift;

    if (gpio_cfg_table[gpio_pin].rsel_shift != 0xff) {
        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pu_reg + shift_addr[gpio_pu ? 1 : 0]) = pupd_shift;
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pd_reg + shift_addr[gpio_pd ? 1 : 0]) = pupd_shift;
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].rsel_reg + shift_addr[gpio_rsel0 ? 1 : 0]) = rsel_shift;
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].rsel_reg + shift_addr[gpio_rsel1 ? 1 : 0]) = rsel_shift << 1;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pu_reg + shift_addr[gpio_pu ? 1 : 0]) = pupd_shift;
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pd_reg + shift_addr[gpio_pd ? 1 : 0]) = pupd_shift;
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].rsel_reg + shift_addr[gpio_rsel0 ? 1 : 0]) = rsel_shift;
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].rsel_reg + shift_addr[gpio_rsel1 ? 1 : 0]) = rsel_shift << 1;
        }
    } else {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    return HAL_GPIO_STATUS_OK;
}

hal_gpio_status_t hal_gpio_pull_up(hal_gpio_pin_t gpio_pin)
{
    uint8_t  shift;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }
    if (gpio_cfg_table[gpio_pin].rsel_shift != 0xff) {
        hal_gpio_set_rsel_register(gpio_pin, 1, 0, 0, 0);
    } else {
        shift = gpio_cfg_table[gpio_pin].pupd_shift;
        if (shift != 0xff) {
            if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
                GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pd_reg + GPIO_CLR_ADDR) = 1 << shift;
                GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pu_reg + GPIO_SET_ADDR) = 1 << shift;
            } else {
                GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pd_reg + GPIO_CLR_ADDR) = 1 << shift;
                GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pu_reg + GPIO_SET_ADDR) = 1 << shift;
            }
        } else {
#ifdef HAL_GPIO_FEATURE_PUPD
            return hal_gpio_set_pupd_register(gpio_pin, 0, 1, 0); //pull up R0
#endif
        }
    }
    return HAL_GPIO_STATUS_OK;
}



hal_gpio_status_t hal_gpio_pull_down(hal_gpio_pin_t gpio_pin)
{
    uint8_t  shift;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }
    if (gpio_cfg_table[gpio_pin].rsel_shift != 0xff) {
        hal_gpio_set_rsel_register(gpio_pin, 0, 1, 0, 0);
    } else {
        shift = gpio_cfg_table[gpio_pin].pupd_shift;
        if (shift != 0xff) {
            if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
                GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pu_reg + GPIO_CLR_ADDR) = 1 << shift;
                GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pd_reg + GPIO_SET_ADDR) = 1 << shift;
            } else {
                GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pu_reg + GPIO_CLR_ADDR) = 1 << shift;
                GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pd_reg + GPIO_SET_ADDR) = 1 << shift;
            }
        } else {
#ifdef HAL_GPIO_FEATURE_PUPD
            return hal_gpio_set_pupd_register(gpio_pin, 1, 1, 0); //pull down R0
#endif
        }
    }

    return HAL_GPIO_STATUS_OK;
}



hal_gpio_status_t hal_gpio_disable_pull(hal_gpio_pin_t gpio_pin)
{
    uint8_t  shift;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }
    if (gpio_cfg_table[gpio_pin].rsel_shift != 0xff) {
        hal_gpio_set_rsel_register(gpio_pin, 0, 0, 0, 0);
    } else {
        shift = gpio_cfg_table[gpio_pin].pupd_shift;
        if (shift != 0xff) {
            if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
                GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pu_reg + GPIO_CLR_ADDR) = 1 << shift;
                GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pd_reg + GPIO_CLR_ADDR) = 1 << shift;
            } else {
                GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pu_reg + GPIO_CLR_ADDR) = 1 << shift;
                GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pd_reg + GPIO_CLR_ADDR) = 1 << shift;
            }
        } else {
#ifdef HAL_GPIO_FEATURE_PUPD
            return hal_gpio_set_pupd_register(gpio_pin, 0, 0, 0); //disable pull
#endif
        }
    }

    return HAL_GPIO_STATUS_OK;
}



#ifdef HAL_GPIO_FEATURE_CLOCKOUT

hal_gpio_status_t hal_gpio_set_clockout(hal_gpio_clock_t gpio_clock_num, hal_gpio_clock_mode_t clock_mode)
{
    if (gpio_clock_num >= HAL_GPIO_CLOCK_MAX) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    switch (gpio_clock_num) {
        case HAL_GPIO_CLOCK_0:
            GPIO_CLKO_CTRL_A_REG.CLK_MODE0 = clock_mode;
            break;
        case HAL_GPIO_CLOCK_1:
            GPIO_CLKO_CTRL_A_REG.CLK_MODE1 = clock_mode;
            break;
        case HAL_GPIO_CLOCK_2:
            GPIO_CLKO_CTRL_A_REG.CLK_MODE2 = clock_mode;
            break;
        case HAL_GPIO_CLOCK_3:
            GPIO_CLKO_CTRL_A_REG.CLK_MODE3 = clock_mode;
            break;
        default:
            break;
    }

    return HAL_GPIO_STATUS_OK;
}
#endif

#ifdef HAL_GPIO_FEATURE_SET_SCHMITT
hal_gpio_status_t hal_gpio_set_schmitt(hal_gpio_pin_t gpio_pin)
{
    uint8_t  shift;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    shift = gpio_cfg_table[gpio_pin].smt_sr_shift;
    if (shift != 0xff) {
        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].smt_reg + GPIO_SET_ADDR) = 1 << shift;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].smt_reg + GPIO_SET_ADDR) = 1 << shift;
        }
    }

    return HAL_GPIO_STATUS_OK;
}

hal_gpio_status_t hal_gpio_clear_schmitt(hal_gpio_pin_t gpio_pin)
{
    uint8_t  shift;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    shift = gpio_cfg_table[gpio_pin].smt_sr_shift;
    if (shift != 0xff) {
        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].smt_reg + GPIO_CLR_ADDR) = 1 << shift;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].smt_reg + GPIO_CLR_ADDR) = 1 << shift;
        }
    }

    return HAL_GPIO_STATUS_OK;
}
#endif



#ifdef HAL_GPIO_FEATURE_SET_SLEW_RATE
hal_gpio_status_t hal_gpio_set_slew_rate(hal_gpio_pin_t gpio_pin)
{
    uint8_t  shift;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    shift = gpio_cfg_table[gpio_pin].smt_sr_shift;
    if (shift != 0xff) {
        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].sr_reg + GPIO_SET_ADDR) = 1 << shift;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].sr_reg + GPIO_SET_ADDR) = 1 << shift;
        }
    }

    return HAL_GPIO_STATUS_OK;
}

hal_gpio_status_t hal_gpio_clear_slew_rate(hal_gpio_pin_t gpio_pin)
{
    uint8_t  shift;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    shift = gpio_cfg_table[gpio_pin].smt_sr_shift;
    if (shift != 0xff) {
        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].sr_reg + GPIO_CLR_ADDR) = 1 << shift;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].sr_reg + GPIO_CLR_ADDR) = 1 << shift;
        }
    }

    return HAL_GPIO_STATUS_OK;
}
#endif


#ifdef HAL_GPIO_FEATURE_PUPD
hal_gpio_status_t hal_gpio_set_pupd_register(hal_gpio_pin_t gpio_pin, uint8_t gpio_pupd, uint8_t gpio_r0, uint8_t gpio_r1)
{
    uint8_t  shift;
    uint8_t  shift_addr;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    shift = gpio_cfg_table[gpio_pin].pupd_r0_r1_shift;

    if (gpio_cfg_table[gpio_pin].pupd_r0_r1_shift != 0xff) {
        if (gpio_pupd) {
            shift_addr = GPIO_SET_ADDR;
        } else {
            shift_addr = GPIO_CLR_ADDR;
        }

        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pupd_reg + shift_addr) = 1 << shift;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pupd_reg + shift_addr) = 1 << shift;
        }

        if (gpio_r0) {
            shift_addr = GPIO_SET_ADDR;
        } else {
            shift_addr = GPIO_CLR_ADDR;
        }

        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].r0_reg + shift_addr) = 1 << shift;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].r0_reg + shift_addr) = 1 << shift;
        }

        if (gpio_r1) {
            shift_addr = GPIO_SET_ADDR;
        } else {
            shift_addr = GPIO_CLR_ADDR;
        }

        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].r1_reg + shift_addr) = 1 << shift;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].r1_reg + shift_addr) = 1 << shift;
        }
    } else {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    return HAL_GPIO_STATUS_OK;
}
#endif

#ifdef HAL_GPIO_FEATURE_HIGH_Z
hal_gpio_status_t hal_gpio_set_high_impedance(hal_gpio_pin_t gpio_pin)
{
    hal_pinmux_status_t ret1;
    hal_gpio_status_t   ret2;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    /* set GPIO mode of pin */
    ret1 = hal_pinmux_set_function(gpio_pin, 0);
    if (ret1 != HAL_PINMUX_STATUS_OK) {
        return HAL_GPIO_STATUS_ERROR;
    }

    /* set input direction of pin */
    ret2 = hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_INPUT);
    if (ret2 != HAL_GPIO_STATUS_OK) {
        return ret2;
    }

    /* Disable input buffer enable function of pin */
    if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
        GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].ies_reg + GPIO_SET_ADDR) = 1 << gpio_cfg_table[gpio_pin].ies_shift;
    } else {
        GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].ies_reg + GPIO_SET_ADDR) = 1 << gpio_cfg_table[gpio_pin].ies_shift;
    }

    /* disable pull function of pin */
    ret2 = hal_gpio_disable_pull(gpio_pin);
    if (ret2 != HAL_GPIO_STATUS_OK) {
        return ret2;
    }

    return HAL_GPIO_STATUS_OK;
}


hal_gpio_status_t hal_gpio_clear_high_impedance(hal_gpio_pin_t gpio_pin)
{

    hal_pinmux_status_t ret1;
    hal_gpio_status_t   ret2;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    /* set GPIO mode of pin. */
    ret1 = hal_pinmux_set_function(gpio_pin, 0);
    if (ret1 != HAL_PINMUX_STATUS_OK) {
        return HAL_GPIO_STATUS_ERROR;
    }

    /* set input direction of pin. */
    ret2 = hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_INPUT);
    if (ret2 != HAL_GPIO_STATUS_OK) {
        return ret2;
    }

    /* Enable input buffer enable function of pin */
    if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
        GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].ies_reg + GPIO_CLR_ADDR) = 1 << gpio_cfg_table[gpio_pin].ies_shift;
    } else {
        GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].ies_reg + GPIO_CLR_ADDR) = 1 << gpio_cfg_table[gpio_pin].ies_shift;
    }

    /* enable pull down of pin. */
    ret2 = hal_gpio_pull_down(gpio_pin);
    if (ret2 != HAL_GPIO_STATUS_OK) {
        return ret2;
    }

    return HAL_GPIO_STATUS_OK;
}
#endif

#ifdef HAL_GPIO_FEATURE_SET_CAPACITANCE
hal_gpio_status_t hal_gpio_set_capacitance(hal_gpio_pin_t gpio_pin, hal_gpio_capacitance_t cap)
{
    uint8_t  shift;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }
    if (cap >= HAL_GPIO_CAPACITANCE_MAX) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    shift = gpio_cfg_table[gpio_pin].eh_shift;
    if (shift != 0xFF) {
        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].eh_reg + GPIO_CLR_ADDR) = GPIO_REG_THREE_BIT_SET_CLR << shift;
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].eh_reg + GPIO_SET_ADDR) = ((cap << 1) | 0x1) << shift;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].eh_reg + GPIO_CLR_ADDR) = GPIO_REG_THREE_BIT_SET_CLR << shift;
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].eh_reg + GPIO_SET_ADDR) = ((cap << 1) | 0x1) << shift;
        }
    } else {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    return HAL_GPIO_STATUS_OK;
}

hal_gpio_status_t hal_gpio_clear_capacitance(hal_gpio_pin_t gpio_pin)
{
    uint8_t  shift;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    shift = gpio_cfg_table[gpio_pin].eh_shift;
    if (shift != 0xFF) {
        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].eh_reg + GPIO_CLR_ADDR) = GPIO_REG_THREE_BIT_SET_CLR << shift;
        } else {
            GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].eh_reg + GPIO_CLR_ADDR) = GPIO_REG_THREE_BIT_SET_CLR << shift;
        }
    } else {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    return HAL_GPIO_STATUS_OK;
}
#endif

#ifdef HAL_GPIO_FEATURE_SET_DRIVING
hal_gpio_status_t hal_gpio_set_driving_current(hal_gpio_pin_t gpio_pin, hal_gpio_driving_current_t driving)
{
    uint32_t mask;
    uint8_t  shift;
    uint32_t temp;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }
    if (driving >= HAL_GPIO_DRIVING_CURRENT_MAX) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }
    hal_nvic_save_and_set_interrupt_mask(&mask);

    shift = gpio_cfg_table[gpio_pin].drv_shift;

    if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
        temp = GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].drv_reg);
    } else {
        temp = GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].drv_reg);
    }
    temp &= ~(GPIO_REG_THREE_BIT_SET_CLR << shift);
    temp |= (driving << shift);
    if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
        GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].drv_reg) = temp;
    } else {
        GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].drv_reg) = temp;
    }

    hal_nvic_restore_interrupt_mask(mask);

    return HAL_GPIO_STATUS_OK;
}


hal_gpio_status_t hal_gpio_get_driving_current(hal_gpio_pin_t gpio_pin, hal_gpio_driving_current_t *driving)
{

    uint8_t  shift;
    uint32_t temp;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    shift = gpio_cfg_table[gpio_pin].drv_shift;

    if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
        temp = GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].drv_reg);
    } else {
        temp = GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].drv_reg);
    }
    temp = (temp >> shift) & GPIO_REG_THREE_BIT_SET_CLR;

    *driving = (hal_gpio_driving_current_t)(temp);

    return HAL_GPIO_STATUS_OK;

}
#endif

#ifdef __cplusplus
}
#endif

#endif  /* HAL_GPIO_MODULE_ENABLED */


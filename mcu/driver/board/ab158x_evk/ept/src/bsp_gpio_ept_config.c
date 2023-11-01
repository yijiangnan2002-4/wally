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

#include "air_chip.h"
#include "hal_log.h"
#include "ept_gpio_drv.h"
#include "ept_eint_drv.h"
#include "bsp_gpio_ept_define.h"
#include "bsp_gpio_ept_config.h"
#include "hal_resource_assignment.h"

GPIO_BASE_REGISTER_T *ept_gpio_base = (GPIO_BASE_REGISTER_T *)(GPIO_BASE);
GPIO_CFG0_REGISTER_T *ept_gpio_cfg0 = (GPIO_CFG0_REGISTER_T *)(IO_CFG_0_BASE);
GPIO_CFG1_REGISTER_T *ept_gpio_cfg1 = (GPIO_CFG1_REGISTER_T *)(IO_CFG_1_BASE);

const uint32_t GPIO_REG_PIN_MASK[2] = {EPT_GPIO_PIN_MASK_0, EPT_GPIO_PIN_MASK_1};

//#define EPT_DEBUG_LOG


/**
  * @brief  write the joint data about GPIO mode to GPIO mode register one by one
  * @param  None
  * @retval None
  */
void gpio_mode_init(void)
{
#ifdef GPIO_MODE_REG_MAX_NUM
    uint32_t i, j;
    uint32_t reg_mask[GPIO_MODE_REG_MAX_NUM];
    uint32_t temp;
    uint32_t mode_temp[GPIO_MODE_REG_MAX_NUM] = { GPIO_MODE_ALL_VALUE };

    for (i = 0; i < GPIO_MODE_REG_MAX_NUM; i++) {

        reg_mask[i] = 0;

        for (j = 0; j < GPIO_MODE_ONE_REG_CONTROL_NUM; j++) {

            temp = j + i * GPIO_MODE_ONE_REG_CONTROL_NUM; /*GPIO number*/

            /*check the mask bit is available*/
            if ((1 << (temp % 32)) &  GPIO_REG_PIN_MASK[temp / 32]) {
                reg_mask[i] |=  0xf << (GPIO_MODE_ONE_CONTROL_BITS * j);
            }
        }
    }

    for (i = 0; i < GPIO_MODE_REG_MAX_NUM; i++) {
        ept_gpio_base->GPIO_MODE.CLR[i] = reg_mask[i];
        ept_gpio_base->GPIO_MODE.SET[i] = mode_temp[i];

#ifdef EPT_DEBUG_LOG
        log_hal_msgid_info("GPIO_MODE[%d]: =0x%.8x, reg_mask[%d]=0x%.8x\r\n", 4, i, mode_temp[i], i, reg_mask[i]);
#endif

    }
#endif
}



/**
  * @brief  write the joint data about GPIO direction configuration to GPIO direction register one by one
  * @param  None
  * @retval None
  */
void gpio_dir_init(void)
{
#ifdef GPIO_DIR_REG_MAX_NUM
    uint32_t i;
    uint32_t dir_temp[GPIO_DIR_REG_MAX_NUM] = { GPIO_DIR_ALL_VALUE };

    for (i = 0; i < GPIO_DIR_REG_MAX_NUM; i++) {
        ept_gpio_base->GPIO_DIR.CLR[i] = GPIO_REG_PIN_MASK[i];
        ept_gpio_base->GPIO_DIR.SET[i] = dir_temp[i];
    }
#endif
}



/**
  * @brief  write the joint data about GPIO pull configuration about pullsel to GPIO pullsel register one by one
  * @param  None
  * @retval None
  */
void gpio_pu_init(void)
{
#ifdef GPIO_PU_REG_MAX_NUM
    uint32_t pullsel_temp[GPIO_PU_REG_MAX_NUM] = { GPIO_PU_ALL_VALUE };
    uint32_t reg_mask[GPIO_PU_REG_MAX_NUM] = { GPIO_PU_PD_ALL_MASK };

    ept_gpio_cfg0->GPIO_PU.CLR = reg_mask[0];
    ept_gpio_cfg0->GPIO_PU.SET = pullsel_temp[0];
    ept_gpio_cfg1->GPIO_PU[0].CLR = reg_mask[1];
    ept_gpio_cfg1->GPIO_PU[0].SET = pullsel_temp[1];
    ept_gpio_cfg1->GPIO_PU[1].CLR = reg_mask[2];
    ept_gpio_cfg1->GPIO_PU[1].SET = pullsel_temp[2];

#ifdef EPT_DEBUG_LOG
    log_hal_msgid_info("cfg0_PU0: reg_mask[0]=0x%.8x\r\n", 1, reg_mask[0]);
    // log_hal_msgid_info("cfg0_PU1: reg_mask[1]=0x%.8x\r\n", 1, reg_mask[1]);

    log_hal_msgid_info("cfg0_PU0: pullsel_temp[0]=0x%.8x\r\n", 1, pullsel_temp[0]);
    // log_hal_msgid_info("cfg0_PU1: pullsel_temp[1]=0x%.8x\r\n", 1, pullsel_temp[1]);
#endif

#endif
}



/**
  * @brief  write the joint data about GPIO pull configuration about pullen to GPIO pullen register one by one
  * @param  None
  * @retval None
  */
void gpio_pd_init(void)
{

#ifdef GPIO_PD_REG_MAX_NUM
    uint32_t pullsel_temp[GPIO_PD_REG_MAX_NUM] = { GPIO_PD_ALL_VALUE };
    uint32_t reg_mask[GPIO_PD_REG_MAX_NUM] = { GPIO_PU_PD_ALL_MASK };

    ept_gpio_cfg0->GPIO_PD.CLR = reg_mask[0];
    ept_gpio_cfg0->GPIO_PD.SET = pullsel_temp[0];
    ept_gpio_cfg1->GPIO_PD[0].CLR = reg_mask[1];
    ept_gpio_cfg1->GPIO_PD[0].SET = pullsel_temp[1];
    ept_gpio_cfg1->GPIO_PD[1].CLR = reg_mask[2];
    ept_gpio_cfg1->GPIO_PD[1].SET = pullsel_temp[2];


#ifdef EPT_DEBUG_LOG
    log_hal_msgid_info("cfg0_PD0: reg_mask[0]=0x%.8x\r\n", 1, reg_mask[0]);
    // log_hal_msgid_info("cfg0_PD1: reg_mask[1]=0x%.8x\r\n",  1, reg_mask[1]);

    log_hal_msgid_info("cfg0_PD0: pullsel_temp[0]=0x%.8x\r\n", 1, pullsel_temp[0]);
    // log_hal_msgid_info("cfg0_PD1: pullsel_temp[1]=0x%.8x\r\n",  1, pullsel_temp[1]);
#endif

#endif
}



/**
  * @brief  write the joint data about GPIO output data to GPIO output data register one by one
  * @param  None
  * @retval None
  */
void gpio_output_init(void)
{
#ifdef GPIO_OUTPUT_LEVEL_REG_MAX_NUM
    uint32_t i;
    uint32_t out_temp[GPIO_OUTPUT_LEVEL_REG_MAX_NUM] = { GPIO_OUTPUT_LEVEL_ALL_VALUE };

    for (i = 0; i < GPIO_OUTPUT_LEVEL_REG_MAX_NUM; i++) {
        ept_gpio_base->GPIO_DOUT.CLR[i] = GPIO_REG_PIN_MASK[i];
        ept_gpio_base->GPIO_DOUT.SET[i] = out_temp[i];
    }
#endif
}


/**
  * @brief  write the joint data about GPIO pull configuration about PUPD to GPIO pupd register one by one
  * @param  None
  * @retval None
  */
void gpio_pupd_init(void)
{
#ifdef GPIO_PUPD_REG_MAX_NUM
    uint32_t pupd_temp[GPIO_PUPD_REG_MAX_NUM] = { GPIO_PUPD_ALL_VALUE };
    uint32_t reg_mask[GPIO_PUPD_REG_MAX_NUM] = { GPIO_PUPD_ALL_MASK };

    ept_gpio_cfg1->GPIO_PUPD.CLR = reg_mask[0];
    ept_gpio_cfg1->GPIO_PUPD.SET = pupd_temp[0];


#ifdef EPT_DEBUG_LOG
    log_hal_msgid_info("cfg0_PUPD: reg_mask[0]=0x%.8x\r\n", 1, reg_mask[0]);
    log_hal_msgid_info("cfg0_PUPD: pullsel_temp[0]=0x%.8x\r\n", 1, pupd_temp[0]);
    log_hal_msgid_info("cfg0_PUPD_REG:0x%0x\r\n", 1, ept_gpio_cfg0->GPIO_PUPD.RW);
#endif


#endif
}


/**
  * @brief  write the joint data about GPIO pull configuration about R0 to GPIO R0 register one by one
  * @param  None
  * @retval None
  */
void gpio_r0_r1_init(void)
{
#ifdef GPIO_PUPD_REG_MAX_NUM
    uint32_t r0_temp[GPIO_R0_REG_MAX_NUM] = { GPIO_R0_ALL_VALUE };
    uint32_t r1_temp[GPIO_R1_REG_MAX_NUM] = { GPIO_R1_ALL_VALUE };
    uint32_t reg_mask[GPIO_R0_REG_MAX_NUM] = { GPIO_PUPD_ALL_MASK };

    ept_gpio_cfg1->GPIO_R0.CLR = reg_mask[0];
    ept_gpio_cfg1->GPIO_R1.CLR = reg_mask[0];
    ept_gpio_cfg1->GPIO_R0.SET = r0_temp[0];
    ept_gpio_cfg1->GPIO_R1.SET = r1_temp[0];

#ifdef EPT_DEBUG_LOG
    log_hal_msgid_info("cfg0_R0: reg_mask[0]=0x%.8x\r\n", 1, reg_mask[0]);
    log_hal_msgid_info("cfg0_R0: r0_temp[0]=0x%.8x\r\n", 1, r0_temp[0]);
    log_hal_msgid_info("cfg0_R1: r1_temp[0]=0x%.8x\r\n", 1, r1_temp[0]);
    log_hal_msgid_info("cfg0_R0_REG:0x%0x\r\n", 1, ept_gpio_cfg0->GPIO_R0.RW);
    log_hal_msgid_info("cfg0_R1_REG:0x%0x\r\n", 1, ept_gpio_cfg0->GPIO_R1.RW);

#endif


#endif
}

/**
  * @brief  write the joint data about GPIO pull configuration about rsel register one by one
  * @param  None
  * @retval None
  */
void gpio_rsel_init(void)
{
#ifdef GPIO_PUPD_REG_MAX_NUM
    uint32_t rsel_value[GPIO_RSEL_REG_MAX_NUM] = { GPIO_RSEL_ALL_VALUE };
    uint32_t rsel_mask[GPIO_RSEL_REG_MAX_NUM] = { GPIO_RSEL_ALL_MASK };

    ept_gpio_cfg1->GPIO_RSEL.CLR = rsel_mask[0];
    ept_gpio_cfg1->GPIO_RSEL.SET = rsel_value[0];

#ifdef EPT_DEBUG_LOG
    log_hal_msgid_info("rsel_value: rsel_value 0x%08x\r\n", 1, rsel_value[0]);
    log_hal_msgid_info("rsel_mask: rsel_mask 0x%08x\r\n", 1, rsel_mask[0]);

#endif


#endif
}

//switch adc pin to digital in case of leakage
void adc_switch_digital(void)
{
    ept_gpio_cfg0->GPIO_G.SET = 0xfff;
    ept_gpio_cfg1->GPIO_G[0].SET = 0x1fffffff;
}

void gpio_bond_pkg_type_init(void)
{
    #define BOND_TYPE_MAX 3
    #define BOND_PKG_TYPE0_MASK (1 << 7)
    #define BOND_PKG_TYPE1_MASK (1 << 8)
    #define BOND_PKG_TYPE2_MASK (1 << 9)
    #define PUPD_RW_REG  *(volatile uint32_t*)0x420F00F0
    #define PUPD_SET_REG *(volatile uint32_t*)0x420F00F4
    #define PUPD_CLR_REG *(volatile uint32_t*)0x420F00F8
    #define BOND_PU 1
    #define BOND_PD 0

    uint8_t bond_table[BOND_TYPE_MAX] = {
#if defined(AIR_BTA_IC_PREMIUM_G3_TYPE_P)
        BOND_PD, BOND_PU, BOND_PD
#elif defined(AIR_BTA_IC_PREMIUM_G3_TYPE_S)
        BOND_PU, BOND_PU, BOND_PU
#elif defined(AIR_BTA_IC_PREMIUM_G3_TYPE_A)
        BOND_PD, BOND_PD, BOND_PU
#elif defined(AIR_BTA_IC_PREMIUM_G3_TYPE_D)
        BOND_PD, BOND_PD, BOND_PD
#else
        BOND_PU, BOND_PU, BOND_PU
#endif
    };
    uint32_t bond_shift_table[BOND_TYPE_MAX] = {BOND_PKG_TYPE0_MASK, BOND_PKG_TYPE1_MASK, BOND_PKG_TYPE2_MASK};
    for (uint32_t i = 0; i < BOND_TYPE_MAX; i++) {
        if (bond_table[i] == BOND_PD) {
            PUPD_SET_REG = bond_shift_table[i];
        } else {
            PUPD_CLR_REG = bond_shift_table[i];
        }
    }
}

void bsp_ept_backup_varname(void)
{
    ept_backup_varname_for_log_t *ept_para;
    ept_para = (ept_backup_varname_for_log_t *)HW_SYSRAM_PRIVATE_MEMORY_EPT_VAR_BACKUP_START;

    ept_para->uart0_tx = HAL_UART0_TXD_PIN;
    ept_para->uart0_rx = HAL_UART0_RXD_PIN;
    ept_para->uart1_tx = HAL_UART1_TXD_PIN;
    ept_para->uart1_rx = HAL_UART1_RXD_PIN;
    ept_para->uart2_tx = HAL_UART2_TXD_PIN;
    ept_para->uart2_rx = HAL_UART2_RXD_PIN;
}

/**
  * @brief  Main program to make the configuration of EPT to take effect
  * @param  None
  * @retval None
  */
void bsp_ept_gpio_setting_init(void)
{
#ifdef EPT_DEBUG_LOG
    log_hal_msgid_info("call bsp_ept_gpio_setting_init\r\n", 0);
#endif

    gpio_output_init();
    gpio_dir_init();
    gpio_mode_init();
    gpio_pu_init();
    gpio_pd_init();
    gpio_rsel_init();
    gpio_pupd_init();
    gpio_r0_r1_init();
    adc_switch_digital();
    gpio_bond_pkg_type_init();
    bsp_ept_backup_varname();
}


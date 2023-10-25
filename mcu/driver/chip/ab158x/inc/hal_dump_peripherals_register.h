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

#ifndef __HAL_DUMP_PERIPHERALS_REGISTER_H__
#define __HAL_DUMP_PERIPHERALS_REGISTER_H__

#include "hal_define.h"
#include "hal_feature_config.h"
#include "air_chip.h"
#include "memory_map.h"
#include "hal_core_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Because the hardware may not have power, dumping these registers
 * may cause problems with the exception flow, so temporarily remove them.
 * {"i2s_dma", (unsigned int *)(I2S_DMA_BASE), (unsigned int *)(I2S_DMA_BASE + 0x110), 1}, \
 */

#define HAL_DUMP_MODULE_REGISTER_ENABLE
/*add your module here only dump RG that not read-update.
*  Example:  {"<module_name>", <dump_start_address>, <dump_stop_address>, 1},
*
*/
#define HAL_DUMP_PERIPHERAL_REGISTER  \
        {"uart0_0"             ,   (unsigned int *)(UART0_BASE            + 0x08),    (unsigned int *)(UART0_BASE               + 0x10 ), 1}, \
        {"uart0_1"             ,   (unsigned int *)(UART0_BASE            + 0x14),    (unsigned int *)(UART0_BASE               + 0x2C ), 1}, \
        {"uart0_2"             ,   (unsigned int *)(UART0_BASE            + 0x30),    (unsigned int *)(UART0_BASE               + 0x78 ), 1}, \
        {"uart1_0"             ,   (unsigned int *)(UART1_BASE            + 0x08),    (unsigned int *)(UART1_BASE               + 0x10 ), 1}, \
        {"uart1_1"             ,   (unsigned int *)(UART1_BASE            + 0x14),    (unsigned int *)(UART1_BASE               + 0x2C ), 1}, \
        {"uart1_2"             ,   (unsigned int *)(UART1_BASE            + 0x30),    (unsigned int *)(UART1_BASE               + 0x78 ), 1}, \
        {"uart2_0"             ,   (unsigned int *)(UART2_BASE            + 0x08),    (unsigned int *)(UART2_BASE               + 0x10 ), 1}, \
        {"uart2_1"             ,   (unsigned int *)(UART2_BASE            + 0x14),    (unsigned int *)(UART2_BASE               + 0x2C ), 1}, \
        {"uart2_2"             ,   (unsigned int *)(UART2_BASE            + 0x30),    (unsigned int *)(UART2_BASE               + 0x78 ), 1}, \
        {"gpio"                ,   (unsigned int *)(GPIO_BASE             + 0x00),    (unsigned int *)(GPIO_BASE                + 0x124), 1}, \
        {"i2c_master_AO"       ,   (unsigned int *)(I2C_AO_BASE           + 0x00),    (unsigned int *)(I2C_AO_BASE              + 0x90 ), 1}, \
        {"ccni_cm4"            ,   (unsigned int *)(CM4_CCNI_BASE         + 0x00),    (unsigned int *)(CM4_CCNI_BASE            + 0xC  ), 1}, \
        {"ccni_dsp0"           ,   (unsigned int *)(DSP0_CCNI_BASE        + 0x00),    (unsigned int *)(DSP0_CCNI_BASE           + 0xC  ), 1}, \
        {"hw_semaphore_reg"    ,   (unsigned int *)(SEMAPHORE_BASE              ),    (unsigned int *)(SEMAPHORE_BASE           + 0x218), 1}, \
        {"mcu_dma0"            ,   (unsigned int *)(DMA_0_BASE                  ),    (unsigned int *)(DMA_0_BASE               + 0x12C), 1}, \
        {"mcu_dma1"            ,   (unsigned int *)(DMA_1_BASE                  ),    (unsigned int *)(DMA_1_BASE               + 0x22C), 1}, \
        {"uart0_dma"           ,   (unsigned int *)(UART_DMA_0_BASE             ),    (unsigned int *)(UART_DMA_0_BASE          + 0x270), 1}, \
        {"uart1_dma"           ,   (unsigned int *)(UART_DMA_1_BASE             ),    (unsigned int *)(UART_DMA_1_BASE          + 0x270), 1}, \
        {"uart2_dma"           ,   (unsigned int *)(UART_DMA_2_BASE             ),    (unsigned int *)(UART_DMA_2_BASE          + 0x270), 1}, \
        {"i2c0_dma"            ,   (unsigned int *)(I2C0_PDMA_BASE              ),    (unsigned int *)(I2C0_PDMA_BASE           + 0x230), 1}, \
        {"i2c1_dma"            ,   (unsigned int *)(I2C1_PDMA_BASE              ),    (unsigned int *)(I2C1_PDMA_BASE           + 0x230), 1}, \
        {"i2c2_dma"            ,   (unsigned int *)(I2C2_PDMA_BASE              ),    (unsigned int *)(I2C2_PDMA_BASE           + 0x230), 1}, \
        {"i3c0_dma"            ,   (unsigned int *)(I3C0_PDMA_BASE              ),    (unsigned int *)(I3C0_PDMA_BASE           + 0x230), 1}, \
        {"i3c1_dma"            ,   (unsigned int *)(I3C1_PDMA_BASE              ),    (unsigned int *)(I3C1_PDMA_BASE           + 0x230), 1}, \
        {"bt"                  ,   (unsigned int *)(BT_BASE                     ),    (unsigned int *)(BT_BASE                  + 0x96F), 1}, \
        {"bt_modem"            ,   (unsigned int *)(BT_MODEM_BASE               ),    (unsigned int *)(BT_MODEM_BASE            + 0xCE8), 1}, \
        {"cksys_230"           ,   (unsigned int *)(0x42030230                  ),    (unsigned int *)(0x42030230               + 0x18 ), 1}, \
        {"cksys_280"           ,   (unsigned int *)(0x42030280                  ),    (unsigned int *)(0x42030280               + 0x14 ), 1}, \
        {"cksys_304"           ,   (unsigned int *)(0x42030304                  ),    (unsigned int *)(0x42030304               + 0x4  ), 1}, \
        {"cksys_xo_clk_B00"    ,   (unsigned int *)(0x42040B00                  ),    (unsigned int *)(0x42040B00               + 0x4  ), 1}, \
        {"cksys_xo_clk_B30"    ,   (unsigned int *)(0x42040B30                  ),    (unsigned int *)(0x42040B30               + 0x4  ), 1}, \
        {"cksys_xo_clk_B60"    ,   (unsigned int *)(0x42040B60                  ),    (unsigned int *)(0x42040B60               + 0x4  ), 1}, \
        {"cksys_xo_clk_B90"    ,   (unsigned int *)(0x42040B90                  ),    (unsigned int *)(0x42040B90               + 0x4  ), 1}, \
        {"mixedsys_d_100"      ,   (unsigned int *)(0x42050100                  ),    (unsigned int *)(0x42050100               + 0x4  ), 1}, \
        {"mixedsys_d_200"      ,   (unsigned int *)(0x42050200                  ),    (unsigned int *)(0x42050200               + 0x8  ), 1}, \
        {"mixedsys_d_250"      ,   (unsigned int *)(0x42050250                  ),    (unsigned int *)(0x42050250               + 0x8  ), 1}, \
        {"xpll_ctrl_004"       ,   (unsigned int *)(0x42060004                  ),    (unsigned int *)(0x42060004               + 0x4  ), 1}, \
        {"xpll_ctrl_104"       ,   (unsigned int *)(0x42060104                  ),    (unsigned int *)(0x42060104               + 0x4  ), 1}, \
        {"dcxo_pwr_ctrl_014"   ,   (unsigned int *)(0x42070014                  ),    (unsigned int *)(0x42070014               + 0x8  ), 1}, \
        {"cksys_bus_clk_300"   ,   (unsigned int *)(0x422B0300                  ),    (unsigned int *)(0x422B0300               + 0x4  ), 1}, \
        {"dcxo_cfg_00C"        ,   (unsigned int *)(DCXO_CFG_BASE        + 0x0C ),    (unsigned int *)(DCXO_CFG_BASE            + 0x10 ), 1}, \
        {"dcxo_cfg_028"        ,   (unsigned int *)(DCXO_CFG_BASE        + 0x28 ),    (unsigned int *)(DCXO_CFG_BASE            + 0x30 ), 1}, \

        /* warning warning warning!!!!!
            please note below module could possibly lead to exception dump failed!!!
            if you need dump these register,please make sure exception dump works well.

        {"I2S_DMA",            (unsigned int *)(0xC9000000),               (unsigned int *)(0xC9000000 + 0x1070),1 },   \
        {"bt",                 (unsigned int *)(BT_BASE),                  (unsigned int *)(BT_BASE + 0x96F), 1},       \
        {"bt_modem",           (unsigned int *)(BT_MODEM_BASE),            (unsigned int *)(BT_MODEM_BASE + 0xCE8), 1}, \
        {"bt_sleep",           (unsigned int *)(BT_SLEEP_BASE),            (unsigned int *)(BT_SLEEP_BASE + 0x20C), 1}, \
        */

/*start : bit operations*/
#define GET_BYTE_0(x)           (((uint32_t)x >>  0)  &  0x000000ff)            /* Get byte 0 of a word*/
#define GET_BYTE_1(x)           (((uint32_t)x >>  8)  &  0x000000ff)            /* Get byte 1 of a word*/
#define GET_BYTE_2(x)           (((uint32_t)x >> 16)  &  0x000000ff)            /* Get byte 2 of a word*/
#define GET_BYTE_3(x)           (((uint32_t)x >> 24)  &  0x000000ff)            /* Get byte 3 of a word*/

#define SET_BYTE_0_ff(x)        ((uint32_t)x |=  0x000000ff)                    /* Set byte 0 of a word to 0xff*/
#define SET_BYTE_1_ff(x)        ((uint32_t)x |=  0x0000ff00)                    /* Set byte 1 of a word to 0xff*/
#define SET_BYTE_2_ff(x)        ((uint32_t)x |=  0x00ff0000)                    /* Set byte 2 of a word to 0xff*/
#define SET_BYTE_3_ff(x)        ((uint32_t)x |=  0xff000000)                    /* Set byte 3 of a word to 0xff*/

#define SET_BYTE_0_ch(x,ch)     ((uint32_t)x |=  ((uint8_t)ch <<  0))           /* Set byte 0 of a word to a char*/
#define SET_BYTE_1_ch(x,ch)     ((uint32_t)x |=  ((uint8_t)ch <<  8))           /* Set byte 1 of a word to a char*/
#define SET_BYTE_2_ch(x,ch)     ((uint32_t)x |=  ((uint8_t)ch << 16))           /* Set byte 2 of a word to a char*/
#define SET_BYTE_3_ch(x,ch)     ((uint32_t)x |=  ((uint8_t)ch << 24))           /* Set byte 3 of a word to a char*/

#define CLEAR_BYTE_0(x)         ((uint32_t)x & 0xffffff00)                      /* Clear byte 0 of a word*/
#define CLEAR_BYTE_1(x)         ((uint32_t)x & 0xffff00ff)                      /* Clear byte 1 of a word*/
#define CLEAR_BYTE_2(x)         ((uint32_t)x & 0xff00ffff)                      /* Clear byte 2 of a word*/
#define CLEAR_BYTE_3(x)         ((uint32_t)x & 0x00ffffff)                      /* Clear byte 3 of a word*/


#define GET_BIT(x, bit_n)       (((uint32_t)x &   (1U << bit_n)) >> bit_n)      /* Get   the bit_n status of x*/
#define SET_BIT(x, bit_n)       ( (uint32_t)x |=  (1U << bit))                  /* Set   the bit_n to 1   of x*/
#define CLEAR_BIT(x, bit_n)     ( (uint32_t)x &= ~(1U << bit_n))                /* Clear the bit_n to 0   of x*/

#define GET_BIT_N_TO_M(x,m,n)   ( ((uint32_t)x << (31-(m))) >> ((31-(m))+(n)))  /* Get   the bit[m:n] status of x*/
#define SET_BIT_N_TO_M(x,m,n)   (  (uint32_t)x |   ((~((~0U)<<(n-m+1)))<<(m)))      /* Set   the bit[m:n] to 1   of x*/
#define CLEAR_BIT_N_TO_M(x,m,n) (  (uint32_t)x &  ~((~((~0U)<<(n-m+1)))<<(m)))      /* Clear the bit[m:n] to 0   of x*/
/*end: bit operations*/

#ifdef __cplusplus
}
#endif

#endif /* __HAL_PLATFORM_H__ */


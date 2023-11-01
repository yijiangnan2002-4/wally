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

#ifndef __AIR_CHIP_H__
#define __AIR_CHIP_H__


#if defined(CORE_CM33)
#define __CM33_REV                0x0000U   /* Core revision r0p1 */
#define __SAUREGION_PRESENT       1U        /* SAU regions present */
#define __MPU_PRESENT             1U        /* MPU present */
#define __NVIC_PRIO_BITS          4U        /* Number of Bits used for Priority Levels */
//#define __Vendor_SysTickConfig    0       /* Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT             1U        /* FPU present */
#define __DSP_PRESENT             1U        /* DSP extension present */
#define CM4_HIF_IRQ               22

typedef enum IRQn {
    /****  CM4 internal exceptions  **********/
    Reset_IRQn                  = -15,  /* Reset */
    NonMaskableInt_IRQn         = -14,  /* NMI */
    HardFault_IRQn              = -13,  /* HarFault */
    MemoryManagement_IRQn       = -12,  /* Memory Management */
    BusFault_IRQn               = -11,  /* Bus Fault  */
    UsageFault_IRQn             = -10,  /* Usage Fault */
    SecureFault_IRQn            = -9,   /* Secure Fault */
    SVC_IRQn                    = -5,   /* SV Call*/
    DebugMonitor_IRQn           = -4,   /* Debug Monitor */
    PendSV_IRQn                 = -2,   /* Pend SV */
    SysTick_IRQn                = -1,   /* System Tick */

    /****  Specific external/peripheral interrupt ****/
    OS_GPT_IRQn         = 0,
    MCU_DMA0_IRQn       = 1,
    MCU_DMA1_IRQn       = 2,
    UART_DMA0_IRQn      = 3,
    UART_DMA1_IRQn      = 4,
    UART_DMA2_IRQn      = 5,
    I2C_DMA0_IRQn       = 6,
    I2C_DMA1_IRQn       = 7,
    I2C_DMA2_IRQn       = 8,
    I3C_DMA0_IRQn       = 9,
    I3C_DMA1_IRQn       = 10,
    SPI_MST0_IRQn       = 11,
    SPI_MST1_IRQn       = 12,
    SPI_MST2_IRQn       = 13,
    SPI_SLV_IRQn        = 14,
    SDIO_MST0_IRQn      = 15,
    UART0_IRQn          = 16,
    UART1_IRQn          = 17,
    UART2_IRQn          = 18,
    CRYPTO_IRQn         = 19,
    TRNG_IRQn           = 20,
    I2S_SLAVE_IRQn      = 21,
    I2C0_IRQn           = 22,
    I2C1_IRQn           = 23,
    I2C2_IRQn           = 24,
    I2C_AO_IRQn         = 25,
    I3C0_IRQn           = 26,
    I3C1_IRQn           = 27,
    RTC_IRQn            = 28,
    GPT_IRQn            = 29,
    GPT_SEC_IRQn        = 30,
    SPM_IRQn            = 31,
    WDT_IRQn            = 32,
    EINT_SEC_IRQn       = 33,
    EINT_IRQn           = 34,
    SFC_IRQn            = 35,
    ESC_IRQn            = 36,
    USB_IRQn            = 37,
    DSP0_IRQn           = 38,
    CAP_TOUCH_IRQn      = 39,
    AUDIOSYS0_IRQn      = 40,
    AUDIOSYS1_IRQn      = 41,
    AUDIOSYS2_IRQn      = 42,
    AUDIOSYS3_IRQn      = 43,
    ANC_IRQn            = 44,
    ANC_RAMP_IRQn       = 45,
    ANC_DMA_IRQn        = 46,
    VAD_IRQn            = 47,
    BT_IRQn             = 48,
    BT_AURX_IRQn        = 49,
    BT_AUTX_IRQn        = 50,
    BT_TIMER_IRQn       = 51,
    BT_PLAY_EN_IRQn     = 52,
    VOW_SNR_IRQn        = 53,
    VOW_FIFO_IRQn       = 54,
    SEC_VIOLATION_IRQn  = 55,
    MEM_ILLEGAL_IRQn    = 56,
    BUS_ERR_IRQn        = 57,
    MBX_TX0_IRQn        = 58,
    MBX_TX1_IRQn        = 59,
    MBX_TX2_IRQn        = 60,
    MBX_TX3_IRQn        = 61,
    MBX_TX4_IRQn        = 62,
    MBX_TX5_IRQn        = 63,
    MBX_RX0_IRQn        = 64,
    MBX_RX1_IRQn        = 65,
    MBX_RX2_IRQn        = 66,
    MBX_RX3_IRQn        = 67,
    MBX_RX4_IRQn        = 68,
    MBX_RX5_IRQn        = 69,
    PMU_IRQn            = 70,
    IRRX_IRQn           = 71,
    DSP_ERR_IRQn        = 72,
    SW_IRQn             = 73,
    CM33_reserved0_IRQn = 74,
    IRQ_NUMBER_MAX      = 75
} IRQn_Type;

typedef IRQn_Type hal_nvic_irq_t;
#define NVIC_PRIORITYGROUP_0         0x7 /*!< 0 bits for pre-emption priority   8 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         0x6 /*!< 1 bits for pre-emption priority   7 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         0x5 /*!< 2 bits for pre-emption priority   6 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         0x4 /*!< 3 bits for pre-emption priority   5 bits for subpriority */
#define NVIC_PRIORITYGROUP_4         0x3 /*!< 4 bits for pre-emption priority   4 bits for subpriority */
#define NVIC_PRIORITYGROUP_5         0x2 /*!< 5 bits for pre-emption priority   3 bits for subpriority */
#define NVIC_PRIORITYGROUP_6         0x1 /*!< 6 bits for pre-emption priority   2 bits for subpriority */
#define NVIC_PRIORITYGROUP_7         0x0 /*!< 7 bits for pre-emption priority   1 bits for subpriority */

#include "core_cm33.h"                  /* Core Peripheral Access Layer */
#include "system_cmsis.h"

#elif defined(CORE_DSP0)

extern char build_date_time_str[];

#ifdef __cplusplus
#define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
#define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */

typedef enum IRQn {
    /****  DSP internal exceptions  **********/
                                /*priority -> level 4 highest*/
    WDT_IRQn            = 0,    /*level 4*/      /*WDT and NMI*/
    Write_Error_IRQn    = 1,    /*level 4*/

    /****  DPS specific external/peripheral interrupt ****/
    SPM_IRQn            = 2,    /*level 4*/
    OS_GPT_IRQn         = 3,    /*level 4*/
    BT_IRQn             = 4,    /*level 4*/
    BT_PLAY_EN_IRQn     = 5,    /*level 4*/
    ANC_IRQn            = 6,    /*level 3*/
    ANC_RAMP_IRQn       = 7,    /*level 3*/
    ANC_DMA_IRQn        = 8,    /*level 3*/
    CM33_IRQn           = 9,    /*level 3*/
    MCU_DMA1_IRQn       = 10,   /*level 2*/
    UART_DMA0_IRQn      = 11,   /*level 2*/
    I2C_DMA1_IRQn       = 12,   /*level 2*/
    I3C_DMA0_IRQn       = 13,   /*level 2*/
    EINT_IRQn           = 14,   /*level 2*/
    GPT_IRQn            = 15,   /*level 2*/
    AUDIOSYS0_IRQn      = 16,   /*level 2*/
    AUDIOSYS1_IRQn      = 17,   /*level 2*/
    AUDIOSYS2_IRQn      = 18,   /*level 2*/
    AUDIOSYS3_IRQn      = 19,   /*level 2*/
    VOW_SNR_IRQn        = 20,   /*level 2*/
    VOW_FIFO_IRQn       = 21,   /*level 2*/
    VAD_IRQn            = 22,   /*level 2*/
    BT_AURX_IRQn        = 23,   /*level 2*/
    BT_AUTX_IRQn        = 24,   /*level 2*/
    I2S_SLAVE_IRQn      = 25,   /*level 1*/
    SPI_MST0_IRQn       = 26,   /*level 1*/
    UART0_IRQn          = 27,   /*level 1*/
    I2C1_IRQn           = 28,   /*level 1*/
    I3CO_IRQn           = 29,   /*level 1*/
    CRYPTO_IRQn         = 30,   /*level 1*/
    RGU_IRQn            = 31,   /*level 1*/
   /*level 1*/
    IRQ_NUMBER_MAX      } IRQn_Type;

typedef IRQn_Type hal_nvic_irq_t;
#else
#error "must be have a Core define!!!"
#endif

/* ================================================== */

#ifdef AIR_CPU_IN_SECURITY_MODE

/* PD APB Peripheral */
#define AESOTF_BASE          0x40010000 /*AES_OnTheFly*/
#define AESOTF_ESC_BASE      0x43010000 /*AES_OnTheFly_ESC*/
#define TRNG_BASE            0x43030000 /*TRNG*/
#define DMA_0_BASE           0x40020000 /*DMA Controller*/
#define DMA_1_BASE           0x40030000 /*DMA Controller*/
#define UART_DMA_0_BASE      0x40080000 /*UART_0 DMA Controller*/
#define UART_DMA_1_BASE      0x40090000 /*UART_1 DMA Controller*/
#define UART_DMA_2_BASE      0x400A0000 /*UART_2 DMA Controller*/
#define I2S_DMA_BASE         0xC9000000 /*I2S DMA Controller*/
//#define INFRA_MBIST_BASE     0xA0030000 /*INFRA MBIST CONFIG*/
#define SFC_BASE               0x40040000 /*Serial Flash Contoller*/
#define EMI_BASE               0xA3040000 /*External Memory Interface*/
#define ESC_BASE               0x43040000 /*ESC Memory Interface*/
#define CRYPTO_BASE            0x43020000 /*Crypto_Engine*/
#define SPI_MASTER_0_BASE      0x43050000 /*SPI MASTER 0*/
#define SPI_MASTER_1_BASE      0x43060000 /*SPI MASTER 1*/
#define SPI_MASTER_2_BASE      0x43070000 /*SPI MASTER 2*/
#define SPI_SLAVE_BASE         0x43080000 /*SPI_SLAVE*/
#define SPI_SLAVE_PAD_BASE     0x42100000 /*SPI_SLAVE_PAD*/
#define SPI_SLAVE_BYPASS_BASE  0xA2010040 /*SPI_SLAVE_BYPASS*/
#define UART0_BASE           0x40050000 /*UART 0*/
#define UART1_BASE           0x40060000 /*UART 1*/
#define UART2_BASE           0x40070000 /*UART 2*/
#define I2C_0_BASE           0x400C0000 /*I2C_0*/
#define I2C_1_BASE           0x430D0000 /*I2C_1*/
#define I2C_2_BASE           0x430E0000 /*I2C_2*/
#define I3C_0_BASE           0x430F0000 /*I3C_0*/
#define I3C_1_BASE           0x43100000 /*I3C_1*/
#define AUXADC_BASE          0x43090000 /*Auxiliary ADC Unit*/
#define ABB_BASE             0x42090000
#define I2C0_PDMA_BASE       0x400D0000 /* I2C0 PDMA*/
#define I2C1_PDMA_BASE       0x43110000 /* I2C1 PDMA*/
#define I2C2_PDMA_BASE       0x43120000 /* I2C2 PDMA*/
#define I3C0_PDMA_BASE       0x43130000 /* I3C0 PDMA */
#define I3C1_PDMA_BASE       0x43140000 /* I3C1 PDMA */

/* PD AHB Peripheral */
//#define DMA_AHB_BASE         0xA1000000 /*PD DMA AHB*/
//#define AUDIO_AHB_BASE       0xA1010000 /*PD ADUIO AHB*/
//#define ASYS_AHB_BASE        0xA1020000 /*PD ASYS AHB*/
#define SDIO_MASTER_BASE    0x41010000 /*SDIO MASTER*/

/* PD CMSYS AHB Peripheral */
#define CMSYS_CFG_BASE       0xE0100000 /*cmsys_cfg*/
#define CMSYS_CFG_EXT_BASE   0xE00FE000 /*cmsys_cfg_ext*/
#define CMSYS_MBIST_BASE     0xE0110000 /*cmsys_mbist*/
#define CM4_MCU_BASE         0xE0130000 /*Reserved for MDM*/
#define CMSYS_L1CACHE_BASE   0xE0180000 /*cmsys_l1cache*/

/* AO APB Peripheral */
#define CONFIG_BASE          0xA2010000 /*Configuration Registers(Clock, Power Down, Version and Reset)*/
#define TOP_MISC_CFG_BASE    0xA2010000 /*Configuration Registers(Clock, Power Down, Version and Reset)*/
#define EMI_AO_BASE          0xA2010400
//#define CKSYS_BASE           0xA2020000 /*BBPLL control*/
//#define CKSYS_XO_CLK_BASE    0xA2030000 /*XPLL control*/
//#define BBPLL_CTRL_BASE      0xA2040000 /*26M, clk switch control (clk divider related setting)*/
//#define XPLL_CTRL_BASE       0xA2050000 /*26M, top clk control (dcm, cg)*/
//#define XTAL_CTRL_BASE       0xA2060000 /*26M, RF Xtal control register*/
//#define PMU_BASE             0xA2070000 /*PMU configure*/
#define RTC_BASE             0x420A0000 /*Real Time Clock*/
#define WDT_BASE             0x420B0000 /*Reset Generation Unit*/
#define EFUSE_BASE           0x420C0000 /*EFUSE*/
#define GPIO_BASE            0x420D0000 /*General Purpose Inputs/Outputs*/
#define IO_CFG_0_BASE        0x420E0000 /*io_cfg_0*/
#define IO_CFG_1_BASE        0x420F0000 /*io_cfg_1*/
//#define SPM_BASE             0xA20F0000 /*System Power Manager */
#define CAPTOUCH_HIFBASE     0x42120000 /*captouch hif*/
#define CAPTOUCH_BASE        0x42121000 /*captouch*/
#define EINT_BASE            0x42150300 /*External Interrupts*/
//#define GPT_BASE             0x42170000 /*General Purpose Timer */
#define GPT_BASE             0x42160000 /*General Purpose Timer */
// #define GPT_BASE_1             0x52170000 /*General Purpose Timer_1 */
#define OS_GPT_BASE          0x42180000 /*OS General Purpose Timer */

#define PWM0_BASE            0x42190000 /*Pulse-Width Modulation Outputs 0*/
#define PWM1_BASE            0x421A0000 /*Pulse-Width Modulation Outputs 1*/
#define PWM2_BASE            0x421B0000 /*Pulse-Width Modulation Outputs 2*/
#define PWM3_BASE            0x421C0000 /*Pulse-Width Modulation Outputs 3*/
#define PWM4_BASE            0x421D0000 /*Pulse-Width Modulation Outputs 4*/
#define PWM5_BASE            0x421E0000 /*Pulse-Width Modulation Outputs 5*/
#define PWM6_BASE            0x421F0000 /*Pulse-Width Modulation Outputs 6*/
#define PWM7_BASE            0x42200000 /*Pulse-Width Modulation Outputs 7*/
#define PWM8_BASE            0x42210000 /*Pulse-Width Modulation Outputs 8*/
#define PWM9_BASE            0x42220000 /*Pulse-Width Modulation Outputs 9*/
#define PWM10_BASE           0x42230000 /*Pulse-Width Modulation Outputs 10*/
#define PWM11_BASE           0x42240000 /*Pulse-Width Modulation Outputs 11*/
#define PWM12_BASE           0x42250000 /*Pulse-Width Modulation Outputs 12*/
#define PWM13_BASE           0x42260000 /*Pulse-Width Modulation Outputs 13*/

#define DCXO_CFG_BASE        0x430A0000 /* DCXO CFG */
#ifdef AIR_LIMIT_TZ_ENABLE /* TODO: workaround, should be 0x422C0000 */
#define MCU_CFG_PRI_BASE     0x522C0000 /*CM33 always-on CFG for SPM */
#else
#define MCU_CFG_PRI_BASE     0x422C0000 /*CM33 all in security */
#endif

//#define INFRA_CFG_BASE       0x522D0000 /*bus configuration registers*/
#define I2C_AO_BASE          0x42270000 /*I2C_AO*/
#define SEMAPHORE_BASE       0x42280000 /* HareWare Semaphore */

#define CM4_CCNI_BASE      (0x42290000)
#define DSP0_CCNI_BASE     (0x422A0000)

#define USB_BASE             0x41020000 /*USB*/
#define USB_SIFSLV_BASE      0x41030000 /*USB PHY, SIFSLV*/
#define USB_PHY_MSTAR        0x41030000 /*USB PHY*/

#define BT_BASE            0xA0010000 /* BT */
#define BT_MODEM_BASE      0xA1000000 /* BT Modem */
#define BT_SLEEP_BASE      0xA0000000 /* BT Sleep */

#define IRRX_BASE          0x430B0000 /* IRRX*/

#else /* AIR_CPU_IN_SECURITY_MODE */

/* PD APB Peripheral */
#define AESOTF_BASE          0x50010000 /*AES_OnTheFly*/
#define AESOTF_ESC_BASE      0x53010000 /*AES_OnTheFly_ESC*/
#define TRNG_BASE            0x53030000 /*TRNG*/
#define DMA_0_BASE           0x50020000 /*DMA Controller*/
#define DMA_1_BASE           0x50030000 /*DMA Controller*/
#define UART_DMA_0_BASE      0x50080000 /*UART_0 DMA Controller*/
#define UART_DMA_1_BASE      0x50090000 /*UART_1 DMA Controller*/
#define UART_DMA_2_BASE      0x500A0000 /*UART_2 DMA Controller*/
#define I2S_DMA_BASE         0xD9000000 /*I2S DMA Controller*/
//#define INFRA_MBIST_BASE     0xA0030000 /*INFRA MBIST CONFIG*/
#define SFC_BASE               0x50040000 /*Serial Flash Contoller*/
#define EMI_BASE               0xA3040000 /*External Memory Interface*/
#define ESC_BASE               0x53040000 /*ESC Memory Interface*/
#define CRYPTO_BASE            0x53020000 /*Crypto_Engine*/
#define SPI_MASTER_0_BASE      0x53050000 /*SPI MASTER 0*/
#define SPI_MASTER_1_BASE      0x53060000 /*SPI MASTER 1*/
#define SPI_MASTER_2_BASE      0x53070000 /*SPI MASTER 2*/
#define SPI_SLAVE_BASE         0x53080000 /*SPI_SLAVE*/
#define SPI_SLAVE_PAD_BASE     0x52100000 /*SPI_SLAVE_PAD*/
#define SPI_SLAVE_BYPASS_BASE  0xA2010040 /*SPI_SLAVE_BYPASS*/
#define UART0_BASE           0x50050000 /*UART 0*/
#define UART1_BASE           0x50060000 /*UART 1*/
#define UART2_BASE           0x50070000 /*UART 2*/
#define I2C_0_BASE           0x500C0000 /*I2C_0*/
#define I2C_1_BASE           0x530D0000 /*I2C_1*/
#define I2C_2_BASE           0x530E0000 /*I2C_1*/
#define I3C_0_BASE           0x530F0000 /*I3C_0*/
#define I3C_1_BASE           0x53100000 /*I3C_1*/
#define AUXADC_BASE          0x53090000 /*Auxiliary ADC Unit*/
#define I2C0_PDMA_BASE       0x500D0000 /* I2C0 PDMA*/
#define I2C1_PDMA_BASE       0x53110000 /* I2C1 PDMA*/
#define I2C2_PDMA_BASE       0x53120000 /* I2C2 PDMA*/
#define I3C0_PDMA_BASE       0x43130000 /* I3C0 PDMA */
#define I3C1_PDMA_BASE       0x43140000 /* I3C1 PDMA */

/* PD AHB Peripheral */
//#define DMA_AHB_BASE         0xA1000000 /*PD DMA AHB*/
//#define AUDIO_AHB_BASE       0xA1010000 /*PD ADUIO AHB*/
//#define ASYS_AHB_BASE        0xA1020000 /*PD ASYS AHB*/
#define SDIO_MASTER_BASE    0x51010000 /*SDIO MASTER*/

/* PD CMSYS AHB Peripheral */
#define CMSYS_CFG_BASE       0xE0100000 /*cmsys_cfg*/
#define CMSYS_CFG_EXT_BASE   0xE00FE000 /*cmsys_cfg_ext*/
#define CMSYS_MBIST_BASE     0xE0110000 /*cmsys_mbist*/
#define CM4_MCU_BASE         0xE0130000 /*Reserved for MDM*/
#define CMSYS_L1CACHE_BASE   0xE0180000 /*cmsys_l1cache*/

/* AO APB Peripheral */
#define CONFIG_BASE          0xA2010000 /*Configuration Registers(Clock, Power Down, Version and Reset)*/
#define TOP_MISC_CFG_BASE    0xA2010000 /*Configuration Registers(Clock, Power Down, Version and Reset)*/
#define EMI_AO_BASE          0xA2010400
//#define CKSYS_BASE           0xA2020000 /*BBPLL control*/
//#define CKSYS_XO_CLK_BASE    0xA2030000 /*XPLL control*/
//#define BBPLL_CTRL_BASE      0xA2040000 /*26M, clk switch control (clk divider related setting)*/
//#define XPLL_CTRL_BASE       0xA2050000 /*26M, top clk control (dcm, cg)*/
//#define XTAL_CTRL_BASE       0xA2060000 /*26M, RF Xtal control register*/
//#define PMU_BASE             0xA2070000 /*PMU configure*/
#define RTC_BASE             0x520A0000 /*Real Time Clock*/
#define WDT_BASE             0x520B0000 /*Reset Generation Unit*/
#define EFUSE_BASE           0x520C0000 /*EFUSE*/
#define GPIO_BASE            0x520D0000 /*General Purpose Inputs/Outputs*/
#define IO_CFG_0_BASE        0x520E0000 /*io_cfg_0*/
#define IO_CFG_1_BASE        0x520F0000 /*io_cfg_1*/
//#define SPM_BASE             0xA20F0000 /*System Power Manager */
#define CAPTOUCH_HIFBASE     0x42120000 /*captouch hif*/
#define CAPTOUCH_BASE        0x42121000 /*captouch*/
#define EINT_BASE            0x52150300 /*External Interrupts*/
#define GPT_BASE             0x52160000 /*General Purpose Timer */
// #define GPT_BASE_1             0x52170000 /*General Purpose Timer_1 */
#define OS_GPT_BASE          0x52180000 /*OS General Purpose Timer */

#define PWM0_BASE            0x52190000 /*Pulse-Width Modulation Outputs 0*/
#define PWM1_BASE            0x521A0000 /*Pulse-Width Modulation Outputs 1*/
#define PWM2_BASE            0x521B0000 /*Pulse-Width Modulation Outputs 2*/
#define PWM3_BASE            0x521C0000 /*Pulse-Width Modulation Outputs 3*/
#define PWM4_BASE            0x521D0000 /*Pulse-Width Modulation Outputs 4*/
#define PWM5_BASE            0x521E0000 /*Pulse-Width Modulation Outputs 5*/
#define PWM6_BASE            0x521F0000 /*Pulse-Width Modulation Outputs 6*/
#define PWM7_BASE            0x52200000 /*Pulse-Width Modulation Outputs 7*/
#define PWM8_BASE            0x52210000 /*Pulse-Width Modulation Outputs 8*/
#define PWM9_BASE            0x52220000 /*Pulse-Width Modulation Outputs 9*/
#define PWM10_BASE           0x52230000 /*Pulse-Width Modulation Outputs 10*/
#define PWM11_BASE           0x52240000 /*Pulse-Width Modulation Outputs 11*/
#define PWM12_BASE           0x52250000 /*Pulse-Width Modulation Outputs 12*/
#define PWM13_BASE           0x52260000 /*Pulse-Width Modulation Outputs 13*/

#define DCXO_CFG_BASE        0x530A0000 /* DCXO_CFG */
//#define CKSYS_BUS_CLK_BASE   0xA21D0000 /*96M, top clk control (dcm, cg)*/
#define MCU_CFG_PRI_BASE     0x522C0000 /*CM33 always-on CFG*/
//#define INFRA_CFG_BASE       0x522D0000 /*bus configuration registers*/
#define I2C_AO_BASE          0x52270000 /*I2C_AO*/
#define SEMAPHORE_BASE       0x52280000 /* HareWare Semaphore */

#define CM4_CCNI_BASE      (0x52290000)
#define DSP0_CCNI_BASE     (0x522A0000)

#define USB_BASE             0x51020000 /*USB*/
#define USB_SIFSLV_BASE      0x51030000 /*USB PHY, SIFSLV*/
#define USB_PHY_MSTAR        0x51030000 /*USB PHY*/

#define BT_BASE            0xB0010000 /* BT */
#define BT_MODEM_BASE      0xB1000000 /* BT Modem */
#define BT_SLEEP_BASE      0xB0000000 /* BT Sleep */

#define IRRX_BASE          0x530B0000 /* IRRX*/

#endif /* AIR_CPU_IN_SECURITY_MODE */

#define I2C_AO_CLOCK_FREQUENCY  26000000  /* 26Mhz */
#define I2C_CLOCK_FREQUENCY     104000000 /* 104Mhz */
#define I3C_CLOCK_FREQUENCY     104000000 /* 104Mhz */

/* structure type of CCNI
 */
typedef struct {
    __IO uint32_t IRQ0_MASK;           /*CM4_CCNI:   CM4 to DSP0
                                        DSP0_CCNI:  DSP0 to CM4
                                        DSP1_CCNI   DSP1 to CM4
                                        N9_CCNI       N9 to CM4
                                        */
    __IO uint32_t IRQ0_SET;
    __IO uint32_t IRQ0_CLR;
    __IO uint32_t IRQ0_STATUS;
    __IO uint32_t IRQ1_MASK;           /* CM4_CCNI:   CM4 to DSP1
                                        DSP0_CCNI:  DSP0 to DSP1
                                        DSP1_CCNI   DSP1 to DSP0
                                        N9_CCNI       N9 to DSP0
                                        */
    __IO uint32_t IRQ1_SET;
    __IO uint32_t IRQ1_CLR;
    __IO uint32_t IRQ1_STATUS;
    __IO uint32_t IRQ2_MASK;           /* CM4_CCNI:   CM4 to N9
                                        DSP0_CCNI:  DSP0 to N9
                                        DSP1_CCNI   DSP1 to N9
                                        N9_CCNI       N9 to DSP1
                                        */
    __IO uint32_t IRQ2_SET;
    __IO uint32_t IRQ2_CLR;
    __IO uint32_t IRQ2_STATUS;
} CCNI_REGISTER_T;

#define CM4_CCNI      ((CCNI_REGISTER_T *) (CM4_CCNI_BASE))
#define DSP0_CCNI     ((CCNI_REGISTER_T *) (DSP0_CCNI_BASE))

/* structure type of top_misc_cfg
 */
typedef struct {
    __IO uint32_t GPIO_CLKO_CTRL_A;   /* CLK0~3 out mode */
    __IO uint32_t GPIO_CLKO_CTRL_B;   /* CLK4   out mode */

} TOP_MISC_CFG_T;
#define TOP_MISC_CFG    ((TOP_MISC_CFG_T *)TOP_MISC_CFG_BASE)


/* structure type of CMSYS_CFG
 */
typedef struct {
    __IO uint32_t MISC1;
    __IO uint32_t STCALIB;
    __IO uint32_t AHB_SLAVE_WAY_EN;
    __IO uint32_t AHB_DEC_ERR_EN;
    __IO uint32_t AHB_SIDEBAND;
    __IO uint32_t AHB_BUFFERALBE;
    __IO uint32_t AHB_FIFO_TH;
    __IO uint32_t FORCE_IDLE_OFF;
    uint32_t RESERVED0[1];
    __IO uint32_t CG_FREERUN_EN;
    __IO uint32_t CG_GATE_EN;
    uint32_t RESERVED1[1];
    __IO uint32_t TCM_BUS_CTRL;
    __IO uint32_t INT_ACTIVE_HL0;
    __IO uint32_t INT_ACTIVE_HL1;
    uint32_t RESERVED2[1];
    __IO uint32_t DCM_CTRL_REG;
} CMSYS_CFG_REGISTER_T;

#define CMSYS_CFG   ((CMSYS_CFG_REGISTER_T *)CMSYS_CFG_BASE)

/* structure type of CMSYS_CFG_EXT
 */
typedef struct {
    __IO uint32_t CG_EN;
    __IO uint32_t DCM_EN;
} CMSYS_CFG_EXT_REGISTER_T;

#define CMSYS_CFG_EXT   ((CMSYS_CFG_EXT_REGISTER_T *)CMSYS_CFG_EXT_BASE)


typedef struct {
    __IO uint32_t CMCFG_BOOT_VECTOR0;
    __IO uint32_t CMCFG_BOOT_VECTOR1;
    __IO uint32_t CMCFG_BOOT_FROM_SLV;
    uint32_t RESERVED0[1];
    __IO uint32_t DSP0CFG_BOOT_VECTOR;
    __IO uint32_t DSP0CFG_BOOT_VECTOR_SELECT;
    __IO uint32_t DSP0CFG_STALL;
    __IO uint32_t DSP0_DEEP_SLEEP;
    __IO uint32_t DSP0_DEBUG0;
    __IO uint32_t DSP0_DEBUG1;
    __I  uint32_t DSP0_FATALERR_INFO;
    uint32_t RESERVED1[1];
    __IO uint32_t DUMMY_RW;
    __I  uint32_t DUMMY_RO;
} MCU_CFG_PRI_T;

#define MCU_CFG_PRI  ((MCU_CFG_PRI_T *) MCU_CFG_PRI_BASE)



typedef struct {
    __I uint32_t GLOBAL_STATUS0;
        uint32_t RESERVED0[3];
    __I uint32_t GLOBAL_STATUS1;
        uint32_t RESERVED1[59];
    __I uint32_t CH0_STATUS0;
    __O uint32_t CH0_LOCK0;
    __O uint32_t CH0_RELEASE0;
        uint32_t RESERVED2[1];
    __I uint32_t CH0_STATUS1;
    __O uint32_t CH0_LOCK1;
    __O uint32_t CH0_RELEASE1;
        uint32_t RESERVED3[57];
    __I uint32_t CH1_STATUS0;
    __O uint32_t CH1_LOCK0;
    __O uint32_t CH1_RELEASE0;
        uint32_t RESERVED4[1];
    __I uint32_t CH1_STATUS1;
    __O uint32_t CH1_LOCK1;
    __O uint32_t CH1_RELEASE1;
} SMPH_REGISTER_T;

#define SMPH     ((SMPH_REGISTER_T *) SEMAPHORE_BASE)

/* Register definitions for UART */
typedef struct {
    __I uint32_t RBR; /*00: RX buffer register */

    __O uint32_t THR; /*04: TX holding register */

    __IO uint32_t DLM_DLL; /*08: Divisor latch register */

    union {
        struct {
            __IO uint8_t ETBEI; /* Tx interrupt */
            __IO uint8_t ELSI_ERBFI; /* Rx interrupt */
            __IO uint8_t XOFFI; /* XOFF interrupt */
            __IO uint8_t CTSI_RTSI; /* CTS & RTS interrupt */
        } IER_CELLS;
        __IO uint32_t IER; /*0C: Interrupt enable register */
    } IER_UNION;

    __I uint32_t IIR; /*10: Interrupt identification register */

    union {
        struct {
            __IO uint8_t FIFOE; /* Enable Rx & Tx FIFOs */
            __IO uint8_t RFTL_TFTL; /* Rx & Tx FIFO trigger threshold */
            __O uint8_t CLRR; /* Clear Rx FIFO */
            __O uint8_t CLRT; /* Clear Tx FIFO */
        } FCR_CELLS;
        __O uint32_t FCR; /*14: FIFO control register */
    } FCR_UNION;

    union {
        struct {
            __IO uint8_t SW_FLOW_CONT; /* Software flow control */
            __IO uint8_t HW_FLOW_CONT; /* Hardware flow control */
            __O uint8_t SEND_XOFF; /* Send XOFF */
            __O uint8_t SEND_XON; /* Send XON */
        } EFR_CELLS;
        __O uint32_t EFR; /*18: Enhanced feature register */
    } EFR_UNION;

    union {
        struct {
            __IO uint8_t PAR_STB_WLS; /* Parity, stop bits, & word length setting */
            __IO uint8_t SB; /* Set break */
            __I uint8_t RESERVED[2];
        } LCR_CELLS;
        __IO uint32_t LCR; /*1C: Line control register */
    } LCR_UNION;

    union {
        struct {
            __IO uint8_t RTS; /* RTS state */
            __IO uint8_t LOOP; /* Enable loop-back mode */
            __I uint8_t XOFF_STATUS; /* XOFF status */
            __I uint8_t RESERVED;
        } MCR_CELLS;
        __IO uint32_t MCR; /*20: Modem control register */
    } MCR_UNION;

    union {
        struct {
            __IO uint8_t XOFF; /* XON character for software flow control */
            __IO uint8_t XON; /* XOFF character for software flow control */
            __I uint8_t RESERVED[2];
        } XON_XOFF_CELLS;
        __IO uint32_t XON_XOFF; /*24: XON & XOFF register */
    } XON_XOFF_UNION;

    __I uint32_t LSR; /*28: Line status register */

    __IO uint32_t SCR; /*2C: Scratch register */

    union {
        struct {
            __IO uint8_t AUTOBAUD_EN; /* Enable auto-baud */
            __IO uint8_t AUTOBAUD_SEL; /* Auto-baud mode */
            __IO uint8_t AUTOBAUD_SLEEP_ACK; /* Enable auto-baud sleep ack */
            __I uint8_t RESERVED;
        } AUTOBAUD_CON_CELLS;
        __IO uint32_t AUTOBAUD_CON; /*30: Autoband control register */
    } AUTOBAUD_CON_UNION;

    __IO uint32_t HIGHSPEED; /*34: High speed mode register */

    union {
        struct {
            __IO uint8_t SAMPLE_COUNT; /* Sample counter */
            __IO uint8_t SAMPLE_POINT; /* Sample point */
            __I uint8_t RESERVED[2];
        } SAMPLE_REG_CELLS;
        __IO uint32_t SAMPLE_REG; /*38: Sample counter & sample point register */
    } SAMPLE_REG_UNION;

    union {
        struct {
            __I uint8_t AUTOBAUD_RATE; /* Auto-baud baudrate */
            __I uint8_t AUTOBAUD_STAT; /* Auto-baud state */
            __I uint8_t RESERVED[2];
        } AUTOBAUD_REG_CELLS;
        __I uint32_t AUTOBAUD_REG; /*3C: Autobaud monitor register */
    } AUTOBAUD_REG_UNION;

    union {
        struct {
            __IO uint8_t AUTOBAUD_SAMPLE; /* Clock division for auto-baud detection */
            __IO uint8_t AUTOBAUD_RATEFIX; /* System clock rate for auto-baud detection */
            __IO uint8_t RATEFIX; /* System clock rate for Tx/Rx */
            __I uint8_t RESERVED;
        } RATEFIX_CELLS;
        __IO uint32_t RATEFIX; /*40: Clock rate fix register */
    } RATEFIX_UNION;

    __IO uint32_t GUARD; /*44: Guard interval register */

    union {
        struct {
            __IO uint8_t ESCAPE_CHAR; /* Escape character setting */
            __IO uint8_t ESCAPE_EN; /* Enable escape character */
            __I uint8_t RESERVED[2];
        } ESCAPE_REG_CELLS;
        __IO uint32_t ESCAPE_REG; /*48: Escape character register */
    } ESCAPE_REG_UNION;

    __IO uint32_t SLEEP_REG; /*4C: Sleep mode control register */

    union {
        struct {
            __IO uint8_t RX_DMA_EN; /* Enable Rx DMA mode */
            __IO uint8_t TX_DMA_EN; /* Enable Tx DMA mode */
            __IO uint8_t FIFO_LSR_SEL; /* FIFO LSR mode */
            __I uint8_t RESERVED;
        } DMA_CON_CELLS;
        __IO uint32_t DMA_CON; /*50: DMA mode control register */
    } DMA_CON_UNION;

    __IO uint32_t RXTRIG; /*54: Rx FIFO trigger threshold register */

    __IO uint32_t FRACDIV; /*58: Fractional divider register */

    union {
        struct {
            __IO uint8_t RX_TO_MODE; /* Rx timeout mode */
            __IO uint8_t TO_CNT_AUTORST; /* Time-out counter auto reset */
            __I uint8_t RESERVED[2];
        } RX_TO_CON_CELLS;
        __IO uint32_t RX_TO_CON; /*5C: Rx timeout mode control register */
    } RX_TO_CON_UNION;

    __IO uint32_t RX_TOC_DEST; /*60: Rx timeout counter destination value register */
} UART_REGISTER_T;

#define UART0   ((UART_REGISTER_T *)(UART0_BASE))
#define UART1   ((UART_REGISTER_T *)(UART1_BASE))
#define UART2   ((UART_REGISTER_T *)(UART2_BASE))

/* Total register field bit definition */
/* Bit definition for Interrupt Enable Register */
#define UART_IER_ETBEI_OFFSET   (0) /* generated if RBR has data or the RX Trigger Level is reached */
#define UART_IER_ETBEI_MASK     (0x1<<UART_IER_ETBEI_OFFSET)

/* Bit definition for Interrupt Identification Register */
#define UART_IIR_ID_OFFSET      (0) /* Interrupt Source Identification */
#define UART_IIR_ID_MASK        (0x3F<<UART_IIR_ID_OFFSET)

/* Bit definition for FIFO Control Register */
#define UART_FCR_CLRT_OFFSET    (24) /* control bit to clear tx fifo */
#define UART_FCR_CLRT_MASK      (0x1<<UART_FCR_CLRT_OFFSET)
#define UART_FCR_CLRR_OFFSET    (16) /* control bit to clear rx fifo */
#define UART_FCR_CLRR_MASK      (0x1<<UART_FCR_CLRR_OFFSET)
#define UART_FCR_RFTL_OFFSET    (10) /* RX FIFO trigger threshold */
#define UART_FCR_RFTL_MASK      (0x3<<UART_FCR_RFTL_OFFSET)
#define UART_FCR_TFTL_OFFSET    (8) /* TX FIFO trigger threshold */
#define UART_FCR_TFTL_MASK      (0x3<<UART_FCR_TFTL_OFFSET)
#define UART_FCR_FIFOE_OFFSET   (0) /* Enable Rx & Tx FIFOs */
#define UART_FCR_FIFOE_MASK     (0x1<<UART_FCR_FIFOE_OFFSET)

/* Bit definition for Line Control Register */
#define UART_LCR_SB_OFFSET      (8) /* Set Break */
#define UART_LCR_SB_MASK        (0x1<<UART_LCR_SB_OFFSET)
#define UART_LCR_PARITY_OFFSET  (3) /* Parity */
#define UART_LCR_PARITY_MASK    (0x7<<UART_LCR_PARITY_OFFSET)
#define UART_LCR_STB_OFFSET     (2) /* Number of STOP bits */
#define UART_LCR_STB_MASK       (0x1<<UART_LCR_STB_OFFSET)
#define UART_LCR_WORD_OFFSET    (0) /* Word Length */
#define UART_LCR_WORD_MASK      (0x3<<UART_LCR_WORD_OFFSET)

/* Bit definition for Modem Control Register */
#define UART_MCR_XOFF_STATUS_OFFSET (16) /* whether an XON character is received */
#define UART_MCR_XOFF_STATUS_MASK   (0x1<<UART_MCR_XOFF_STATUS_OFFSET)
#define UART_MCR_LOOP_OFFSET        (8) /* Loop-back control bit */
#define UART_MCR_LOOP_MASK          (0x1<<UART_MCR_LOOP_OFFSET)
#define UART_MCR_RTS_OFFSET         (0) /* Controls the state of the output RTS, even in loop mode */
#define UART_MCR_RTS_MASK           (0x1<<UART_MCR_RTS_OFFSET)

/* Bit definition for XON & XOFF character Register */
#define UART_XON_XOFF_XONCHAR_OFFSET (8) /* define XON character */
#define UART_XON_XOFF_XONCHAR_MASK   (0xff<<UART_XON_XOFF_XONCHAR_OFFSET)
#define UART_XON_XOFF_XOFFCHAR_OFFSET (0) /* define XOFF character */
#define UART_XON_XOFF_XOFFCHAR_MASK   (0xff<<UART_XON_XOFF_XOFFCHAR_OFFSET)

/* Bit definition for Line Status Register */
#define UART_LSR_FIFOERR_OFFSET (7) /* RX FIFO Error Indicator */
#define UART_LSR_FIFOERR_MASK   (0x1<<UART_LSR_FIFOERR_OFFSET)
#define UART_LSR_TEMT_OFFSET    (6) /* TX Holding Register (or TX FIFO) and the TX Shift Register are empty */
#define UART_LSR_TEMT_MASK      (0x1<<UART_LSR_TEMT_OFFSET)
#define UART_LSR_THRE_OFFSET    (5) /* Indicates if there is room for TX Holding Register or TX FIFO is reduced to its Trigger Level */
#define UART_LSR_THRE_MASK      (0x1<<UART_LSR_THRE_OFFSET)
#define UART_LSR_BI_OFFSET  (4) /* Break Interrupt */
#define UART_LSR_BI_MASK    (0x1<<UART_LSR_BI_OFFSET)
#define UART_LSR_FE_OFFSET  (3) /* Framing Error */
#define UART_LSR_FE_MASK    (0x1<<UART_LSR_FE_OFFSET)
#define UART_LSR_PE_OFFSET  (2) /* Parity Error */
#define UART_LSR_PE_MASK    (0x1<<UART_LSR_PE_OFFSET)
#define UART_LSR_OE_OFFSET  (1) /* Overrun Error */
#define UART_LSR_OE_MASK    (0x1<<UART_LSR_OE_OFFSET)
#define UART_LSR_DR_OFFSET  (0) /* Data Ready */
#define UART_LSR_DR_MASK    (0x1<<UART_LSR_DR_OFFSET)

/* Bit definition for Auto Baud Detect Enable Register */
#define UART_AUTOBAUD_CON_SLEEP_ACK_SEL_OFFSET   (16) /* Sleep ack select when autobaud_en */
#define UART_AUTOBAUD_CON_SLEEP_ACK_SEL_MASK     (0x1<<UART_AUTOBAUD_CON_SLEEP_ACK_SEL_OFFSET)
#define UART_AUTOBAUD_CON_AUTOBAUD_SEL_OFFSET    (8) /* Auto-baud select */
#define UART_AUTOBAUD_CON_AUTOBAUD_SEL_MASK      (0x1<<UART_AUTOBAUD_CON_AUTOBAUD_SEL_OFFSET)
#define UART_AUTOBAUD_CON_AUTOBAUD_EN_OFFSET     (0) /* Auto-baud enable signal */
#define UART_AUTOBAUD_CON_AUTOBAUD_EN_MASK       (0x1<<UART_AUTOBAUD_CON_AUTOBAUD_EN_OFFSET)

/* Bit definition for High Speed Mode Register */
#define UART_HIGHSPEED_SPEED_OFFSET (0) /* UART sample counter base */
#define UART_HIGHSPEED_SPEED_MASK   (0x3<<UART_HIGHSPEED_SPEED_OFFSET)

/* Bit definition for Sample counter & sample point register */
#define UART_SAMPLE_REG_SAMPLE_POINT_OFFSET (8) /* Sample point */
#define UART_SAMPLE_REG_SAMPLE_POINT_MASK   (0xff<<UART_SAMPLE_REG_SAMPLE_POINT_OFFSET)
#define UART_SAMPLE_REG_SAMPLE_COUNT_OFFSET (0) /* Sample counter */
#define UART_SAMPLE_REG_SAMPLE_COUNT_MASK   (0xff<<UART_SAMPLE_REG_SAMPLE_COUNT_OFFSET)

/* Bit definition for Auto Baud Monitor Register */
#define UART_AUTOBAUD_REG_BAUD_STAT_OFFSET  (8) /* frame detect result */
#define UART_AUTOBAUD_REG_BAUD_STAT_MASK    (0x0F<<UART_AUTOBAUD_REG_BAUD_STAT_OFFSET)
#define UART_AUTOBAUD_REG_BAUD_RATE_OFFSET  (0) /* baudrate detect result */
#define UART_AUTOBAUD_REG_BAUD_RATE_MASK    (0x0F<<UART_AUTOBAUD_REG_BAUD_RATE_OFFSET)

/* Bit definition for Clock Rate Fix Register */
#define UART_RATEFIX_RATEFIX_OFFSET (16) /* System clock rate for Tx/Rx */
#define UART_RATEFIX_RATEFIX_MASK   (0x1<<UART_RATEFIX_RATEFIX_OFFSET)
#define UART_RATEFIX_AUTOBAUD_RATEFIX_OFFSET (8) /* System clock rate for auto-baud detection */
#define UART_RATEFIX_AUTOBAUD_RATEFIX_MASK   (0x1<<UART_RATEFIX_AUTOBAUD_RATEFIX_OFFSET)
#define UART_RATEFIX_AUTOBAUD_SAMPLE_OFFSET (0) /* Clock division for auto-baud detection */
#define UART_RATEFIX_AUTOBAUD_SAMPLE_MASK   (0x3f<<UART_RATEFIX_AUTOBAUD_SAMPLE_OFFSET)

/* Bit definition for Guard time added register */
#define UART_GUARD_GUARD_EN_OFFSET  (4) /* Guard interval add enable signal */
#define UART_GUARD_GUARD_EN_MASK    (0x1<<UART_GUARD_GUARD_EN_OFFSET)
#define UART_GUARD_GUARD_CNT_OFFSET (0) /* Guard interval count value */
#define UART_GUARD_GUARD_CNT_MASK   (0x0F<<UART_GUARD_GUARD_CNT_OFFSET)

/* Bit definition for Escape character register */
#define UART_ESCAPE_REG_EN_OFFSET     (8) /* Add escape character when TX and remove escape character when RX */
#define UART_ESCAPE_REG_EN_MASK       (0x1<<UART_ESCAPE_REG_EN_OFFSET)
#define UART_ESCAPE_REG_CHAR_OFFSET   (0) /* define escape character */
#define UART_ESCAPE_REG_CHAR_MASK     (0xff<<UART_ESCAPE_REG_CHAR_OFFSET)

/* Bit definition for Sleep enable register */
#define UART_SLEEP_EN_OFFSET    (0) /* Sleep enable bit */
#define UART_SLEEP_EN_MASK      (0x1<<UART_SLEEP_EN_OFFSET)
#define UART_SLEEP_IDLE_FC_EN_OFFSET    (8) /* Sleep idle flow control enable bit */
#define UART_SLEEP_IDLE_FC_EN_MASK      (0x1<<UART_SLEEP_IDLE_FC_EN_OFFSET)

/* Bit definition for Rx Trigger Address register */
#define UART_RXTRIG_OFFSET (0) /* When {rtm,rtl}=2'b11, The Rx FIFO threshold will be Rxtrig */
#define UART_RXTRIG_MASK   (0x0F<<UART_RXTRIG_OFFSET)

/* Bit definition for Fractional Divider Address register */
#define UART_FRACDIV_OFFSET   (0) /* Add sampling count when in state stop to parity */
#define UART_FRACDIV_MASK     (0x3ff<<UART_FRACDIV_OFFSET)

/* Bit definition for Rx timeout mode control register */
#define UART_RX_TO_CON_TO_CNT_AUTORST_OFFSET (8) /* Time-out counter auto reset */
#define UART_RX_TO_CON_TO_CNT_AUTORST_MASK   (0x1<<UART_RX_TO_CON_TO_CNT_AUTORST_OFFSET)
#define UART_RX_TO_CON_RX_TO_MODE_OFFSET (0) /* Rx timeout mode */
#define UART_RX_TO_CON_RX_TO_MODE_MASK   (0x1<<UART_RX_TO_CON_RX_TO_MODE_OFFSET)


/* Bit definition from BTA view */
/* Bit definition for Interrupt Enable Register */
#define UART_IER_CTSI_OFFSET    (1) /* generated when a rising edge is detected on the CTS modem control line */
#define UART_IER_CTSI_MASK      (0x1<<UART_IER_CTSI_OFFSET)
#define UART_IER_RTSI_OFFSET    (0) /* generated when a rising edge is detected on the RTS modem control line */
#define UART_IER_RTSI_MASK      (0x1<<UART_IER_RTSI_OFFSET)
#define UART_IER_XOFFI_OFFSET   (0) /* generated when an XOFF character is received */
#define UART_IER_XOFFI_MASK     (0x1<<UART_IER_XOFFI_OFFSET)
#define UART_IER_ELSI_OFFSET    (1) /* generated if BI, FE, PE or OE (LSR[4:1]) becomes set */
#define UART_IER_ELSI_MASK      (0x1<<UART_IER_ELSI_OFFSET)
#define UART_IER_ERBFI_OFFSET   (0) /* generated if the THR is empty or TX FIFO reduced to its Trigger Level */
#define UART_IER_ERBFI_MASK     (0x1<<UART_IER_ERBFI_OFFSET)

/* Bit definition for Enhanced Feature Register */
#define UART_EFR_SEND_XON_OFFSET    (0) /* Send XON */
#define UART_EFR_SEND_XON_MASK      (0x1<<UART_EFR_SEND_XON_OFFSET)
#define UART_EFR_SEND_XOFF_OFFSET   (0) /* Send XOFF */
#define UART_EFR_SEND_XOFF_MASK     (0x1<<UART_EFR_SEND_XOFF_OFFSET)
#define UART_EFR_HW_TX_FLOWCTRL_OFFSET (1) /* Hardware flow control */
#define UART_EFR_HW_TX_FLOWCTRL_MASK   (0x1<<UART_EFR_HW_TX_FLOWCTRL_OFFSET)
#define UART_EFR_HW_RX_FLOWCTRL_OFFSET (0) /* Hardware flow control */
#define UART_EFR_HW_RX_FLOWCTRL_MASK   (0x1<<UART_EFR_HW_RX_FLOWCTRL_OFFSET)
#define UART_EFR_SW_TX_FLOWCTRL_OFFSET (1) /* Software flow control */
#define UART_EFR_SW_TX_FLOWCTRL_MASK   (0x1<<UART_EFR_SW_TX_FLOWCTRL_OFFSET)
#define UART_EFR_SW_RX_FLOWCTRL_OFFSET (0) /* Software flow control */
#define UART_EFR_SW_RX_FLOWCTRL_MASK   (0x1<<UART_EFR_SW_RX_FLOWCTRL_OFFSET)

/* Bit definition for DMA mode control register */
#define UART_DMA_CON_FIFO_LSR_SEL_OFFSET (0) /* fifo lsr mode selection */
#define UART_DMA_CON_FIFO_LSR_SEL_MASK   (0x1<<UART_DMA_CON_FIFO_LSR_SEL_OFFSET)
#define UART_DMA_CON_TX_DMA_EN_OFFSET    (0) /* TX_DMA mechanism enable signal */
#define UART_DMA_CON_TX_DMA_EN_MASK      (0x1<<UART_DMA_CON_TX_DMA_EN_OFFSET)
#define UART_DMA_CON_RX_DMA_EN_OFFSET    (0) /* RX_DMA mechanism enable signal */
#define UART_DMA_CON_RX_DMA_EN_MASK      (0x1<<UART_DMA_CON_RX_DMA_EN_OFFSET)



#define UART_IIR_NONE      (0x0) /* No interrupt pending */
#define UART_IIR_LSR       (0x1<<UART_IIR_ID_OFFSET) /* Line Status Interrupt */
#define UART_IIR_RDT       (0x8<<UART_IIR_ID_OFFSET) /* RX Data Timeout */
#define UART_IIR_RDR       (0x4<<UART_IIR_ID_OFFSET) /* RX Data Received */
#define UART_IIR_THRE      (0x2<<UART_IIR_ID_OFFSET) /* TX Holding Register Empty */
#define UART_IIR_RCVXOFF   (0x10<<UART_IIR_ID_OFFSET) /* Software Flow Control */
#define UART_IIR_CTSRTS    (0x20<<UART_IIR_ID_OFFSET) /* Hardware Flow Control */
#define UART_FCR_RXTRIG_1       (0x0<<UART_FCR_RFTL_OFFSET) /* RX FIFO trigger = 1 */
#define UART_FCR_RXTRIG_6       (0x1<<UART_FCR_RFTL_OFFSET) /* RX FIFO trigger = 6 */
#define UART_FCR_RXTRIG_12      (0x2<<UART_FCR_RFTL_OFFSET) /* RX FIFO trigger = 12 */
#define UART_FCR_RXTRIG_USER    (0x3<<UART_FCR_RFTL_OFFSET) /* RX FIFO trigger = RXTRIG */
#define UART_FCR_TXTRIG_1       (0x0<<UART_FCR_TFTL_OFFSET) /* RX FIFO trigger = 1 */
#define UART_FCR_TXTRIG_4       (0x1<<UART_FCR_TFTL_OFFSET) /* RX FIFO trigger = 4 */
#define UART_FCR_TXTRIG_8       (0x2<<UART_FCR_TFTL_OFFSET) /* RX FIFO trigger = 8 */
#define UART_FCR_TXTRIG_14      (0x3<<UART_FCR_TFTL_OFFSET) /* RX FIFO trigger = 14 */
#define UART_LCR_PAR_NONE      (0x4<<UART_LCR_PARITY_OFFSET) /* Odd Even Parity */
#define UART_LCR_PAR_ODD       (0x1<<UART_LCR_PARITY_OFFSET) /* Even Parity Select */
#define UART_LCR_PAR_EVEN      (0x0<<UART_LCR_PARITY_OFFSET) /* Even Parity Select */
#define UART_LCR_STB_1      (0x0<<UART_LCR_STB_OFFSET) /* 1 STOP bits */
#define UART_LCR_STB_2      (0x1<<UART_LCR_STB_OFFSET) /* 2 STOP bits */
#define UART_LCR_WORD_5BITS (0x0<<UART_LCR_WORD_OFFSET) /* Word Length = 5BITS */
#define UART_LCR_WORD_6BITS (0x1<<UART_LCR_WORD_OFFSET) /* Word Length = 6BITS */
#define UART_LCR_WORD_7BITS (0x2<<UART_LCR_WORD_OFFSET) /* Word Length = 7BITS */
#define UART_LCR_WORD_8BITS (0x3<<UART_LCR_WORD_OFFSET) /* Word Length = 8BITS */
#define UART_HIGHSPEED_SPEED_MODE0  (0x0<<UART_HIGHSPEED_SPEED_OFFSET)  /* baud_rate = system clock frequency/16/{DLH, DLL} */
#define UART_HIGHSPEED_SPEED_MODE1  (0x1<<UART_HIGHSPEED_SPEED_OFFSET)  /* baud_rate = system clock frequency/8/{DLH, DLL} */
#define UART_HIGHSPEED_SPEED_MODE2  (0x2<<UART_HIGHSPEED_SPEED_OFFSET)  /* baud_rate = system clock frequency/14/{DLH, DLL} */
#define UART_HIGHSPEED_SPEED_MODE3  (0x3<<UART_HIGHSPEED_SPEED_OFFSET)  /* baud_rate = system clock frequency/(sampe_count+1)/{DLM, DLL} */


/*************************** USB register definition start line  ********************************/
#ifndef USB_HW_H
#define USB_HW_H

#define PLATFORM_1568   1

#if (defined(DRV_USB_IP_V4))
//=============Controller===================//
#define USB_FADDR  (USB_BASE+0x00) /* RW */
#define USB_POWER  (USB_BASE+0x01)
#define USB_INTRTX  (USB_BASE+0x02) /* 16-bit, status, read only */
#define USB_INTRRX  (USB_BASE+0x04)  /* 16-bit, status, read only */
#define USB_INTRTXE  (USB_BASE+0x06) /* 16-bit, RW */
#define USB_INTRRXE  (USB_BASE+0x08)  /* 16-bit, RW */
#define USB_INTRUSB  (USB_BASE+0x0A)  /* 8-bit, status, read only*/
#define USB_INTRUSBE (USB_BASE+0x0B) /* 8-bit, RW */
#define USB_FRAME  (USB_BASE+0x0C) /* 16-bit, read only */ /*Max Frame length = 11 bits*/
#define USB_INDEX  (USB_BASE+0x0E) /* RW, 4bit available*/
#define USB_TESTMODE (USB_BASE+0x0F) /* RW, 8-bit  */

#define USB_CSR0  (USB_BASE+0x12) /* 16-bit */
#define USB_COUNT0  (USB_BASE+0x18) /* RO, EP0 only*/
#define USB_NAKLIMIT0 (USB_BASE+0x1B) /* RW, host mode only*/
#define USB_TXMAXP  (USB_BASE+0x10) /* 16-bit, RW*/
#define USB_TXCSR  (USB_BASE+0x12)
#define USB_RXMAXP  (USB_BASE+0x14) /* 16-bit, RW*/
#define USB_RXCSR  (USB_BASE+0x16)
#define USB_RXCOUNT (USB_BASE+0x18) /* RO, 14bits */
#define USB_TXTYPE  (USB_BASE+0x1A) /* RW, host mode only */
#define USB_TXINTERVAL (USB_BASE+0x1B) /* RW, host mode only */
#define USB_RXTYPE  (USB_BASE+0x1C) /* RW, host mode only */
#define USB_RXINTERVAL (USB_BASE+0x1D) /* RW, host mode only */

#define USB_EP0   (USB_BASE+0x20) /* 4 byte as 1 queue */
#define USB_EP1   (USB_BASE+0x24)
#define USB_EP2   (USB_BASE+0x28)
#define USB_EP3   (USB_BASE+0x32)
#define USB_EP4   (USB_BASE+0x40)

#define USB_DEVCTL  (USB_BASE+0x60) /* 8-bit  */
#define USB_PWRUPCNT (USB_BASE+0x61) /* RW */
#define USB_TXFIFOSZ (USB_BASE+0x62) /* RW */
#define USB_RXFIFOSZ (USB_BASE+0x63) /* RW */
#define USB_TXFIFOADD (USB_BASE+0x64) /* RW */
#define USB_RXFIFOADD (USB_BASE+0x66) /* RW */

#define USB_BUSPERF3  (USB_BASE+0x74) /* RW */

#define USB_VPLEN  (USB_BASE+0x7B) /* RW, 8-bits */
#define USB_RSTINFO  (USB_BASE+0x7F) /* RW */


#define USB_L1INTS  (USB_BASE+0xA0) /* RW */
#define USB_L1INTM  (USB_BASE+0xA4) /* RW */
#define USB_L1INTP  (USB_BASE+0xA8) /* RW */
#define USB_L1INTC  (USB_BASE+0xAC) /* RW */


#define USB_DMA_INTR_STATUS   (USB_BASE+0x200)     /* 8-bits, W1C */
#define USB_DMA_INTR_UNMASK   (USB_BASE+0x201)     /* 8-bits, R only */
#define USB_DMA_INTR_UNMASK_CLEAR (USB_BASE+0x202)     /* 8-bits, W1S */
#define USB_DMA_INTR_UNMASK_SET  (USB_BASE+0x203)     /* 8-bits, W1S */


#define USB_DMALIMITER   (USB_BASE+0x210)     /* 8-bits, RW */
#define USB_DMA_CONFIG   (USB_BASE+0x220)     /* 8-bits, RW */

#define USB_DMACNTL(_n)   (USB_BASE+0x1F4+(0x10*(_n)))  /* RW, _n = 1~6 */
#define USB_DMAADDR(_n)  (USB_BASE+0x1F8+(0x10*(_n)))  /* RW, _n = 1~6 */
#define USB_DMACNT(_n)   (USB_BASE+0x1FC+(0x10*(_n)))  /* RW, _n = 1~6 */

#define USB_DMA_REALCNT(_n)  (USB_BASE+0x270+(0x10*(_n)))  /* RO, _n = 1~6, 32bits  */
#define USB_DMA_TIMER(_n)   (USB_BASE+0x274+(0x10*(_n)))  /* RW, _n = 1~6, 16bits */


#define USB_RQPKTCOUNT(_n)  (USB_BASE+0x300+(0x04*(_n)))  /* RW, 16bits,  _n = 1~3, host mode RX ep only */

//===================Controller variable===========================//
/* USB_POWER */
#define USB_POWER_ENABLESUSPENDM (0x01)   /* RW */
#define USB_POWER_SUSPENDMODE  (0x02)   /* P: RO ; H: WO */ /*Read clear by the intr. register*/
#define USB_POWER_RESUME    (0x04)   /* RW */
#define USB_POWER_RESET    (0x08)   /* P: RO ; H: RW */
#define USB_POWER_HSMODE    (0x10)   /* RO */
#define USB_POWER_HSENAB    (0x20)   /* RW */
#define USB_POWER_SOFTCONN   (0x40)   /* RW */
#define USB_POWER_ISOUPDATE   (0x80)   /* RW */

/* USB_INTRTX */
#define USB_INTRTX_EP0   (0x01)   /*RO*/
#define USB_INTRTX_EP1   (0x02)   /*RO*/
#define USB_INTRTX_EP2   (0x04)   /*RO*/
#define USB_INTRTX_EP3   (0x08)   /*RO*/
#define USB_INTRTX_EP4   (0x10)   /*RO*/

/* USB_INTRRX */
#define USB_INTRRX_EP1   (0x02)   /*RO*/
#define USB_INTRRX_EP2   (0x04)   /*RO*/
#define USB_INTRRX_EP3   (0x08)   /*RO*/

/* USB_INTRTXE & USB_INTRRXE */
#define USB_INTRE_EPEN   (0x01)   /* RW */
#define USB_INTRE_EPDIS   (0x00)   /* RW */

/* USB_INTRUSB */
#define USB_INTRUSB_SUSPEND (0x01)   /*RO*/
#define USB_INTRUSB_RESUME (0x02)   /*RO*/
#define USB_INTRUSB_RESET  (0x04)   /*RO*/
#define USB_INTRUSB_BABBLE  (0x04)   /*RO*/
#define USB_INTRUSB_SOF  (0x08)   /*RO*/
#define USB_INTRUSB_CONN  (0x10)   /*RO*/
#define USB_INTRUSB_DISCON  (0x20)   /*RO*/
#define USB_INTRUSB_SESSREQ (0x40)   /*RO*/
#define USB_INTRUSB_VBUSERROR (0x80)   /*RO*/

/* USB_INTRUSBE */
#define USB_INTRUSBE_SUSPEND (0x01)   /*RW*/
#define USB_INTRUSBE_RESUME (0x02)   /*RW*/
#define USB_INTRUSBE_RESET  (0x04)   /*RW*/
#define USB_INTRUSBE_BABBLE (0x04)   /*RW*/
#define USB_INTRUSBE_SOF  (0x08)   /*RW*/
#define USB_INTRUSBE_CONN  (0x10)   /*RW*/
#define USB_INTRUSBE_DISCON (0x20)   /*RW*/
#define USB_INTRUSBE_SESSREQ (0x40)   /*RW*/
#define USB_INTRUSBE_VBUSERROR (0x80)   /*RW*/

/* USB_TESTMODE */
#define USB_TESTMODE_TESTSE0NAK  (0x01) /* RW */
#define USB_TESTMODE_TESTJ   (0x02) /* RW */
#define USB_TESTMODE_TESTK   (0x04) /* RW */
#define USB_TESTMODE_TESTPACKET  (0x08) /* RW */
#define USB_TESTMODE_FORCEHS   (0x10) /* RW */
#define USB_TESTMODE_FORCEFS   (0x20) /* RW */
#define USB_TESTMODE_FIFOACCESS  (0x40) /* WO, AC */
#define USB_TESTMODE_FORCEHOST  (0x80) /* RW */

/* USB_DEVCTL */
#define USB_DEVCTL_SESSION   (0x01)  /* RW */
#define USB_DEVCTL_HOSTREQ  (0x02)  /* RW */
#define USB_DEVCTL_HOSTMODE  (0x04)  /* RO */
#define USB_DEVCTL_VBUS   (0x18)  /* RO */
#define USB_DEVCTL_LSDEV   (0x20)  /* RO */
#define USB_DEVCTL_FSDEV   (0x40)  /* RO */
#define USB_DEVCTL_BDEVICE   (0x80)  /* RO */

#define USB_DEVCTL_ABOVE_VBUS_VALID   (0x18)
#define USB_DEVCTL_ABOVE_A_VALID    (0x10)
#define USB_DEVCTL_ABOVE_SESSION_END   (0x01)
#define USB_DEVCTL_BELOW_SESSION_END   (0x00)

/* USB_CSR0 */
#define USB_CSR0_RXPKTRDY    (0x0001)   /* RO */
#define USB_CSR0_TXPKTRDY    (0x0002)   /* RW, AC */
#define USB_CSR0_SENTSTALL    (0x0004)   /* RC */
#define USB_CSR0_DATAEND    (0x0008)   /* WO, AC */
#define USB_CSR0_SETUPEND    (0x0010)   /* RO */
#define USB_CSR0_SENDSTALL    (0x0020)   /* WO, AC */
#define USB_CSR0_SERVICEDRXPKTRDY  (0x0040)   /* WO, AC */
#define USB_CSR0_SERVICEDSETUPEND  (0x0080)   /* WO, AC */
#define USB_CSR0_FLUSHFIFO    (0x0100)   /* WO, AC */
/* in Host mode */
#define USB_CSR0_RXSTALL    (0x0004)
#define USB_CSR0_SETUPPKT    (0x0008)
#define USB_CSR0_ERROR     (0x0010)
#define USB_CSR0_REQPKT    (0x0020)
#define USB_CSR0_STATUSPKT    (0x0040)
#define USB_CSR0_NAKTIMEOUT   (0x0080)
#define USB_CSR0_FLUSHFIFO    (0x0100)
#define USB_CSR0_DISPING    (0x0800)

/* USB_TXMAXP */
#define USB_TXMAXP_MAXPAYLOAD_MASK (0x07FF)
#define USB_TXMAXP_HIGHSPEED_MASK (0xF800)

/* USB_TXCSR */
#define USB_TXCSR_TXPKTRDY    (0x0001)   /* RW */
#define USB_TXCSR_FIFONOTEMPTY  (0x0002)   /* RO */
#define USB_TXCSR_UNDERRUN   (0x0004)   /* RW */
#define USB_TXCSR_FLUSHFIFO   (0x0008)   /* WO */
#define USB_TXCSR_SENDSTALL   (0x0010)   /* RW */
#define USB_TXCSR_SENTSTALL   (0x0020)   /* RC */
#define USB_TXCSR_CLRDATATOG   (0x0040)   /* WO */
#define USB_TXCSR_INCOMPTX    (0x0080)   /* RC */
#define USB_TXCSR_SETTXPKTRDY_TWICE (0x0100)   /* RC */
#define USB_TXCSR_DMAREQMODE   (0x0400)   /* RW */
#define USB_TXCSR_FRCDATATOG   (0x0800)   /* RW */
#define USB_TXCSR_DMAREQENAB   (0x1000)   /* RW */
#define USB_TXCSR_ISO     (0x4000)   /* RW */
#define USB_TXCSR_AUTOSET    (0x8000)   /* RW */
/* in Host mode */
#define USB_TXCSR_ERROR    (0x0004)
#define USB_TXCSR_RXSTALL    (0x0020)
#define USB_TXCSR_NAKTIMEOUT   (0x0080)

/* USB_RXMAXP */
#define USB_RXMAXP_MAXPAYLOAD_MASK (0x07FF)
#define USB_RXMAXP_HIGHSPEED_MASK (0xF800)

/* USB_RXCSR */
#define USB_RXCSR_RXPKTRDY    (0x0001)   /* RC */
#define USB_RXCSR_FIFOFULL    (0x0002)   /* RO, AC */
#define USB_RXCSR_OVERRUN    (0x0004)   /* RC */
#define USB_RXCSR_DATAERROR   (0x0008)   /* RO */
#define USB_RXCSR_FLUSHFIFO   (0x0010)   /* WO, AC */
#define USB_RXCSR_SENDSTALL   (0x0020)   /* RW */
#define USB_RXCSR_SENTSTALL   (0x0040)   /* RC */
#define USB_RXCSR_CLRDATATOG   (0x0080)   /* WO */
#define USB_RXCSR_INCOMPRX    (0x0100)   /* RC */
#define USB_RXCSR_KEEPERRCTATUS  (0x0200)   /* RC */
#define USB_RXCSR_DMAREQMODE   (0x0800)   /* RW */
#define USB_RXCSR_DISNYET    (0x1000)   /* RW */
#define USB_RXCSR_PIDERROR    (0x1000)   /* RO */
#define USB_RXCSR_DMAREQENAB   (0x2000)   /* RW */
#define USB_RXCSR_ISO     (0x4000)   /* RW */
#define USB_RXCSR_AUTOCLEAR   (0x8000)   /* RW */
/* in Host mode */
#define USB_RXCSR_ERROR    (0x0004)
#define USB_RXCSR_NAKTIMEOUT   (0x0008)
#define USB_RXCSR_REQPKT    (0x0020)
#define USB_RXCSR_RXSTALL    (0x0040)
#define USB_RXCSR_SETPEQPKT_TWICE (0x0400)
#define USB_RXCSR_AUTOREQ    (0x4000)

/* USB_TXTYPE */
#define USB_TXTYPE_EPNUM_MASK  (0x0f)
#define USB_TXTYPE_ISO     (0x10)
#define USB_TXTYPE_BULK    (0x20)
#define USB_TXTYPE_INTR    (0x30)
#define USB_TXTYPE_PROTOCOL_MASK  (0x30)

/* USB_RXTYPE */
#define USB_RXTYPE_EPNUM_MASK  (0x0f)
#define USB_RXTYPE_ISO     (0x10)
#define USB_RXTYPE_BULK    (0x20)
#define USB_RXTYPE_INTR    (0x30)
#define USB_RXTYPE_PROTOCOL_MASK  (0x30)

/* USB_PWRUPCNT */
#define USB_PWRUPCNT_MASK    (0x0f)

/* USB_FIFOSZ */
#define USB_FIFOSZ_SIZE_MASK   (0x0F)
#define USB_FIFOSZ_DPB     (0x10)
#define USB_FIFOSZ_SIZE_8    (0x00)
#define USB_FIFOSZ_SIZE_16    (0x01)
#define USB_FIFOSZ_SIZE_32    (0x02)
#define USB_FIFOSZ_SIZE_64    (0x03)
#define USB_FIFOSZ_SIZE_128   (0x04)
#define USB_FIFOSZ_SIZE_256   (0x05)
#define USB_FIFOSZ_SIZE_512   (0x06)
#define USB_FIFOSZ_SIZE_1024   (0x07)
#define USB_FIFOSZ_SIZE_2048   (0x08)
#define USB_FIFOSZ_SIZE_4096   (0x09)

/* USB_FIFOADD */
#define USB_FIFOADD_MASK    (0x1FFF)

/* USB_RXFIFOADD */
#define USB_RXFIFOADD_DATAERRINTREN   (0x8000)
#define USB_RXFIFOADD_OVERRUNINTREN   (0x4000)

/* USB_FIFO2ADD */
#define USB_FIFO2ADD_MASK    (0x1FFF)
#define USB_FIFO2ADD_EN    (0x8000)

/* USB_BUSPERF3 */
#define USB_BUSPERF3_DISUSBRESET   (0x0001)
#define USB_BUSPERF3_SWRST    (0x0002)



/* USB_RSTINFO */
#define USB_RSTINFO_WTFSSE0   (0x00F0)
#define USB_RSTINFO_WTCHRP   (0x000F)



/* USB_L1INTS */
#define USB_L1INTS_TX_INT_STATUS   (0x0001)
#define USB_L1INTS_RX_INT_STATUS   (0x0002)
#define USB_L1INTS_USBCOM_INT_STATUS  (0x0004)
#define USB_L1INTS_DMA_INT_STATUS  (0x0008)

/* USB_L1INTM */
#define USB_L1INTM_TX_INT_UNMASK   (0x0001)
#define USB_L1INTM_RX_INT_UNMASK   (0x0002)
#define USB_L1INTM_USBCOM_INT_UNMASK (0x0004)
#define USB_L1INTM_DMA_INT_UNMASK  (0x0008)
#define USB_L1INTM_PSR_INT_UNMASK  (0x0010)
#define USB_L1INTM_QINT_UNMASK   (0x0020)
#define USB_L1INTM_QHIF_INT_UNMASK  (0x0040)
#define USB_L1INTM_DPDM_INT_UNMASK  (0x0080)
#define USB_L1INTM_VBUSVALID_INT_UNMASK (0x0100)
#define USB_L1INTM_IDDIG_INT_UNMASK  (0x0200)
#define USB_L1INTM_DRVVBUS_INT_UNMASK (0x0400)
#define USB_L1INTM_POWERDOWN_INT_UNMASK (0x0800)


//========================definition of global value===================//
/* descriptor setting limitation */
#define USB_MAX_FIFO_SIZE    4096

#define USB_FIFO_START_ADDRESS  512

#define USB_BULK_FIFO_UNIT_SIZE  64
#ifdef  __ISO_HB__
#define USB_ISO_FIFO_UNIT_SIZE   3072
#else
#define USB_ISO_FIFO_UNIT_SIZE   1024
#endif


#ifdef  __INTR_HB__
#define USB_INTR_FIFO_UNIT_SIZE   3072
#else
#define USB_INTR_FIFO_UNIT_SIZE   1024
#endif


#elif (defined(DRV_USB_IP_V3))
//=============Controller===================//
#define USB_FADDR  (USB_BASE+0x00) /* RW */
#define USB_POWER  (USB_BASE+0x01)
#define USB_INTRTX  (USB_BASE+0x02) /* 16-bit, status, read only */
#define USB_INTRRX  (USB_BASE+0x04)  /* 16-bit, status, read only */
#define USB_INTRTXE  (USB_BASE+0x06) /* 16-bit, RW */
#define USB_INTRRXE  (USB_BASE+0x08)  /* 16-bit, RW */
#define USB_INTRUSB  (USB_BASE+0x0A)  /* 8-bit, status, read only*/
#define USB_INTRUSBE (USB_BASE+0x0B) /* 8-bit, RW */
#define USB_FRAME  (USB_BASE+0x0C) /* 16-bit, read only */ /*Max Frame length = 11 bits*/
#define USB_INDEX  (USB_BASE+0x0E) /* RW, 4bit available*/
#define USB_TESTMODE (USB_BASE+0x0F) /* RW, 8-bit  */

#define USB_CSR0  (USB_BASE+0x12) /* 16-bit */
#define USB_COUNT0  (USB_BASE+0x18) /* RO, EP0 only*/
#define USB_NAKLIMIT0 (USB_BASE+0x1B) /* RW, host mode only*/
#define USB_TXMAXP  (USB_BASE+0x10) /* 16-bit, RW*/
#define USB_TXCSR  (USB_BASE+0x12)
#define USB_RXMAXP  (USB_BASE+0x14) /* 16-bit, RW*/
#define USB_RXCSR  (USB_BASE+0x16)
#define USB_RXCOUNT (USB_BASE+0x18) /* RO, 14bits */
#define USB_TXTYPE  (USB_BASE+0x1A) /* RW, host mode only */
#define USB_TXINTERVAL (USB_BASE+0x1B) /* RW, host mode only */
#define USB_RXTYPE  (USB_BASE+0x1C) /* RW, host mode only */
#define USB_RXINTERVAL (USB_BASE+0x1D) /* RW, host mode only */

#define USB_EP0   (USB_BASE+0x20) /* 4 byte as 1 queue */

#define USB_DEVCTL  (USB_BASE+0x60) /* 8-bit  */
#define USB_PWRUPCNT (USB_BASE+0x61) /* RW */
#define USB_TXFIFOSZ (USB_BASE+0x62) /* RW */
#define USB_RXFIFOSZ (USB_BASE+0x63) /* RW */
#define USB_TXFIFOADD (USB_BASE+0x64) /* RW */
#define USB_RXFIFOADD (USB_BASE+0x66) /* RW */
#define USB_SWRST  (USB_BASE+0x70) /* RW */
#define USB_OPSTATE  (USB_BASE+0x71) /* RW */
#define USB_VPLEN  (USB_BASE+0x7B) /* RW, 8-bits */
#define USB_RSTINFO  (USB_BASE+0x7F) /* RW */


#define USB_EPn_TXCSR(_n)  (USB_BASE+0x102+(0x10*(_n))) /* R */
#define USB_EPn_RXCSR(_n)  (USB_BASE+0x106+(0x10*(_n))) /* R */

#define USB_DMAINTR   (USB_BASE+0x200)     /* 8-bits, W0C */
#define USB_DMALIMITER   (USB_BASE+0x210)     /* 8-bits, RW */
#define USB_DMACNTL(_n)   (USB_BASE+0x1F4+(0x10*(_n)))  /* RW, _n = 1~6 */
#define USB_DMAADDR(_n)  (USB_BASE+0x1F8+(0x10*(_n)))  /* RW, _n = 1~6 */
#define USB_DMACNT(_n)   (USB_BASE+0x1FC+(0x10*(_n)))  /* RW, _n = 1~6 */
#define USB_DMAPPCNTL(_n)  (USB_BASE+0x274+(0x10*(_n)))  /* RW, _n = 1~6 */
#define USB_DMAPPADDR(_n)  (USB_BASE+0x278+(0x10*(_n)))  /* RW, _n = 1~6 */
#define USB_DMAPPCNT(_n)  (USB_BASE+0x27C+(0x10*(_n)))  /* RW, _n = 1~6 */

#define USB_RQPKTCOUNT(_n)  (USB_BASE+0x300+(0x04*(_n)))  /* RW, 16bits,  _n = 1~3, host mode RX ep only */

#define USB_DMA_REALCNT(_n)  (USB_BASE+0x3F0+(0x10*(_n)))  /* RO, _n = 1~6, 32bits  */
#define USB_DMA_PP_REALCNT(_n) (USB_BASE+0x3F4+(0x10*(_n)))  /* RO, _n = 1~6, 32bits  */
#define USB_DMA_TIMER(_n)   (USB_BASE+0x3F8+(0x10*(_n)))  /* RW, _n = 1~6, 16bits */
//===================Controller variable===========================//
/* USB_POWER */
#define USB_POWER_ENABLESUSPENDM (0x01)   /* RW */
#define USB_POWER_SUSPENDMODE  (0x02)   /* P: RO ; H: WO */ /*Read clear by the intr. register*/
#define USB_POWER_RESUME    (0x04)   /* RW */
#define USB_POWER_RESET    (0x08)   /* P: RO ; H: RW */
#define USB_POWER_HSMODE    (0x10)   /* RO */
#define USB_POWER_HSENAB    (0x20)   /* RW */
#define USB_POWER_SOFTCONN   (0x40)   /* RW */
#define USB_POWER_ISOUPDATE   (0x80)   /* RW */

/* USB_INTRTX */
#define USB_INTRTX_EP0   (0x01)   /*RO*/
#define USB_INTRTX_EP1   (0x02)   /*RO*/
#define USB_INTRTX_EP2   (0x04)   /*RO*/
#define USB_INTRTX_EP3   (0x08)   /*RO*/
#define USB_INTRTX_EP4   (0x10)   /*RO*/

/* USB_INTRRX */
#define USB_INTRRX_EP1   (0x02)   /*RO*/
#define USB_INTRRX_EP2   (0x04)   /*RO*/
#define USB_INTRRX_EP3   (0x08)   /*RO*/

/* USB_INTRTXE & USB_INTRRXE */
#define USB_INTRE_EPEN   (0x01)   /* RW */
#define USB_INTRE_EPDIS   (0x00)   /* RW */

/* USB_INTRUSB */
#define USB_INTRUSB_SUSPEND (0x01)   /*RO*/
#define USB_INTRUSB_RESUME (0x02)   /*RO*/
#define USB_INTRUSB_RESET  (0x04)   /*RO*/
#define USB_INTRUSB_BABBLE  (0x04)   /*RO*/
#define USB_INTRUSB_SOF  (0x08)   /*RO*/
#define USB_INTRUSB_CONN  (0x10)   /*RO*/
#define USB_INTRUSB_DISCON  (0x20)   /*RO*/
#define USB_INTRUSB_SESSREQ (0x40)   /*RO*/
#define USB_INTRUSB_VBUSERROR (0x80)   /*RO*/

/* USB_INTRUSBE */
#define USB_INTRUSBE_SUSPEND (0x01)   /*RW*/
#define USB_INTRUSBE_RESUME (0x02)   /*RW*/
#define USB_INTRUSBE_RESET  (0x04)   /*RW*/
#define USB_INTRUSBE_BABBLE (0x04)   /*RW*/
#define USB_INTRUSBE_SOF  (0x08)   /*RW*/
#define USB_INTRUSBE_CONN  (0x10)   /*RW*/
#define USB_INTRUSBE_DISCON (0x20)   /*RW*/
#define USB_INTRUSBE_SESSREQ (0x40)   /*RW*/
#define USB_INTRUSBE_VBUSERROR (0x80)   /*RW*/

/* USB_TESTMODE */
#define USB_TESTMODE_TESTSE0NAK  (0x01) /* RW */
#define USB_TESTMODE_TESTJ   (0x02) /* RW */
#define USB_TESTMODE_TESTK   (0x04) /* RW */
#define USB_TESTMODE_TESTPACKET  (0x08) /* RW */
#define USB_TESTMODE_FORCEHS   (0x10) /* RW */
#define USB_TESTMODE_FORCEFS   (0x20) /* RW */
#define USB_TESTMODE_FIFOACCESS  (0x40) /* WO, AC */
#define USB_TESTMODE_FORCEHOST  (0x80) /* RW */

/* USB_DEVCTL */
#define USB_DEVCTL_SESSION   (0x01)  /* RW */
#define USB_DEVCTL_HOSTREQ  (0x02)  /* RW */
#define USB_DEVCTL_HOSTMODE  (0x04)  /* RO */
#define USB_DEVCTL_VBUS   (0x18)  /* RO */
#define USB_DEVCTL_LSDEV   (0x20)  /* RO */
#define USB_DEVCTL_FSDEV   (0x40)  /* RO */
#define USB_DEVCTL_BDEVICE   (0x80)  /* RO */

#define USB_DEVCTL_ABOVE_VBUS_VALID   (0x18)
#define USB_DEVCTL_ABOVE_A_VALID    (0x10)
#define USB_DEVCTL_ABOVE_SESSION_END   (0x01)
#define USB_DEVCTL_BELOW_SESSION_END   (0x00)

/* USB_CSR0 */
#define USB_CSR0_RXPKTRDY    (0x0001)   /* RO */
#define USB_CSR0_TXPKTRDY    (0x0002)   /* RW, AC */
#define USB_CSR0_SENTSTALL    (0x0004)   /* RC */
#define USB_CSR0_DATAEND    (0x0008)   /* WO, AC */
#define USB_CSR0_SETUPEND    (0x0010)   /* RO */
#define USB_CSR0_SENDSTALL    (0x0020)   /* WO, AC */
#define USB_CSR0_SERVICEDRXPKTRDY  (0x0040)   /* WO, AC */
#define USB_CSR0_SERVICEDSETUPEND  (0x0080)   /* WO, AC */
#define USB_CSR0_FLUSHFIFO    (0x0100)   /* WO, AC */
/* in Host mode */
#define USB_CSR0_RXSTALL    (0x0004)
#define USB_CSR0_SETUPPKT    (0x0008)
#define USB_CSR0_ERROR     (0x0010)
#define USB_CSR0_REQPKT    (0x0020)
#define USB_CSR0_STATUSPKT    (0x0040)
#define USB_CSR0_NAKTIMEOUT   (0x0080)
#define USB_CSR0_FLUSHFIFO    (0x0100)
#define USB_CSR0_DISPING    (0x0800)

/* USB_TXMAXP */
#define USB_TXMAXP_MAXPAYLOAD_MASK (0x07FF)
#define USB_TXMAXP_HIGHSPEED_MASK (0xF800)

/* USB_TXCSR */
#define USB_TXCSR_TXPKTRDY    (0x0001)   /* RW */
#define USB_TXCSR_FIFONOTEMPTY  (0x0002)   /* RO */
#define USB_TXCSR_UNDERRUN   (0x0004)   /* RW */
#define USB_TXCSR_FLUSHFIFO   (0x0008)   /* WO */
#define USB_TXCSR_SENDSTALL   (0x0010)   /* RW */
#define USB_TXCSR_SENTSTALL   (0x0020)   /* RC */
#define USB_TXCSR_CLRDATATOG   (0x0040)   /* WO */
#define USB_TXCSR_INCOMPTX    (0x0080)   /* RC */
#define USB_TXCSR_AUTOSET_SPKT  (0x0200)   /* RW */
#define USB_TXCSR_DMAREQMODE   (0x0400)   /* RW */
#define USB_TXCSR_FRCDATATOG   (0x0800)   /* RW */
#define USB_TXCSR_DMAREQENAB   (0x1000)   /* RW */
#define USB_TXCSR_MODE     (0x2000)   /* RW */
#define USB_TXCSR_ISO     (0x4000)   /* RW */
#define USB_TXCSR_AUTOSET    (0x8000)   /* RW */
/* in Host mode */
#define USB_TXCSR_ERROR    (0x0004)
#define USB_TXCSR_RXSTALL    (0x0020)
#define USB_TXCSR_NAKTIMEOUT   (0x0080)

/* USB_RXMAXP */
#define USB_RXMAXP_MAXPAYLOAD_MASK (0x07FF)
#define USB_RXMAXP_HIGHSPEED_MASK (0xF800)

/* USB_RXCSR */
#define USB_RXCSR_RXPKTRDY    (0x0001)   /* RC */
#define USB_RXCSR_FIFOFULL    (0x0002)   /* RO, AC */
#define USB_RXCSR_OVERRUN    (0x0004)   /* RC */
#define USB_RXCSR_DATAERROR   (0x0008)   /* RO */
#define USB_RXCSR_FLUSHFIFO   (0x0010)   /* WO, AC */
#define USB_RXCSR_SENDSTALL   (0x0020)   /* RW */
#define USB_RXCSR_SENTSTALL   (0x0040)   /* RC */
#define USB_RXCSR_CLRDATATOG   (0x0080)   /* WO */
#define USB_RXCSR_INCOMPRX    (0x0100)   /* RC */
#define USB_RXCSR_INCOMPRXINTREN  (0x0200)   /* RC */
#define USB_RXCSR_AUTOCLREN_SPKT  (0x0400)   /* RC */
#define USB_RXCSR_DMAREQMODE   (0x0800)   /* RW */
#define USB_RXCSR_DISNYET    (0x1000)   /* RW */
#define USB_RXCSR_PIDERROR    (0x1000)   /* RO */
#define USB_RXCSR_DMAREQENAB   (0x2000)   /* RW */
#define USB_RXCSR_ISO     (0x4000)   /* RW */
#define USB_RXCSR_AUTOCLEAR   (0x8000)   /* RW */
/* in Host mode */
#define USB_RXCSR_ERROR    (0x0004)
#define USB_RXCSR_NAKTIMEOUT   (0x0008)
#define USB_RXCSR_REQPKT    (0x0020)
#define USB_RXCSR_RXSTALL    (0x0040)
#define USB_RXCSR_AUTOREQ    (0x4000)

/* USB_TXTYPE */
#define USB_TXTYPE_EPNUM_MASK  (0x0f)
#define USB_TXTYPE_ISO     (0x10)
#define USB_TXTYPE_BULK    (0x20)
#define USB_TXTYPE_INTR    (0x30)
#define USB_TXTYPE_PROTOCOL_MASK  (0x30)

/* USB_RXTYPE */
#define USB_RXTYPE_EPNUM_MASK  (0x0f)
#define USB_RXTYPE_ISO     (0x10)
#define USB_RXTYPE_BULK    (0x20)
#define USB_RXTYPE_INTR    (0x30)
#define USB_RXTYPE_PROTOCOL_MASK  (0x30)

/* USB_PWRUPCNT */
#define USB_PWRUPCNT_MASK    (0x0f)

/* USB_FIFOSZ */
#define USB_FIFOSZ_SIZE_MASK   (0x0F)
#define USB_FIFOSZ_DPB     (0x10)
#define USB_FIFOSZ_SIZE_8    (0x00)
#define USB_FIFOSZ_SIZE_16    (0x01)
#define USB_FIFOSZ_SIZE_32    (0x02)
#define USB_FIFOSZ_SIZE_64    (0x03)
#define USB_FIFOSZ_SIZE_128   (0x04)
#define USB_FIFOSZ_SIZE_256   (0x05)
#define USB_FIFOSZ_SIZE_512   (0x06)
#define USB_FIFOSZ_SIZE_1024   (0x07)
#define USB_FIFOSZ_SIZE_2048   (0x08)
#define USB_FIFOSZ_SIZE_4096   (0x09)

/* USB_FIFOADD */
#define USB_FIFOADD_MASK    (0x1FFF)

/* USB_RXFIFOADD */
#define USB_RXFIFOADD_DATAERRINTREN   (0x8000)
#define USB_RXFIFOADD_OVERRUNINTREN   (0x4000)

/* USB_FIFO2ADD */
#define USB_FIFO2ADD_MASK    (0x1FFF)
#define USB_FIFO2ADD_EN    (0x8000)

/* USB_SWRST */
#define USB_SWRST_DISUSBRESET   (0x0001)
#define USB_SWRST_SWRST    (0x0002)
#define USB_SWRST_FRC_VBUSVALID  (0x0004)
#define USB_SWRST_UNDO_SRPFIX   (0x0008)
#define USB_SWRST_REDUCE_DLY   (0x0010)


/* USB_RSTINFO */
#define USB_RSTINFO_WTFSSE0   (0x00F0)
#define USB_RSTINFO_WTCHRP   (0x000F)

/* USB_DMAINTR */
#define USB_DMA_INTR_MASK    0xFFFFFF00
#define USB_DMA_LIMITER_MASK   0xFFFF00FF
#define USB_DMA_PPFINISH_MASK   0x0000FFFF

/* USB_DMAPPINTR */
#define USB_PPA_FINISH1    (0x01)   /*RO*/
#define USB_PPB_FINISH1     (0x02)   /*RO*/
#define USB_PPA_FINISH2    (0x04)   /*RO*/
#define USB_PPB_FINISH2     (0x08)   /*RO*/
#define USB_PPA_FINISH3    (0x10)   /*RO*/
#define USB_PPB_FINISH3     (0x20)   /*RO*/
#define USB_PPA_FINISH4    (0x40)   /*RO*/
#define USB_PPB_FINISH4     (0x80)   /*RO*/

/* USB_DMACNTL */
#define USB_DMACNTL_DMAEN    (0x0001)
#define USB_DMACNTL_DMADIR   (0x0002)
#define USB_DMACNTL_DMAMODE   (0x0004)
#define USB_DMACNTL_INTEN    (0x0008)
#define USB_DMACNTL_EP_MASK   (0x00F0)
#define USB_DMACNTL_BUSERR   (0x0100)
#define USB_DMACNTL_BURSTMODE_MASK (0x0600)
#define USB_DMACNTL_PPEN    (0x0800)
#define USB_DMACNTL_PPRST    (0x1000)
#define USB_DMACNTL_ENDMAMODE2  (0x2000)

/* USB_DMAPPCNTL */
#define USB_DMAPPCNTL_DMAEN   (0x0001)

/* USB_DMA_TIMER */
#define USB_DMA_TIMER_ENTIMER    (0x0080)
#define USB_DMA_TIMER_TIMEOUT_MASK  (0x007F)
#define USB_DMA_TIMER_TIMEOUT_STATUS (0x0100)

//========================definition of global value===================//
/* descriptor setting limitation */
#define USB_MAX_FIFO_SIZE    4096

#define USB_FIFO_START_ADDRESS  512

#define USB_BULK_FIFO_UNIT_SIZE  64
#ifdef  __ISO_HB__
#define USB_ISO_FIFO_UNIT_SIZE   3072
#else
#define USB_ISO_FIFO_UNIT_SIZE   1024
#endif


#ifdef  __INTR_HB__
#define USB_INTR_FIFO_UNIT_SIZE   3072
#else
#define USB_INTR_FIFO_UNIT_SIZE   1024
#endif


#if (defined(DRV_USB_PHY_COST_DOWN))

//======================USB PHY =============================//
/* USB phy register */
#define USB_PHYCR1_0    (USB_BASE+0x600) /* RW */
#define USB_PHYCR1_1    (USB_BASE+0x601) /* RW */
#define USB_PHYCR1_2    (USB_BASE+0x602) /* RW */
#define USB_PHYCR1_3    (USB_BASE+0x603) /* RW */

#define USB_PHYCR2_0    (USB_BASE+0x604) /* RW */
#define USB_PHYCR2_1    (USB_BASE+0x605) /* RW */
#define USB_PHYCR2_2    (USB_BASE+0x606) /* RW */
#define USB_PHYCR2_3    (USB_BASE+0x607) /* RW */

#define USB_PHYCR3_0    (USB_BASE+0x608) /* RW */
#define USB_PHYCR3_1    (USB_BASE+0x609) /* RW */
#define USB_PHYCR3_2    (USB_BASE+0x60A) /* RW */
#define USB_PHYCR3_3    (USB_BASE+0x60B) /* RW */

#define USB_PHYCR4_0    (USB_BASE+0x60C) /* RW */
//#define USB_PHYCR4_1    (USB_BASE+0x60D) /* RW */
//#define USB_PHYCR4_2    (USB_BASE+0x60E) /* RW */
#define USB_PHYCR4_3    (USB_BASE+0x60F) /* RW */

//#define USB_PHYCR5_0    (USB_BASE+0x610) /* RW */
//#define USB_PHYCR5_1    (USB_BASE+0x611) /* RW */
//#define USB_PHYCR5_2    (USB_BASE+0x612) /* RW */
#define USB_PHYCR5_3    (USB_BASE+0x613) /* RW */

#define USB_PHYIR1_0    (USB_BASE+0x614) /* RW */
#define USB_PHYIR1_1    (USB_BASE+0x615) /* RW */
#define USB_PHYIR1_2    (USB_BASE+0x616) /* RW */
#define USB_PHYIR1_3    (USB_BASE+0x617) /* RW */

#define USB_PHYIR2_0    (USB_BASE+0x618) /* RW */
#define USB_PHYIR2_1    (USB_BASE+0x619) /* RW */
#define USB_PHYIR2_2    (USB_BASE+0x61A) /* RW */
#define USB_PHYIR2_3    (USB_BASE+0x61B) /* RW */

#define USB_PHYIR3_0    (USB_BASE+0x61C) /* RW */
#define USB_PHYIR3_1    (USB_BASE+0x61D) /* RW */
#define USB_PHYIR3_2    (USB_BASE+0x61E) /* RW */
//#define USB_PHYIR3_3    (USB_BASE+0x61F) /* RW */

#define USB_PHYIR4_0    (USB_BASE+0x620) /* RW */
#define USB_PHYIR4_1    (USB_BASE+0x621) /* RW */
#define USB_PHYIR4_2    (USB_BASE+0x622) /* RW */
#define USB_PHYIR4_3    (USB_BASE+0x623) /* RW */

#define USB_PHYIR5_0    (USB_BASE+0x624) /* RW */
#define USB_PHYIR5_1    (USB_BASE+0x625) /* RW */
#define USB_PHYIR5_2    (USB_BASE+0x626) /* RW */
//#define USB_PHYIR5_3    (USB_BASE+0x627) /* RW */

#define USB_PHYIR6_0    (USB_BASE+0x628) /* RW */
#define USB_PHYIR6_1    (USB_BASE+0x629) /* RW */
//#define USB_PHYIR6_2    (USB_BASE+0x62A) /* RW */
//#define USB_PHYIR6_3    (USB_BASE+0x62B) /* RW */

//============================USB PHY variable========================//

/* USB_PHY ALL*/
#define USB_PHY_CLEAR_MASK     (0x00)

/* USB_PHYCR1_0 */
#define USB_PHYCR1_0_BGR_BGR_EN    (0x01)
#define USB_PHYCR1_0_BGR_I_SRC_EN    (0x02)
#define USB_PHYCR1_0_BGR_CHIP_EN    (0x04)
#define USB_PHYCR1_0_IADJ_MASK    (0x70)
#define USB_PHYCR1_0_IADJ_MASK2    (0x40)
#define USB_PHYCR1_0_IADJ_MASK3    (0x50)
#define USB_PHYCR1_0_IADJ_RESISTER_SET      (0x60)
/* USB_PHYCR1_1 */
/* USB_PHYCR1_2 */
#define USB_PHYCR1_2_RG_PLL_DIV  (0x0A)
#define USB_PHYCR1_2_RG_PLL_DIV2  (0x08)
/* USB_PHYCR1_3 */
/* USB_PHYCR2_0 */
/* USB_PHYCR2_1 */
/* USB_PHYCR2_2 */
#define USB_PHYCR2_2_HS_TRIM_TH   (0x08)
#define USB_PHYCR2_2_HSDISC_DEGL  (0x80)
#define USB_PHYCR2_2_HS_DISC_TH   (0x40)
/* USB_PHYCR2_3 */
#define USB_PHYCR2_3_PLL_EN     (0x02)
/* USB_PHYCR3_0 */
#define USB_PHYCR3_0_CDR_FILT     (0x02)
#define USB_PHYCR3_0_EARLY_HSTX_I    (0x40)
#define USB_PHYCR3_0_HS_TX_ANA_SER_EN  (0x80)
/* USB_PHYCR3_1 */
/* USB_PHYCR3_2 */
#define USB_PHYCR3_2_FORCE_DATA_IN   (0x02)
#define USB_PHYCR3_2_FORCE_TX_VALID   (0x01)
#define USB_PHYCR3_2_FORCE_DP_PULLDOWN  (0x04)
#define USB_PHYCR3_2_FORCE_DM_PULLDOWN  (0x08)
#define USB_PHYCR3_2_FORCE_DP_DM_PULLDOWN (0x0C)
#define USB_PHYCR3_2_FORCE_DRV_VBUS   (0x10)
/* USB_PHYCR3_3 */
#define USB_PHYCR3_3_FORCE_OP_MODE   (0x01)
#define USB_PHYCR3_3_FORCE_TERM_SELECT  (0x02)
#define USB_PHYCR3_3_FORCE_SUSPENDM   (0x04)
#define USB_PHYCR3_3_FORCE_XCVR_SELECT  (0x08)
#define USB_PHYCR3_3_FORCE_DP_HIGH   (0x0B)
#define USB_PHYCR3_3_FORCE_IDPULLUP   (0x20)
#define USB_PHYCR3_3_UTMI_MUXSEL    (0x10)
/* USB_PHYCR4_0 */
#define USB_PHYCR4_0_FORCE_USB_CLKOFF    (0x20)
#define USB_PHYCR4_0_FORCE_AUX_EN    (0x80)
#define USB_PHYCR4_0_FORCE_OTG_PROBE    (0x40)
#define USB_PHYCR4_0_FORCE_BVALID       (0x10)
#define USB_PHYCR4_0_FORCE_IDDIG        (0x08)
#define USB_PHYCR4_0_FORCE_VBUSVALID    (0x04)
#define USB_PHYCR4_0_FORCE_SESSEND      (0x02)
#define USB_PHYCR4_0_FORCE_AVALID       (0x01)
/* USB_PHYCR4_1 */
/* USB_PHYCR4_2 */
/* USB_PHYCR4_3 */
#define USB_PHYCR4_3_UART_MODE    (0x08)
/* USB_PHYCR5_0 */
/* USB_PHYCR5_1 */
#define USB_PHYIR5_1_RG_SQTH0 (0x03)
#define USB_PHYIR5_1_RG_RG_RCVB0 (0x20)
#define USB_PHYIR5_1_RG_RG_SQB0 (0x40)
/* USB_PHYCR5_2 */
/* USB_PHYCR5_3 */
#define USB_PHYCR5_3_TERM_SELECT    (0x04)
#define USB_PHYCR5_3_XCVR_SELECT_MASK  (0x30)
#define USB_PHYCR5_3_XCVR_SELECT_L   (0x10)
#define USB_PHYCR5_3_DP_PULL_DOWN   (0x40)
#define USB_PHYCR5_3_DM_PULL_DOWN   (0x80)
#define USB_PHYCR5_3_DP_DM_PULL_DOWN  (0xC0)
#define USB_PHYCR5_3_OP_MODE     (0x01)
#define USB_PHYCR5_3_SUSPENDM   (0x08)
/* USB_PHYIR1_0 */
#define USB_PHYIR1_0_IDPULLUP     (0x01)
#define USB_PHYIR1_0_DRVVBUS     (0x02)
#define USB_PHYIR1_0_TX_VALID     (0x04)
/* USB_PHYIR1_1 */
#define USB_PHYIR1_1_RG_DM1_ABIST_SELE   (0x0A)
#define USB_PHYIR1_1_RG_EN_PD_DM   (0x20)

/* USB_PHYIR1_2 */
/* USB_PHYIR1_3 */
/* USB_PHYIR2_0 */
/* USB_PHYIR2_1 */
#define  USB_PHYIR2_1_RG_USB11_TMODE_EN0  (0x08)
/* USB_PHYIR2_2 */
/* USB_PHYIR2_3 */
/* USB_PHYIR3_0 */
#define USB_PHYIR3_0_LINESTATE_DP    (0x40)
#define USB_PHYIR3_0_LINESTATE_DM    (0x80)
/* USB_PHYIR3_1 */
/* USB_PHYIR3_2 */
/* USB_PHYIR3_3 */
/* USB_PHYIR4_0 */
#define USB_PHYIR4_0_HS_TERMC_MASK  (0x70)
#define USB_PHYIR4_0_RG_CALIB_SELE0_ENABLE  (0xC0)
#define USB_PHYIR4_0_RG_CALIB_SELE0_DISABLE  (0x40)
#define USB_PHYIR4_0_RG_TX_I_TRIM0  (0x03)
#define USB_PHYIR4_0_RG_CALIB_SELE0   (0x50)
/* USB_PHYIR4_1 */
/* USB_PHYIR4_2 */
#define USB_PHYIR4_2_RG_HSTX_SRCTRL0    (0x01)
#define USB_PHYIR4_2_RG_HSTX_DBIST0  (0xC0)
/* USB_PHYIR4_3 */
#define USB_PHYIR4_3_DEGLICH     (0xAA)
#define USB_PHYIR4_3_RG_DISCD   (0x02)
/* USB_PHYIR5_0 */
/* USB_PHYIR5_1 */
#define USB_PHYIR5_1_RG_SQTH0  (0x03)
/* USB_PHYIR5_2 */
/* USB_PHYIR5_3 */
/* USB_PHYIR6_0 */
#define USB_PHYIR6_0_RG_DP_100K_EN  (0x01)
#define USB_PHYIR6_0_RG_DM_100K_EN  (0x02)
/* USB_PHYIR6_1 */
#define USB_PHYIR6_1_BGR_DIV_L    (0x10)
#define USB_PHYIR6_1_RG_OTG_VBUSTH  (0x02)
#define USB_PHYIR6_1_VBUSCMP_EN    (0x04)
/* USB_PHYIR6_2 */
/* USB_PHYIR6_3 */


//////////////////////////////////////////////////////
#define USB_PHYCR2_3_HS_TERMC     (0x08)
#define USB_PHYCR4_1_FORCE_BGR_ON   (0x4F)


#else   // Old PHY  (MT6235 / MT6238 /MT6516 /MT6268A / MT6253T / MT6236)


/* USB phy register */
#define USB_PHYCR1_0    (USB_BASE+0x600) /* RW */
#define USB_PHYCR1_1    (USB_BASE+0x601) /* RW */
#define USB_PHYCR1_2    (USB_BASE+0x602) /* RW */
#define USB_PHYCR1_3    (USB_BASE+0x603) /* RW */

#define USB_PHYCR2_0    (USB_BASE+0x604) /* RW */
#define USB_PHYCR2_1    (USB_BASE+0x605) /* RW */
#define USB_PHYCR2_2    (USB_BASE+0x606) /* RW */
#define USB_PHYCR2_3    (USB_BASE+0x607) /* RW */

#define USB_PHYCR3_0    (USB_BASE+0x608) /* RW */
#define USB_PHYCR3_1    (USB_BASE+0x609) /* RW */
#define USB_PHYCR3_2    (USB_BASE+0x60A) /* RW */
#define USB_PHYCR3_3    (USB_BASE+0x60B) /* RW */

#define USB_PHYCR4_0    (USB_BASE+0x60C) /* RW */
#define USB_PHYCR4_1    (USB_BASE+0x60D) /* RW */
#define USB_PHYCR4_2    (USB_BASE+0x60E) /* RW */
#define USB_PHYCR4_3    (USB_BASE+0x60F) /* RW */

#define USB_PHYCR5_0    (USB_BASE+0x610) /* RW */
#define USB_PHYCR5_1    (USB_BASE+0x611) /* RW */
#define USB_PHYCR5_2    (USB_BASE+0x612) /* RW */
#define USB_PHYCR5_3    (USB_BASE+0x613) /* RW */

#define USB_PHYIR1_0    (USB_BASE+0x614) /* RW */
#define USB_PHYIR1_1    (USB_BASE+0x615) /* RW */
#define USB_PHYIR1_2    (USB_BASE+0x616) /* RW */
#define USB_PHYIR1_3    (USB_BASE+0x617) /* RW */

#define USB_PHYIR2_0    (USB_BASE+0x618) /* RW */
#define USB_PHYIR2_1    (USB_BASE+0x619) /* RW */
#define USB_PHYIR2_2    (USB_BASE+0x61A) /* RW */
#define USB_PHYIR2_3    (USB_BASE+0x61B) /* RW */


/* USB_PHY ALL*/
#define USB_PHY_CLEAR_MASK     (0x00)

/* USB_PHYCR1_0 */
#define USB_PHYCR1_0_PLL_EN     (0x80)

/* USB_PHYCR1_1 */
#define USB_PHYCR1_1_PLL_CCP_SET    (0x30)
#define USB_PHYCR1_1_PLL_CCP_MASK    (0xF0)

/* USB_PHYCR1_2 */
#define USB_PHYCR1_2_PLL_VCOG_H    (0x08)
#define USB_PHYCR1_2_HS_RCVB     (0x40)

/* USB_PHYCR2_0 */
#define USB_PHYCR2_0_HS_SQB_MASK    (0x0F)
#define USB_PHYCR2_0_HS_SQD_MASK    (0xF0)
#define USB_PHYCR2_0_HS_SQD_SET    (0xA0)

/* USB_PHYCR2_2 */
#define USB_PHYCR2_2_FORCE_DATA_IN   (0x40)
#define USB_PHYCR2_2_FORCE_TX_VALID   (0x20)
#define USB_PHYCR2_2_HS_DISCP     (0x01)

/* USB_PHYCR2_3 */
#define USB_PHYCR2_3_HS_TERMC_MASK   (0x1F)
#define USB_PHYCR2_3_FORCE_DP_PULLDOWN  (0x20)
#define USB_PHYCR2_3_FORCE_DM_PULLDOWN  (0x40)
#define USB_PHYCR2_3_FORCE_DP_DM_PULLDOWN (0x60)
#define USB_PHYCR2_3_FORCE_DRV_VBUS   (0x80)
#define USB_PHYCR2_3_HS_TERMC     (0x08)
#define USB_PHYCR2_3_HS_TERMC2     (0x0B)
#define USB_PHYCR2_3_FORCE_SUSPENDM      (0x04)

/* USB_PHYCR3_0 */
#define USB_PHYCR3_0_IADJ_MASK    (0x07)
#define USB_PHYCR3_0_IADJ_MASK2    (0x04)
#define USB_PHYCR3_0_IADJ_RESISTER_SET      (0x06)
#define USB_PHYCR3_0_CLEAR_MASK    (0x00)

/* USB_PHYCR3_2 */
#define USB_PHYCR3_2_TEST_CTRL_MASK   (0x0F)
#define USB_PHYCR3_2_TEST_CTRL1_SET   (0x02)
#define USB_PHYCR3_2_TEST_CTRL2_SET   (0x04)
#define USB_PHYCR3_2_CLK_MODE     (0x80)

/* USB_PHYCR4_1 */
#define USB_PHYCR4_1_BGR_BGR_EN    (0x01)
#define USB_PHYCR4_1_BGR_CLK_EN    (0x02)
#define USB_PHYCR4_1_BGR_I_SRC_EN    (0x04)
#define USB_PHYCR4_1_BGR_CHIP_EN    (0x08)
#define USB_PHYCR4_1_BGR_SELPH    (0x10)
#define USB_PHYCR4_1_BGR_DIV_L    (0x40)
#define USB_PHYCR4_1_FORCE_BGR_ON   (0x4F)

/* USB_PHYCR5_0 */
#define USB_PHYCR5_0_VBUSCMP_EN    (0x80)
#define USB_PHYCR5_0_CDR_FILT     (0x02)

/* USB_PHYCR5_2 */
#define USB_PHYCR5_2_FORCE_OP_MODE   (0x01)
#define USB_PHYCR5_2_FORCE_TERM_SELECT  (0x02)
#define USB_PHYCR5_2_FORCE_SUSPENDM   (0x04)
#define USB_PHYCR5_2_FORCE_XCVR_SELECT  (0x08)
#define USB_PHYCR5_2_FORCE_DP_HIGH   (0x0B)
#define USB_PHYCR5_2_FORCE_IDPULLUP   (0x80)
#define USB_PHYCR5_2_UTMI_MUXSEL    (0x40)

/* USB_PHYCR5_3 */
#define USB_PHYCR5_3_TERM_SELECT    (0x04)
#define USB_PHYCR5_3_XCVR_SELECT_MASK  (0x30)
#define USB_PHYCR5_3_XCVR_SELECT_L   (0x10)
#define USB_PHYCR5_3_DP_PULL_DOWN   (0x40)
#define USB_PHYCR5_3_DM_PULL_DOWN   (0x80)
#define USB_PHYCR5_3_DP_DM_PULL_DOWN  (0xC0)
#define USB_PHYCR5_3_OP_MODE     (0x01)
#define USB_PHYCR5_3_SUSPENDM   (0x08)

/* USB_PHYIR1_0 */
#define USB_PHYIR1_0_IDPULLUP     (0x01)
#define USB_PHYIR1_0_DRVVBUS     (0x02)
#define USB_PHYIR1_0_TX_VALID     (0x04)

/* USB_PHYIR1_3 */
#define USB_PHYIR1_3_LINESTATE_DP    (0x40)
#define USB_PHYIR1_3_LINESTATE_DM    (0x80)

/* USB_PHYIR2_3 */
#define USB_PHYIR2_3_FORCE_USB_CLKOFF    (0x20)
#define USB_PHYIR2_3_FORCE_AUX_EN    (0x80)


#endif // PHY


//USB IP V3
#if (defined(DRV_USB_PHY_COST_DOWN))
#define  USB_LINE_STATE  USB_PHYIR3_0
#endif



//USB IP V3
#elif (defined(DRV_USB_IP_V2))

#define OTG_INT_STAT    (USB_BASE+0x10)
#define OTG_INT_EN     (USB_BASE+0x14)
#define OTG_STATUS     (USB_BASE+0x18)
#define OTG_CTRL     (USB_BASE+0x1c)

#define USB_FM_PKT_NUML    (USB_BASE+0x20)
#define USB_FM_PKT_NUMH    (USB_BASE+0x24)
#define USB_FM_ERR_STAT    (USB_BASE+0x28)
#define USB_FM_CTL     (USB_BASE+0x2c)
#define USB_FM_PKT_CNTL    (USB_BASE+0x30)
#define USB_FM_PKT_CNTH    (USB_BASE+0x34)
#define USB_FM_TIMEOUT    (USB_BASE+0x38)
#define USB_FM_STATUS    (USB_BASE+0x3c)
#define USB_FM_ADDITNL_STAT   (USB_BASE+0x50)
#define USB_FM_ENDPT    (USB_BASE+0x68)
#define USB_FM_INT_MASK    (USB_BASE+0x6c)
#define USB_PHY_EXTRA    (USB_BASE+0x70)

#define USB_INT_STAT    (USB_BASE+0x80)
#define USB_INT_ENB     (USB_BASE+0x84)
#define USB_ERR_STAT    (USB_BASE+0x88)
#define USB_ERR_ENB     (USB_BASE+0x8c)
#define USB_STAT     (USB_BASE+0x90)
#define USB_CTL      (USB_BASE+0x94)
#define USB_ADDR     (USB_BASE+0x98)
#define USB_BDT_PAGE_01    (USB_BASE+0x9c)
#define USB_BDT_PAGE_02    (USB_BASE+0xb0)
#define USB_BDT_PAGE_03    (USB_BASE+0xb4)
#define USB_FRM_NUML    (USB_BASE+0xa0)
#define USB_FRM_NUMH    (USB_BASE+0xa4)
#define USB_TOKEN     (USB_BASE+0xa8)
#define USB_SOF_THLD    (USB_BASE+0xac)
#define USB_ENDPT_CTL_BASE   (USB_BASE+0xc0)
#define USB_ENDPT_CTL(n)   (USB_ENDPT_CTL_BASE+4*n)

#define USB_DMA_ENB     (USB_BASE+0x410)
#define USB_DMA_DIS     (USB_BASE+0x414)
#define USB_DMA_ADDR_CNTER_CLR  (USB_BASE+0x418)
#define USB_DMA_FM_SELECT   (USB_BASE+0x41c)
#define USB_SOFT_RST    (USB_BASE+0x420)
#define USB_PHY_CTL     (USB_BASE+0x450)

#define USB_FIFO_RX0_EVEN   (USB_BASE+0x200)
#define USB_FIFO_RX0_ODD   (USB_BASE+0x208)
#define USB_FIFO_TX0_EVEN   (USB_BASE+0x210)
#define USB_FIFO_TX0_ODD   (USB_BASE+0x218)
#define USB_FIFO_RX1    (USB_BASE+0x220)
#define USB_FIFO_TX1    (USB_BASE+0x260)
#define USB_FIFO_RX2    (USB_BASE+0x2a0)
#define USB_FIFO_TX2    (USB_BASE+0x2e0)
#define USB_FIFO_RX3    (USB_BASE+0x320)
#define USB_FIFO_TX3    (USB_BASE+0x328)


#define USB_BDT_RX0_EVEN   (USB_BASE+0x330)
#define USB_BDT_RX0_ODD    (USB_BASE+0x338)
#define USB_BDT_TX0_EVEN   (USB_BASE+0x340)
#define USB_BDT_TX0_ODD    (USB_BASE+0x348)
#define USB_BDT_RX1     (USB_BASE+0x350)
#define USB_BDT_TX1     (USB_BASE+0x358)
#define USB_BDT_RX2     (USB_BASE+0x360)
#define USB_BDT_TX2     (USB_BASE+0x368)
#define USB_BDT_RX3     (USB_BASE+0x370)
#define USB_BDT_TX3     (USB_BASE+0x378)

/* VUSB Endpoint control register masks */
/* Define the bits within the endpoint control register */
#define VUSB_ENDPT_HSHK_BIT   (0x01)
#define VUSB_ENDPT_STALL_BIT   (0x02)
#define VUSB_ENDPT_TX_EN_BIT  (0x04)
#define VUSB_ENDPT_RX_EN_BIT  (0x08)
#define VUSB_ENDPT_CTL_EP_CTL_DIS (0x10)
#define VUSB_ENDPT_CTL_RETRY_DIS (0x40)
#define VUSB_ENDPT_CTL_HOST_WO_HUB (0x80)

#define VUSB_ENDPT_DISABLE   (0x00)
#define VUSB_ENDPT_CONTROL   (VUSB_ENDPT_HSHK_BIT | VUSB_ENDPT_TX_EN_BIT | \
          VUSB_ENDPT_RX_EN_BIT | VUSB_ENDPT_CTL_RETRY_DIS)
#define VUSB_ENDPT_BULK_RX   (VUSB_ENDPT_HSHK_BIT | VUSB_ENDPT_RX_EN_BIT | \
          VUSB_ENDPT_CTL_EP_CTL_DIS | VUSB_ENDPT_CTL_RETRY_DIS)
#define VUSB_ENDPT_BULK_TX   (VUSB_ENDPT_HSHK_BIT | VUSB_ENDPT_TX_EN_BIT | \
          VUSB_ENDPT_CTL_EP_CTL_DIS | VUSB_ENDPT_CTL_RETRY_DIS)
#define VUSB_ENDPT_BULK_BIDIR  (VUSB_ENDPT_HSHK_BIT | VUSB_ENDPT_TX_EN_BIT | \
          VUSB_ENDPT_RX_EN_BIT | VUSB_ENDPT_CTL_EP_CTL_DIS | VUSB_ENDPT_CTL_RETRY_DIS)
#define VUSB_ENDPT_ISO_RX   (VUSB_ENDPT_RX_EN_BIT | VUSB_ENDPT_CTL_EP_CTL_DIS | \
          VUSB_ENDPT_CTL_RETRY_DIS)
#define VUSB_ENDPT_ISO_TX   (VUSB_ENDPT_TX_EN_BIT | VUSB_ENDPT_CTL_EP_CTL_DIS | \
          VUSB_ENDPT_CTL_RETRY_DIS)
#define VUSB_ENDPT_ISO_BIDIR  (VUSB_ENDPT_RX_EN_BIT | VUSB_ENDPT_TX_EN_BIT | \
          VUSB_ENDPT_CTL_EP_CTL_DIS | VUSB_ENDPT_CTL_RETRY_DIS)

/* VUSB Control register masks */
#define  VUSB_CTL_USB_EN   (0x01)
#define  VUSB_CTL_SOF_EN   (0x01)
#define  VUSB_CTL_ODD_RST   (0x02)
#define  VUSB_CTL_RESUME   (0x04)
#define  VUSB_CTL_HOST_MODE_EN  (0x08)
#define  VUSB_CTL_RESET    (0x10)
#define  VUSB_CTL_TOKEN_BUSY  (0x20)
#define  VUSB_CTL_TXD_SUSPEND  (0x20)
#define  VUSB_CTL_SINGLE_ENDED_0 (0x40)
#define  VUSB_CTL_JSTATE   (0x80)

/* VUSB Interrupt status register masks */
#define VUSB_INT_STAT_RESET   (0x01)
#define VUSB_INT_STAT_ERROR   (0x02)
#define VUSB_INT_STAT_SOF   (0x04)
#define VUSB_INT_STAT_TOKEN_DONE (0x08)
#define VUSB_INT_STAT_SLEEP   (0x10)
#define VUSB_INT_STAT_RESUME  (0x20)
#define VUSB_INT_STAT_ATTACH  (0x40)
#define VUSB_INT_STAT_STALL   (0x80)

/* VUSB Interrupt enable register masks*/
#define VUSB_INT_ENB_RESET   (0x01)
#define VUSB_INT_ENB_ERROR   (0x02)
#define VUSB_INT_ENB_SOF   (0x04)
#define VUSB_INT_ENB_TOKEN_DONE  (0x08)
#define VUSB_INT_ENB_SLEEP   (0x10)
#define VUSB_INT_ENB_RESUME   (0x20)
#define VUSB_INT_ENB_ATTACH   (0x40)
#define VUSB_INT_ENB_STALL   (0x80)

/* VUSB Fast mode error status register masks */
#define USB_FM_ERR_STA_OVR_FLW  (0x80)
#define VUSB_FM_ERR_STAT_TOKEN_DONE (0x40)
#define VUSB_FM_ERR_SUC_ERR   (0x04)
#define VUSB_FM_ERR_NAK_ERR   (0x02)
#define VUSB_FM_ERR_SHORT_ERR  (0x01)

/* VUSB Fast mode control register masks */
#define VUSB_FM_CTL_FMENB   (0x1)
#define VUSB_FM_CTL_SUCERREN  (0x8)
#define VUSB_FM_CTL_EP_RX_ODD_SHIFT  1
#define VUSB_FM_CTL_EP_TX_ODD_SHIFT  2
#define VUSB_FM_CTL_EP_TOG_BIT_SHIFT 6

/* VUSB Fast mode endpoint register masks*/
#define VUSB_FM_EP_TX    (0x10)
#define VUSB_FM_EP_TX_RES   (0x80)
#define VUSB_FM_EP_ENDPT_MASK  (0x0f)

/* VUSB FM DMA index */
#define VUSB_FM_DMA_RX1    0
#define VUSB_FM_DMA_TX1    1
#define VUSB_FM_DMA_RX2    2
#define VUSB_FM_DMA_TX2    3

/* VUSB EXTRA register masks*/
#define VUSB_PHY_RESUME_INT   (0x80)
#define VUSB_PHY_RESUME_INT_ENB  (0x04)
#define VUSB_PHY_SUSPEND   (0x02)

/* VUSB SOFT RST register masks*/
#define VUSB_SOFT_RST_EN   (0x01)

/* VUSB BDT masks */
#define VUSB_BDT_OWNS_BIT   (1 << 7)
#define VUSB_BDT_DATA01_BIT   (1 << 6)
#define VUSB_BDT_KEEP_BIT   (1 << 5)
#define VUSB_BDT_NINC_BIT   (1 << 4)
#define VUSB_BDT_DTS_BIT   (1 << 3)
#define VUSB_BDT_STALL_BIT   (1 << 2)
#define VUSB_BDT_BC_SHIFT   16
#define VUSB_BDT_DATA01_SHIFT  6
#define VUSB_BDT_BC_MASK   0x03ff0000

#define VUSB_BDT_PID_MASKS   (0x3C)
#define VUSB_BDT_NAK_PID   (0x28)
#define VUSB_BDT_ERROR_PID   (0x3c)
#define VUSB_BDT_STALL_PID   (0x38)
#define VUSB_BDT_BUS_TIMEOUT_PID (0x00)


/* OTG Interrupt Status Register Bit Masks */
#define  OTG_INT_STATUS_A_VBUS    (0x01)
#define  OTG_INT_STATUS_B_SESS_END   (0x04)
#define  OTG_INT_STATUS_SESS_VLD   (0x08)
#define  OTG_INT_STATUS_LINE_STATE_CHANGE (0x20)
#define  OTG_INT_STATUS_1_MSEC    (0x40)
#define  OTG_INT_STATUS_ID     (0x80)

/* OTG Interrupt Enable Register Bit Masks */
#define  OTG_INT_ENABLE_A_VBUS    (0x01)
#define  OTG_INT_ENABLE_B_SESS_END   (0x04)
#define  OTG_INT_ENABLE_SESS_VLD   (0x08)
#define  OTG_INT_ENABLE_1_MSEC    (0x40)
#define  OTG_INT_ENABLE_ID     (0x80)

/*OTG Status register masks*/
#define  OTG_STATUS_A_VBUS     (0x01)
#define  OTG_STATUS_B_SESS_END    (0x04)
#define  OTG_STATUS_SESS_VLD    (0x08)
#define  OTG_STATUS_LINE_STATE_CHANGE  (0x20)
#define  OTG_STATUS_1_MSEC     (0x40)
#define  OTG_STATUS_ID      (0x80)

/*OTG Control register masks*/
#define  OTG_CTL_VBUS_DSCHG     (0x01)
#define  OTG_CTL_VBUS_CHG     (0x02)
#define  OTG_CTL_OTG_ENABLE     (0x04)
#define  OTG_CTL_VBUS_ON     (0x08)
#define  OTG_CTL_DM_LOW      (0x10)
#define  OTG_CTL_DP_LOW      (0x20)
#define  OTG_CTL_DM_HIGH     (0x40)
#define  OTG_CTL_DP_HIGH     (0x80)


#define  OTG_CTL_RESET_DP_DM    (~(OTG_CTL_DM_LOW|OTG_CTL_DP_LOW|OTG_CTL_DM_HIGH|OTG_CTL_DP_HIGH))
#define  OTG_CTL_J_STATE     (OTG_CTL_DP_HIGH|OTG_CTL_DM_LOW)
#define  OTG_CTL_K_STATE     (OTG_CTL_DM_HIGH|OTG_CTL_DP_LOW)
#define  OTG_CTL_DP_DM_HIGH    (OTG_CTL_DP_HIGH|OTG_CTL_DM_HIGH)
#define  OTG_CTL_DP_DM_LOW    (OTG_CTL_DP_LOW|OTG_CTL_DM_LOW)


/* Token register masks */
#define  VUSB_TOKEN_ENDPT     (0x0f)
#define  VUSB_TOKEN_PID      (0xf0)
#define  VUSB_TOKEN_OUT      (0x10)
#define  VUSB_TOKEN_IN      (0x90)
#define  VUSB_TOKEN_SETUP     (0xd0)

#elif (defined(DRV_USB_IP_V1))

#define USB_FADDR  (USB_BASE+0x0)
#define USB_POWER  (USB_BASE+0x1)
#define USB_INTRIN1  (USB_BASE+0x2) /*status, read only*/
#define USB_INTRIN2  (USB_BASE+0x3)  /*status, read only*/
#define USB_INTROUT1 (USB_BASE+0x4)  /*status, read only*/
#define USB_INTROUT2 (USB_BASE+0x5)  /*status, read only*/
#define USB_INTRUSB  (USB_BASE+0x6)  /*status, read only*/
#define USB_INTRIN1E (USB_BASE+0x7)
#define USB_INTRIN2E (USB_BASE+0x8)
#define USB_INTROUT1E (USB_BASE+0x9)
#define USB_INTROUT2E (USB_BASE+0xa)
#define USB_INTRUSBE (USB_BASE+0xb)
#define USB_FRAME1  (USB_BASE+0xc) /*read only*/
#define USB_FRAME2  (USB_BASE+0xd) /*read only*/ /*Max Frame length = 11 bits*/
#define USB_INDEX  (USB_BASE+0xe) /*RW, 4bit available*/
#define USB_RSTCTRL  (USB_BASE+0xf)

#define USB_INMAXP  (USB_BASE+0x10) /*RW*/
#define USB_CSR0  (USB_BASE+0x11)
#define USB_INCSR1  (USB_BASE+0x11)
#define USB_INCSR2  (USB_BASE+0x12)
#define USB_OUTMAXP  (USB_BASE+0x13) /*RW*/
#define USB_OUTCSR1  (USB_BASE+0x14)
#define USB_OUTCSR2  (USB_BASE+0x15)
#define USB_COUNT0  (USB_BASE+0x16) /*RO, EP0 only*/
#define USB_OUTCOUNT1 (USB_BASE+0x16)
#define USB_OUTCOUNT2 (USB_BASE+0x17) /*RO,11bits*/

#define USB_EP0   (USB_BASE+0x20) /*4 byte as 1 queue*/

#define USB_ENABLE  (USB_BASE+0x230)
#define USB_SLEWCON (USB_BASE+0x240)

/*USB_FADDR*/
#define USB_FADDR_ADDRMASK  (0x7f)   /*RO*/
#define USB_FADDR_UPDATE  (0x80)   /*RW*/

/*USB_POWER*/
#define USB_POWER_SETSUSPEND (0x01)   /*RW*/
#define USB_POWER_SUSPENDSTAT (0x02)   /*RO*/ /*Read clear by the intr. register*/
#define USB_POWER_RESUME  (0x04)   /*RW*/
#define USB_POWER_RESET   (0x08)   /*RO*/
#define USB_POWER_SWRSTENAB  (0x10)   /*RW*/
#define USB_POWER_ISOUPDATE  (0x80)   /*RW*/

/*USB_RSTCTRL*/
#define USB_RSTCTRL_SWRST  (0x80)  /*RW*/

/*USB_INTRIN1, USB_INTRIN2 is not needed*/
#define USB_INTRIN1_EP0   (0x01)   /*RO*/
#define USB_INTRIN1_EP1   (0x02)   /*RO*/
#define USB_INTRIN1_EP2   (0x04)   /*RO*/
#define USB_INTRIN1_EP3   (0x08)   /*RO*/

/*USB_INTROUT1, USB_INTROUT2 is not needed*/
#define USB_INTROUT1_EP1  (0x02)   /*RO*/
#define USB_INTROUT1_EP2  (0x04)   /*RO*/

/*USB_INTRUSB*/
#define USB_INTRUSB_SUSPEND  (0x01)   /*RO*/
#define USB_INTRUSB_RESUME  (0x02)   /*RO*/
#define USB_INTRUSB_RESET  (0x04)   /*RO*/
#define USB_INTRUSB_SOF   (0x08)   /*RO*/

/*USB_INTRIN1E, USB_INTRIN2E is not needed*/
#define USB_INTRIN1E_EPEN  (0x01)   /*RW*/

/*USB_INTROUT1E, USB_INTROUT2E is not needed*/
#define USB_INTROUT1E_EPEN  (0x01)   /*RW*/

/*USB_INTRUSBE*/
#define USB_INTRUSBE_SUSPEND (0x01)   /*RW*/
#define USB_INTRUSBE_RESUME  (0x02)   /*RW*/
#define USB_INTRUSBE_RESET  (0x04)   /*RW*/
#define USB_INTRUSBE_SOF  (0x08)   /*RW*/

/*USB_INMAXP*/
#define USB_INMAXP_MASK   (0xff)   /*RW*/

/*USB_OUTMAXP*/
#define USB_OUTMAXP_MASK  (0xff)   /*RW*/

/*USB_CSR0*/
#define USB_CSR0_OUTPKTRDY    (0x01)   /*RO*/
#define USB_CSR0_INPKTRDY    (0x02)   /*RW,AC*/
#define USB_CSR0_SENTSTALL    (0x04)   /*RC*/
#define USB_CSR0_DATAEND    (0x08)   /*WO,AC*/
#define USB_CSR0_SETUPEND    (0x10)   /*RO*/
#define USB_CSR0_SENDSTALL    (0x20)   /*WO,AC*/
#define USB_CSR0_SERVICEDOUTPKTRDY  (0x40)   /*WO,AC*/
#define USB_CSR0_SERVICESETUPEND  (0x80)   /*WO,AC*/

/*USB_INCSR1*/
#define USB_INCSR1_INPKTRDY    (0x01)   /*RW*/
#define USB_INCSR1_FIFONOTEMPTY   (0x02)   /*RC*/
#define USB_INCSR1_UNDERRUN    (0x04)   /*RC*/
#define USB_INCSR1_FLUSHFIFO   (0x08)   /*WO*/
#define USB_INCSR1_SENDSTALL   (0x10)   /*RW*/
#define USB_INCSR1_SENTSTALL   (0x20)   /*RC*/
#define USB_INCSR1_CLRDATATOG   (0x40)   /*WO*/

/*USB_INCSR2*/
#define USB_INCSR2_FRCDATATOG   (0x08)   /*RW*/
#define USB_INCSR2_DMAENAB    (0x10)   /*RW*/
#define USB_INCSR2_MODE     (0x20)   /*RW*/
#define USB_INCSR2_ISO     (0x40)   /*RW*/
#define USB_INCSR2_AUTOSET    (0x80)   /*RW*/

/*USB_OUTCSR1*/
#define USB_OUTCSR1_OUTPKTRDY   (0x01)   /*RC*/
#define USB_OUTCSR1_FIFOFULL   (0x02)   /*R,AC*/
#define USB_OUTCSR1_OVERRUN    (0x04)   /*RC*/
#define USB_OUTCSR1_DATAERROR   (0x08)   /*RO*/
#define USB_OUTCSR1_FLUSHFIFO   (0x10)   /*WO,AC*/
#define USB_OUTCSR1_SENDSTALL   (0x20)   /*RW*/
#define USB_OUTCSR1_SENTSTALL   (0x40)   /*RC*/
#define USB_OUTCSR1_CLRDATATOG   (0x80)   /*WO*/

/*USB_OUTCSR2*/
#define USB_OUTCSR2_DMAMODE    (0x10)   /*RW*/
#define USB_OUTCSR2_DMAENAB    (0x20)   /*RW*/
#define USB_OUTCSR2_ISO     (0x40)   /*RW*/
#define USB_OUTCSR2_AUTOCLEAR   (0x80)   /*RW*/

/*USB_ENABLE*/
#define USB_ENABLE_EN     (0x1)
#define USB_ENABLE_DIS     (0x0)

/* USB_SLEWCON */
#define USB_SLEWCON_PUB     (0x01)

#endif

//======================USB PHY =============================//
#define DRV_USB_PHY_T14  // USB_FPGA_DVT : PHY version choosed by DE's setting

#if (defined(DRV_USB_PHY_U65))
/* USB phy register */
#define USB_U2PHYAC0_0    (USB_SIFSLV_BASE+0x800) /* RW */
#define USB_U2PHYAC0_1    (USB_SIFSLV_BASE+0x801) /* RW */
#define USB_U2PHYAC0_2    (USB_SIFSLV_BASE+0x802) /* RW */
#define USB_U2PHYAC0_3    (USB_SIFSLV_BASE+0x803) /* RW */

#define USB_U2PHYAC1_0    (USB_SIFSLV_BASE+0x804) /* RW */
#define USB_U2PHYAC1_1    (USB_SIFSLV_BASE+0x805) /* RW */
#define USB_U2PHYAC1_2    (USB_SIFSLV_BASE+0x806) /* RW */
#define USB_U2PHYAC1_3    (USB_SIFSLV_BASE+0x807) /* RW */

#define USB_U2PHYACR0_0    (USB_SIFSLV_BASE+0x810) /* RW */
#define USB_U2PHYACR0_1    (USB_SIFSLV_BASE+0x811) /* RW */
#define USB_U2PHYACR0_2    (USB_SIFSLV_BASE+0x812) /* RW */
#define USB_U2PHYACR0_3    (USB_SIFSLV_BASE+0x813) /* RW */

#define USB_U2PHYACR1_0    (USB_SIFSLV_BASE+0x814) /* RW */
#define USB_U2PHYACR1_1    (USB_SIFSLV_BASE+0x815) /* RW */
#define USB_U2PHYACR1_2    (USB_SIFSLV_BASE+0x816) /* RW */
#define USB_U2PHYACR1_3    (USB_SIFSLV_BASE+0x817) /* RW */

#define USB_U2PHYACR2_0    (USB_SIFSLV_BASE+0x818) /* RW */
#define USB_U2PHYACR2_1    (USB_SIFSLV_BASE+0x819) /* RW */
#define USB_U2PHYACR2_2    (USB_SIFSLV_BASE+0x81A) /* RW */
#define USB_U2PHYACR2_3    (USB_SIFSLV_BASE+0x81B) /* RW */

#define USB_U2PHYACR3_0    (USB_SIFSLV_BASE+0x81C) /* RW */
#define USB_U2PHYACR3_1    (USB_SIFSLV_BASE+0x81D) /* RW */
#define USB_U2PHYACR3_2    (USB_SIFSLV_BASE+0x81E) /* RW */
#define USB_U2PHYACR3_3    (USB_SIFSLV_BASE+0x81F) /* RW */

#define USB_U2PHYACR4_0    (USB_SIFSLV_BASE+0x820) /* RW */
#define USB_U2PHYACR4_1    (USB_SIFSLV_BASE+0x821) /* RW */
#define USB_U2PHYACR4_2    (USB_SIFSLV_BASE+0x822) /* RW */
#define USB_U2PHYACR4_3    (USB_SIFSLV_BASE+0x823) /* RW */

#define USB_U2PHYACHG_0    (USB_SIFSLV_BASE+0x824) /* RW */
#define USB_U2PHYACHG_1    (USB_SIFSLV_BASE+0x825) /* RW */
#define USB_U2PHYACHG_2    (USB_SIFSLV_BASE+0x826) /* RW */
#define USB_U2PHYACHG_3    (USB_SIFSLV_BASE+0x827) /* RW */

#define USB_U2PHYDCR0_0    (USB_SIFSLV_BASE+0x860) /* RW */
#define USB_U2PHYDCR0_1    (USB_SIFSLV_BASE+0x861) /* RW */
#define USB_U2PHYDCR0_2    (USB_SIFSLV_BASE+0x862) /* RW */
#define USB_U2PHYDCR0_3    (USB_SIFSLV_BASE+0x863) /* RW */

#define USB_U2PHYDCR1_0    (USB_SIFSLV_BASE+0x864) /* RW */
#define USB_U2PHYDCR1_1    (USB_SIFSLV_BASE+0x865) /* RW */
#define USB_U2PHYDCR1_2    (USB_SIFSLV_BASE+0x866) /* RW */
#define USB_U2PHYDCR1_3    (USB_SIFSLV_BASE+0x867) /* RW */

#define USB_U2PHYDTM0_0    (USB_SIFSLV_BASE+0x868) /* RW */
#define USB_U2PHYDTM0_1    (USB_SIFSLV_BASE+0x869) /* RW */
#define USB_U2PHYDTM0_2    (USB_SIFSLV_BASE+0x86A) /* RW */
#define USB_U2PHYDTM0_3    (USB_SIFSLV_BASE+0x86B) /* RW */

#define USB_U2PHYDTM1_0    (USB_SIFSLV_BASE+0x86C) /* RW */
#define USB_U2PHYDTM1_1    (USB_SIFSLV_BASE+0x86D) /* RW */
#define USB_U2PHYDTM1_2    (USB_SIFSLV_BASE+0x86E) /* RW */
#define USB_U2PHYDTM1_3    (USB_SIFSLV_BASE+0x86F) /* RW */

#define USB_U2PHYDMON0_0   (USB_SIFSLV_BASE+0x870) /* RW */
#define USB_U2PHYDMON0_1   (USB_SIFSLV_BASE+0x871) /* RW */
#define USB_U2PHYDMON0_2   (USB_SIFSLV_BASE+0x872) /* RW */
#define USB_U2PHYDMON0_3   (USB_SIFSLV_BASE+0x873) /* RW */

#define USB_U1PHYCR0_0    (USB_SIFSLV_BASE+0x8C0) /* RW */
#define USB_U1PHYCR0_1    (USB_SIFSLV_BASE+0x8C1) /* RW */
#define USB_U1PHYCR0_2    (USB_SIFSLV_BASE+0x8C2) /* RW */
#define USB_U1PHYCR0_3    (USB_SIFSLV_BASE+0x8C3) /* RW */

#define USB_U1PHYCR1_0    (USB_SIFSLV_BASE+0x8C4) /* RW */
#define USB_U1PHYCR1_1    (USB_SIFSLV_BASE+0x8C5) /* RW */
#define USB_U1PHYCR1_2    (USB_SIFSLV_BASE+0x8C6) /* RW */
#define USB_U1PHYCR1_3    (USB_SIFSLV_BASE+0x8C7) /* RW */

#define USB_U1PHYCR2_0    (USB_SIFSLV_BASE+0x8C8) /* RW */
#define USB_U1PHYCR2_1    (USB_SIFSLV_BASE+0x8C9) /* RW */
#define USB_U1PHYCR2_2    (USB_SIFSLV_BASE+0x8CA) /* RW */
#define USB_U1PHYCR2_3    (USB_SIFSLV_BASE+0x8CB) /* RW */

#define USB_U1PHYACHG_0    (USB_SIFSLV_BASE+0x8CC) /* RW */
#define USB_U1PHYACHG_1    (USB_SIFSLV_BASE+0x8CD) /* RW */
#define USB_U1PHYACHG_2    (USB_SIFSLV_BASE+0x8CE) /* RW */
#define USB_U1PHYACHG_3    (USB_SIFSLV_BASE+0x8CF) /* RW */

#define USB_REGFPPC_0     (USB_SIFSLV_BASE+0x8E0) /* RW */
#define USB_REGFPPC_1     (USB_SIFSLV_BASE+0x8E1) /* RW */
#define USB_REGFPPC_2     (USB_SIFSLV_BASE+0x8E2) /* RW */
#define USB_REGFPPC_3     (USB_SIFSLV_BASE+0x8E3) /* RW */

#define USB_VERSIONC_0    (USB_SIFSLV_BASE+0x8F0) /* RW */
#define USB_VERSIONC_1    (USB_SIFSLV_BASE+0x8F1) /* RW */
#define USB_VERSIONC_2    (USB_SIFSLV_BASE+0x8F2) /* RW */
#define USB_VERSIONC_3    (USB_SIFSLV_BASE+0x8F3) /* RW */

#define USB_REGFCOM_0     (USB_SIFSLV_BASE+0x8FC) /* RW */
#define USB_REGFCOM_1     (USB_SIFSLV_BASE+0x8FD) /* RW */
#define USB_REGFCOM_2     (USB_SIFSLV_BASE+0x8FE) /* RW */
#define USB_REGFCOM_3     (USB_SIFSLV_BASE+0x8FF) /* RW */
//============================USB PHY variable========================//

#define U2PHYDCR0_0_RG_EARLY_HSTX_I (0x40)

#define U2PHYACR0_0_RG_LS_CROSS  (0x08)
#define U2PHYACR0_0_RG_FS_CROSS  (0x02)

#define U2PHYACR2_0_RG_SQTH  (0x03)
#define U2PHYACR2_0_RG_SQB    (0x40)
#define U2PHYACR2_0_RG_RCVB  (0x20)
#define U2PHYACR2_0_RG_DISCB  (0x10)

#define U2PHYACR2_3_RG_OTG_VBUSCMP_EN  (0x04)

#define U2PHYDCR1_2_RG_UART_EN       (0x40) //U2PHYDCR1_2
#define U2PHYDCR1_2_RG_USB_CLKEN      (0x20) //U2PHYDCR1_2

#define U2PHYAC0_0_RG_BGR_BGR_EN     (0x01) //U2PHYAC0_0
#define U2PHYAC0_0_RG_BGR_CHP_EN     (0x04) //U2PHYAC0_0
#define U2PHYAC0_0_RG_BGR_ISRC_EN    (0x02)  //U2PHYAC0_0

#define U2PHYDCR0_0_RG_HSTX_ANA_SER_EN  (0x80)

#define USB_U2PHYACR1_2_RG_DISCD    (0x02)

#define U2PHYAC0_2_RG_PLL_DIV      (0x0A) //U2PHYAC0_2

#define U2PHYACR1_1_RG_HSTX_SRCTRL    (0x04) //U2PHYACR1_1


#define U2PHYACR2_3_RG_DM_100K_EN     (0x02) //U2PHYACR2_3
#define U2PHYACR2_3_RG_DP_100K_EN     (0x01) //U2PHYACR2_3
#define U2PHYACR2_3_RG_OTG_VBUSCMP_EN     (0x04) //U2PHYACR2_3

#define U2PHYACR3_0_RG_OTG_VBUSTH         (0x40)
#define U2PHYACR3_2_RG_BC11_DISABLE    (0x04)
#define U2PHYACR3_3_RG_USBRESERVED    (0x20)

#define USB_U2PHYDMON0_3_VBUSVALID_MAC  (0x04)
#define USB_U2PHYDMON0_2_LINESTATE_DP   (0x40)
#define USB_U2PHYDMON0_2_LINESTATE_DM   (0x80)

#define U2PHYDTM_0_RG_SUSPENDM      (0x08) //U2PHYDTM0_0
#define U2PHYDTM0_0_RG_TERMSEL      (0x04) //U2PHYDTM0_0
#define U2PHYDTM0_0_RG_OPMODE       (0x01) //U2PHYDTM0_0

#define U2PHYDTM0_1_UTMI_MUSEL      (0x80) //U2PHYDTM0_1

#define U2PHYDTM0_2_FORCE_SUSPENDM    (0x04) //U2PHYDTM0_2

#define U2PHYDTM0_3_FORCE_UART_EN      (0x04)//U2PHYDTM0_3

#define U1PHYCR0_1_RG_USB11_FSLS_ENBGRI (0x08) //U1PHYCR0_1
#define U1PHYCR1_0_RG_USB11_USBRESERVED (0x01)
#define U1PHYCR1_0_RG_USB11_USBRESERVED_PMU (0x80)

//UNIT IP LINE State (MT6251 (USB1.1) , MT6276(USB2.0))
#define  USB_LINE_STATE  USB_U2PHYDMON0_2

#elif (defined(DRV_USB_PHY_M60_V1)||defined(DRV_USB_PHY_M60_V2))
#define U2PHYAC0 (USB_SIFSLV_BASE+0x800) //USB2.0 PHYA Common Registers
#define U2PHYAC1 (USB_SIFSLV_BASE+0x804) //USB2.0 PHYA Common Registers
#define U2PHYAC2 (USB_SIFSLV_BASE+0x808) //USB2.0 PHYA Common Registers
#define U2PHYACR0 (USB_SIFSLV_BASE+0x810) //USB2.0 PHYA Control Registers
#define U2PHYACR1 (USB_SIFSLV_BASE+0x814) //USB2.0 PHYA Control Registers
#define U2PHYACR2 (USB_SIFSLV_BASE+0x818) //USB2.0 PHYA Control Registers
#define U2PHYACR3 (USB_SIFSLV_BASE+0x81C) //USB2.0 PHYA Control Registers
#define U2PHYACR4 (USB_SIFSLV_BASE+0x820) //USB2.0 PHYA Control Registers
#define U2PHYDCR0 (USB_SIFSLV_BASE+0x860) //USB2.0 PHYD Control Registers
#define U2PHYDCR1 (USB_SIFSLV_BASE+0x864) //USB2.0 PHYD Control Registers
#define U2PHYDTM0 (USB_SIFSLV_BASE+0x868) //USB2.0 PHYD TestMode Registers
#define U2PHYDTM1 (USB_SIFSLV_BASE+0x86C) //USB2.0 PHYD TestMode Registers
#define U2PHYDMON0 (USB_SIFSLV_BASE+0x870) //USB2.0 PHYD Monitor Registers
#define U2PHYDMON1 (USB_SIFSLV_BASE+0x874) //USB2.0 PHYD Monitor Registers
#define U2PHYDMON2 (USB_SIFSLV_BASE+0x878) //USB2.0 PHYD Monitor Registers
#define U1PHYCR0 (USB_SIFSLV_BASE+0x8C0) //USB1.1 PHY Control Registers
#define U1PHYCR1 (USB_SIFSLV_BASE+0x8C4) //USB1.1 PHY Control Registers
#define U1PHYCR2 (USB_SIFSLV_BASE+0x8C8) //USB1.1 PHY Control Registers
#define REGFPPC  (USB_SIFSLV_BASE+0x8E0) //RegFile Per-Page Common Registers
#define VERSIONC (USB_SIFSLV_BASE+0x8F0) //Version Code
#define REGFCOM  (USB_SIFSLV_BASE+0x8FC) //RegFile Common Registers

#define U2PHYDTM0_FORCE_UART_EN     (1<<26)
#define U2PHYDTM0_FORCE_SUSPENDM    (1<<18)
#define U2PHYDTM0_RG_SUSPENDM     (1<<3)
#define U2PHYDTM1_RG_UART_EN     (1<<16)
#define U2PHYAC0_RG_USB20_USBPLL_FBDIV_6_0_CLR (0x7F<<16) //[6:0]
#define U2PHYAC0_RG_USB20_USBPLL_FBDIV_6_0  (9<<16) //[6:0]
#define U2PHYAC0_RG_USB20_INTR_EN    (1<<14)
#define U2PHYACR2_RG_USB20_OTG_VBUSCMP_EN  (1<<27)
#define U2PHYACR3_RG_USB20_PHY_REV_7   (1<<7)
#define U2PHYDTM1_RG_VBUSVALID     (1<<5)

#define U2PHYDTM1_RG_SESSEND     (1<<4)
#define U2PHYDTM1_RG_BVALID      (1<<3)
#define U2PHYDTM1_RG_AVALID      (1<<2)
#define U2PHYDTM1_RG_IDDIG      (1<<1)
#define U2PHYDTM1_force_vbusvalid    (1<<13)
#define U2PHYDTM1_force_sessend     (1<<12)
#define U2PHYDTM1_force_bvalid     (1<<11)
#define U2PHYDTM1_force_avalid     (1<<10)
#define U2PHYDTM1_force_iddig     (1<<9)


#elif (defined(DRV_USB_PHY_U40)||defined(DRV_USB_PHY_U40_V2)||defined(DRV_USB_PHY_T55_V2))
#define USB_U2PHYAC0 (USB_SIFSLV_BASE+0x800)   //USB2.0 PHYA Common Registers
#define USB_U2PHYAC1 (USB_SIFSLV_BASE+0x804)   //USB2.0 PHYA Common Registers
#define USB_U2PHYAC2 (USB_SIFSLV_BASE+0x808)   //USB2.0 PHYA Common Registers
#define USB_U2PHYACR0 (USB_SIFSLV_BASE+0x810)   //USB2.0 PHYA Control Registers
#define USB_U2PHYACR1 (USB_SIFSLV_BASE+0x814)   //USB2.0 PHYA Control Registers
#define USB_U2PHYACR2 (USB_SIFSLV_BASE+0x818)   //USB2.0 PHYA Control Registers
#define USB_U2PHYACR3 (USB_SIFSLV_BASE+0x81C)   //USB2.0 PHYA Control Registers
#define USB_U2PHYACR4 (USB_SIFSLV_BASE+0x820)   //USB2.0 PHYA Control Registers
#define USB_U2PHYDCR0 (USB_SIFSLV_BASE+0x860)   //USB2.0 PHYD Control Registers
#define USB_U2PHYDCR1 (USB_SIFSLV_BASE+0x864)   //USB2.0 PHYD Control Registers
#define USB_U2PHYDTM0 (USB_SIFSLV_BASE+0x868)   //USB2.0 PHYD TestMode Registers
#define USB_U2PHYDTM1 (USB_SIFSLV_BASE+0x86C)   //USB2.0 PHYD TestMode Registers
#define USB_U2PHYDMON0 (USB_SIFSLV_BASE+0x870)   //USB2.0 PHYD Monitor Registers
#define USB_U2PHYDMON1 (USB_SIFSLV_BASE+0x874)   //USB2.0 PHYD Monitor Registers
#define USB_U2PHYDMON2 (USB_SIFSLV_BASE+0x878)   //USB2.0 PHYD Monitor Registers
#define USB_U1PHYCR0 (USB_SIFSLV_BASE+0x8C0)   //USB1.1 PHY Control Registers
#define USB_U1PHYCR1 (USB_SIFSLV_BASE+0x8C4)   //USB1.1 PHY Control Registers
#define USB_U1PHYCR2 (USB_SIFSLV_BASE+0x8C8)   //USB1.1 PHY Control Registers
#define USB_REGFPPC  (USB_SIFSLV_BASE+0x8E0)   //RegFile Per-Page Common Registers
#define USB_VERSIONC (USB_SIFSLV_BASE+0x8F0)   //Version Code
#define USB_REGFCOM  (USB_SIFSLV_BASE+0x8FC)   //RegFile Common Registers

//Rom Code Setup
#define U2PHYDTM0_FORCE_UART_EN    (1<<26) //BIT26
#define U2PHYDTM1_RG_UART_EN    (1<<16) //BIT16
#define U2PHYACR3_RG_USB20_PHY_REV_7  (1<<7) //BIT7
#define U2PHYAC0_RG_USB20_USBPLL_FBDIV  (0x09<<16) //BIT16~22 = 7¡¦b0001001
#define U2PHYDTM0_FORCE_SUSPENDM   (1<<18) //BIT18
#define U2PHYACR2_RG_USB20_OTG_VBUSCMP_EN (1<<27) //BIT27
#define U2PHYAC0_RG_USB20_INTR_EN   (1<<14)
#define U2PHYAC0_RG_USB_LVSH_EN    (1<<31)
#define U2PHYDTM1_RG_VBUSVALID    (1<<5)
#define U2PHYDTM1_RG_SESSEND    (1<<4)
#define U2PHYDTM1_RG_BVALID     (1<<3)
#define U2PHYDTM1_RG_AVALID     (1<<2)
#define U2PHYDTM1_RG_IDDIG     (1<<1)
#define U2PHYDTM1_force_vbusvalid   (1<<13)
#define U2PHYDTM1_force_sessend    (1<<12)
#define U2PHYDTM1_force_bvalid    (1<<11)
#define U2PHYDTM1_force_avalid    (1<<10)
#define U2PHYDTM1_force_iddig    (1<<9)
#define U2PHYACR4_rg_usb20_gpio_ctl    (1<<9)
#define U2PHYACR4_usb20_gpio_mode    (1<<8)

//ROM CODE POWER OFF
#define U2PHYDTM0_RG_SUSPENDM    (1<<3) //BIT3
//U2PHYDTM0_FORCE_SUSPENDM            //BIT18
#elif defined(DRV_USB_PHY_T55)
#define USB_U1PHYCR0      (USB_SIFSLV_BASE+0x8C0)   //USB1.1 PHY Control Registers
#define USB_U1PHYCR1      (USB_SIFSLV_BASE+0x8C4)   //USB1.1 PHY Control Registers
#define USB_U1PHYCR2      (USB_SIFSLV_BASE+0x8C8)   //USB1.1 PHY Control Registers

#define U1PHYCR0_RG_USB_LVSH_EN                 (1<<8) //BIT8
#define U1PHYCR0_RG_USB11_FSLS_ENBGRI  (1<<11) //BIT11
#define U1PHYCR1_RG_USB11_PHY_REV_7   (1<<15) //BIT15

#elif defined(DRV_USB_PHY_T14)
#define USBPHYACR0 (USB_SIFSLV_BASE+0x800)   //USB2.0 PHYA Common Registers
#define USBPHYACR1 (USB_SIFSLV_BASE+0x804)   //USB2.0 PHYA Common Registers
#define USBPHYACR2 (USB_SIFSLV_BASE+0x808)   //USB2.0 PHYA Common Registers
#define USBPHYACR4 (USB_SIFSLV_BASE+0x810)   //USB2.0 PHYA Common Registers
#define USBPHYACR5 (USB_SIFSLV_BASE+0x814)   //USB2.0 PHYA Common Registers
#define USBPHYACR6 (USB_SIFSLV_BASE+0x818)   //USB2.0 PHYA Common Registers
#define U2PHYACR3 (USB_SIFSLV_BASE+0x81C)   //USB20 PHYA Control 3 Register
#define U2PHYACR4 (USB_SIFSLV_BASE+0x820)   //USB20 PHYA Control 4 Register
#define U2PHYAMON0 (USB_SIFSLV_BASE+0x824)   //USB20 PHYA Monitor 0 Register
#define U2PHYDCR0 (USB_SIFSLV_BASE+0x860)   //USB20 PHYD Control 0 Register
#define U2PHYDCR1 (USB_SIFSLV_BASE+0x864)   //USB20 PHYD Control 1 Register
#define U2PHYDTM0 (USB_SIFSLV_BASE+0x868)   //USB20 PHYD Control UTMI 0 Register
#define U2PHYDTM1 (USB_SIFSLV_BASE+0x86C)   //USB20 PHYD Control UTMI 1 Register
#define U2PHYDMON0 (USB_SIFSLV_BASE+0x870)   //USB20 PHYD Monitor 0 Register
#define U2PHYDMON1 (USB_SIFSLV_BASE+0x874)   //USB20 PHYD Monitor 1 Register
#define U2PHYDMON2 (USB_SIFSLV_BASE+0x878)   //USB20 PHYD Monitor 2 Register
#define U2PHYDMON3 (USB_SIFSLV_BASE+0x87C)   //USB20 PHYD Monitor 3 Register
#define U2PHYCR3 (USB_SIFSLV_BASE+0x880)   //USB20 sifslv Control Register
#define REGFCOM     (USB_SIFSLV_BASE+0x8FC)   //USB Common Register
#define FMCR0     (USB_SIFSLV_BASE+0xF00)   //Frequency Meter Control 0 Registers
#define FMCR1     (USB_SIFSLV_BASE+0xF04)   //Frequency Meter Control 1 Registers
#define FMCR2     (USB_SIFSLV_BASE+0xF08)   //Frequency Meter Control 2 Registers
#define FMMONR0     (USB_SIFSLV_BASE+0xF0C)   //Frequency Meter Monitor 0 Registers
#define FMMONR1     (USB_SIFSLV_BASE+0xF10)   //Frequency Meter Monitor 1 Registers

#define U2PHYDTM0_FORCE_UART_EN         (1 << 26)
#define U2PHYDTM1_RG_UART_EN            (1 << 16)
#define U2PHYACR4_RG_USB20_GPIO_CTL     (1 << 9) 
#define U2PHYACR4_USB20_GPIO_MODE       (1 << 8) 
#define USBPHYACR6_RG_USB20_BC11_SW_EN  (1 << 23)
#define USBPHYACR0_RG_USB20_INTR_EN     (1 << 5)
#define U2PHYDTM0_RG_SUSPENDM           (1 << 3)
#define U2PHYDTM0_FORCE_SUSPENDM        (1 << 18)
#define U2PHYDTM1_RG_VBUSVALID          (1 << 5)
#define U2PHYDTM1_RG_SESSEND            (1 << 4)
#define U2PHYDTM1_RG_BVALID             (1 << 3)
#define U2PHYDTM1_RG_AVALID             (1 << 2)
#define U2PHYDTM1_RG_IDDIG              (1 << 1)

#define U2PHYDTM1_FORCE_VBUSVALID       (1 << 13)
#define U2PHYDTM1_FORCE_SESSEND         (1 << 12)
#define U2PHYDTM1_FORCE_BVALID          (1 << 11)
#define U2PHYDTM1_FORCE_AVALID          (1 << 10)
#define U2PHYDTM1_FORCE_IDDIG           (1 << 9)

#else
    //#error please add USB PHY define in usb_hw.h!
#endif
//======================USB PHY =============================//
#endif  /* USB_HW_H */

/************************ USB register definition end line    *******************************/

/************************ EMI register definition start line  *******************************
 */
#define EMI_MR_DATA_MASK 0xFF
#define EMI_MREG_RDATA_OFFSET (8)
#define EMI_MREG_RDATA_MASK (0xFF << EMI_MREG_RDATA_OFFSET)
#define EMI_MREG_BANK_OFFSET (20)
#define EMI_MREG_BANK_MASK (0x7 << EMI_MREG_BANK_MASK)
#define EMI_MRGE_EDGE_TRIGGER_OFFSET (16)
#define EMI_MRGE_EDGE_TRIGGER_MASK (0x1 << EMI_MRGE_EDGE_TRIGGER_OFFSET)
#define EMI_MRGE_WDATA_OFFSET (24)
#define EMI_MRGE_WDATA_MASK (0xFF << EMI_MRGE_WDATA_OFFSET)
#define EMI_MRGE_W_OFFSET (17)
#define EMI_MRGE_W_MASK (0x1 << EMI_MRGE_W_OFFSET)

#define EMI_MRGE_ACC_IDLE_OFFSET (0)
#define EMI_MRGE_ACC_IDLE_MASK (0x1 << EMI_MRGE_ACC_IDLE_OFFSET)

#define EMI_DQ7_IN_DEL_OFFSET (24)
#define EMI_DQ6_IN_DEL_OFFSET (16)
#define EMI_DQ5_IN_DEL_OFFSET (8)
#define EMI_DQ4_IN_DEL_OFFSET (0)
#define EMI_DQ3_IN_DEL_OFFSET (24)
#define EMI_DQ2_IN_DEL_OFFSET (16)
#define EMI_DQ1_IN_DEL_OFFSET (8)
#define EMI_DQ0_IN_DEL_OFFSET (0)
#define EMI_DQS0_IN_DEL_OFFSET (16)



#define EMI_BIST_STR_ADDR_OFFSET (16)
#define EMI_BIST_STR_ADDR_MASK (0xFFFF << EMI_BIST_STR_ADDR_OFFSET)
#define EMI_BIST_BG_DATA_OFFSET (16)
#define EMI_BIST_BG_DATA_MASK (0xFFFF << EMI_BIST_BG_DATA_OFFSET)
#define EMI_BIST_DATA_INV_OFFSET (12)
#define EMI_BIST_DATA_INV_MASK (0x1 << EMI_BIST_DATA_INV_OFFSET)
#define EMI_BIST_DATA_RANDOM_OFFSET (13)
#define EMI_BIST_DATA_RANDOM_MASK (0x1 << EMI_BIST_DATA_RANDOM_OFFSET)
#define EMI_BIST_END_OFFSET (1)
#define EMI_BIST_END_MASK (0x1 << EMI_BIST_END_OFFSET)
#define EMI_BIST_FAIL_OFFSET (0)
#define EMI_BIST_FAIL_MASK (0x1 << EMI_BIST_FAIL_OFFSET)

#define EMI_REQ_MASK_MASK (0x7)


/*EMI register definition start*/
typedef struct {
    union {
        struct {
            __I uint8_t EMI_IDLE;        /*EMI_CONM[0]*/
            __I uint8_t EMI_SRAM_IDLE;   /*EMI_CONM[8]*/
            __IO uint8_t REQ_MASK;       /*EMI_CONM[16:18]*/
            __IO uint8_t AUTO_RESP;      /*EMI_CONM[24]*/
        } EMI_CONM_CELLS;
        __IO uint32_t EMI_CONM; /*EMI_CONM */
    } EMI_CONM_UNION;//offset+0x0000
    __IO uint8_t  RESERVED0[12];
    __IO uint32_t EMI_GENA;//offset+0x0010
    __IO uint8_t  RESERVED1[12];
    __IO uint32_t EMI_RDCT;//offset+0x0020
    __IO uint8_t  RESERVED2[12];

    union {
        struct {
            __IO uint16_t DLL_SETTING;        /*EMI_DLLV0[0:2]*/
            __IO uint8_t DLL_UPT_CNT;          /*EMI_DLLV0[16:21]*/
            __IO uint8_t DLL_EN_PRE_CNT;       /*EMI_DLLV0[24:26]*/
        } EMI_DLLV0_CELLS;
        __IO uint32_t EMI_DLLV0; /*EMI_DLLV0 */
    } EMI_DLLV0_UNION;//offset+0x0030

    union {
        struct {
            __I uint8_t CAL_DONE;              /*EMI_DLLV1[0]*/
            __I uint8_t DLL_CAL_VALUE;         /*EMI_DLLV1[8:12]*/
            __IO uint8_t  RESERVED[2];  /*EMI_DLLV1[16:31]*/
        } EMI_DLLV1_CELLS;
        __IO uint32_t EMI_DLLV1; /*EMI_DLLV1 */
    } EMI_DLLV1_UNION;//offset+0x0034

    __IO uint8_t  RESERVED3[56];

    union {
        struct {
            __IO uint8_t HFSLP_EXIT_REQ;              /*EMI_HFSLP[0]*/
            __IO uint8_t HFSLP_EXIT_ACK;              /*EMI_HFSLP[8]*/
            __IO uint8_t HFSLP_ENT_STA;               /*EMI_HFSLP[16]*/
            __IO uint8_t RESERVED;      /*EMI_HFSLP[24:28]*/
        } EMI_HFSLP_CELLS;
        __IO uint32_t EMI_HFSLP; /*EMI_HFSLP */
    } EMI_HFSLP_UNION;//offset+0x0070

    __IO uint8_t  RESERVED6[12];
    __IO uint32_t EMI_DSRAM;//offset+0x0080
    __IO uint8_t  RESERVED7[12];

    union {
        struct {
            __IO uint8_t HFSLP_EXIT_CS_H_CYCLE_64K;          /*EMI_MSRAM0[0:2]*/
            __IO uint8_t HFSLP_EXIT_CS_L_CYCLE;              /*EMI_MSRAM0[8:11]*/
            __IO uint8_t HFSLP_ENTER_CS_H_CYCLE_64K;         /*EMI_MSRAM0[16:17]*/
            __IO uint8_t RESERVED;             /*EMI_MSRAM0[24:28]*/
        } EMI_MSRAM0_CELLS;
        __IO uint32_t EMI_MSRAM0; /*EMI_MSRAM0 */
    } EMI_MSRAM0_UNION;//offset+0x0090
    union {
        struct {
            __IO uint8_t FIX_WAIT_CYC_EN;          /*EMI_MSRAM1[0:2]*/
            __IO uint8_t RD_WAIT_CYCL;              /*EMI_MSRAM1[8:11]*/
            __IO uint8_t RD_DEL_SEL_M;         /*EMI_MSRAM1[16:17]*/
            __IO uint8_t RESERVED;             /*EMI_MSRAM1[24:28]*/
        } EMI_MSRAM1_CELLS;
        __IO uint32_t EMI_MSRAM1; /*EMI_MSRAM1*/
    } EMI_MSRAM1_UNION;//offset+0x0094

    __IO uint8_t  RESERVED8[8];
    __IO uint32_t EMI_MREG_RW;//offset+0x00A0
    __IO uint8_t  RESERVED9[12];

    union {
        struct {
            __IO uint8_t BW_FILTER_EN;        /*EMI_ARBA0[0:2]*/
            __IO uint8_t MAX_GNT_CNT;            /*EMI_ARBA0[8:11]*/
            __IO uint8_t SOFT_MODE_SETTING;      /*EMI_ARBA0[16:17]*/
            __IO uint8_t BW_FILTER_LEN;            /*EMI_ARBA0[24:28]*/
        } EMI_ARBA0_CELLS;
        __IO uint32_t EMI_ARBA0; /*EMI_ARBA0 */
    } EMI_ARBA0_UNION;//offset+0x00B0

    union {
        struct {
            __IO uint8_t SV_CNT_EN;       /*EMI_ARBA1[0]*/
            __IO uint8_t SV_MODE;            /*EMI_ARBA1[8:9]*/

            __IO uint16_t SV_VALUE;      /*EMI_ARBA1[16:25]*/
        } EMI_ARBA1_CELLS;
        __IO uint32_t EMI_ARBA1; /*EMI_ARBA1 */
    } EMI_ARBA1_UNION;//offset+0x00B4

    __IO uint8_t  RESERVED10[8];

    union {
        struct {
            __IO uint8_t BW_FILTER_EN;        /*EMI_ARBB0[0:2]*/
            __IO uint8_t MAX_GNT_CNT;            /*EMI_ARBB0[8:11]*/
            __IO uint8_t SOFT_MODE_SETTING;      /*EMI_ARBB0[16:17]*/
            __IO uint8_t  RESERVED[1];             /*EMI_ARBB0[24:28]*/
        } EMI_ARBB0_CELLS;
        __IO uint32_t EMI_ARBB0; /*EMI_ARBA0 */
    } EMI_ARBB0_UNION;//offset+0x00C0

    union {
        struct {
            __IO uint8_t SV_CNT_EN;       /*EMI_ARBB1[0:2]*/
            __IO uint8_t SV_MODE;            /*EMI_ARBB1[8:11]*/
            __IO uint16_t SV_VALUE;      /*EMI_ARBB1[16:25]*/
        } EMI_ARBB1_CELLS;
        __IO uint32_t EMI_ARBB1; /*EMI_ARBB1 */
    } EMI_ARBB1_UNION;//offset+0x00C4

    __IO uint8_t  RESERVED11[8];

    union {
        struct {
            __IO uint8_t BW_FILTER_EN;        /*EMI_ARBC0[0:2]*/
            __IO uint8_t MAX_GNT_CNT;            /*EMI_ARBC0[8:11]*/
            __IO uint8_t SOFT_MODE_SETTING;      /*EMI_ARBC0[16:17]*/
            __IO uint8_t  RESERVED[1];             /*EMI_ARBC0[24:28]*/
        } EMI_ARBC0_CELLS;
        __IO uint32_t EMI_ARBC0; /*EMI_ARBC0 */
    } EMI_ARBC0_UNION;//offset+0x00D0

    union {
        struct {
            __IO uint8_t SV_CNT_EN;       /*EMI_ARBC1[0:2]*/
            __IO uint8_t SV_MODE;            /*EMI_ARBC1[8:11]*/
            __IO uint16_t SV_VALUE;      /*EMI_ARBC1[16:25]*/
        } EMI_ARBC1_CELLS;
        __IO uint32_t EMI_ARBC1; /*EMI_ARBC1 */
    } EMI_ARBC1_UNION;//offset+0x00D4

    __IO uint8_t  RESERVED12[8];
    union {
        struct {
            __IO uint8_t HI_PRIO_EN_M0;       /*EMI_SLCT[0]*/
            __IO uint8_t HI_PRIO_EN_M1;          /*EMI_SLCT[8]*/
            __IO uint8_t HI_PRIO_EN_M2;      /*EMI_SLCT[16]*/
            __IO uint8_t  RESERVED[1];             /*EMI_SLCT[24:28]*/
        } EMI_SLCT_CELLS;
        __IO uint32_t EMI_SLCT; /*EMI_SLCT */
    } EMI_SLCT_UNION;//offset+0x00E0
    __IO uint8_t  RESERVED13[12];
    __IO uint32_t EMI_ABCT;//offset+0x00F0
    __IO uint8_t  RESERVED14[12];
    __IO uint32_t EMI_BMEN;//offset+0x0100
    __IO uint8_t  RESERVED15[4];
    __IO uint32_t EMI_BCNT;//offset+0x0108
    __IO uint8_t  RESERVED16[4];
    __IO uint32_t EMI_TACT;//offset+0x0110
    __IO uint8_t  RESERVED17[4];
    __IO uint32_t EMI_TSCT;//offset+0x0118
    __IO uint8_t  RESERVED18[4];
    __IO uint32_t EMI_WACT;//offset+0x0120
    __IO uint8_t  RESERVED19[4];
    __IO uint32_t EMI_WSCT;//offset+0x0128
    __IO uint8_t  RESERVED20[4];
    __IO uint32_t EMI_BACT; //offset+0x0130
    __IO uint32_t EMI_BSCT0;//offset+0x0134
    __IO uint32_t EMI_BSCT1;//offset+0x0138
    __IO uint8_t  RESERVED21[4];
    __IO uint32_t EMI_TTYPE1;//offset+0x0140
    __IO uint8_t  RESERVED22[12];
    __IO uint32_t EMI_MBISTA;//offset+0x0150
    __IO uint32_t EMI_MBISTB;//offset+0x0154
    union {
        struct {
            __IO uint16_t BIST_ERR_INFO_SEL;      /*EMI_MBISTC[0:15]*/
            __IO uint16_t BIST_ADDR_OFFSET;          /*EMI_MBISTC[16:31]*/
        } EMI_MBISTC_CELLS;
        __IO uint32_t EMI_MBISTC; /*EMI_MBISTC */
    } EMI_MBISTC_UNION;//offset+0x0158
    __IO uint32_t EMI_MBISTD;//offset+0x015C
} EMI_REGISTER_T;

/*EMI AO register definition start*/
typedef struct {
    __IO uint32_t EMI_IOA;//offset + 0x0000
    __IO uint32_t EMI_IOB;//offset + 0x0004
    union {
        struct {
            __IO uint8_t DQ4_IN_DEL;        /*EMI_IDLC[0:4]*/
            __IO uint8_t DQ5_IN_DEL;        /*EMI_IDLC[8:12]*/
            __IO uint8_t DQ6_IN_DEL;        /*EMI_IDLC[16:20]*/
            __IO uint8_t DQ7_IN_DEL;        /*EMI_IDLC[24:28]*/
        } EMI_IDLC_CELLS;
        __IO uint32_t EMI_IDLC; /*EMI_IDLC */
    } EMI_IDLC_UNION;//offset + 0x0008

    union {
        struct {
            __IO uint8_t DQ0_IN_DEL;        /*EMI_IDLD[0:4]*/
            __IO uint8_t DQ1_IN_DEL;        /*EMI_IDLD[8:12]*/
            __IO uint8_t DQ2_IN_DEL;        /*EMI_IDLD[16:20]*/
            __IO uint8_t DQ3_IN_DEL;        /*EMI_IDLD[24:28]*/
        } EMI_IDLD_CELLS;
        __IO uint32_t EMI_IDLD; /*EMI_IDLD */
    } EMI_IDLD_UNION;//offset + 0x000c

    union {
        struct {
            __IO uint8_t  RESERVED1[2]; /*EMI_IDLE[0:15]*/
            __IO uint8_t   DQS0_IN_DEL;            /*EMI_IDLE[16:20]*/
            __IO uint8_t  RESERVED2;  /*EMI_IDLE[24:31]*/
        } EMI_IDLE_CELLS;
        __IO uint32_t EMI_IDLE; /*EMI_IDLE */
    } EMI_IDLE_UNION;//offset + 0x0010
    union {
           struct {
               __IO uint8_t DQ4_OUT_DEL;        /*EMI_ODLC[0:4]*/
               __IO uint8_t DQ5_OUT_DEL;        /*EMI_ODLC[8:12]*/
               __IO uint8_t DQ6_OUT_DEL;        /*EMI_ODLC[16:20]*/
               __IO uint8_t DQ7_OUT_DEL;        /*EMI_ODLC[24:28]*/
           } EMI_ODLC_CELLS;
           __IO uint32_t EMI_ODLC; /*EMI_ODLC */
       } EMI_ODLC_UNION;//offset + 0x0014

       union {
           struct {
               __IO uint8_t DQ0_OUT_DEL;        /*EMI_ODLD[0:4]*/
               __IO uint8_t DQ1_OUT_DEL;        /*EMI_ODLD[8:12]*/
               __IO uint8_t DQ2_OUT_DEL;        /*EMI_ODLD[16:20]*/
               __IO uint8_t DQ3_OUT_DEL;        /*EMI_ODLD[24:28]*/
           } EMI_ODLD_CELLS;
           __IO uint32_t EMI_ODLD; /*EMI_ODLD */
       } EMI_ODLD_UNION;//offset + 0x0018

       union {
           struct {
               __IO uint16_t DQS0_OUT_DEL;        /*EMI_ODLE[0:4]*/
               __IO uint16_t DQM0_OUT_DEL;        /*EMI_ODLE[16:20]*/
           } EMI_ODLE_CELLS;
           __IO uint32_t EMI_ODLE; /*EMI_ODLE */
       } EMI_ODLE_UNION;//offset + 0x001c
       union {
           struct {
               __IO uint8_t ECCLK_OUT_DEL;           /*EMI_ODLF[0:4]*/
               __IO uint8_t CA_OUT_DEL;             /*EMI_ODLF[8:12]*/
               __IO uint8_t  RESERVED[2];  /*EMI_ODLF[24:31]*/
           } EMI_ODLF_CELLS;
           __IO uint32_t EMI_ODLF; /*EMI_ODLF */
       } EMI_ODLF_UNION;//offset + 0x0020
} EMI_AO_REGISTER_T;

/************************ end register definition start line  *******************************
 */


/* *************************flash hardware definition start line**********************************
*/

/*flash register structure definition*/
#if 0
typedef struct {
    __IO uint32_t RW_SF_MAC_CTL;         /*!<  SFC control register Address offset: 0x00 */
    __IO uint32_t RW_SF_DIRECT_CTL;        /*!< SFC control register Address offset: 0x04 */
    __IO uint32_t RW_SF_MISC_CTL;      /*!<  SFC control register Address offset:   0x08 */
    __IO uint32_t RW_SF_MISC_CTL2;      /*!<  SFC control register Address offset:   0x0C */
    __IO uint32_t RW_SF_MAC_OUTL;      /*!<  SFC control register Address offset:   0x10 */
    __IO uint32_t RW_SF_MAC_INL;      /*!<  SFC thresh registerAddress offset:   0x14 */
    __IO uint32_t RW_SF_RESET_CTL;      /*!<  SFC thresh registerAddress offset:   0x18 */
    __IO uint32_t RW_SF_STA2_CTL;      /*!<  SFC thresh registerAddress offset:   0x1C */
    __IO uint32_t RW_SF_DLY_CTL1;      /*!<  SFC thresh registerAddress offset:   0x20*/
    __IO uint32_t RW_SF_DLY_CTL2;      /*!<  SFC thresh registerAddress offset:   0x24 */
    __IO uint32_t RW_SF_DLY_CTL3;      /*!<  SFC thresh registerAddress offset:   0x28 */
    __IO uint32_t RW_SF_DUMMY1;         /*!<  SFC thresh registerAddress offset:   0x2C */
    __IO uint32_t RW_SF_DLY_CTL4;      /*!<  SFC thresh registerAddress offset:   0x30 */
    __IO uint32_t RW_SF_DLY_CTL5;      /*!<  SFC thresh registerAddress offset:   0x34 */
    __IO uint32_t RW_SF_DLY_CTL6;      /*!<  SFC thresh registerAddress offset:   0x38 */
    __IO uint32_t RW_SF_DUMMY2;         /*!<  SFC thresh registerAddress offset:   0x3C */
    __IO uint32_t RW_SF_DIRECT_CTL2;      /*!<  SFC thresh registerAddress offset:   0x40 */
    __IO uint32_t RW_SF_MISC_CTL3;      /*!<  SFC thresh registerAddress offset:   0x44 */
} SFC_REGISTER_T;
#endif

/*flash register structure definition*/
typedef struct {
    __IO uint32_t RW_SF_MAC_CTL;         /*!<  SFC control register Address offset: 0x00 */
    __IO uint32_t RW_SF_DIRECT_CTL;        /*!< SFC control register Address offset: 0x04 */
    __IO uint32_t RW_SF_MISC_CTL;      /*!<  SFC control register Address offset:   0x08 */
    __IO uint32_t RW_SF_MISC_CTL2;      /*!<  SFC control register Address offset:   0x0C */
    __IO uint32_t RW_SF_MAC_OUTL;      /*!<  SFC control register Address offset:   0x10 */
    __IO uint32_t RW_SF_MAC_INL;      /*!<  SFC thresh registerAddress offset:   0x14 */
    __IO uint32_t RW_SF_RESET_CTL;      /*!<  SFC thresh registerAddress offset:   0x18 */
    __IO uint32_t RW_SF_STA2_CTL;      /*!<  SFC thresh registerAddress offset:   0x1C */
    __IO uint32_t RW_SF_DLY_CTL1;      /*!<  SFC thresh registerAddress offset:   0x20*/
    __IO uint32_t RW_SF_DLY_CTL2;      /*!<  SFC thresh registerAddress offset:   0x24 */
    __IO uint32_t RW_SF_DLY_CTL3;      /*!<  SFC thresh registerAddress offset:   0x28 */
    __IO uint32_t RW_SF_DUMMY1;         /*!<  SFC thresh registerAddress offset:   0x2C */
    __IO uint32_t RW_SF_DLY_CTL4;      /*!<  SFC thresh registerAddress offset:   0x30 */
    __IO uint32_t RW_SF_DLY_CTL5;      /*!<  SFC thresh registerAddress offset:   0x34 */
    __IO uint32_t RW_SF_DLY_CTL6;      /*!<  SFC thresh registerAddress offset:   0x38 */
    __IO uint32_t RW_SF_DUMMY2;         /*!<  SFC thresh registerAddress offset:   0x3C */
    __IO uint32_t RW_SF_DIRECT_CTL2;      /*!<  SFC thresh registerAddress offset:   0x40 */
    __IO uint32_t RW_SF_MISC_CTL3;      /*!<  SFC thresh registerAddress offset:   0x44 */
} SFC_REGISTER_T;


// Performance Monitor
typedef struct {
    __IO uint32_t SF_PERF_MON1;         /*!<  SFC control register Address offset: 0x80 */
    __IO uint32_t SF_PERF_MON2;        /*!< SFC counter register Address offset: 0x84 */
    __IO uint32_t SF_PERF_MON3;      /*!<  SFC thresh registerAddress offset:   0x88 */
    __IO uint32_t SF_PERF_MON4;      /*!<  SFC thresh registerAddress offset:   0x8C */
    __IO uint32_t SF_PERF_MON5;      /*!<  SFC thresh registerAddress offset:   0x90 */
    __IO uint32_t SF_PERF_MON6;      /*!<  SFC thresh registerAddress offset:   0x94 */
    __IO uint32_t SF_PERF_MON7;      /*!<  SFC thresh registerAddress offset:   0x98 */
    __IO uint32_t SF_PERF_MON8;      /*!<  SFC thresh registerAddress offset:   0x9C */
    __IO uint32_t SF_PERF_MON9;      /*!<  SFC thresh registerAddress offset:   0xA0*/
    __IO uint32_t SF_PERF_MON10;      /*!<  SFC thresh registerAddress offset:   0xA4 */
    __IO uint32_t SF_PERF_MON11;      /*!<  SFC thresh registerAddress offset:   0xA8 */
    __IO uint32_t SF_PERF_MON12;         /*!<  SFC thresh registerAddress offset:   0xAC */
    __IO uint32_t SF_PERF_MON13;      /*!<  SFC thresh registerAddress offset:   0xB0 */
} SFC_PM_REGISTER_T;


typedef struct {
    __IO uint32_t RW_SF_GPRAM_DATA;      /*!<  SFC thresh registerAddress offset:   0x800 */
    __IO uint32_t RW_SF_GPRAM_DATA_OF_4; /*!<  SFC thresh registerAddress offset:   0x804 */
} SFC_GPRAM_REGISTER_T;

#define SFC_PM_BASE         (SFC_BASE + 0x80)
#define SFC_GPRAMADDR       (SFC_BASE + 0x800)
#define SFC                 ((SFC_REGISTER_T *) (SFC_BASE))
#define SFC_GPRAM           ((SFC_GPRAM_REGISTER_T *) (SFC_GPRAMADDR))

/* SFC generic offset definition */
#define SFC_GENERIC_1_BIT_OFFSET      (1)
#define SFC_GENERIC_2_BIT_OFFSET      (2)
#define SFC_GENERIC_4_BIT_OFFSET      (4)
#define SFC_GENERIC_8_BIT_OFFSET      (8)
#define SFC_GENERIC_10_BIT_OFFSET    (10)
#define SFC_GENERIC_16_BIT_OFFSET    (16)
#define SFC_GENERIC_24_BIT_OFFSET    (24)
#define SFC_GENERIC_31_BIT_OFFSET    (31)

/* SFC generic mask definition */
#define SFC_GENERIC_0x1_MASK         (0x1)
#define SFC_GENERIC_0x0F_MASK        (0x0F)
#define SFC_GENERIC_0xF0_MASK        (0xF0)
#define SFC_GENERIC_0xFF_MASK        (0xFF)
#define SFC_GENERIC_0xF000_MASK      (0xF000)
#define SFC_GENERIC_0x00FF_MASK      (0x00FF)
#define SFC_GENERIC_0x0FFFFFFF_MASK  (0x0FFFFFFF)
#define SFC_GENERIC_0x000000FF_MASK  (0x000000FF)
#define SFC_GENERIC_0x0000FF00_MASK  (0x0000FF00)
#define SFC_GENERIC_0x00FF0000_MASK  (0x00FF0000)
#define SFC_GENERIC_0xFF000000_MASK  (0xFF000000)
#define SFC_GENERIC_0xFFFFFF00_MASK  (0xFFFFFF00)
#ifdef AIR_CPU_IN_SECURITY_MODE
#define SFC_GENERIC_FLASH_BANK_MASK  (0x08000000)
#else
#define SFC_GENERIC_FLASH_BANK_MASK  (0x18000000)
#endif
#define SFC_GENERIC_DPD_SW_MASK      (0x000F0F00)
#define SFC_GENERIC_DPD_SW_IO_MASK   (0x0F0F0F0F)

/* structure type of ESC
 */
#define ESC_GPRAM_BASE  (ESC_BASE + 0x800)
typedef struct {
    __IO uint32_t ESC_MAC_CTL;
    __IO uint32_t ESC_DIRECT_CTL;
    __IO uint32_t ESC_MISC_CTL1;
    __IO uint32_t ESC_MISC_CTL2;
    __IO uint32_t ESC_MAC_OUTL;
    __IO uint32_t ESC_MAC_INL;
    __IO uint32_t ESC_STA1_CTL;
    __IO uint32_t ESC_STA2_CTL;
    __IO uint32_t ESC_DLY_CTL1;
    __IO uint32_t ESC_DLY_CTL2;
    __IO uint32_t ESC_DLY_CTL3;
    __IO uint32_t ESC_DLY_CTL4;
    __IO uint32_t ESC_QIO_CTRL;
    __IO uint32_t reserve1[5];
    __IO uint32_t ESC_STA3;
    __IO uint32_t reserve2[5];
    __IO uint32_t ESC_DEBUG_1;
    __IO uint32_t ESC_DEBUG_2;
    __IO uint32_t reserve3[2];
    __IO uint32_t ESC_DEBUG_MUX;
    __IO uint32_t reserve4[3];
    __IO uint32_t ESC_MISC_CTL3;
    __IO uint32_t ESC_MAC_IRQ;
} ESC_REGISTER_T;

#ifdef AIR_CPU_IN_SECURITY_MODE
#define ESC_GENERIC_MEM_BANK_MASK  (0x00000000)
#else
#define SFC_GENERIC_MEM_BANK_MASK  (0x10000000)
#endif

///TODO:: only for compiler
#define ANA_CFGSYS_BASE      0xA21D0000 /*Analog die Configuration Registers  (Clock, Reset, etc.)*/


/* *************************flash hardware definition end line***********************************/

/**************************dma hardware definition start line***********************************/

#define DMA_GLB_RUNNING_BIT_MASK(channel)   (1 << (2 * (channel)))
#define DMA_GLB_IRQ_STA_BIT_MASK(channel)   ((1 << (2 * (channel) + 1)))
#define DMA_GLB_IRQ_CFG_BIT_MASK(channel)   (1 << (channel))

#define MCU_DMA_0_RG_GLB_STA           (*(volatile uint32_t *)(DMA_0_BASE + 0x00))
#define MCU_DMA_0_RG_GLB_CPU0_INT_CFG  (*(volatile uint32_t *)(DMA_0_BASE + 0x08))
#define MCU_DMA_0_RG_GLB_CPU0_INT_SET  (*(volatile uint32_t *)(DMA_0_BASE + 0x0C))
#define MCU_DMA_0_RG_GLB_CPU0_INT_CLR  (*(volatile uint32_t *)(DMA_0_BASE + 0x10))
#define MCU_DMA_0_RG_GLB_CPU1_INT_CFG  (*(volatile uint32_t *)(DMA_0_BASE + 0x14))
#define MCU_DMA_0_RG_GLB_CPU1_INT_SET  (*(volatile uint32_t *)(DMA_0_BASE + 0x18))
#define MCU_DMA_0_RG_GLB_CPU1_INT_CLR  (*(volatile uint32_t *)(DMA_0_BASE + 0x1C))
#define MCU_DMA_0_RG_GLB_CPU2_INT_CFG  (*(volatile uint32_t *)(DMA_0_BASE + 0x50))
#define MCU_DMA_0_RG_GLB_CPU2_INT_SET  (*(volatile uint32_t *)(DMA_0_BASE + 0x54))
#define MCU_DMA_0_RG_GLB_CPU2_INT_CLR  (*(volatile uint32_t *)(DMA_0_BASE + 0x58))
#define MCU_DMA_0_RG_GLB_CPU3_INT_CFG  (*(volatile uint32_t *)(DMA_0_BASE + 0x60))
#define MCU_DMA_0_RG_GLB_CPU3_INT_SET  (*(volatile uint32_t *)(DMA_0_BASE + 0x64))
#define MCU_DMA_0_RG_GLB_CPU3_INT_CLR  (*(volatile uint32_t *)(DMA_0_BASE + 0x68))
#define MCU_DMA_0_RG_GLB_LIMITER       (*(volatile uint32_t *)(DMA_0_BASE + 0x28))
#define MCU_DMA_0_RG_GLB_SWRST         (*(volatile uint32_t *)(DMA_0_BASE + 0x20))
#define MCU_DMA_0_RG_GLB_BUSY          (*(volatile uint32_t *)(DMA_0_BASE + 0x40))
#define MCU_DMA_0_RG_GLB_INTR          (*(volatile uint32_t *)(DMA_0_BASE + 0x44))
#define MCU_DMA_0_RG_GLB_CLK_CFG       (*(volatile uint32_t *)(DMA_0_BASE + 0x70))
#define MCU_DMA_0_RG_GLB_CLK_SET       (*(volatile uint32_t *)(DMA_0_BASE + 0x74))
#define MCU_DMA_0_RG_GLB_CLK_CLR       (*(volatile uint32_t *)(DMA_0_BASE + 0x78))

#define MCU_DMA_RG_GLB_STA             (*(volatile uint32_t *)(DMA_1_BASE + 0x00))
#define MCU_DMA_RG_GLB_CPU0_INT_CFG    (*(volatile uint32_t *)(DMA_1_BASE + 0x08))
#define MCU_DMA_RG_GLB_CPU0_INT_SET    (*(volatile uint32_t *)(DMA_1_BASE + 0x0C))
#define MCU_DMA_RG_GLB_CPU0_INT_CLR    (*(volatile uint32_t *)(DMA_1_BASE + 0x10))
#define MCU_DMA_RG_GLB_CPU1_INT_CFG    (*(volatile uint32_t *)(DMA_1_BASE + 0x14))
#define MCU_DMA_RG_GLB_CPU1_INT_SET    (*(volatile uint32_t *)(DMA_1_BASE + 0x18))
#define MCU_DMA_RG_GLB_CPU1_INT_CLR    (*(volatile uint32_t *)(DMA_1_BASE + 0x1C))
#define MCU_DMA_RG_GLB_CPU2_INT_CFG    (*(volatile uint32_t *)(DMA_1_BASE + 0x50))
#define MCU_DMA_RG_GLB_CPU2_INT_SET    (*(volatile uint32_t *)(DMA_1_BASE + 0x54))
#define MCU_DMA_RG_GLB_CPU2_INT_CLR    (*(volatile uint32_t *)(DMA_1_BASE + 0x58))
#define MCU_DMA_RG_GLB_CPU3_INT_CFG    (*(volatile uint32_t *)(DMA_1_BASE + 0x60))
#define MCU_DMA_RG_GLB_CPU3_INT_SET    (*(volatile uint32_t *)(DMA_1_BASE + 0x64))
#define MCU_DMA_RG_GLB_CPU3_INT_CLR    (*(volatile uint32_t *)(DMA_1_BASE + 0x68))
#define MCU_DMA_RG_GLB_LIMITER         (*(volatile uint32_t *)(DMA_1_BASE + 0x28))
#define MCU_DMA_RG_GLB_SWRST           (*(volatile uint32_t *)(DMA_1_BASE + 0x20))
#define MCU_DMA_RG_GLB_BUSY            (*(volatile uint32_t *)(DMA_1_BASE + 0x40))
#define MCU_DMA_RG_GLB_INTR            (*(volatile uint32_t *)(DMA_1_BASE + 0x44))
#define MCU_DMA_RG_GLB_CLK_CFG         (*(volatile uint32_t *)(DMA_1_BASE + 0x70))
#define MCU_DMA_RG_GLB_CLK_SET         (*(volatile uint32_t *)(DMA_1_BASE + 0x74))
#define MCU_DMA_RG_GLB_CLK_CLR         (*(volatile uint32_t *)(DMA_1_BASE + 0x78))

#define UART_DMA_0_RG_GLB_STA          (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x00))
#define UART_DMA_0_RG_GLB_CPU0_INT_CFG (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x08))
#define UART_DMA_0_RG_GLB_CPU0_INT_SET (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x0C))
#define UART_DMA_0_RG_GLB_CPU0_INT_CLR (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x10))
#define UART_DMA_0_RG_GLB_CPU1_INT_CFG (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x14))
#define UART_DMA_0_RG_GLB_CPU1_INT_SET (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x18))
#define UART_DMA_0_RG_GLB_CPU1_INT_CLR (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x1C))
#define UART_DMA_0_RG_GLB_CPU2_INT_CFG (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x50))
#define UART_DMA_0_RG_GLB_CPU2_INT_SET (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x54))
#define UART_DMA_0_RG_GLB_CPU2_INT_CLR (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x58))
#define UART_DMA_0_RG_GLB_CPU3_INT_CFG (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x60))
#define UART_DMA_0_RG_GLB_CPU3_INT_SET (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x64))
#define UART_DMA_0_RG_GLB_CPU3_INT_CLR (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x68))
#define UART_DMA_0_RG_GLB_LIMITER      (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x28))
#define UART_DMA_0_RG_GLB_SWRST        (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x20))
#define UART_DMA_0_RG_GLB_BUSY         (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x40))
#define UART_DMA_0_RG_GLB_INTR         (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x44))
#define UART_DMA_0_RG_GLB_CLK_CFG      (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x70))
#define UART_DMA_0_RG_GLB_CLK_SET      (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x74))
#define UART_DMA_0_RG_GLB_CLK_CLR      (*(volatile uint32_t *)(UART_DMA_0_BASE + 0x78))

#define UART_DMA_1_RG_GLB_STA          (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x00))
#define UART_DMA_1_RG_GLB_CPU0_INT_CFG (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x08))
#define UART_DMA_1_RG_GLB_CPU0_INT_SET (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x0C))
#define UART_DMA_1_RG_GLB_CPU0_INT_CLR (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x10))
#define UART_DMA_1_RG_GLB_CPU1_INT_CFG (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x14))
#define UART_DMA_1_RG_GLB_CPU1_INT_SET (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x18))
#define UART_DMA_1_RG_GLB_CPU1_INT_CLR (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x1C))
#define UART_DMA_1_RG_GLB_CPU2_INT_CFG (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x50))
#define UART_DMA_1_RG_GLB_CPU2_INT_SET (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x54))
#define UART_DMA_1_RG_GLB_CPU2_INT_CLR (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x58))
#define UART_DMA_1_RG_GLB_CPU3_INT_CFG (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x60))
#define UART_DMA_1_RG_GLB_CPU3_INT_SET (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x64))
#define UART_DMA_1_RG_GLB_CPU3_INT_CLR (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x68))
#define UART_DMA_1_RG_GLB_LIMITER      (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x28))
#define UART_DMA_1_RG_GLB_SWRST        (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x20))
#define UART_DMA_1_RG_GLB_BUSY         (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x40))
#define UART_DMA_1_RG_GLB_INTR         (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x44))
#define UART_DMA_1_RG_GLB_CLK_CFG      (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x70))
#define UART_DMA_1_RG_GLB_CLK_SET      (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x74))
#define UART_DMA_1_RG_GLB_CLK_CLR      (*(volatile uint32_t *)(UART_DMA_1_BASE + 0x78))

#define UART_DMA_2_RG_GLB_STA          (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x00))
#define UART_DMA_2_RG_GLB_CPU0_INT_CFG (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x08))
#define UART_DMA_2_RG_GLB_CPU0_INT_SET (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x0C))
#define UART_DMA_2_RG_GLB_CPU0_INT_CLR (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x10))
#define UART_DMA_2_RG_GLB_CPU1_INT_CFG (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x14))
#define UART_DMA_2_RG_GLB_CPU1_INT_SET (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x18))
#define UART_DMA_2_RG_GLB_CPU1_INT_CLR (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x1C))
#define UART_DMA_2_RG_GLB_CPU2_INT_CFG (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x50))
#define UART_DMA_2_RG_GLB_CPU2_INT_SET (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x54))
#define UART_DMA_2_RG_GLB_CPU2_INT_CLR (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x58))
#define UART_DMA_2_RG_GLB_CPU3_INT_CFG (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x60))
#define UART_DMA_2_RG_GLB_CPU3_INT_SET (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x64))
#define UART_DMA_2_RG_GLB_CPU3_INT_CLR (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x68))
#define UART_DMA_2_RG_GLB_LIMITER      (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x28))
#define UART_DMA_2_RG_GLB_SWRST        (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x20))
#define UART_DMA_2_RG_GLB_BUSY         (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x40))
#define UART_DMA_2_RG_GLB_INTR         (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x44))
#define UART_DMA_2_RG_GLB_CLK_CFG      (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x70))
#define UART_DMA_2_RG_GLB_CLK_SET      (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x74))
#define UART_DMA_2_RG_GLB_CLK_CLR      (*(volatile uint32_t *)(UART_DMA_2_BASE + 0x78))


#define I2C_DMA_0_RG_GLB_STA          (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x00))
#define I2C_DMA_0_RG_GLB_CPU0_INT_CFG (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x08))
#define I2C_DMA_0_RG_GLB_CPU0_INT_SET (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x0C))
#define I2C_DMA_0_RG_GLB_CPU0_INT_CLR (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x10))
#define I2C_DMA_0_RG_GLB_CPU1_INT_CFG (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x14))
#define I2C_DMA_0_RG_GLB_CPU1_INT_SET (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x18))
#define I2C_DMA_0_RG_GLB_CPU1_INT_CLR (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x1C))
#define I2C_DMA_0_RG_GLB_CPU2_INT_CFG (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x50))
#define I2C_DMA_0_RG_GLB_CPU2_INT_SET (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x54))
#define I2C_DMA_0_RG_GLB_CPU2_INT_CLR (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x58))
#define I2C_DMA_0_RG_GLB_CPU3_INT_CFG (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x60))
#define I2C_DMA_0_RG_GLB_CPU3_INT_SET (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x64))
#define I2C_DMA_0_RG_GLB_CPU3_INT_CLR (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x68))
#define I2C_DMA_0_RG_GLB_LIMITER      (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x28))
#define I2C_DMA_0_RG_GLB_SWRST        (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x20))
#define I2C_DMA_0_RG_GLB_BUSY         (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x40))
#define I2C_DMA_0_RG_GLB_INTR         (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x44))
#define I2C_DMA_0_RG_GLB_CLK_CFG      (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x70))
#define I2C_DMA_0_RG_GLB_CLK_SET      (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x74))
#define I2C_DMA_0_RG_GLB_CLK_CLR      (*(volatile uint32_t *)(I2C0_PDMA_BASE + 0x78))

#define I2C_DMA_1_RG_GLB_STA          (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x00))
#define I2C_DMA_1_RG_GLB_CPU0_INT_CFG (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x08))
#define I2C_DMA_1_RG_GLB_CPU0_INT_SET (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x0C))
#define I2C_DMA_1_RG_GLB_CPU0_INT_CLR (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x10))
#define I2C_DMA_1_RG_GLB_CPU1_INT_CFG (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x14))
#define I2C_DMA_1_RG_GLB_CPU1_INT_SET (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x18))
#define I2C_DMA_1_RG_GLB_CPU1_INT_CLR (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x1C))
#define I2C_DMA_1_RG_GLB_CPU2_INT_CFG (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x50))
#define I2C_DMA_1_RG_GLB_CPU2_INT_SET (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x54))
#define I2C_DMA_1_RG_GLB_CPU2_INT_CLR (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x58))
#define I2C_DMA_1_RG_GLB_CPU3_INT_CFG (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x60))
#define I2C_DMA_1_RG_GLB_CPU3_INT_SET (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x64))
#define I2C_DMA_1_RG_GLB_CPU3_INT_CLR (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x68))
#define I2C_DMA_1_RG_GLB_LIMITER      (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x28))
#define I2C_DMA_1_RG_GLB_SWRST        (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x20))
#define I2C_DMA_1_RG_GLB_BUSY         (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x40))
#define I2C_DMA_1_RG_GLB_INTR         (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x44))
#define I2C_DMA_1_RG_GLB_CLK_CFG      (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x70))
#define I2C_DMA_1_RG_GLB_CLK_SET      (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x74))
#define I2C_DMA_1_RG_GLB_CLK_CLR      (*(volatile uint32_t *)(I2C1_PDMA_BASE + 0x78))

#define I2C_DMA_2_RG_GLB_STA          (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x00))
#define I2C_DMA_2_RG_GLB_CPU0_INT_CFG (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x08))
#define I2C_DMA_2_RG_GLB_CPU0_INT_SET (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x0C))
#define I2C_DMA_2_RG_GLB_CPU0_INT_CLR (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x10))
#define I2C_DMA_2_RG_GLB_CPU1_INT_CFG (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x14))
#define I2C_DMA_2_RG_GLB_CPU1_INT_SET (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x18))
#define I2C_DMA_2_RG_GLB_CPU1_INT_CLR (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x1C))
#define I2C_DMA_2_RG_GLB_CPU2_INT_CFG (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x50))
#define I2C_DMA_2_RG_GLB_CPU2_INT_SET (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x54))
#define I2C_DMA_2_RG_GLB_CPU2_INT_CLR (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x58))
#define I2C_DMA_2_RG_GLB_CPU3_INT_CFG (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x60))
#define I2C_DMA_2_RG_GLB_CPU3_INT_SET (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x64))
#define I2C_DMA_2_RG_GLB_CPU3_INT_CLR (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x68))
#define I2C_DMA_2_RG_GLB_LIMITER      (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x28))
#define I2C_DMA_2_RG_GLB_SWRST        (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x20))
#define I2C_DMA_2_RG_GLB_BUSY         (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x40))
#define I2C_DMA_2_RG_GLB_INTR         (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x44))
#define I2C_DMA_2_RG_GLB_CLK_CFG      (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x70))
#define I2C_DMA_2_RG_GLB_CLK_SET      (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x74))
#define I2C_DMA_2_RG_GLB_CLK_CLR      (*(volatile uint32_t *)(I2C2_PDMA_BASE + 0x78))

#define I3C_DMA_0_RG_GLB_STA          (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x00))
#define I3C_DMA_0_RG_GLB_CPU0_INT_CFG (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x08))
#define I3C_DMA_0_RG_GLB_CPU0_INT_SET (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x0C))
#define I3C_DMA_0_RG_GLB_CPU0_INT_CLR (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x10))
#define I3C_DMA_0_RG_GLB_CPU1_INT_CFG (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x14))
#define I3C_DMA_0_RG_GLB_CPU1_INT_SET (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x18))
#define I3C_DMA_0_RG_GLB_CPU1_INT_CLR (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x1C))
#define I3C_DMA_0_RG_GLB_CPU2_INT_CFG (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x50))
#define I3C_DMA_0_RG_GLB_CPU2_INT_SET (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x54))
#define I3C_DMA_0_RG_GLB_CPU2_INT_CLR (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x58))
#define I3C_DMA_0_RG_GLB_CPU3_INT_CFG (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x60))
#define I3C_DMA_0_RG_GLB_CPU3_INT_SET (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x64))
#define I3C_DMA_0_RG_GLB_CPU3_INT_CLR (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x68))
#define I3C_DMA_0_RG_GLB_LIMITER      (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x28))
#define I3C_DMA_0_RG_GLB_SWRST        (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x20))
#define I3C_DMA_0_RG_GLB_BUSY         (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x40))
#define I3C_DMA_0_RG_GLB_INTR         (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x44))
#define I3C_DMA_0_RG_GLB_CLK_CFG      (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x70))
#define I3C_DMA_0_RG_GLB_CLK_SET      (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x74))
#define I3C_DMA_0_RG_GLB_CLK_CLR      (*(volatile uint32_t *)(I3C0_PDMA_BASE + 0x78))

#define I3C_DMA_1_RG_GLB_STA          (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x00))
#define I3C_DMA_1_RG_GLB_CPU0_INT_CFG (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x08))
#define I3C_DMA_1_RG_GLB_CPU0_INT_SET (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x0C))
#define I3C_DMA_1_RG_GLB_CPU0_INT_CLR (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x10))
#define I3C_DMA_1_RG_GLB_CPU1_INT_CFG (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x14))
#define I3C_DMA_1_RG_GLB_CPU1_INT_SET (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x18))
#define I3C_DMA_1_RG_GLB_CPU1_INT_CLR (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x1C))
#define I3C_DMA_1_RG_GLB_CPU2_INT_CFG (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x50))
#define I3C_DMA_1_RG_GLB_CPU2_INT_SET (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x54))
#define I3C_DMA_1_RG_GLB_CPU2_INT_CLR (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x58))
#define I3C_DMA_1_RG_GLB_CPU3_INT_CFG (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x60))
#define I3C_DMA_1_RG_GLB_CPU3_INT_SET (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x64))
#define I3C_DMA_1_RG_GLB_CPU3_INT_CLR (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x68))
#define I3C_DMA_1_RG_GLB_LIMITER      (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x28))
#define I3C_DMA_1_RG_GLB_SWRST        (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x20))
#define I3C_DMA_1_RG_GLB_BUSY         (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x40))
#define I3C_DMA_1_RG_GLB_INTR         (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x44))
#define I3C_DMA_1_RG_GLB_CLK_CFG      (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x70))
#define I3C_DMA_1_RG_GLB_CLK_SET      (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x74))
#define I3C_DMA_1_RG_GLB_CLK_CLR      (*(volatile uint32_t *)(I3C1_PDMA_BASE + 0x78))

#define I2S_DMA_RG_GLB_STA             (*(volatile uint32_t *)(I2S_DMA_BASE + 0x00))
#define I2S_DMA_RG_GLB_CPU0_INT_CFG    (*(volatile uint32_t *)(I2S_DMA_BASE + 0x08))
#define I2S_DMA_RG_GLB_CPU0_INT_SET    (*(volatile uint32_t *)(I2S_DMA_BASE + 0x0C))
#define I2S_DMA_RG_GLB_CPU0_INT_CLR    (*(volatile uint32_t *)(I2S_DMA_BASE + 0x10))
#define I2S_DMA_RG_GLB_CPU1_INT_CFG    (*(volatile uint32_t *)(I2S_DMA_BASE + 0x14))
#define I2S_DMA_RG_GLB_CPU1_INT_SET    (*(volatile uint32_t *)(I2S_DMA_BASE + 0x18))
#define I2S_DMA_RG_GLB_CPU1_INT_CLR    (*(volatile uint32_t *)(I2S_DMA_BASE + 0x1C))
#define I2S_DMA_RG_GLB_LIMITER         (*(volatile uint32_t *)(I2S_DMA_BASE + 0x28))
#define I2S_DMA_RG_GLB_SWRST           (*(volatile uint32_t *)(I2S_DMA_BASE + 0x20))
#define I2S_DMA_RG_GLB_BUSY            (*(volatile uint32_t *)(I2S_DMA_BASE + 0x40))
#define I2S_DMA_RG_GLB_INTR            (*(volatile uint32_t *)(I2S_DMA_BASE + 0x44))
#define I2S_DMA_RG_GLB_CLK_CFG         (*(volatile uint32_t *)(I2S_DMA_BASE + 0x70))
#define I2S_DMA_RG_GLB_CLK_SET         (*(volatile uint32_t *)(I2S_DMA_BASE + 0x74))
#define I2S_DMA_RG_GLB_CLK_CLR         (*(volatile uint32_t *)(I2S_DMA_BASE + 0x78))

/* GDMA channel register definition */
typedef struct {
    __IO uint32_t GDMA_SRC;          /*!<  general dma source address register */
    __IO uint32_t GDMA_DST;          /*!<  general dma destination address register */
    __IO uint32_t GDMA_WPPT;         /*!<  general dma wrap point address register */
    __IO uint32_t GDMA_WPTO;         /*!<  general dma wrap to address register */
    __IO uint32_t GDMA_COUNT;        /*!<  general dma transfer counter  register */
    union {
        struct {
            __IO uint8_t GDMA_DIRECTION ; /* general dma incremental source address */
            __IO uint8_t GDMA_SIZE;       /* general dma data size */
            __IO uint8_t GDMA_SETTING;    /* general dma throttle and handshake control for dma transter */
            __IO uint8_t GDMA_ITEN;       /* general dma interrupt enable */
        } GDMA_CON_CELLS;
        __IO uint32_t GDMA_CON;           /*!<  general dma control register */
    } GDMA_CON_UNION;
    __IO uint32_t GDMA_START;        /*!<  general dma start register */
    __IO uint32_t GDMA_INTSTA;       /*!<  general dma interrupt status register*/
    __O  uint32_t GDMA_ACKINT;       /*!<  generall dma interrupt acknowledge register*/
    __I  uint32_t GDMA_RLCT;         /*!<  general dma remaining length of current transfer register*/
    __IO uint32_t GDMA_LIMITER;      /*!<  general dma bandwidth limiter*/
} GDMA_REGISTER_T;

/* GDMA_CON definition */
#define GDMA_CON_SINC_OFFSET                 (0)
#define GDMA_CON_SINC_MASK                   (0x1 << GDMA_CON_SINC_OFFSET)

#define GDMA_CON_DINC_OFFSET                 (1)
#define GDMA_CON_DINC_MASK                   (0x1 << GDMA_CON_DINC_OFFSET)

#define GDMA_CON_WPSD_OFFSET                 (2)
#define GDMA_CON_WPSD_MASK                   (0x1 << GDMA_CON_WPSD_OFFSET)

#define GDMA_CON_WPEN_OFFSET                 (3)
#define GDMA_CON_WPEN_MASK                   (0x1 << GDMA_CON_WPEN_OFFSET)

#define GDMA_CON_SIZE_OFFSET                 (8)
#define GDMA_CON_SIZE_MASK                   (0x3 << GDMA_CON_SIZE_OFFSET)
#define GDMA_CON_SIZE_BYTE                    0
#define GDMA_CON_SIZE_HALF_WORD               1
#define GDMA_CON_SIZE_WORD                    2

#define GDMA_CON_DREQ_OFFSET                 (16)
#define GDMA_CON_DREQ_MASK                   (0x1 << GDMA_CON_DREQ_OFFSET)

#define GDMA_CON_BURST_OFFSET                (18)
#define GDMA_CON_BURST_MASK                  (0x3 << GDMA_CON_BURST_OFFSET)
#define GDMA_CON_BURST_SINGLE                 0
#define GDMA_CON_BURST_4_BEAT                 2

#define GDMA_CON_ITEN_OFFSET                 (24)
#define GDMA_CON_ITEN_MASK                   (0x1 << GDMA_CON_ITEN_OFFSET)

/* GDMA_COUNT definition */
#define GDMA_COUNT_BIT_OFFSET                (0)
#define GDMA_COUNT_BIT_MASK                  (0xFFFF << GDMA_COUNT_BIT_OFFSET)

/* GDMA_START definition */
#define GDMA_START_BIT_OFFSET                (15)
#define GDMA_START_BIT_MASK                  (0x1 << GDMA_START_BIT_OFFSET)

/* GDMA_INTSTA definition */
#define GDMA_INTSTA_BIT_OFFSET               (15)
#define GDMA_INTSTA_BIT_MASK                 (0x1 << GDMA_INTSTA_BIT_OFFSET)

/* GDMA_ACKINT definition */
#define GDMA_ACKINT_BIT_OFFSET               (15)
#define GDMA_ACKINT_BIT_MASK                 (0x1 << GDMA_ACKINT_BIT_OFFSET)

/* GDMA_GLB index definition */
#define GDMA_GLB_INDEX                       (0)

/* PDMA channel register definition */
typedef struct {
    __IO uint32_t PDMA_WPPT;         /*!<  peripheral dma wrap point address register */
    __IO uint32_t PDMA_WPTO;         /*!<  peripheral dma wrap to address register */
    __IO uint32_t PDMA_COUNT;        /*!<  peripheral dma transfer counter  register */
    union {
        struct {
            __IO uint8_t PDMA_DIRECTION ; /* peripheral dma incremental source address */
            __IO uint8_t PDMA_SIZE;       /* peripheral dma data size */
            __IO uint8_t PDMA_SETTING;    /* peripheral dma throttle and handshake control for dma transter */
            __IO uint8_t PDMA_ITEN;       /* peripheral dma interrupt enable */
        } PDMA_CON_CELLS;
        __IO uint32_t PDMA_CON;           /*!<  peripheral dma control register */
    } PDMA_CON_UNION;
    __IO uint32_t PDMA_START;        /*!<  peripheral dma start register */
    __IO uint32_t PDMA_INTSTA;       /*!<  peripheral dma interrupt status register*/
    __O  uint32_t PDMA_ACKINT;       /*!<  peripheral dma interrupt acknowledge register*/
    __I  uint32_t PDMA_RLCT;         /*!<  peripheral dma remaining length of current transfer register*/
    __IO uint32_t PDMA_LIMITER;      /*!< peripheral  dma bandwidth register*/
    __IO uint32_t PDMA_PGMADDR;      /*!<  peripheral dma programmable address register*/
} PDMA_REGISTER_T;

/* PDMA_CON definition */
#define PDMA_CON_SINC_OFFSET                 (0)
#define PDMA_CON_SINC_MASK                   (0x1 << PDMA_CON_SINC_OFFSET)

#define PDMA_CON_DINC_OFFSET                 (1)
#define PDMA_CON_DINC_MASK                   (0x1 << PDMA_CON_DINC_OFFSET)

#define PDMA_CON_WPSD_OFFSET                 (2)
#define PDMA_CON_WPSD_MASK                   (0x1 << PDMA_CON_WPSD_OFFSET)

#define PDMA_CON_WPEN_OFFSET                 (3)
#define PDMA_CON_WPEN_MASK                   (0x1 << PDMA_CON_WPEN_OFFSET)

#define PDMA_CON_DIR_OFFSET                  (4)
#define PDMA_CON_DIR_MASK                    (0x1 << PDMA_CON_DIR_OFFSET)
#define PDMA_CON_DIR_TX                       0
#define PDMA_CON_DIR_RX                       1

#define PDMA_CON_SIZE_OFFSET                 (8)
#define PDMA_CON_SIZE_MASK                   (0x3 << PDMA_CON_SIZE_OFFSET)
#define PDMA_CON_SIZE_BYTE                    0
#define PDMA_CON_SIZE_HALF_WORD               1
#define PDMA_CON_SIZE_WORD                    2

#define PDMA_CON_DREQ_OFFSET                 (16)
#define PDMA_CON_DREQ_MASK                   (0x1 << PDMA_CON_DREQ_OFFSET)

#define PDMA_CON_B2W_OFFSET                  (17)
#define PDMA_CON_B2W_MASK                    (0x1 << PDMA_CON_B2W_OFFSET)

#define PDMA_CON_BURST_OFFSET                (18)
#define PDMA_CON_BURST_MASK                  (0x3 << PDMA_CON_BURST_OFFSET)
#define PDMA_CON_BURST_SINGLE                 0
#define PDMA_CON_BURST_4_BEAT                 2

#define PDMA_CON_ITEN_OFFSET                 (24)
#define PDMA_CON_ITEN_MASK                   (0x1 << PDMA_CON_ITEN_OFFSET)

/* PDMA_START definition */
#define PDMA_START_BIT_OFFSET                (15)
#define PDMA_START_BIT_MASK                  (0x1 << PDMA_START_BIT_OFFSET)

/* PDMA_INTSTA definition */
#define PDMA_INTSTA_BIT_OFFSET               (15)
#define PDMA_INTSTA_BIT_MASK                 (0x1 << PDMA_INTSTA_BIT_OFFSET)

/* PDMA_ACKINT definition */
#define PDMA_ACKINT_BIT_OFFSET               (15)
#define PDMA_ACKINT_BIT_MASK                 (0x1 << PDMA_ACKINT_BIT_OFFSET)

/* PDMA_GLB index definition */
#define PDMA_GLB_INDEX_OFFSET                (0)
#define PDMA_GLB_INDEX_MASK                  (0x1 << PDMA_GLB_INDEX_OFFSET)

/* VDMA channel register definition */
typedef struct {
    __IO uint32_t VDMA_COUNT;        /*!<  virtual fifo dma transfer counter  register */
    union {
        struct {
            __IO uint8_t VDMA_DIRECTION ; /* virtual fifo dma incremental source address */
            __IO uint8_t VDMA_SIZE;       /* virtual fifo dma data size */
            __IO uint8_t VDMA_SETTING;    /* virtual fifo dma throttle and handshake control for dma transter */
            __IO uint8_t VDMA_ITEN;      /* virtual fifo dma interrupt enable */
        } VDMA_CON_CELLS;
        __IO uint32_t VDMA_CON;          /*!<  peripheral dma control register */
    } VDMA_CON_UNION;
    __IO uint32_t VDMA_START;        /*!<  virtual fifo dma start register */
    __IO uint32_t VDMA_INTSTA;       /*!<  virtual fifol dma interrupt status register*/
    __O  uint32_t VDMA_ACKINT;       /*!<  virtual fifo dma interrupt acknowledge register*/
    __IO uint32_t DUMMY1_OFFSET[1];  /*!< virtual fifo dma dummy offser register*/
    __O  uint32_t VDMA_LIMITER;      /*!< virtual fifo dma bandwidth register*/
    __IO uint32_t VDMA_PGMADDR;      /*!<  virtual fifo dma programmable address register*/
    __I  uint32_t VDMA_WRPTR;        /*!<  virtual fifo dma write pointer register */
    __I  uint32_t VDMA_RDPTR;        /*!<  virtual fifo dma read  pointer register */
    __I  uint32_t VDMA_FFCNT;        /*!<  virtual fifo dma fifo count register */
    __I  uint32_t VDMA_FFSTA;        /*!<  virtual fifo dma fifo status  register */
    __IO uint32_t VDMA_ALTLEN;       /*!<  virtual fifo dma fifo alert lentgh register */
    __IO uint32_t VDMA_FFSIZE;       /*!<  virtual fifo dma fifo size  register */
    __IO uint32_t DUMMY2_OFFSET[6];  /*!< virtual fifo dma dummy offser register*/
    __IO uint32_t VDMA_SW_MV_BYTE;   /*!<  virtual fifo dma software move byte  register */
    __IO uint32_t VDMA_BNDRY_ADDR;     /*!<  virtual fifo dma boundary address  register */
    __IO uint32_t VDMA_BYTE_TO_BNDRY;  /*!<  virtual fifo dma byte to boundary address register */
    __IO uint32_t VDMA_BYTE_AVAIL;     /*!<  virtual fifo dma fifo byte avaiable in FIFO register */
} VDMA_REGISTER_T;

/* VDMA_CON definition */
#define VDMA_CON_DIR_OFFSET                  (4)
#define VDMA_CON_DIR_MASK                    (0x1 << VDMA_CON_DIR_OFFSET)
#define VDMA_CON_DIR_TX                       0
#define VDMA_CON_DIR_RX                       1

#define VDMA_CON_SIZE_OFFSET                 (8)
#define VDMA_CON_SIZE_MASK                   (0x3 << VDMA_CON_SIZE_OFFSET)
#define VDMA_CON_SIZE_BYTE                    0
#define VDMA_CON_SIZE_HALF_WORD               1
#define VDMA_CON_SIZE_WORD                    2

#define VDMA_CON_DREQ_OFFSET                 (16)
#define VDMA_CON_DREQ_MASK                   (0x1 << VDMA_CON_DREQ_OFFSET)

#define VDMA_CON_ITEN_OFFSET                 (24)
#define VDMA_CON_ITEN_MASK                   (0x1 << VDMA_CON_ITEN_OFFSET)

/* VDMA_START definition */
#define VDMA_START_BIT_OFFSET                (15)
#define VDMA_START_BIT_MASK                  (0x1 << VDMA_START_BIT_OFFSET)

/* VDMA_INTSTA definition */
#define VDMA_INTSTA_BIT_OFFSET               (15)
#define VDMA_INTSTA_BIT_MASK                 (0x1 << VDMA_INTSTA_BIT_OFFSET)

/* VDMA_ACKINT definition */
#define VDMA_ACKINT_BIT_OFFSET               (15)
#define VDMA_ACKINT_BIT_MASK                 (0x1 << VDMA_ACKINT_BIT_OFFSET)

/* VDMA_FFSTA definition */
#define VDMA_FFSTA_FULL_BIT_OFFSET           (0)
#define VDMA_FFSTA_FULL_BIT_MASK             (0x1 << VDMA_FFSTA_FULL_BIT_OFFSET)
#define VDMA_FFSTA_EMPTY_BIT_OFFSET          (1)
#define VDMA_FFSTA_EMPTY_BIT_MASK            (0x1 << VDMA_FFSTA_EMPTY_BIT_OFFSET)
#define VDMA_FFSTA_ALT_BIT_OFFSET            (2)
#define VDMA_FFSTA_ALT_BIT_MASK              (0x1 << VDMA_FFSTA_ALT_BIT_OFFSET)

/* VDMA_GLB index definition */
#define VDMA_GLB_INDEX_OFFSET                (0)
#define VDMA_GLB_INDEX_MASK                  (0x1 << VDMA_GLB_INDEX_OFFSET)

/* base address definition for channels */
#define GDMA_RG_0_BASE              ((GDMA_REGISTER_T *)(DMA_0_BASE + 0x100))
#define GDMA_RG_1_0_BASE            ((GDMA_REGISTER_T *)(DMA_1_BASE + 0x100))
#define GDMA_RG_1_1_BASE            ((GDMA_REGISTER_T *)(DMA_1_BASE + 0x200))
#define VDMA_RG_UART0_TX_BASE       ((VDMA_REGISTER_T *)(UART_DMA_0_BASE + 0x110))
#define VDMA_RG_UART0_RX_BASE       ((VDMA_REGISTER_T *)(UART_DMA_0_BASE + 0x210))
#define VDMA_RG_UART1_TX_BASE       ((VDMA_REGISTER_T *)(UART_DMA_1_BASE + 0x110))
#define VDMA_RG_UART1_RX_BASE       ((VDMA_REGISTER_T *)(UART_DMA_1_BASE + 0x210))
#define VDMA_RG_UART2_TX_BASE       ((VDMA_REGISTER_T *)(UART_DMA_2_BASE + 0x110))
#define VDMA_RG_UART2_RX_BASE       ((VDMA_REGISTER_T *)(UART_DMA_2_BASE + 0x210))
#define PDMA_RG_I2C0_TX_BASE        ((PDMA_REGISTER_T *)(I2C0_PDMA_BASE + 0x108))
#define PDMA_RG_I2C0_RX_BASE        ((PDMA_REGISTER_T *)(I2C0_PDMA_BASE + 0x208))
#define PDMA_RG_I2C1_TX_BASE        ((PDMA_REGISTER_T *)(I2C1_PDMA_BASE + 0x108))
#define PDMA_RG_I2C1_RX_BASE        ((PDMA_REGISTER_T *)(I2C1_PDMA_BASE + 0x208))
#define PDMA_RG_I2C2_TX_BASE        ((PDMA_REGISTER_T *)(I2C2_PDMA_BASE + 0x108))
#define PDMA_RG_I2C2_RX_BASE        ((PDMA_REGISTER_T *)(I2C2_PDMA_BASE + 0x208))
#define PDMA_RG_I3C0_TX_BASE        ((PDMA_REGISTER_T *)(I3C0_PDMA_BASE + 0x108))
#define PDMA_RG_I3C0_RX_BASE        ((PDMA_REGISTER_T *)(I3C0_PDMA_BASE + 0x208))
#define PDMA_RG_I3C1_TX_BASE        ((PDMA_REGISTER_T *)(I3C1_PDMA_BASE + 0x108))
#define PDMA_RG_I3C1_RX_BASE        ((PDMA_REGISTER_T *)(I3C1_PDMA_BASE + 0x208))
#define VDMA_RG_I2S0_TX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0x110))
#define VDMA_RG_I2S0_RX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0x210))
#define VDMA_RG_I2S1_TX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0x310))
#define VDMA_RG_I2S1_RX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0x410))
#define VDMA_RG_I2S2_TX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0x510))
#define VDMA_RG_I2S2_RX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0x610))
#define VDMA_RG_I2S3_TX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0x710))
#define VDMA_RG_I2S3_RX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0x810))
#define VDMA_RG_I2S4_TX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0x910))
#define VDMA_RG_I2S4_RX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0xA10))
#define VDMA_RG_I2S5_TX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0xB10))
#define VDMA_RG_I2S5_RX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0xC10))
#define VDMA_RG_I2S6_TX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0xD10))
#define VDMA_RG_I2S6_RX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0xE10))
#define VDMA_RG_I2S7_TX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0xF10))
#define VDMA_RG_I2S7_RX_BASE        ((VDMA_REGISTER_T *)(I2S_DMA_BASE + 0x1010))

/************************ dma end register definition end line  ********************************/



/************************ GPT end register definition start line  *******************************
 */
typedef struct {
    __I   uint32_t GPT_IRQSTA;
    __IO  uint32_t GPT_IRQMSK0;
    __IO  uint32_t GPT_IRQMSK1;
    __IO  uint32_t GPT_IRQMSK2;
    __IO  uint32_t GPT_WAKEUPMSK0;
    __IO  uint32_t GPT_WAKEUPMSK1;
    __IO  uint32_t GPT_WAKEUPMSK2;
    __I   uint32_t GPT_CLRSTA;
    __I   uint32_t GPT_WCOMPSTA;
} GPT_REGISTER_GLOABL_T;

typedef struct {
    union {
        struct {
            __IO uint8_t EN;
            __IO uint8_t MODE;
            __IO uint8_t SW_CG;
            __I  uint8_t RESERVED[1];
        } GPT_CON_CELLS;
        __IO uint32_t GPT_CON;
    } GPT_CON_UNION; /*!< timer enable and mode config*/
    __O uint32_t GPT_CLR ;
    __IO uint32_t GPT_CLK ; /*!< clock select and prescale config*/
    __IO uint32_t GPT_IRQ_EN ; /*!< interrupt enable*/
    __I uint32_t GPT_IRQ_STA ; /*!< interrupt status*/
    __O uint32_t GPT_IRQ_ACK; /*!< interrupt ack*/
    __IO uint32_t GPT_COUNT ; /*!< gpt0~4 count,gpt5 low word*/
    __IO uint32_t GPT_COMPARE ; /*!< gpt0~4 threshold,gpt5 low word*/
} GPT_REGISTER_T;


#define _GPT_BASE_ (GPT_BASE)
#define _GPT_BASEADDR_ (_GPT_BASE_ + 0x000)

#define GPT0 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0x000))
#define GPT1 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0x100))
#define GPT2 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0x200))
#define GPT3 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0x300))
#define GPT4 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0x400))
#define GPT5 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0x500))
#define GPT6 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0x600))
#define GPT7 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0x700))
#define GPT8 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0x800))
#define GPT9 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0x900))
#define GPT10 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0xA00))
#define GPT11 ((GPT_REGISTER_T*)(_GPT_BASE_ + 0xB00))
#define GPTGLB ((GPT_REGISTER_GLOABL_T*)(_GPT_BASE_ + 0xC00))
#define GPT(x) ((GPT_REGISTER_T*)(_GPT_BASE_ + (0x100*x)))


typedef struct {
    __I   uint32_t OS_GPT_IRQSTA;
    __IO  uint32_t OS_GPT_IRQMSK_CM4;
    __IO  uint32_t OS_GPT_IRQMSK_DSP;
    __IO  uint32_t OS_GPT_WAKEUPMSK_CM4;
    __IO  uint32_t OS_GPT_WAKEUPMSK_DSP;
    __I   uint32_t OS_GPT_CLRSTA;
    __I   uint32_t OS_GPT_WCOMPSTA;
} OS_GPT_REGISTER_GLOABL_T;

#define OS_GPT0     ((GPT_REGISTER_T*)(OS_GPT_BASE + 0x0))
#define OS_GPT1     ((GPT_REGISTER_T*)(OS_GPT_BASE + 0x100))
#define OS_GPTGLB   ((OS_GPT_REGISTER_GLOABL_T*)(OS_GPT_BASE + 0x200))


/************************ GPT end register definition end line  *******************************
 */



/************************ CACHE end register definition start line  ***************************
 */
/* structure type to access the CACHE register
 */
typedef struct {
    __IO uint32_t CACHE_CON;
    __IO uint32_t CACHE_OP;
    __IO uint32_t CACHE_HCNT0L;
    __IO uint32_t CACHE_HCNT0U;
    __IO uint32_t CACHE_CCNT0L;
    __IO uint32_t CACHE_CCNT0U;
    __IO uint32_t CACHE_HCNT1L;
    __IO uint32_t CACHE_HCNT1U;
    __IO uint32_t CACHE_CCNT1L;
    __IO uint32_t CACHE_CCNT1U;
    uint32_t RESERVED0[1];
    __IO uint32_t CACHE_REGION_EN;
    uint32_t RESERVED1[16372];                  /**< (0x10000-12*4)/4 */
    __IO uint32_t CACHE_ENTRY_N[16];
    __IO uint32_t CACHE_END_ENTRY_N[16];
} CACHE_REGISTER_T;

/* CACHE_CON register definitions
 */
#define CACHE_CON_MCEN_OFFSET                   (0)
#define CACHE_CON_MCEN_MASK                     (0x1UL<<CACHE_CON_MCEN_OFFSET)

#define CACHE_CON_CNTEN0_OFFSET                 (2)
#define CACHE_CON_CNTEN0_MASK                   (0x1UL<<CACHE_CON_CNTEN0_OFFSET)

#define CACHE_CON_CNTEN1_OFFSET                 (3)
#define CACHE_CON_CNTEN1_MASK                   (0x1UL<<CACHE_CON_CNTEN1_OFFSET)

#define CACHE_CON_MDRF_OFFSET                   (7)
#define CACHE_CON_MDRF_MASK                     (0x1UL<<CACHE_CON_MDRF_OFFSET)

#define CACHE_CON_CACHESIZE_OFFSET              (8)
#define CACHE_CON_CACHESIZE_MASK                (0x3UL<<CACHE_CON_CACHESIZE_OFFSET)

/* CACHE_OP register definitions
 */
#define CACHE_OP_EN_OFFSET                      (0)
#define CACHE_OP_EN_MASK                        (0x1UL<<CACHE_OP_EN_OFFSET)

#define CACHE_OP_OP_OFFSET                      (1)
#define CACHE_OP_OP_MASK                        (0xFUL<<CACHE_OP_OP_OFFSET)

#define CACHE_OP_TADDR_OFFSET                   (5)
#define CACHE_OP_TADDR_MASK                     (0x7FFFFFFUL<<CACHE_OP_TADDR_OFFSET)

/* CACHE_HCNT0L register definitions
 */
#define CACHE_HCNT0L_CHIT_CNT0_MASK             (0xFFFFFFFFUL<<0)

/* CACHE_HCNT0U register definitions
 */
#define CACHE_HCNT0U_CHIT_CNT0_MASK             (0xFFFFUL<<0)

/* CACHE_CCNT0L register definitions
 */
#define CACHE_CCNT0L_CACC_CNT0_MASK             (0xFFFFFFFFUL<<0)

/* CACHE_CCNT0U register definitions
 */
#define CACHE_CCNT0U_CACC_CNT0_MASK             (0xFFFFUL<<0)

/* CACHE_HCNT1L register definitions
 */
#define CACHE_HCNT1L_CHIT_CNT1_MASK             (0xFFFFFFFFUL<<0)

/* CACHE_HCNT1U register definitions
 */
#define CACHE_HCNT1U_CHIT_CNT1_MASK             (0xFFFFUL<<0)

/* CACHE_CCNT1L register definitions
 */
#define CACHE_CCNT1L_CACC_CNT1_MASK             (0xFFFFFFFFUL<<0)

/* CACHE_CCNT1U register definitions
 */
#define CACHE_CCNT1U_CACC_CNT1_MASK             (0xFFFFUL<<0)

/* CACHE_ENTRY_N register definitions
 */
#define CACHE_ENTRY_N_C_MASK                    (0x1UL<<8)
#define CACHE_ENTRY_BASEADDR_MASK               (0xFFFFFUL<<12)

/* CACHE_END_ENTRY_N register definitions
 */
#define CACHE_END_ENTRY_N_BASEADDR_MASK         (0xFFFFFUL<<12)

/* memory mapping of MT7687
 */
#define CACHE                                   ((CACHE_REGISTER_T *)CMSYS_L1CACHE_BASE)

/************************ CACHE end register definition end line  ***********************
 */



/* structure type to access the ADC register
 */
typedef struct {
    uint32_t RESERVED0[1];
    __IO uint32_t AUXADC_CON1;
    union {
        struct {
            __I uint8_t ADC_STAT;
            __IO uint8_t SOFT_RST;
            __IO uint8_t RESERVED0;
            __IO uint8_t RESERVED1;
        } AUXADC_CON3_CELLS;
        __IO uint32_t AUXADC_CON3;
    } AUXADC_CON3_UNION;
    __IO uint32_t AUXADC_CON4;
    __I  uint32_t AUXADC_DATA0;
    __I  uint32_t AUXADC_DATA1;
    __I  uint32_t AUXADC_DATA2;
    __I  uint32_t AUXADC_DATA3;
    __I  uint32_t AUXADC_DATA4;
    __I  uint32_t AUXADC_DATA5;
    __I  uint32_t AUXADC_DATA6;
    uint32_t RESERVED3[18];
    __IO  uint32_t MACRO_CON2;
    __IO  uint32_t ANA_EN_CON;
    __IO  uint32_t AUXADC_AVG_NUM;
    __I   uint32_t AUXADC_AVG_READY;
    __I   uint32_t AUXADC_AVG_DATA;
} ADC_REGISTER_T;

/* AUXADC_CON3 register definitions
 */
#define AUXADC_CON3_SOFT_RST_MASK                       (0x1UL)

#define AUXADC_CON3_ADC_STA_MASK                        (0x1UL)

/* AUXADC_CON4 register definitions
 */
#define AUXADC_CON4_AUTOSET_MASK                        (0x1UL)

/* MACRO_CON2 register definitions
 */
#define MACRO_CON2_RG_AUXADC_LDO_EN_MASK                (0x1UL)

/* ANA_EN_CON register definitions
 */
#define ANA_EN_CON_AUXADC_EN_MASK                       (0x1UL)


/* memory mapping of MT2822
 */
#define ADC                                     ((ADC_REGISTER_T *)AUXADC_BASE)



/************************ I2C end register definition start line  ***************************
 */
/* Register definitions for I2C Master */
typedef struct {
    __IO uint32_t DATA_PORT;         /* 00 */
    __IO uint32_t SLAVE_ADDR;        /* 04 */
    __IO uint32_t INTR_MASK;         /* 08 */
    __IO uint32_t INTR_STAT;         /* 0C */
    union {
        struct {
            __IO uint8_t RS_STOP;
            __IO uint8_t DMA_EN;
            __IO uint8_t DIR_CHANGE;
            __IO uint8_t TRANSFER_LEN_CHANGE;
        } CONTROL_CELLS;
        __IO uint32_t CONTROL;
    } CONTROL_UNION;             /* 10 */
    union {
        struct {
            __IO uint8_t HS_NACKERR_DET_EN;
            __IO uint8_t ACKERR_DET_EN;
            __IO uint8_t CLK_EXT_EN;
            __IO uint8_t RESERVED0;
        } CONTROL2_CELLS;
        __IO uint32_t CONTROL2;  /* 14 */
    } CONTROL2_UNION;
    __IO uint32_t RESERVED0[2];      /* 18/1C */
    __IO uint32_t TRANSFER_LEN;      /* 20 */
    __IO uint32_t TRANSFER_LEN_AUX;  /* 24 */
    __IO uint32_t TRANSAC_LEN;       /* 28 */
    __IO uint32_t DELAY_LEN;         /* 2C */
    union {
        struct {
            __IO uint8_t STEP_CNT_DIV;
            __IO uint8_t SAMPLE_CNT_DIV;
            __IO uint8_t DATA_READ_TIME;
            __IO uint8_t DATA_READ_ADJ;
        } TIMING_CELLS;
        __IO uint32_t TIMING;
    } TIMING_UNION;              /* 30 */
    __IO uint32_t CLOCK_DIV;         /* 34 */
    __IO uint32_t START;             /* 38 */
    __IO uint32_t RESERVED1;         /* 3C */
    union {
        struct {
            __I uint8_t WR_FULL_RD_EMPTY;
            __I uint8_t FIFO_OFFSET;
            __I uint8_t WR_ADDR;
            __I uint8_t RD_ADDR;
        } FIFO_STAT_CELLS;
        __I uint32_t FIFO_STAT;
    } FIFO_STAT_UNION;           /* 40 */
    __IO uint32_t RESERVED2;         /* 44 */
    __O  uint32_t FIFO_ADDR_CLR;     /* 48 */
    __IO uint32_t RESERVED3;         /* 4C */
    __IO uint32_t IO_CONFIG;         /* 50 */
    __IO uint32_t RESERVED4[3];      /* 54/58/5C */
    union {
        struct {
            __I uint8_t HS_EN;
            __I uint8_t MASTER_CODE;
            __I uint8_t HS_STEP_CNT_DIV;
            __I uint8_t HS_SAMPLE_CNT_DIV;
        } HS_CELLS;
        __I uint32_t HS;
    } HS_UNION;                  /* 60 */
    __IO uint32_t RESERVED5[3];      /* 64/68/6C */
    __O  uint32_t SOFTRESET;         /* 70 */
    __I  uint32_t DEBUGSTAT;         /* 74 */
    __IO uint32_t DEBUGCTRL;         /* 78 */
    __IO uint32_t RESERVED6;         /* 7C */
    __IO uint32_t SPARE;             /* 80 */
    __IO uint32_t RESERVED7[3];      /* 84/88/8C */
    union {
        struct {
            __IO uint16_t ACK_ERR_TRANSFER_NO;
            __IO uint16_t ACK_ERR_TRANSAC_NO;
        } ACKERR_FLAG_CELLS;
        __IO uint32_t ACKERR_FLAG;
    } ACKERR_FLAG_UNION;            /* 90 */
} I2C_REGISTER_T;

/************************ I2C end register definition end line  *****************************
 */



typedef struct {
    __IO uint32_t BUS_DCM_CON_0;        /* BUS_DCM_CON_0 @A21D0100 */
    __IO uint32_t BUS_DCM_CON_1;        /* BUS_DCM_CON_1 @A21D0104 */
    __IO uint32_t reserver1[2];         /* alignment for 0x110- 0x108*/
    __IO uint32_t CM4_DCM_CON_0;        /* CM4_DCM_CON_0 @A21D00110 */
    __IO uint32_t CM4_DCM_CON_1;        /* CM4_DCM_CON_1 @A21D00114 */
    __IO uint32_t reserver2[6];         /* alignment for 0x130-0x118*/
    __IO uint32_t SYS_FREE_DCM_CON;     /* SYS_FREE_DCM_CON @A21D130 */
    __IO uint32_t reserver3[3];         /* alignment for 0x140-0x134*/
    __IO uint32_t SFC_DCM_CON_0;        /* SFC_DCM_CON_0 @A21D0140 */
    __IO uint32_t SFC_DCM_CON_1;        /* SFC_DCM_CON_1 @A21D0144 */
    __IO uint32_t reserver4[10];        /* alignment for 0x170-0x148*/
    __IO uint32_t CLK_FREQ_SWCH;        /* CLK_FREQ_SWCH @A21D0170 */
    __IO uint32_t reserver5[99];        /* alignment for 0x300-0x174*/
    __I  uint32_t PDN_COND0;            /* PDN_COND0 @A21D0300 */
    __IO uint32_t reserver6[3];         /* alignment for 0x310-0x304*/
    __IO uint32_t PDN_SETD0;            /* PDN_SETD0 @A21D0310 */
    __IO uint32_t reserver7[3];         /* alignment for 0x320-0x314*/
    __IO uint32_t PDN_CLRD0;            /* PDN_CLRD0 @A21D0320 */
} CKSYS_BUS_CLK_REGISTER_T;

#define _CKSYS_BUS_CLK_BASE_            (CKSYS_BUS_CLK_BASE)
#define _CKSYS_BUS_CLK_BASE_ADDR_       (_CKSYS_BUS_CLK_BASE_ + 0x0100)
#define CKSYS_BUS_CLK ((CKSYS_BUS_CLK_REGISTER_T *)(_CKSYS_BUS_CLK_BASE_ADDR_))

typedef struct {
    __IO uint32_t CKSYS_TST_SEL_0;           /* CKSYS_TST_SEL_0 @A2020220 */
    __IO uint32_t CKSYS_TST_SEL_1;           /* CKSYS_TST_SEL_1 @A2020224 */
    __IO uint32_t reserver1[6];              /* alignment for 0x240- 0x228*/
    __IO uint32_t CKSYS_CLK_CFG_0;           /* CKSYS_CLK_CFG_0 @A2020240 */
    __IO uint32_t CKSYS_CLK_CFG_1;           /* CKSYS_CLK_CFG_1 @A2020244 */
    __IO uint32_t reserver2[2];              /* alignment for 0x250-0x248*/
    __IO uint32_t CKSYS_CLK_UPDATE_0;        /* CKSYS_CLK_UPDATE_0 @A2020250 */
    __IO uint32_t CKSYS_CLK_UPDATE_1;        /* CKSYS_CLK_UPDATE_0 @A2020254 */
    __IO uint32_t reserver3[2];              /* alignment for 0x260-0x258*/
    __I  uint32_t CKSYS_CLK_UPDATE_STATUS_0; /* CKSYS_CLK_UPDATE_STATUS_0 @A2020260 */
    __I  uint32_t CKSYS_CLK_UPDATE_STATUS_1; /* CKSYS_CLK_UPDATE_STATUS_1 @A2020264 */
    __IO uint32_t reserver4[2];              /* alignment for 0x270-0x268*/
    __IO uint32_t CKSYS_CLK_FORCE_ON_0;      /* CKSYS_CLK_FORCE_ON_0 @A2020270 */
    __IO uint32_t CKSYS_CLK_FORCE_ON_1;      /* CKSYS_CLK_FORCE_ON_1 @A2020274 */
    __IO uint32_t reserver5[2];              /* alignment for 0x280-0x278*/
    __IO uint32_t CKSYS_CLK_DIV_0;           /* CKSYS_CLK_DIV_0 @A2020280 */
    __IO uint32_t CKSYS_CLK_DIV_1;           /* CKSYS_CLK_DIV_1 @A2020284 */
    __IO uint32_t CKSYS_CLK_DIV_2;           /* CKSYS_CLK_DIV_2 @A2020288 */
    __IO uint32_t CKSYS_CLK_DIV_3;           /* CKSYS_CLK_DIV_3 @A202028C */
    __IO uint32_t CKSYS_CLK_DIV_4;           /* CKSYS_CLK_DIV_4 @A2020290 */
    __IO uint32_t CKSYS_CLK_DIV_5;           /* CKSYS_CLK_DIV_5 @A2020294 */
    __IO uint32_t reserver6[2];              /* alignment for 0x2A0-0x298*/
    __IO uint32_t CKSYS_XTAL_FREQ;           /* CKSYS_XTAL_FREQ @A20202A0 */
    __IO uint32_t CKSYS_REF_CLK_SEL;         /* CKSYS_REF_CLK_SEL @A20202A4 */
    __IO uint32_t reserver7[86];             /* alignment for 0x400-0x2A8*/
    __IO uint32_t PLL_ABIST_FQMTR_CON0;      /* PLL_ABIST_FQMTR_CON0 @A2020400 */
    __I  uint32_t PLL_ABIST_FQMTR_CON1;      /* PLL_ABIST_FQMTR_CON1 @A2020404 */
    __IO uint32_t PLL_ABIST_FQMTR_CON2;      /* PLL_ABIST_FQMTR_CON2 @A2020408 */
    __I  uint32_t PLL_ABIST_FQMTR_DATA;      /* PLL_ABIST_FQMTR_DATA @A202040C */
} CKSYS_REGISTER_T;

#define _CKSYS_BASE_ADDR_       (CKSYS_BASE + 0x220)
#define CKSYS ((CKSYS_REGISTER_T *)(_CKSYS_BASE_ADDR_))

typedef struct {
    __I uint32_t XO_PDN_COND0;        /* XO_PDN_COND0 @A203B00 */
    __IO uint32_t reserver1[3];       /* alignment for 0xB10- 0xB04*/
    __IO uint32_t XO_PDN_SETD0;       /* XO_PDN_SETD0 @A203B10 */
    __IO uint32_t reserver2[3];       /* alignment for 0xB20- 0xB14*/
    __IO uint32_t XO_PDN_CLRD0;       /* XO_PDN_CLRD0 @A203B20 */
    __IO uint32_t reserver3[55];      /* alignment for 0xC00- 0xB24*/
    __IO uint32_t XO_DCM_CON_0;       /* XO_DCM_CON_0 @A203C00 */
    __IO uint32_t XO_DCM_CON_1;       /* XO_DCM_CON_1 @A203C04 */
} CKSYS_XO_CLK_REGISTER_T;

#define _CKSYS_XO_CLK_BASE_  (CKSYS_XO_CLK_BASE+0xB00)
#define CKSYS_XO_CLK ((CKSYS_XO_CLK_REGISTER_T *)(_CKSYS_XO_CLK_BASE_))

/* CLK_SOURCE_SEL used offset and mask definitions */
#define CLK_SYS_SEL_OFFSET     (0)
#define CLK_SYS_SEL_MASK       (0xFF<<CLK_SYS_SEL_OFFSET)
#define CLK_SFC_SEL_OFFSET     (8)
#define CLK_SFC_SEL_MASK       (0xFF<<CLK_SFC_SEL_OFFSET)
#define CLK_CONN_SEL_OFFSET    (16)
#define CLK_CONN_SEL_MASK      (0xFF<<CLK_CONN_SEL_OFFSET)
#define CLK_SPIMST_SEL_OFFSET  (24)
#define CLK_SPIMST_SEL_MASK    (0xFF<<CLK_SPIMST_SEL_OFFSET)
#define CLK_XTALCTL_SEL_OFFSET (0)
#define CLK_XTALCTL_SEL_MASK   (0xFF<<CLK_XTALCTL_SEL_OFFSET)
#define CLK_SDIOMST_SEL_OFFSET (8)
#define CLK_SDIOMST_SEL_MASK   (0xFF<<CLK_SDIOMST_SEL_OFFSET)


/* SPI master register definitions begin. */

/* register list defination */
typedef struct {
    union {
        struct {
            __IO uint8_t CTRL0;  /*!< SPI maste general configure set 0 */
            __IO uint8_t DEASSERT_EN; /*!< Deassert mode enable */
            __IO uint8_t PAUSE_EN;    /*!< Pause mode enable */
            __IO uint8_t RESERVED;  /*!< Device select */
        } CTRL0_CELLS;
        __IO uint32_t CTRL0; /*!< SPI Master Control 0 Register */
    } CTRL0_UNION;

    union {
        struct {
            __IO uint8_t CTRL1; /*!< SPI maste general configure set 1 */
            __IO uint8_t GET_DLY;    /*!< Receive data get delay */
            __IO uint8_t TXDMA_EN;   /*!< TX DMA enable */
            __IO uint8_t RXDMA_EN;   /*!< RX DMA enable */
        } CTRL1_CELLS;
        __IO uint32_t CTRL1; /*!< SPI Master Control 1 Register */
    } CTRL1_UNION;

    union {
        struct {
            __O uint8_t CMD_ACT;  /*!< Command activate */
            __O uint8_t RESUME;   /*!< Resume */
            __O uint8_t RST;      /*!< Reset */
            __I uint8_t RESERVED; /*!< Reserved bytes */
        } TRIG_CELLS;
        __O uint32_t TRIG; /*!< SPI Master Trigger Register */
    } TRIG_UNION;

    __IO uint32_t IE; /*!< SPI Master Interrupt Enable Register */

    __I uint32_t INT; /*!< SPI Master Interrupt Register */

    __I uint32_t STA; /*!< SPI Master Status Register */

    __O uint32_t TX_DATA; /*!< SPI Master TX Data Register */

    __I uint32_t RX_DATA; /*!< SPI Master RX Data Register */

    __IO uint32_t TX_SRC; /*!< SPI Master TX Source Address Register */

    __IO uint32_t RX_DST; /*!< SPI Master RX Destination Address Register */

    union {
        struct {
            __IO uint16_t CS_HOLD_CNT;  /*!< Chip select hold time */
            __IO uint16_t CS_SETUP_CNT; /*!< Chip select setup time */
        } CFG0_CELLS;
        __IO uint32_t CFG0; /*!< SPI Master Configuration 0 Register */
    } CFG0_UNION;

    union {
        struct {
            __IO uint16_t SCK_HIGH_CNT; /*!< SPI matser clock high time */
            __IO uint16_t SCK_LOW_CNT;  /*!< SPI matser clock low time */
        } CFG1_CELLS;
        __IO uint32_t CFG1; /*!< SPI Master Configuration 1 Register */
    } CFG1_UNION;

    union {
        struct {
            __IO uint8_t CS_IDLE_CNT; /*!< Chip select idle time */
            __IO uint8_t PACKET_LOOP_CNT; /*!< Transmission loop times */
            __IO uint16_t PACKET_LENGTH_CNT; /*!< Transmission package length */
        } CFG2_CELLS;
        __IO uint32_t CFG2; /*!< SPI Master Configuration 2 Register */
    } CFG2_UNION;

    union {
        struct {
            __IO uint8_t COMMAND_CNT; /*!< Command count */
            __IO uint8_t DUMMY_CNT; /*!< Dummy count */
            __I uint8_t RESERVED[2]; /*!< Reserved bytes */
        } CFG3_CELLS;
        __IO uint32_t CFG3; /*!< SPI Master Configuration 3 Register */
    } CFG3_UNION;

    union {
        struct {
            __IO uint8_t SEL_ADDR; /*!< SPI master selection address */
            __IO uint8_t SEL_WDATA; /*!< SPI master selection write data */
            __IO uint8_t SEL_RDATA; /*!< SPI master selection read data */
            __IO uint8_t RESERVED; /*!< Reserved bytes */
        } CFG4_CELLS;
        __IO uint32_t CFG4; /*!< SPI Master Delay Select 0 Register */
    } CFG4_UNION;
} SPIM_REGISTER_T;

#define  SPI_MASTER_0   ((SPIM_REGISTER_T *) SPI_MASTER_0_BASE)
#define  SPI_MASTER_1   ((SPIM_REGISTER_T *) SPI_MASTER_1_BASE)
#define  SPI_MASTER_2   ((SPIM_REGISTER_T *) SPI_MASTER_2_BASE)

/* SPIM_CTRL0 register definitions */
#define SPIM_CTRL0_CPHA_OFFSET                  (0)
#define SPIM_CTRL0_CPHA_MASK                    (0x1<<SPIM_CTRL0_CPHA_OFFSET)
#define SPIM_CTRL0_CPOL_OFFSET                  (1)
#define SPIM_CTRL0_CPOL_MASK                    (0x1<<SPIM_CTRL0_CPOL_OFFSET)
#define SPIM_CTRL0_TXMSBF_OFFSET                (2)
#define SPIM_CTRL0_TXMSBF_MASK                  (0x1<<SPIM_CTRL0_TXMSBF_OFFSET)
#define SPIM_CTRL0_RXMSBF_OFFSET                (3)
#define SPIM_CTRL0_RXMSBF_MASK                  (0x1<<SPIM_CTRL0_RXMSBF_OFFSET)
#define SPIM_CTRL0_MSBF_OFFSET                  (2)
#define SPIM_CTRL0_MSBF_MASK                    (0x3<<SPIM_CTRL0_MSBF_OFFSET)
#define SPIM_CTRL0_TYPE_OFFSET                  (4)
#define SPIM_CTRL0_TYPE_MASK                    (0x3<<SPIM_CTRL0_TYPE_OFFSET)
#define SPIM_CTRL0_RW_MODE_OFFSET               (6)
#define SPIM_CTRL0_RW_MODE_MASK                 (0x1<<SPIM_CTRL0_RW_MODE_OFFSET)
#define SPIM_CTRL0_DEASSERT_EN_OFFSET           (8)
#define SPIM_CTRL0_DEASSERT_EN_MASK             (0x1<<SPIM_CTRL0_DEASSERT_EN_OFFSET)
#define SPIM_CTRL0_PAUSE_EN_OFFSET              (16)
#define SPIM_CTRL0_PAUSE_EN_MASK                (0x1<<SPIM_CTRL0_PAUSE_EN_OFFSET)

/* SPIM_CTRL1 register definitions */
#define SPIM_CTRL1_SAMPLE_SEL_OFFSET            (0)
#define SPIM_CTRL1_SAMPLE_SEL_MASK              (0x1<<SPIM_CTRL1_SAMPLE_SEL_OFFSET)
#define SPIM_CTRL1_CS_POL_OFFSET                (1)
#define SPIM_CTRL1_CS_POL_MASK                  (0x1<<SPIM_CTRL1_CS_POL_OFFSET)
#define SPIM_CTRL1_TX_ENDIAN_OFFSET             (2)
#define SPIM_CTRL1_TX_ENDIAN_MASK               (0x1<<SPIM_CTRL1_TX_ENDIAN_OFFSET)
#define SPIM_CTRL1_RX_ENDIAN_OFFSET             (3)
#define SPIM_CTRL1_RX_ENDIAN_MASK               (0x1<<SPIM_CTRL1_RX_ENDIAN_OFFSET)
#define SPIM_CTRL1_ENDIAN_OFFSET                (2)
#define SPIM_CTRL1_ENDIAN_MASK                  (0x3<<SPIM_CTRL1_ENDIAN_OFFSET)
#define SPIM_CTRL1_GET_TICK_DLY_OFFSET          (8)
#define SPIM_CTRL1_GET_TICK_DLY_MASK            (0x7<<SPIM_CTRL1_GET_TICK_DLY_OFFSET)
#define SPIM_CTRL1_TX_DMA_EN_OFFSET             (16)
#define SPIM_CTRL1_TX_DMA_EN_MASK               (0x1<<SPIM_CTRL1_TX_DMA_EN_OFFSET)
#define SPIM_CTRL1_RX_DMA_EN_OFFSET             (24)
#define SPIM_CTRL1_RX_DMA_EN_MASK               (0x1<<SPIM_CTRL1_RX_DMA_EN_OFFSET)

/* SPIM_TRIG register definitions */
#define SPIM_TRIG_CMD_ACT_OFFSET                (0)
#define SPIM_TRIG_CMD_ACT_MASK                  (0x1<<SPIM_TRIG_CMD_ACT_OFFSET)
#define SPIM_TRIG_RESUME_OFFSET                 (8)
#define SPIM_TRIG_RESUME_MASK                   (0x1<<SPIM_TRIG_RESUME_OFFSET)
#define SPIM_TRIG_RST_OFFSET                    (16)
#define SPIM_TRIG_RST_MASK                      (0x1<<SPIM_TRIG_RST_OFFSET)

/* SPIM_IE register definitions */
#define SPIM_IE_OFFSET                          (0)
#define SPIM_IE_MASK                            (0x3<<SPIM_IE_OFFSET)
#define SPIM_IE_FINISH_IE_OFFSET                (0)
#define SPIM_IE_FINISH_IE_MASK                  (0x1<<SPIM_IE_FINISH_IE_OFFSET)
#define SPIM_IE_PAUSE_IE_OFFSET                 (1)
#define SPIM_IE_PAUSE_IE_MASK                   (0x1<<SPIM_IE_PAUSE_IE_OFFSET)

/* SPIM_INT register definitions */
#define SPIM_INT_FINISH_INT_OFFSET              (0)
#define SPIM_INT_FINISH_INT_MASK                (0x1<<SPIM_INT_FINISH_INT_OFFSET)
#define SPIM_INT_PAUSE_INT_OFFSET               (1)
#define SPIM_INT_PAUSE_INT_MASK                 (0x1<<SPIM_INT_PAUSE_INT_OFFSET)

/* SPIM_STATUS register definitions */
#define SPIM_STATUS_BUSY_OFFSET                 (0)
#define SPIM_STATUS_BUSY_MASK                   (0x1<<SPIM_STATUS_BUSY_OFFSET)

/* SPIM_CFG0 register definitions */
#define SPIM_CFG0_CS_HOLD_COUNT_OFFSET          (0)
#define SPIM_CFG0_CS_HOLD_COUNT_MASK            (0xffff<<SPIM_CFG0_CS_HOLD_COUNT_OFFSET)
#define SPIM_CFG0_CS_SETUP_COUNT_OFFSET         (16)
#define SPIM_CFG0_CS_SETUP_COUNT_MASK           (0xffff<<SPIM_CFG0_CS_SETUP_COUNT_OFFSET)

/* SPIM_CFG1 register definitions */
#define SPIM_CFG1_SCK_HIGH_COUNT_OFFSET         (0)
#define SPIM_CFG1_SCK_HIGH_COUNT_MASK           (0xffff<<SPIM_CFG1_SCK_HIGH_COUNT_OFFSET)
#define SPIM_CFG1_SCK_LOW_COUNT_OFFSET          (16)
#define SPIM_CFG1_SCK_LOW_COUNT_MASK            (0xffff<<SPIM_CFG1_SCK_LOW_COUNT_OFFSET)

/* SPIM_CFG2 register definitions */
#define SPIM_CFG2_CS_IDLE_COUNT_OFFSET          (0)
#define SPIM_CFG2_CS_IDLE_COUNT_MASK            (0xff<<SPIM_CFG2_CS_IDLE_COUNT_OFFSET)
#define SPIM_CFG2_PACKET_LOOP_COUNT_OFFSET      (8)
#define SPIM_CFG2_PACKET_LOOP_COUNT_MASK        (0xff<<SPIM_CFG2_PACKET_LOOP_COUNT_OFFSET)
#define SPIM_CFG2_PACKET_LENGTH_OFFSET          (16)
#define SPIM_CFG2_PACKET_LENGTH_MASK            (0xffff<<SPIM_CFG2_PACKET_LENGTH_OFFSET)

/* SPIM_CFG3 register definitions */
#define SPIM_CFG3_COMMAND_COUNT_OFFSET          (0)
#define SPIM_CFG3_COMMAND_COUNT_MASK            (0xff<<SPIM_CFG3_COMMAND_COUNT_OFFSET)
#define SPIM_CFG3_DUMMY_COUNT_OFFSET            (8)
#define SPIM_CFG3_DUMMY_COUNT_MASK              (0xff<<SPIM_CFG3_DUMMY_COUNT_OFFSET)

/* SPIMST_CFG4 register definitions */
#define SPIM_CFG4_SEL_ADDR_OFFSET               (0)
#define SPIM_CFG4_SEL_ADDR_MASK                 (0xf<<SPIM_CFG4_SEL_ADDR_OFFSET)
#define SPIM_CFG4_SEL_WDATA_OFFSET              (8)
#define SPIM_CFG4_SEL_WDATA_MASK                (0x7<<SPIM_CFG4_SEL_WDATA_OFFSET)
#define SPIM_CFG4_SEL_RDATA_OFFSET              (16)
#define SPIM_CFG4_SEL_RDATA_MASK                (0x7<<SPIM_CFG4_SEL_RDATA_OFFSET)

/* SPI master register definitions end. */


/* SPI slaver register definitions begin. */

/* register list defination */
typedef struct {
    union {
        struct {
            __IO uint8_t CTRL0; /*!<  */
            __IO uint8_t CTRL1; /*!<  */
            __IO uint8_t MISO_EARLY_TRANS; /*!<  */
            __IO uint8_t DIR_MODE; /*!<  */
        } CTRL_CELLS;
        __IO uint32_t CTRL; /*!< SPI Slave Control Register */
    } CTRL_UNION;

    union {
        struct {
            __O uint8_t SW_ON; /*!<  */
            __O uint8_t SW_RST; /*!<  */
            __O uint8_t TXDMA_SW_RDY; /*!<  */
            __O uint8_t RXDMA_SW_RDY; /*!<  */
        } TRIG_CELLS;
        __O uint32_t TRIG; /*!< SPI Slave Trigger Register */
    } TRIG_UNION;

    __IO uint32_t IE; /*!< SPI Slave Interrupt Enable Register */

    __IO uint32_t INT; /*!< SPI Slave Interrupt Register */

    union {
        struct {
            __I uint16_t STA; /*!<  */
            __I uint8_t AHB_STA; /*!<  */
            __I uint8_t FIFO_STA; /*!<  */
        } STA_CELLS;
        __I uint32_t STA; /*!< SPI Slave Status Register */
    } STA_UNION;

    __I uint32_t TRANS_LENGTH; /*!< SPI Slave Transfer Length Register */

    __I uint32_t TRANS_ADDR; /*!< SPI Slave Transfer Address Register */

    __IO uint32_t TMOUT_THR; /*!< SPI Slave Timeout Threshold Register */

    __IO uint32_t BUFFER_BASE_ADDR; /*!< SPI Slave Buffer Base Address Register */

    __IO uint32_t BUFFER_SIZE; /*!< SPI Slave Buffer Size Register */

    __I uint32_t CMD_RECEIVED; /*!< SPI Slave CMD Received Register */

    __I uint8_t RESERVED1[4];

    __O uint32_t TX_DATA;
    __I uint32_t RX_DATA;
    union {
        struct {
            __IO uint8_t TX_THRESHOLD;
            __IO uint8_t RX_THRESHOLD;
            __IO uint8_t RESERVED[2];
        } FIFO_CTRL_CELLS;
        __IO uint32_t FIFO_CTRL;
    } FIFO_CTRL_UNION;

    union {
        struct {
            __I uint8_t TX_CNT;
            __I uint8_t RESERVED1[1];
            __I uint8_t RX_CNT;
            __I uint8_t RESERVED2[1];
        } FIFO_STA_CELLS;
        __I uint32_t FIFO_STA;
    } FIFO_STA_UNION;

    union {
        struct {
            __IO uint8_t TX_START;
            __IO uint8_t TX_TRANS_TYPE;
            __IO uint8_t RX_START;
            __IO uint8_t RX_TRANS_TYPE;
        } DMA_CTRL_CELLS;
        __IO uint32_t DMA_CTRL;
    } DMA_CTRL_UNION;

    __I uint8_t RESERVED2[12];

    struct {
        __IO uint32_t START_ADDR;
        union {
            struct {
                __IO uint16_t TX_SIZE;
                __IO uint16_t TX_THRESHOLD;
            } SIZE_CELLS;
            __IO uint32_t SIZE;
        } SIZE_UNION;

        union {
            struct {
                __I uint16_t WPTR;
                __I uint16_t RPTR;
            } PTR_CELLS;
            __I uint32_t PTR;
        } PTR_UNION;

        __I uint32_t FIFO_CNT;
        __I uint32_t BYTE_AVALIABLE;
        __O uint32_t SW_MOVE_BYTE;
    } TXDMA;

    __I uint8_t RESERVED3[8];

    struct {
        __IO uint32_t START_ADDR;
        union {
            struct {
                __IO uint16_t RX_SIZE;
                __IO uint16_t RX_THRESHOLD;
            } SIZE_CELLS;
            __IO uint32_t SIZE;
        } SIZE_UNION;

        union {
            struct {
                __I uint16_t WPTR;
                __I uint16_t RPTR;
            } PTR_CELLS;
            __I uint32_t PTR;
        } PTR_UNION;

        __I uint32_t FIFO_CNT;
        __I uint32_t BYTE_AVALIABLE;
        __O uint32_t SW_MOVE_BYTE;

    } RXDMA;
} SPIS_REGISTER_T;

typedef struct {
    __IO uint32_t PAD_DUMMY_CNT; /*!< SPI Slave Pad Dummy Count Register */

    union {
        struct {
            __IO uint8_t CMD_PWOFF; /*!<  */
            __IO uint8_t CMD_PWON; /*!<  */
            __IO uint8_t CMD_RS; /*!<  */
            __IO uint8_t CMD_WS; /*!<  */
        } CMD_DEF0_CELLS;
        __IO uint32_t CMD_DEF0; /* SPI Slave Pad Command Define 0 Register */
    } CMD_DEF0_UNION;

    union {
        struct {
            __IO uint8_t CMD_CR; /*!<  */
            __IO uint8_t CMD_CW; /*!<  */
            __IO uint8_t CMD_RD; /*!<  */
            __IO uint8_t CMD_WR; /*!<  */
        } CMD_DEF1_CELLS;
        __IO uint32_t CMD_DEF1; /*!< SPI Slave Pad Command Define 1 Register */
    } CMD_DEF1_UNION;

    union {
        struct {
            __IO uint8_t CMD_CT; /*!<  */
            __I uint8_t RESERVED[3]; /*!<  */
        } CMD_DEF2_CELLS;
        __IO uint32_t CMD_DEF2; /*!< SPI Slave Pad Command Define 2 Register */
    } CMD_DEF2_UNION;

    union {
        struct {
            __IO uint8_t MOSI0_DLYSEL; /*!<  */
            __IO uint8_t MOSI1_DLYSEL; /*!<  */
            __IO uint8_t MOSI2_DLYSEL; /*!<  */
            __IO uint8_t MOSI3_DLYSEL; /*!<  */
        } DLYSEL0_CELLS;
        __IO uint32_t DLYSEL0; /*!< SPI slave Pad delay select 0 Register */
    } DLYSEL0_UNION;

    union {
        struct {
            __IO uint8_t MISO0_DLYSEL; /*!<  */
            __IO uint8_t MISO1_DLYSEL; /*!<  */
            __IO uint8_t MISO2_DLYSEL; /*!<  */
            __IO uint8_t MISO3_DLYSEL; /*!<  */
        } DLYSEL1_CELLS;
        __IO uint32_t DLYSEL1; /*!< SPI slave Pad delay select 1 Register */
    } DLYSEL1_UNION;

    union {
        struct {
            __IO uint8_t SCK_DLYSEL; /*!<  */
            __I uint8_t RESERVED[3]; /*!<  */
        } DLYSEL2_CELLS;
        __IO uint32_t DLYSEL2; /*!< SPI slave Pad delay select 2 Register */
    } DLYSEL2_UNION;
} SPIS_PAD_REGISTER_T;

typedef struct{
    __IO uint32_t BYPASS_CTRL;
}SPIS_BYPASS_REGISTER_T;

#define SPI_SLAVE_0 ((SPIS_REGISTER_T *)(SPI_SLAVE_BASE))
#define SPI_SLAVE_PAD_0 ((SPIS_PAD_REGISTER_T *)(SPI_SLAVE_PAD_BASE))
#define SPI_SLAVE_BYPASS_0 ((SPIS_BYPASS_REGISTER_T *)(SPI_SLAVE_BYPASS_BASE))

/* Bit definition for SPIS Control Register */
#define SPIS_CTRL_DIRECT_MODE_OFFSET    (24)
#define SPIS_CTRL_DIRECT_MODE_MASK      (0x1<<SPIS_CTRL_DIRECT_MODE_OFFSET)
#define SPIS_CTRL_MISO_EARLY_OFFSET     (16)
#define SPIS_CTRL_MISO_EARLY_MASK       (0x1<<SPIS_CTRL_MISO_EARLY_OFFSET)
#define SPIS_CTRL_DEC_ADDR_EN_OFFSET    (9)
#define SPIS_CTRL_DEC_ADDR_EN_MASK      (0x1<<SPIS_CTRL_DEC_ADDR_EN_OFFSET)
#define SPIS_CTRL_SW_RDY_EN_OFFSET      (8)
#define SPIS_CTRL_SW_RDY_EN_MASK        (0x1<<SPIS_CTRL_SW_RDY_EN_OFFSET)
#define SPIS_CTRL_SIZE_OF_ADDR_OFFSET   (6)
#define SPIS_CTRL_SIZE_OF_ADDR_MASK     (0x1<<SPIS_CTRL_SIZE_OF_ADDR_OFFSET)
#define SPIS_CTRL_TYPE_OFFSET           (4)
#define SPIS_CTRL_TYPE_MASK             (0x3<<SPIS_CTRL_TYPE_OFFSET)
#define SPIS_CTRL_RXMSBF_OFFSET         (3)
#define SPIS_CTRL_RXMSBF_MASK           (0x1<<SPIS_CTRL_RXMSBF_OFFSET)
#define SPIS_CTRL_TXMSBF_OFFSET         (2)
#define SPIS_CTRL_TXMSBF_MASK           (0x1<<SPIS_CTRL_TXMSBF_OFFSET)
#define SPIS_CTRL_CPOL_OFFSET           (1)
#define SPIS_CTRL_CPOL_MASK             (0x1<<SPIS_CTRL_CPOL_OFFSET)
#define SPIS_CTRL_CPHA_OFFSET           (0)
#define SPIS_CTRL_CPHA_MASK             (0x1<<SPIS_CTRL_CPHA_OFFSET)

/* Bit definition for SPIS Trigger Register */
#define SPIS_TRIG_RXDMA_SW_RDY_OFFSET   (24)
#define SPIS_TRIG_RXDMA_SW_RDY_MASK     (0x1<<SPIS_TRIG_RXDMA_SW_RDY_OFFSET)
#define SPIS_TRIG_TXDMA_SW_RDY_OFFSET   (16)
#define SPIS_TRIG_TXDMA_SW_RDY_MASK     (0x1<<SPIS_TRIG_TXDMA_SW_RDY_OFFSET)
#define SPIS_TRIG_SW_RST_OFFSET         (8)
#define SPIS_TRIG_SW_RST_MASK           (0x1<<SPIS_TRIG_SW_RST_OFFSET)
#define SPIS_TRIG_SW_ON_OFFSET          (0)
#define SPIS_TRIG_SW_ON_MASK            (0x1<<SPIS_TRIG_SW_ON_OFFSET)

/* Bit definition for SPIS Interrupt Enable Register */
#define SPIS_IE_OFFSET                  (0)
#define SPIS_IE_MASK                    (0x1ff<<SPIS_IE_OFFSET)
#define SPIS_IE_RD_CFG_FINISH_OFFSET    (0)
#define SPIS_IE_RD_CFG_FINISH_MASK      (0x1<<SPIS_IE_RD_CFG_FINISH_OFFSET)
#define SPIS_IE_WR_CFG_FINISH_OFFSET    (1)
#define SPIS_IE_WR_CFG_FINISH_MASK      (0x1<<SPIS_IE_WR_CFG_FINISH_OFFSET)
#define SPIS_IE_RD_TRANS_FINISH_OFFSET  (2)
#define SPIS_IE_RD_TRANS_FINISH_MASK    (0x1<<SPIS_IE_RD_TRANS_FINISH_OFFSET)
#define SPIS_IE_WR_TRANS_FINISH_OFFSET  (3)
#define SPIS_IE_WR_TRANS_FINISH_MASK    (0x1<<SPIS_IE_WR_TRANS_FINISH_OFFSET)
#define SPIS_IE_POWER_OFF_OFFSET        (4)
#define SPIS_IE_POWER_OFF_MASK          (0x1<<SPIS_IE_POWER_OFF_OFFSET)
#define SPIS_IE_POWER_ON_OFFSET         (5)
#define SPIS_IE_POWER_ON_MASK           (0x1<<SPIS_IE_POWER_ON_OFFSET)
#define SPIS_IE_RD_DATA_ERR_OFFSET      (6)
#define SPIS_IE_RD_DATA_ERR_MASK        (0x1<<SPIS_IE_RD_DATA_ERR_OFFSET)
#define SPIS_IE_WR_DATA_ERR_OFFSET      (7)
#define SPIS_IE_WR_DATA_ERR_MASK        (0x1<<SPIS_IE_WR_DATA_ERR_OFFSET)
#define SPIS_IE_TMOUT_ERR_OFFSET        (8)
#define SPIS_IE_TMOUT_ERR_MASK          (0x1<<SPIS_IE_TMOUT_ERR_OFFSET)
#define SPIS_IE_DIRECT_MODE_OFFSET      (8)
#define SPIS_IE_DIRECT_MODE_MASK        (0x3f<<SPIS_IE_DIRECT_MODE_OFFSET)
#define SPIS_IE_IDLE_TMOUT_OFFSET       (8)
#define SPIS_IE_IDLE_TMOUT_MASK         (0x1<<SPIS_IE_IDLE_TMOUT_OFFSET)
#define SPIS_IE_TX_FIFO_EMPTY_OFFSET    (9)
#define SPIS_IE_TX_FIFO_EMPTY_MASK      (0x1<<SPIS_IE_TX_FIFO_EMPTY_OFFSET)
#define SPIS_IE_RX_FIFO_FULL_OFFSET     (10)
#define SPIS_IE_RX_FIFO_FULL_MASK       (0x1<<SPIS_IE_RX_FIFO_FULL_OFFSET)
#define SPIS_IE_TX_DMA_EMPTY_OFFSET     (11)
#define SPIS_IE_TX_DMA_EMPTY_MASK       (0x1<<SPIS_IE_TX_DMA_EMPTY_OFFSET)
#define SPIS_IE_RX_DMA_FULL_OFFSET      (12)
#define SPIS_IE_RX_DMA_FULL_MASK        (0x1<<SPIS_IE_RX_DMA_FULL_OFFSET)
#define SPIS_IE_RX_OVERRUN_OFFSET       (13)
#define SPIS_IE_RX_OVERRUN_MASK         (0x1<<SPIS_IE_RX_OVERRUN_OFFSET)

/* Bit definition for SPIS Interrupt Register */
#define SPIS_INT_OFFSET                  (0)
#define SPIS_INT_MASK                    (0x1ff<<SPIS_INT_OFFSET)
#define SPIS_INT_RD_CFG_FINISH_OFFSET    (0)
#define SPIS_INT_RD_CFG_FINISH_MASK      (0x1<<SPIS_INT_RD_CFG_FINISH_OFFSET)
#define SPIS_INT_WR_CFG_FINISH_OFFSET    (1)
#define SPIS_INT_WR_CFG_FINISH_MASK      (0x1<<SPIS_INT_WR_CFG_FINISH_OFFSET)
#define SPIS_INT_RD_TRANS_FINISH_OFFSET  (2)
#define SPIS_INT_RD_TRANS_FINISH_MASK    (0x1<<SPIS_INT_RD_TRANS_FINISH_OFFSET)
#define SPIS_INT_WR_TRANS_FINISH_OFFSET  (3)
#define SPIS_INT_WR_TRANS_FINISH_MASK    (0x1<<SPIS_INT_WR_TRANS_FINISH_OFFSET)
#define SPIS_INT_POWER_OFF_OFFSET        (4)
#define SPIS_INT_POWER_OFF_MASK          (0x1<<SPIS_INT_POWER_OFF_OFFSET)
#define SPIS_INT_POWER_ON_OFFSET         (5)
#define SPIS_INT_POWER_ON_MASK           (0x1<<SPIS_INT_POWER_ON_OFFSET)
#define SPIS_INT_RD_DATA_ERR_OFFSET      (6)
#define SPIS_INT_RD_DATA_ERR_MASK        (0x1<<SPIS_INT_RD_DATA_ERR_OFFSET)
#define SPIS_INT_WR_DATA_ERR_OFFSET      (7)
#define SPIS_INT_WR_DATA_ERR_MASK        (0x1<<SPIS_INT_WR_DATA_ERR_OFFSET)
#define SPIS_INT_TMOUT_ERR_OFFSET        (8)
#define SPIS_INT_TMOUT_ERR_MASK          (0x1<<SPIS_INT_TMOUT_ERR_OFFSET)
#define SPIS_INT_DIRECT_MODE_OFFSET      (8)
#define SPIS_INT_DIRECT_MODE_MASK        (0x3f<<SPIS_INT_DIRECT_MODE_OFFSET)
#define SPIS_INT_IDLE_TMOUT_OFFSET       (8)
#define SPIS_INT_IDLE_TMOUT_MASK         (0x1<<SPIS_INT_IDLE_TMOUT_OFFSET)
#define SPIS_INT_TX_FIFO_EMPTY_OFFSET    (9)
#define SPIS_INT_TX_FIFO_EMPTY_MASK      (0x1<<SPIS_INT_TX_FIFO_EMPTY_OFFSET)
#define SPIS_INT_RX_FIFO_FULL_OFFSET     (10)
#define SPIS_INT_RX_FIFO_FULL_MASK       (0x1<<SPIS_INT_RX_FIFO_FULL_OFFSET)
#define SPIS_INT_TX_DMA_EMPTY_OFFSET     (11)
#define SPIS_INT_TX_DMA_EMPTY_MASK       (0x1<<SPIS_INT_TX_DMA_EMPTY_OFFSET)
#define SPIS_INT_RX_DMA_FULL_OFFSET      (12)
#define SPIS_INT_RX_DMA_FULL_MASK        (0x1<<SPIS_INT_RX_DMA_FULL_OFFSET)
#define SPIS_INT_RX_OVERRUN_OFFSET      (13)
#define SPIS_INT_RX_OVERRUN_MASK        (0x1<<SPIS_INT_RX_OVERRUN_OFFSET)

/* Bit definition for SPIS Interrupt Register */
#define SPIS_STA_OFFSET                  (0)
#define SPIS_STA_MASK                    (0x3fff<<SPIS_STA_OFFSET)
#define SPIS_AHB_STA_OFFSET              (16)
#define SPIS_AHB_STA_MASK                (0x7<<SPIS_AHB_STA_OFFSET)
#define SPIS_STA_SLV_ON_OFFSET           (0)
#define SPIS_STA_SLV_ON_MASK             (0x1<<SPIS_STA_SLV_ON_OFFSET)
#define SPIS_STA_CFG_SUCCESS_OFFSET      (1)
#define SPIS_STA_CFG_SUCCESS_MASK        (0x1<<SPIS_STA_CFG_SUCCESS_OFFSET)
#define SPIS_STA_TXRX_FIFO_RDY_OFFSET    (2)
#define SPIS_STA_TXRX_FIFO_RDY_MASK      (0x1<<SPIS_STA_TXRX_FIFO_RDY_OFFSET)
#define SPIS_STA_RD_ERR_OFFSET           (3)
#define SPIS_STA_RD_ERR_MASK             (0x1<<SPIS_STA_RD_ERR_OFFSET)
#define SPIS_STA_WR_ERR_OFFSET           (4)
#define SPIS_STA_WR_ERR_MASK             (0x1<<SPIS_STA_WR_ERR_OFFSET)
#define SPIS_STA_RDWR_FINISH_OFFSET      (5)
#define SPIS_STA_RDWR_FINISH_MASK        (0x1<<SPIS_STA_RDWR_FINISH_OFFSET)
#define SPIS_STA_TIMOUT_ERR_OFFSET       (6)
#define SPIS_STA_TIMOUT_ERR_MASK         (0x1<<SPIS_STA_TIMOUT_ERR_OFFSET)
#define SPIS_STA_CMD_ERR_OFFSET          (7)
#define SPIS_STA_CMD_ERR_MASK            (0x1<<SPIS_STA_CMD_ERR_OFFSET)
#define SPIS_STA_CFG_READ_FINISH_OFFSET  (8)
#define SPIS_STA_CFG_READ_FINISH_MASK    (0x1<<SPIS_STA_CFG_READ_FINISH_OFFSET)
#define SPIS_STA_CFG_WRITE_FINISH_OFFSET (9)
#define SPIS_STA_CFG_WRITE_FINISH_MASK   (0x1<<SPIS_STA_CFG_WRITE_FINISH_OFFSET)
#define SPIS_STA_RD_FINISH_OFFSET        (10)
#define SPIS_STA_RD_FINISH_MASK          (0x1<<SPIS_STA_RD_FINISH_OFFSET)
#define SPIS_STA_WR_FINISH_OFFSET        (11)
#define SPIS_STA_WR_FINISH_MASK          (0x1<<SPIS_STA_WR_FINISH_OFFSET)
#define SPIS_STA_POWER_OFF_OFFSET        (12)
#define SPIS_STA_POWER_OFF_MASK          (0x1<<SPIS_STA_POWER_OFF_OFFSET)
#define SPIS_STA_POWER_ON_OFFSET         (13)
#define SPIS_STA_POWER_ON_MASK           (0x1<<SPIS_STA_POWER_ON_OFFSET)
#define SPIS_AHB_DIR_OFFSET              (16)
#define SPIS_AHB_DIR_MASK                (0x1<<SPIS_AHB_DIR_OFFSET)
#define SPIS_AHB_STATUS_OFFSET           (17)
#define SPIS_AHB_STATUS_MASK             (0x3<<SPIS_AHB_STATUS_OFFSET)
#define SPIS_STA_TX_FIFO_EMPTY_OFFSET    (24)
#define SPIS_STA_TX_FIFO_EMPTY_MASK      (0x1<<SPIS_STA_TX_FIFO_EMPTY_OFFSET)
#define SPIS_STA_RX_FIFO_FULL_OFFSET     (25)
#define SPIS_STA_RX_FIFO_FULL_MASK       (0x1<<SPIS_STA_RX_FIFO_FULL_OFFSET)
#define SPIS_STA_TX_DMA_EMPTY_OFFSET     (26)
#define SPIS_STA_TX_DMA_EMPTY_MASK       (0x1<<SPIS_STA_TX_DMA_EMPTY_OFFSET)
#define SPIS_STA_RX_DMA_FULL_OFFSET      (27)
#define SPIS_STA_RX_DMA_FULL_MASK        (0x1<<SPIS_STA_RX_DMA_FULL_OFFSET)

/* Bit definition for SPIS CMD Received Register */
#define SPIS_CMD_RECEIVED_OFFSET        (0)
#define SPIS_CMD_RECEIVED_MASK          (0xff<<SPIS_CMD_RECEIVED_OFFSET)

/* Bit definition for SPIS Direct Mode TX Data Register */
#define SPIS_TX_DATA_OFFSET             (0)
#define SPIS_TX_DATA_MASK               (0xff<<SPIS_TX_DATA_OFFSET)

/* Bit definition for SPIS Direct Mode RX Data Register */
#define SPIS_RX_DATA_OFFSET             (0)
#define SPIS_RX_DATA_MASK               (0xff<<SPIS_RX_DATA_OFFSET)

/* Bit definition for SPIS Direct Mode FIFO Control Register */
#define SPIS_TX_FIFO_THRESHOLD_OFFSET   (0)
#define SPIS_TX_FIFO_THRESHOLD          (0x3f<<SPIS_TX_FIFO_THRESHOLD_OFFSET)
#define SPIS_RX_FIFO_THRESHOLD_OFFSET   (8)
#define SPIS_RX_FIFO_THRESHOLD          (0x3f<<SPIS_RX_FIFO_THRESHOLD_OFFSET)

/* Bit definition for SPIS Direct Mode FIFO Status Register */
#define SPIS_TX_FIFO_CNT_OFFSET         (0)
#define SPIS_TX_FIFO_CNT_MASK           (0x7f<<SPIS_TX_FIFO_CNT_OFFSET)
#define SPIS_RX_FIFO_CNT_OFFSET         (16)
#define SPIS_RX_FIFO_CNT_MASK           (0x7f<<SPIS_RX_FIFO_CNT_OFFSET)

/* Bit definition for SPIS Direct Mode DMA Control Register */
#define SPIS_TX_DMA_START_OFFSET         (0)
#define SPIS_TX_DMA_START_MASK           (1<<SPIS_TX_DMA_START_OFFSET)
#define SPIS_TX_DMA_TRANS_TYPE_OFFSET    (8)
#define SPIS_TX_DMA_TRANS_TYPE_MASK      (3<<SPIS_TX_DMA_TRANS_TYPE_OFFSET)
#define SPIS_RX_DMA_START_OFFSET         (16)
#define SPIS_RX_DMA_START_MASK           (1<<SPIS_RX_DMA_START_OFFSET)
#define SPIS_RX_DMA_TRANS_TYPE_OFFSET    (24)
#define SPIS_RX_DMA_TRANS_TYPE_MASK      (3<<SPIS_RX_DMA_TRANS_TYPE_OFFSET)

/* Bit definition for SPIS Direct Mode TX DMA Softwore Control Register */
#define SPIS_TX_DMA_SW_MOVE_BYTE_OFFSET  (0)
#define SPIS_TX_DMA_SW_MOVE_BYTE_MASK    (0xffff<<SPIS_TX_DMA_SW_MOVE_BYTE_OFFSET)

/* Bit definition for SPIS Direct Mode RX DMA Softwore Control Register */
#define SPIS_RX_DMA_SW_MOVE_BYTE_OFFSET  (0)
#define SPIS_RX_DMA_SW_MOVE_BYTE_MASK    (0xffff<<SPIS_RX_DMA_SW_MOVE_BYTE_OFFSET)

/* Bit definition for SPIS CMD Define 0 Register */
#define SPIS_CMD_DEF0_POWEROFF_OFFSET   (0)
#define SPIS_CMD_DEF0_POWEROFF_MASK     (0xff<<SPIS_CMD_DEF0_POWEROFF_OFFSET)
#define SPIS_CMD_DEF0_POWERON_OFFSET    (8)
#define SPIS_CMD_DEF0_POWERON_MASK      (0xff<<SPIS_CMD_DEF0_POWERON_OFFSET)
#define SPIS_CMD_DEF0_RS_OFFSET         (16)
#define SPIS_CMD_DEF0_RS_MASK           (0xff<<SPIS_CMD_DEF0_RS_OFFSET)
#define SPIS_CMD_DEF0_WS_OFFSET         (24)
#define SPIS_CMD_DEF0_WS_MASK           (0xff<<SPIS_CMD_DEF0_WS_OFFSET)

/* Bit definition for SPIS CMD Define 1 Register */
#define SPIS_CMD_DEF1_CR_OFFSET         (0)
#define SPIS_CMD_DEF1_CR_MASK           (0xff<<SPIS_CMD_DEF1_CR_OFFSET)
#define SPIS_CMD_DEF1_CW_OFFSET         (8)
#define SPIS_CMD_DEF1_CW_MASK           (0xff<<SPIS_CMD_DEF1_CW_OFFSET)
#define SPIS_CMD_DEF1_RD_OFFSET         (16)
#define SPIS_CMD_DEF1_RD_MASK           (0xff<<SPIS_CMD_DEF1_RD_OFFSET)
#define SPIS_CMD_DEF1_WR_OFFSET         (24)
#define SPIS_CMD_DEF1_WR_MASK           (0xff<<SPIS_CMD_DEF1_WR_OFFSET)

/* Bit definition for SPIS CMD Define 2 Register */
#define SPIS_CMD_DEF2_CMD_CT_OFFSET        (0)
#define SPIS_CMD_DEF2_CMD_CT_MASK          (0xff<<SPIS_CMD_DEF2_CMD_CT_OFFSET)

/* SPISLV_DLYSEL0 register definitions */
#define SPIS_DLYSEL0_MOSI0_OFFSET          (0)
#define SPIS_DLYSEL0_MOSI0_MASK            (0x7<<SPIS_DLYSEL0_MOSI0_OFFSET)
#define SPIS_DLYSEL0_MOSI1_OFFSET          (8)
#define SPIS_DLYSEL0_MOSI1_MASK            (0x7<<SPIS_DLYSEL0_MOSI1_OFFSET)
#define SPIS_DLYSEL0_MOSI2_OFFSET          (16)
#define SPIS_DLYSEL0_MOSI2_MASK            (0x7<<SPIS_DLYSEL0_MOSI2_OFFSET)
#define SPIS_DLYSEL0_MOSI3_OFFSET          (24)
#define SPIS_DLYSEL0_MOSI3_MASK            (0x7<<SPIS_DLYSEL0_MOSI3_OFFSET)

/* SPISLV_DLYSEL1 register definitions */
#define SPIS_DLYSEL1_MISO0_OFFSET          (0)
#define SPIS_DLYSEL1_MISO0_MASK            (0x7<<SPIS_DLYSEL1_MISO0_OFFSET)
#define SPIS_DLYSEL1_MISO1_OFFSET          (8)
#define SPIS_DLYSEL1_MISO1_MASK            (0x7<<SPIS_DLYSEL1_MISO1_OFFSET)
#define SPIS_DLYSEL1_MISO2_OFFSET          (16)
#define SPIS_DLYSEL1_MISO2_MASK            (0x7<<SPIS_DLYSEL1_MISO2_OFFSET)
#define SPIS_DLYSEL1_MISO3_OFFSET          (24)
#define SPIS_DLYSEL1_MISO3_MASK            (0x7<<SPIS_DLYSEL1_MISO3_OFFSET)

/* SPISLV_DLYSEL2 register definitions */
#define SPIS_DLYSEL2_SCK_OFFSET          (0)
#define SPIS_DLYSEL2_SCK_MASK            (0x7<<SPIS_DLYSEL2_SCK_OFFSET)


/* SPISLV bypass mode register definitions */
#define SPIS_BYPASS_ENABLE_OFFSET    (0)
#define SPIS_BYPASS_ENABLE_MASK      (1 << SPIS_BYPASS_ENABLE_OFFSET)
#define SPIS_BYPASS_MASTER_SELECT_OFFSET    (8)
#define SPIS_BYPASS_MASTER_SELECT_MASK      (3 << SPIS_BYPASS_MASTER_SELECT_OFFSET)

#define SPIS_BYPASS_CS0_ENABLE_OFFSET       (16)
#define SPIS_BYPASS_CS0_ENABLE_MASK         (1 << SPIS_BYPASS_CS0_ENABLE_OFFSET)
#define SPIS_BYPASS_CS1_ENABLE_OFFSET       (17)
#define SPIS_BYPASS_CS1_ENABLE_MASK         (1 << SPIS_BYPASS_CS1_ENABLE_OFFSET)
#define SPIS_BYPASS_CS2_ENABLE_OFFSET       (18)
#define SPIS_BYPASS_CS2_ENABLE_MASK         (1 << SPIS_BYPASS_CS2_ENABLE_OFFSET)
#define SPIS_BYPASS_CS3_ENABLE_OFFSET       (19)
#define SPIS_BYPASS_CS3_ENABLE_MASK         (1 << SPIS_BYPASS_CS3_ENABLE_OFFSET)
#define SPIS_BYPASS_CS_ENABLE_MASK          (SPIS_BYPASS_CS0_ENABLE_MASK | SPIS_BYPASS_CS1_ENABLE_MASK | SPIS_BYPASS_CS2_ENABLE_MASK | SPIS_BYPASS_CS3_ENABLE_MASK)
#define SPIS_BYPASS_MOSI_ENABLE_OFFSET      (24)
#define SPIS_BYPASS_MOSI_ENABLE_MASK        (1 << SPIS_BYPASS_MOSI_ENABLE_OFFSET)
#define SPIS_BYPASS_MISO_ENABLE_OFFSET      (25)
#define SPIS_BYPASS_MISO_ENABLE_MASK        (1 << SPIS_BYPASS_MISO_ENABLE_OFFSET)
#define SPIS_BYPASS_SIO2_ENABLE_OFFSET      (26)
#define SPIS_BYPASS_SIO2_ENABLE_MASK        (1 << SPIS_BYPASS_SIO2_ENABLE_OFFSET)
#define SPIS_BYPASS_SIO3_ENABLE_OFFSET      (27)
#define SPIS_BYPASS_SIO3_ENABLE_MASK        (1 << SPIS_BYPASS_SIO3_ENABLE_OFFSET)
#define SPIS_BYPASS_SIO_ENABLE_MASK         (SPIS_BYPASS_MOSI_ENABLE_MASK | SPIS_BYPASS_MISO_ENABLE_MASK | SPIS_BYPASS_SIO2_ENABLE_MASK | SPIS_BYPASS_SIO3_ENABLE_MASK)

/* SPI slave register definitions end. */




/*************************** EINT definition start line  *******************************/
#define EINT_NUMBER_MAX  65                                                  /*!< The eint max number */

typedef union {
    struct {
        uint8_t  DBC_CON[2];            /*!< bit 0-14: DBC_CON - debounce duration in terms of the number of 32768Hz clock cycles,
                                                                  cycle length is determinded by PRESCALER*/
        uint8_t  DBC_EN;                /*!< bit 16: DBC_EN - enable debounce */
        uint8_t  RSTDBC;                /*!< bit 24: RSTDBC - reset the de-bounce counter */
    } EINT_CON_CELLS;
    uint32_t EINT_CON;
} EINT_CON_UNION;

typedef struct {
    __I uint32_t RO[3];                                            /*!< EINT RW register */
    uint32_t  RESERVED0[1];
    __O uint32_t SET[3];                                           /*!< EINT set register */
    uint32_t  RESERVED1[1];
    __O uint32_t CLR[3];                                           /*!< EINT clear register */
    uint32_t  RESERVED2[1];
} EINT_STD_REGISTER_T;


typedef struct {
    __I uint32_t EINT_STA[3];                                      /*!< 0x300 EINT interrupt status register   */
    __I uint32_t RESERVED0[1];
    __O uint32_t EINT_INTACK[3];                                   /*!< 0x310 EINT interrupt acknowledge register */
    __I uint32_t RESERVED1[1];
    __O uint32_t EINT_EEVT[3];                                     /*!< 0x320 EINT wake-up event register */
    __I uint32_t RESERVED2[1];
    __O uint32_t EINT_EEVTACK[3];                                  /*!< 0x330 EINT wake-up event acknowledge register */
    __I uint32_t RESERVED3[1];
    EINT_STD_REGISTER_T EINT_MASK;                                 /*!< 0x340 EINT interrupt mask register */
    EINT_STD_REGISTER_T EINT_WAKEUP_MASK;                          /*!< 0x370 EINT wakeup event mask register */
    EINT_STD_REGISTER_T EINT_SENS;                                 /*!< 0x3A0 EINT interrupt sensitive register  */
    EINT_STD_REGISTER_T EINT_DUALEDGE_SENS;                        /*!< 0x3D0 EINT interrupt dual edge sensitive register */
    EINT_STD_REGISTER_T EINT_POL;                                  /*!< 0x400 EINT interrupt dual edge sensitive register */
    EINT_STD_REGISTER_T EINT_SOFT;                                 /*!< 0x430 EINT software interrupt register */
    EINT_STD_REGISTER_T EINT_D0EN;                                 /*!< 0x460 EINT domain 0 register for cm33 */
    EINT_STD_REGISTER_T EINT_D1EN;                                 /*!< 0x490 EINT domain 1 register for dsp */
    __O uint32_t RESERVED4[16];
    __IO EINT_CON_UNION EINT_CON_REGISTER[EINT_NUMBER_MAX];        /*!< 0x500~0x600 EINT config register */
    __I uint32_t RESERVED5[63];
    __I uint32_t EINT_TRIGGER_STA[17];      /*!< 0x700~0x740 EINT trigger status register */
    __I uint32_t RESERVED6[48];
} EINT_REGISTER_T;

#define EINT_CON_DBC_CNT_OFFSET    (0)
#define EINT_CON_DBC_CNT_MASK      (0x7FFUL << EINT_CON_DBC_CNT_OFFSET)

#define EINT_CON_DBC_EN_OFFSET     (16)
#define EINT_CON_DBC_EN_MASK       (0x1UL << EINT_CON_DBC_EN_OFFSET)

#define EINT_CON_RSTD_OFFSET       (24)
#define EINT_CON_RSTD_MASK         (0x1UL << EINT_CON_RSTD_OFFSET)

#define EINT_CON_PRESCALER_OFFSET  (12)
#define EINT_CON_PRESCALER_MASK    (0x7UL << EINT_CON_PRESCALER_OFFSET)


/*************************** EINT definition end line  ********************************/


/*************************** EINT definition end line  ********************************/

/************************ RTC register definition start line  *******************************
 */
typedef struct {
    __IO uint32_t RTC_BBPU;     /* Address offset: 0x00 */
    __IO uint32_t RTC_IRQ_STA;  /* Address offset: 0x04 */
    __IO uint32_t RTC_IRQ_EN;   /* Address offset: 0x08*/
    __IO uint32_t RTC_CII_EN;   /* Address offset: 0x0c */
    __IO uint32_t RTC_AL_MASK;  /* Address offset: 0x10 */
    __IO uint32_t RTC_TC0;      /* Address offset: 0x14 */
    __IO uint32_t RTC_TC1;      /* Address offset: 0x18 */
    __IO uint32_t RTC_TC2;      /* Address offset: 0x1c */
    __IO uint32_t RTC_TC3;      /* Address offset: 0x20 */
    __IO uint32_t RTC_AL0;      /* Address offset: 0x24 */
    __IO uint32_t RTC_AL1;      /* Address offset: 0x28 */
    __IO uint32_t RTC_AL2;      /* Address offset: 0x2C */
    __IO uint32_t RTC_AL3;      /* Address offset: 0x30 */
    __IO uint32_t RTC_LPD_CON;  /* Address offset: 0x34 */
    __IO uint32_t RTC_INT_CNT;  /* Address offset: 0x38 */
    __IO uint32_t RTC_EINT_CON0;/* Address offset: 0x3C*/
    __IO uint32_t RTC_OSC32CON0;/* Address offset: 0x40 */
    __IO uint32_t RTC_OSC32CON1;/* Address offset: 0x44 */
    __IO uint32_t RTC_OSC32CON2;/* Address offset: 0x48 */
    __IO uint32_t RTC_OSC32CON3;/* Address offset: 0x4C */
    __IO uint32_t RTC_RESERVED_1[4];/* Address offset: 0x50~0x5C*/
    __IO uint32_t RTC_EINT_CON1;/* Address offset: 0x60*/
    __IO uint32_t RTC_RESERVED_2;/* Address offset: 0x64*/
    __IO uint32_t RTC_PROT;     /* Address offset: 0x68 */
    __IO uint32_t RTC_DIFF;     /* Address offset: 0x6C */
    __IO uint32_t RTC_CALI;     /* Address offset: 0x70 */
    __IO uint32_t RTC_WRTGR;    /* Address offset: 0x74 */
    __IO uint32_t RTC_RESERVED_3[3];/* Address offset: 0x78~0x80*/
    __IO uint32_t RTC_CAP_CON;  /* Address offset: 0x84*/
    __IO uint32_t RTC_RESET_CON;/* Address offset: 0x88*/
    __IO uint32_t RTC_RESERVED_4[2];  /* Address offset: 0x8C~0x90*/
    __IO uint32_t RTC_SPAR_REG;/* Address offset: 0x94*/
    __IO uint32_t RTC_SPAR0;/* Address offset: 0x98*/
    __IO uint32_t RTC_SPAR1;/* Address offset: 0x9C*/
    __IO uint32_t RTC_SPAR2;/* Address offset: 0xA0*/
    __IO uint32_t RTC_SPAR3;    /* Address offset: 0xA4*/
    __IO uint32_t RTC_SPAR4;    /* Address offset: 0xA8*/
    __IO uint32_t RTC_SPAR5;    /* Address offset: 0xAC*/
    __IO uint32_t RTC_RESERVED_5[20];    /* Address offset: 0xB0*/
    __IO uint32_t RTC_DEBUG_CON_0; /* Address offset: 0x100*/
    __IO uint32_t RTC_DEBUG_CON_1; /* Address offset: 0x104*/
    __IO uint32_t AFUNC_CON;      /* Address offset: 0x108*/
    __IO uint32_t RTC_RESERVED_6[4];    /* Address offset: 0x10C*/
    __IO uint32_t AFUNC_RTC_ALARM;/* Address offset: 0x11C*/
}RTC_REGISTER_T;

#define     RTC_PROTECT1                        0x586a
#define     RTC_PROTECT2                        0x9136

#define     RTC_OSC32CON0_MAGIC_KEY             0x1a57
#define     RTC_OSC32CON1_MAGIC_KEY             0x1a85
#define     RTC_OSC32CON2_MAGIC_KEY             0x1653
#define     RTC_OSC32CON3_MAGIC_KEY             0x5D4A

/*RTC_BBPU Register*/
#define     RTC_BBPU_ALARM_PWREN_OFFSET         (0)
#define     RTC_BBPU_RTC_WAKEUP_OFFSET          (1)
#define     RTC_BBPU_TICK_PWREN_OFFSET          (2)
#define     RTC_BBPU_EINT0_PWREN_OFFSET         (3)
//#define     RTC_BBPU_GALARM_PWREN_OFFSET      (4)
#define     RTC_BBPU_EINT1_PWREN_OFFSET         (4)
#define     RTC_BBPU_EINT2_PWREN_OFFSET         (5)
#define     RTC_BBPU_CAP_PWREN_OFFSET           (6)

#define     RTC_BBPU_KEY_BBPU_OFFSET            (8)

#define     RTC_BBPU_ALARM_PWREN_MASK           (1<<RTC_BBPU_ALARM_PWREN_OFFSET)
#define     RTC_BBPU_RTC_WAKEUP_MASK            (1<<RTC_BBPU_RTC_WAKEUP_OFFSET)
#define     RTC_BBPU_TICK_PWREN_MASK            (1<<RTC_BBPU_TICK_PWREN_OFFSET)
#define     RTC_BBPU_EINT0_PWREN_MASK           (1<<RTC_BBPU_EINT0_PWREN_OFFSET)
#define     RTC_BBPU_EINT1_PWREN_MASK           (1<<RTC_BBPU_EINT1_PWREN_OFFSET)
//#define     RTC_BBPU_GALARM_PWREN_MASK        (1<<RTC_BBPU_GALARM_PWREN_OFFSET)
#define     RTC_BBPU_EINT2_PWREN_MASK           (1<<RTC_BBPU_EINT2_PWREN_OFFSET)
#define     RTC_BBPU_CAP_PWREN_MASK             (1<<RTC_BBPU_CAP_PWREN_OFFSET)
#define     RTC_BBPU_KEY_BBPU_MASK              (0xFF<<RTC_BBPU_KEY_BBPU_OFFSET)

/*RTC_CII_EN Register*/
#define     RTC_CII_EN_CII_EN_OFFSET            0
#define     RTC_CII_EN_TC_EN_OFFSET             8
#define     RTC_CII_EN_CII_EN_MASK              (0xF<<)RTC_CII_EN_CII_EN_OFFSET
#define     RTC_CII_EN_TC_EN_MASK               (0x1<<RTC_CII_EN_TC_EN_OFFSET)

/*RTC_AL_MASK Register*/
#define     RTC_AL_MASK_AL_EN_OFFSET            8
#define     RTC_AL_MASK_AL_MASK_OFFSET          0
#define     RTC_AL_MASK_SECOND_OFFSET           0
#define     RTC_AL_MASK_MINUTE_OFFSET           1
#define     RTC_AL_MASK_HOUR_OFFSET             2
#define     RTC_AL_MASK_DOM_OFFSET              3
#define     RTC_AL_MASK_DOW_OFFSET              4
#define     RTC_AL_MASK_MONTH_OFFSET            5
#define     RTC_AL_MASK_YEAR_OFFSET             6
#define     RTC_AL_MASK_AL_EN_MASK              (0x1<<RTC_AL_MASK_AL_EN_OFFSET)
#define     RTC_AL_MASK_AL_MASK_MASK            (0x7F<<RTC_AL_MASK_AL_MASK_OFFSET)

/*RTC IRQ STATUS*/
#define     RTC_IRQ_STA_ALARM_OFFSET            (0)
#define     RTC_IRQ_STA_TICK_OFFSET             (1)
#define     RTC_IRQ_STA_LP_OFFSET               (2)
#define     RTC_IRQ_STA_EINT0_OFFSET            (3)
#define     RTC_IRQ_STA_EINT1_OFFSET            (4)
#define     RTC_IRQ_STA_EINT2_OFFSET            (5)
#define     RTC_IRQ_STA_EINT3_OFFSET            (6)

//#define     RTC_IRQ_STA_GALARM_OFFSET           (4)

#define     RTC_IRQ_STA_ALARM_MASK              (1<<RTC_IRQ_STA_ALARM_OFFSET)
#define     RTC_IRQ_STA_TICK_MASK               (1<<RTC_IRQ_STA_TICK_OFFSET)
#define     RTC_IRQ_STA_LP_MASK                 (1<<RTC_IRQ_STA_LP_OFFSET)
#define     RTC_IRQ_STA_EINT0_MASK              (1<<RTC_IRQ_STA_EINT0_OFFSET)
#define     RTC_IRQ_STA_EINT1_MASK              (1<<RTC_IRQ_STA_EINT1_OFFSET)
#define     RTC_IRQ_STA_EINT2_MASK              (1<<RTC_IRQ_STA_EINT2_OFFSET)
#define     RTC_IRQ_STA_EINT3_MASK              (1<<RTC_IRQ_STA_EINT3_OFFSET)

//#define     RTC_IRQ_STA_GALARM_MASK            (1<<RTC_IRQ_STA_GALARM_OFFSET)

/*RTC_TC0 Register*/
#define     RTC_TC0_SECOND_OFFSET               0
#define     RTC_TC0_MINUTE_OFFSET               8
#define     RTC_TC0_SECOND_MASK                 (0x3F<<RTC_TC0_SECOND_OFFSET)
#define     RTC_TC0_MINUTE_MASK                 (0x3F<<RTC_TC0_MINUTE_OFFSET)

/*RTC_TC1 Register*/
#define     RTC_TC1_HOUR_OFFSET                 0
#define     RTC_TC1_DOM_OFFSET                  8
#define     RTC_TC1_HOUR_MASK                   (0x1F<<RTC_TC1_HOUR_OFFSET)
#define     RTC_TC1_DOM_MASK                    (0x1F<<RTC_TC1_DOM_OFFSET)

/*RTC_TC2 Register*/
#define     RTC_TC2_DOW_OFFSET                  0
#define     RTC_TC2_MONTH_OFFSET                8
#define     RTC_TC2_DOW_MASK                    (0x7<<RTC_TC2_DOW_OFFSET)
#define     RTC_TC2_MONTH_MASK                  (0xf<<RTC_TC2_MONTH_OFFSET)

/*RTC_TC3 Register*/
#define     RTC_TC3_YEAR_OFFSET                 0
#define     RTC_TC3_YEAR_MASK                   0x7f

/*RTC_AL0 Register*/
#define     RTC_AL0_AL_SECOND_OFFSET            0
#define     RTC_AL0_AL_MINUTE_OFFSET            8
#define     RTC_AL0_AL_SECOND_MASK              (0x3F<<RTC_AL0_AL_SECOND_OFFSET)
#define     RTC_AL0_AL_MINUTE_MASK              (0x3F<<RTC_AL0_AL_MINUTE_OFFSET)

/*RTC_AL1 Register*/
#define     RTC_AL1_AL_HOUR_OFFSET              0
#define     RTC_AL1_AL_DOM_OFFSET               8
#define     RTC_AL1_AL_HOUR_MASK                (0x1F<<RTC_AL1_AL_HOUR_OFFSET)
#define     RTC_AL1_AL_DOM_MASK                 (0x1F<<RTC_AL1_AL_DOM_OFFSET)

/*RTC_AL2 Register*/
#define     RTC_AL2_AL_DOW_OFFSET               0
#define     RTC_AL2_AL_MONTH_OFFSET             8
#define     RTC_AL2_AL_DOW_MASK                 (0x7<<RTC_AL2_AL_DOW_OFFSET)
#define     RTC_AL2_AL_MONTH_MASK               (0xf<<RTC_AL2_AL_MONTH_OFFSET)

/*RTC_AL3 Register*/
#define     RTC_AL3_AL_YEAR_OFFSET              0
#define     RTC_AL3_AL_YEAR_MASK                (0x7f)

/*RTC_LPD_CON*/
#define     RTC_LPD_CON_LPSTA_RAW_OFFSET        (0)
#define     RTC_LPD_CON_EOSC32_LPDEN_OFFSET     (1)
#define     RTC_LPD_CON_LPRST_OFFSET            (3)
#define     RTC_LPD_CON_LPD_OPT_OFFSET          (8)

#define     RTC_LPD_CON_LPSTA_RAW_MASK          (1<<RTC_LPD_CON_LPSTA_RAW_OFFSET)
#define     RTC_LPD_CON_EOSC32_LPDEN_MASK       (1<<RTC_LPD_CON_EOSC32_LPDEN_OFFSET)
#define     RTC_LPD_CON_LPRST_MASK              (1<<RTC_LPD_CON_LPRST_OFFSET)
#define     RTC_LPD_CON_LPD_OPT_MASK            (1<<RTC_LPD_CON_LPD_OPT_OFFSET)

/*RTC_EINT_CONx*/
#define     RTC_EINT_CON_0_START_OFFSET        (0)
#define     RTC_EINT_CON_0_IRQ_EN_OFFSET       (0)
#define     RTC_EINT_CON_0_DEB_EN_OFFSET       (1)
#define     RTC_EINT_CON_0_SYN_EN_OFFSET       (2)
#define     RTC_EINT_CON_0_INV_EN_OFFSET       (3)
#define     RTC_EINT_CON_0_EINT_EN_OFFSET      (4)
#define		RTC_EINT_CON_0_EINT_2EDGE_OFFSET   (5)
#define     RTC_EINT_CON_0_EINT_CLR_OFFSET     (7)

#define     RTC_EINT_CON_1_START_OFFSET        (8)
#define     RTC_EINT_CON_1_IRQ_EN_OFFSET       (8)
#define     RTC_EINT_CON_1_DEB_EN_OFFSET       (9)
#define     RTC_EINT_CON_1_SYN_EN_OFFSET       (10)
#define     RTC_EINT_CON_1_INV_EN_OFFSET       (11)
#define     RTC_EINT_CON_1_EINT_EN_OFFSET      (12)
#define		RTC_EINT_CON_1_EINT_2EDGE_OFFSET   (13)
#define     RTC_EINT_CON_1_EINT_CLR_OFFSET     (15)


#define     RTC_EINT_ALL_EN_MASK                (0x3F)
#define     RTC_EINT_IRQ_EN_OFFSET              (0)
#define     RTC_EINT_DEB_EN_OFFSET              (1)
#define     RTC_EINT_SYN_EN_OFFSET              (2)
#define     RTC_EINT_INV_EN_OFFSET              (3)
#define     RTC_EINT_EINT_EN_OFFSET             (4)
#define     RTC_EINT_DUAL_EDGE_OFFSET           (5)
#define     RTC_EINT_EINT_CLR_OFFSET            (7)

#define     RTC_EINT_IRQ_EN_MASK                (1<<RTC_EINT_IRQ_EN_OFFSET)
#define     RTC_EINT_DEB_EN_MASK                (1<<RTC_EINT_DEB_EN_OFFSET)
#define     RTC_EINT_SYN_EN_MASK                (1<<RTC_EINT_SYN_EN_OFFSET)
#define     RTC_EINT_INV_EN_MASK                (1<<RTC_EINT_INV_EN_OFFSET)
#define     RTC_EINT_EINT_EN_MASK               (1<<RTC_EINT_EINT_EN_OFFSET)
#define     RTC_EINT_DUAL_EDGE_MASK             (1<<RTC_EINT_DUAL_EDGE_OFFSET)
#define     RTC_EINT_INV_EN_MASK                (1<<RTC_EINT_INV_EN_OFFSET)

#define     RTC_EINT_EINT_CLR_MASK              (1<<RTC_EINT_EINT_CLR_OFFSET)
/*RTC_OSC32CON0 Register*/
#define     RTC_OSC32CON0_32K_SEL_OFFSET        (0)
#define     RTC_OSC32CON0_TIME_CG_EN_OFFSET     (8)

#define     RTC_OSC32CON0_32K_SEL_MASK          (0x3<<RTC_OSC32CON0_32K_SEL_OFFSET)
#define     RTC_OSC32CON0_TIME_CG_EN_MASK       (0x1<<RTC_OSC32CON0_TIME_CG_EN_OFFSET)

/*RTC_OSC32CON1 Register*/
#define     RTC_OSC32CON1_XOSC_CALI_OFFSET      (0)
#define     RTC_OSC32CON1_EOSC_BIAS_SEL_OFFSET  (5)
#define     RTC_OSC32CON1_EOSC_STP_PWD_OFFSET   (7)
#define     RTC_OSC32CON1_EOSC_CHOP_EN_OFFSET   (8)
#define     RTC_OSC32CON1_EOSC_VCT_EN_OFFSET    (9)
#define     RTC_OSC32CON1_XOSC_CALI_MASK        (0x1f<<RTC_OSC32CON1_XOSC_CALI_OFFSET)
#define     RTC_OSC32CON1_EOSC_BIAS_SEL_MASK    (0x3<<RTC_OSC32CON1_EOSC_BIAS_SEL_OFFSET)
#define     RTC_OSC32CON1_EOSC_STP_PWD_MASK     (1<<RTC_OSC32CON1_EOSC_STP_PWD_OFFSET)
#define     RTC_OSC32CON1_EOSC_CHOP_EN_MASK     (1<<RTC_OSC32CON1_EOSC_CHOP_EN_OFFSET)
#define     RTC_OSC32CON1_EOSC_VCT_EN_MASK      (1<<RTC_OSC32CON1_EOSC_VCT_EN_OFFSET)

/*RTC_OSC32CON2 Register*/
#define     RTC_OSC32CON2_XOSC_PWDB_OFFSET      (0)
#define     RTC_OSC32CON2_EOSC_PWDB_OFFSET      (1)
#define     RTC_OSC32CON2_SETTING_CG_OFFSET     (2)
#define     RTC_OSC32CON2_XOSC_PWDB_MASK        (1<<RTC_OSC32CON2_XOSC_PWDB_OFFSET)
#define     RTC_OSC32CON2_EOSC_PWDB_MASK        (1<<RTC_OSC32CON2_EOSC_PWDB_OFFSET)
#define     RTC_OSC32CON2_SETTING_CG_MASK       (1<<RTC_OSC32CON2_SETTING_CG_OFFSET)

/*RTC_LDO0 Register*/
#define     RTC_LDOCON0_LP_BG_OFFSET            (0)
#define     RTC_LDOCON0_LP_VERF_OFFSET          (4)
#define     RTC_LDOCON0_SEL_LDO08_OFFSET        (5)
#define     RTC_LDOCON0_SEL_LDO09_OFFSET        (8)
#define     RTC_LDOCON0_ATST_VREF1V_OFFSET      (11)
#define     RTC_LDOCON0_PWR_EN_CPT_OFFSET       (12)
#define     RTC_LDOCON0_LDO09_EN_OFFSET         (13)
#define     RTC_LDOCON0_RSEL_BG_OFFSET          (14)

#define     RTC_LDOCON0_LP_BG_MASK              (0xf<<RTC_LDOCON0_LP_BG_OFFSET)
#define     RTC_LDOCON0_LP_VERF_MASK            (0x1<<RTC_LDOCON0_LP_VERF_OFFSET)
#define     RTC_LDOCON0_SEL_LDO08_MASK          (0x7<<RTC_LDOCON0_SEL_LDO08_OFFSET)
#define     RTC_LDOCON0_SEL_LDO09_MASK          (0x7<<RTC_LDOCON0_SEL_LDO09_OFFSET)
#define     RTC_LDOCON0_ATST_VREF1V_MASK        (0x1<<RTC_LDOCON0_ATST_VREF1V_OFFSET)
#define     RTC_LDOCON0_PWR_EN_CPT_MASK         (0x1<<RTC_LDOCON0_PWR_EN_CPT_OFFSET)
#define     RTC_LDOCON0_LDO09_EN_MASK           (0x1<<RTC_LDOCON0_LDO09_EN_OFFSET)
#define     RTC_LDOCON0_RSEL_BG_MASK            (0x3<<RTC_LDOCON0_RSEL_BG_OFFSET)


/*RTC_LDO1 Register*/
#define     RTC_LDOCON1_ISEL_ADC_RTC_OFFSET     (0)
#define     RTC_LDOCON1_ISEL_OP_RTC_OFFSET      (2)
#define     RTC_LDOCON1_ISEL_LDO045_RTC_OFFSET  (4)
#define     RTC_LDOCON1_ISEL_LDO09_RTC_OFFSET   (6)
#define     RTC_LDOCON1_ISEL_VREF_RTC_OFFSET    (8)
#define     RTC_LDOCON1_ISEL_PORL_RTC_OFFSET    (10)
#define     RTC_LDOCON1_ISEL_PORH_RTC_OFFSET    (12)
#define     RTC_LDOCON1_ISEL_LDO08_RTC_OFFSET   (14)

#define     RTC_LDOCON1_ISEL_ADC_RTC_MASK       (0x3<<RTC_LDOCON1_ISEL_ADC_RTC_OFFSET)
#define     RTC_LDOCON1_ISEL_OP_RTC_MASK        (0x3<<RTC_LDOCON1_ISEL_OP_RTC_OFFSET)
#define     RTC_LDOCON1_ISEL_LDO045_RTC_MASK    (0x3<<RTC_LDOCON1_ISEL_LDO045_RTC_OFFSET)
#define     RTC_LDOCON1_ISEL_LDO09_RTC_MASK     (0x3<<RTC_LDOCON1_ISEL_LDO09_RTC_OFFSET)
#define     RTC_LDOCON1_ISEL_VREF_RTC_MASK      (0x3<<RTC_LDOCON1_ISEL_VREF_RTC_OFFSET)
#define     RTC_LDOCON1_ISEL_PORL_RTC_MASK      (0x3<<RTC_LDOCON1_ISEL_PORL_RTC_OFFSET)
#define     RTC_LDOCON1_ISEL_PORH_RTC_MASK      (0x3<<RTC_LDOCON1_ISEL_PORH_RTC_OFFSET)
#define     RTC_LDOCON1_ISEL_LDO08_RTC_MASK     (0x3<<RTC_LDOCON1_ISEL_LDO08_RTC_OFFSET)


/*RTC_LDO2 Register*/

/*RTC_CALI Register*/
#define     RTC_CALI_RW_SEL_OFFSET              (13)
#define     RTC_CALI_VALUE_OFFSET               (0)
#define     RTC_CALI_RW_SEL_MASK                (1<<RTC_CALI_RW_SEL_OFFSET)
#define     RTC_CALI_VALUE_MASK                 (0x1FFF<<RTC_CALI_VALUE_OFFSET)

/*RTC_WRTGR Register*/
#define     RTC_WRTGR_WRTGR_OFFSET              (0)
#define     RTC_WRTGR_CBUSY_OFFSER              (8)
#define     RTC_WRTGR_PWR_STA_OFFSER            (9)
#define     RTC_WRTGR_CLK_STA_OFFSER            (10)
#define     RTC_WRTGR_WRTGR_MASK                (1<<RTC_WRTGR_WRTGR_OFFSET)
#define     RTC_WRTGR_CBUSY_MASK                (1<<RTC_WRTGR_CBUSY_OFFSER)
#define     RTC_WRTGR_PWR_STA_MASK              (1<<RTC_WRTGR_PWR_STA_OFFSER)
#define     RTC_WRTGR_CLK_STA_MASK              (1<<RTC_WRTGR_CLK_STA_OFFSER)

/*RTC_GPIO0_CON Register*/
#define     RTC_GPIO0_CON_SR_OFFSET             (0)
#define     RTC_GPIO0_CON_PD_OFFSET             (1)
#define     RTC_GPIO0_CON_PU_OFFSET             (2)
#define     RTC_GPIO0_CON_SMT_OFFSET            (3)
#define     RTC_GPIO0_CON_E8_OFFSET             (4)
#define     RTC_GPIO0_CON_E4_OFFSET             (5)
#define     RTC_GPIO0_CON_E_OFFSET              (6)
#define     RTC_GPIO0_CON_G_OFFSET              (7)
#define     RTC_GPIO0_CON_DBG_OFFSET            (8)

/*RTC_GPIO1_CON Register*/
#define     RTC_GPIO1_CON_SR_OFFSET             (0)
#define     RTC_GPIO1_CON_PD_OFFSET             (1)
#define     RTC_GPIO1_CON_PU_OFFSET             (2)
#define     RTC_GPIO1_CON_SMT_OFFSET            (3)
#define     RTC_GPIO1_CON_E8_OFFSET             (4)
#define     RTC_GPIO1_CON_E4_OFFSET             (5)
#define     RTC_GPIO1_CON_E_OFFSET              (6)
#define     RTC_GPIO1_CON_G_OFFSET              (7)
#define     RTC_GPIO1_CON_DBG_OFFSET            (8)

/*RTC_GPIO2_CON*/
#define     RTC_GPIO2_CON_SR_OFFSET             (0)
#define     RTC_GPIO2_CON_PD_OFFSET             (1)
#define     RTC_GPIO2_CON_PU_OFFSET             (2)
#define     RTC_GPIO2_CON_SMT_OFFSET            (3)
#define     RTC_GPIO2_CON_E8_OFFSET             (4)
#define     RTC_GPIO2_CON_E4_OFFSET             (5)
#define     RTC_GPIO2_CON_E_OFFSET              (6)
#define     RTC_GPIO2_CON_G_OFFSET              (7)
#define     RTC_GPIO2_CON_DBG_OFFSET            (8)

#define     RTC_GPIO_SR_OFFSET                  (0)
#define     RTC_GPIO_PD_OFFSET                  (1)
#define     RTC_GPIO_PU_OFFSET                  (2)
#define     RTC_GPIO_SMT_OFFSET                 (3)
#define     RTC_GPIO_E8_OFFSET                  (4)
#define     RTC_GPIO_E4_OFFSET                  (5)
#define     RTC_GPIO_E_OFFSET                   (6)
#define     RTC_GPIO_G_OFFSET                   (7)
#define     RTC_GPIO_DBG_OFFSET                 (8)

/*CAP_CON*/
#define     RTC_CAP_CON_RST_OFFSET              (0)
#define     RTC_CAP_CON_ISO_EN_OFFSET           (1)
#define     RTC_CAP_CON_CLK_CG_OFFSET           (2)
#define     RTC_CAP_CON_LSH_EN_OFFSET           (3)

#define     RTC_CAP_CON_RST_MASK                (1<<RTC_CAP_CON_RST_OFFSET)
#define     RTC_CAP_CON_ISO_EN_MASK             (1<<RTC_CAP_CON_ISO_EN_OFFSET)
#define     RTC_CAP_CON_CLK_CG_MASK             (1<<RTC_CAP_CON_CLK_CG_OFFSET)
#define     RTC_CAP_CON_LSH_EN_MASK             (1<<RTC_CAP_CON_LSH_EN_OFFSET)

/*RTC_RESET_CON*/
#define     RTC_RESET_CON_32KOFF_OFFSET         (0)
#define     RTC_RESET_CON_CLR_PWR_OFFSET        (8)
#define     RTC_RESET_CON_CLR_CLK_OFFSET        (9)
#define     RTC_RESET_CON_32KOFF_MASK           (1<<RTC_RESET_CON_32KOFF_OFFSET)
#define     RTC_RESET_CON_CLR_PWR_MASK          (1<<RTC_RESET_CON_CLR_PWR_OFFSET)
#define     RTC_RESET_CON_CLR_CLK_MASK          (1<<RTC_RESET_CON_CLR_CLK_OFFSET)

/*GPIO_CON*/
#define     RTC_GPIO_CON_0_OUT_OFFSET           (0)
#define     RTC_GPIO_CON_1_OUT_OFFSET           (1)
#define     RTC_GPIO_CON_2_OUT_OFFSET           (2)
#define     RTC_GPIO_CON_0_IN_OFFSET            (8)
#define     RTC_GPIO_CON_1_IN_OFFSET            (9)
#define     RTC_GPIO_CON_2_IN_OFFSET            (10)

/*RTC_SPARE_REG*/
#define     RTC_SPAR_REG_BROM_SKIP_OFFSET       (0)
#define     RTC_SPAR_REG_PWR_FLG_OFFSET         (7)
#define     RTC_SPAR_REG_RESRV_OFFSET           (8)
#define     RTC_SPAR_REG_BROM_SKIP_MASK         (1<<RTC_SPAR_REG_BROM_SKIP_OFFSET)
#define     RTC_SPAR_REG_PWR_FLG_MASK           (1<<RTC_SPAR_REG_PWR_FLG_OFFSET)

/*RTC_DEBUG_CON0*/
#define     RTC_DEBUG_CON0_DBG_SEL_OFFSET       (0)
#define     RTC_DEBUG_CON0_GPIO_SEL_OFFSET      (4)
#define     RTC_DEBUG_CON0_DBG_EN_OFFSET        (8)

#define     RTC_DEBUG_CON0_DBG_SEL_MASK         (0x1<<RTC_DEBUG_CON0_DBG_SEL_OFFSET)
#define     RTC_DEBUG_CON0_GPIO_SEL_MASK        (0x7<<RTC_DEBUG_CON0_GPIO_SEL_OFFSET)
#define     RTC_DEBUG_CON0_DBG_EN_MASK          (0xf<<RTC_DEBUG_CON0_DBG_EN_OFFSET)


/************************ RTC register definition end line  *******************************
 */

/* ************************* CRYPTO hardware definition start line**********************************
*/
#define CRYPTO_ENGINE_CTRL_BASE (volatile uint32_t *)(CRYPTO_BASE + 0x4)
#define ENGINE_CTRL_START_OFFSET (4)

#define CRYPTO_ENGINE_STA_BASE (volatile uint32_t *)(CRYPTO_BASE + 0x8)
#define CRYPTO_ENGINE_STA_INT_CLR (16)

/************************ CRYPTO register definition end line  *******************************
 */

/* *************************trng hardware definition start line**********************************
*/

typedef struct {
    __IO uint32_t TRNG_CTRL;         /*!<  trng control register Address offset: 0x00 */
    __IO uint32_t TRNG_TIME;         /*!<  trng time register Address offset: 0x04 */
    __I  uint32_t TRNG_DATA;         /*!<  trng data registerAddress offset: 0x08 */
    __IO uint32_t TRNG_CONF;         /*!<  trng configure registerAddress offset: 0x0C */
    __I  uint32_t TRNG_INT_SET;      /*!<  trng interrupt setting registerAddress offset: 0x10 */
    __IO uint32_t TRNG_INT_CLR;      /*!<  trng interrupt clean  registerAddress offset: 0x14 */
} TRNG_REGISTER_T;


#define  TRNG                           ((TRNG_REGISTER_T *) (TRNG_BASE))

/* the bit value in TRNG  CONTROL register */
#define  TRNG_START_OFFSET              (0)
#define  TRNG_START_MASK                (0x1UL<< TRNG_START_OFFSET)

#define  TRNG_STOP_OFFSET               (0)
#define  TRNG_STOP_MASK                 (0x1UL<< TRNG_STOP_OFFSET)

/* the bit value in TRNG INT_SET register */
#define  TRNG_RDY_OFFSET                (0)
#define  TRNG_RDY_MASK                  (0x1UL<< TRNG_RDY_OFFSET)

/* the bit value in TRNG INT_SET register */
#define  TRNG_TIMEOUT_OFFSET            (1)
#define  TRNG_TIMEOUT_MASK              (0x1UL<< TRNG_TIMEOUT_OFFSET)

/* the bit value in TRNG INT_CLR register */
#define  TRNG_INT_CLR_OFFSET            (0)
#define  TRNG_INT_CLR_MASK              (0x3UL<< TRNG_INT_CLR_OFFSET)

/* the bit value in TRNG  CONF register */
#define  TRNG_H_FIRO_OFFSET             (8)
#define  TRNG_H_FIRO_EN_MASK            (0x1UL<< TRNG_H_FIRO_OFFSET)

#define  TRNG_H_RO_EN_OFFSET            (9)
#define  TRNG_H_RO_EN_MASK              (0x1UL<< TRNG_H_RO_EN_OFFSET)

#define  TRNG_H_GARO_OFFSET             (10)
#define  TRNG_H_GARO_EN_MASK            (0x1UL<< TRNG_H_GARO_OFFSET)

#define  TRNG_H_GARO2_OFFSET            (11)
#define  TRNG_H_GARO2_EN_MASK           (0x1UL<< TRNG_H_GARO_OFFSET)

#define  TRNG_H_GARO3_OFFSET            (12)
#define  TRNG_H_GARO3_EN_MASK           (0x1UL<< TRNG_H_GARO_OFFSET)

#define  TRNG_H_GARO4_OFFSET            (13)
#define  TRNG_H_GARO4_EN_MASK           (0x1UL<< TRNG_H_GARO_OFFSET)

#define  TRNG_H_GARO5_OFFSET            (14)
#define  TRNG_H_GARO5_EN_MASK           (0x1UL<< TRNG_H_GARO_OFFSET)

#define  TRNG_VON_EN_OFFSET             (28)
#define  TRNG_VON_EN_MASK               (0x1UL<< TRNG_VON_EN_OFFSET)


/* *************************trng hardware definition end line**********************************
*/

/************************ WDT register definition start line  ********************************/
typedef struct {
    __IO uint32_t WDT_EN;               /* 0x00 */
    __IO uint32_t WDT_LENGTH;           /* 0x04 */
    __IO uint32_t WDT_INTERVAL;         /* 0x08 */
    __O uint32_t WDT_SW_RESTART;        /* 0x0C */
    __O uint32_t WDT_SW_RST;            /* 0x10 */
    __IO uint32_t WDT_AUTO_RESTART_EN;  /* 0x14 */
    __I uint32_t WDT_STA;               /* 0x18 */
    __IO uint32_t WDT_IE;               /* 0x1c */
    __I uint32_t WDT_INT;               /* 0x20 */
    __IO uint32_t WDT_WAKEUP_EN;        /* 0x24 */
    __I uint32_t WDT_WAKEUP;            /* 0x28 */
    __I uint32_t RESERVED_1;            /* 0x2C */

    __IO uint32_t WDT1_EN;              /* 0x30 */
    __IO uint32_t WDT1_LENGTH;          /* 0x34 */
    __IO uint32_t WDT1_INTERVAL;        /* 0x38 */
    __O uint32_t WDT1_SW_RESTART;       /* 0x3C */
    __O uint32_t WDT1_SW_RST;           /* 0x40 */
    __IO uint32_t WDT1_AUTO_RESTART_EN; /* 0x44 */
    __I uint32_t WDT1_STA;              /* 0x48 */
    __IO uint32_t WDT1_IE;              /* 0x4C */
    __I uint32_t WDT1_INT;              /* 0x50 */
    __IO uint32_t WDT1_WAKEUP_EN;       /* 0x54 */
    __I uint32_t WDT1_WAKEUP;           /* 0x58 */
    __I uint32_t RESERVED_2;            /* 0x5C */

    union {
        struct {
            __IO uint16_t SYSTEM_SW_RST;
            __I uint16_t RESERVED;
        } WDT_SW_RSTSYS_CELLS;
        __IO uint32_t WDT_SW_RSTSYS;
    } WDT_SW_RSTSYS_UNION;             /* 0x60 */
    union {
        struct {
            __IO uint16_t DSPSYS_CORE_SW_RST;
            __IO uint16_t DSPSYS_DEBUG_SW_RST;
        } WDT_RST0_CELLS;
        __IO uint32_t WDT_RST0;
    } WDT_RST0_UNION;                  /* 0x64 */
    union {
        struct {
            __IO uint16_t CONNSYS_SW_RST;
            __I uint16_t RESERVED;
        } WDT_RST1_CELLS;
        __IO uint32_t WDT_RST1;
    } WDT_RST1_UNION;                  /* 0x68 */
    union {
        struct {
            __IO uint16_t AUDIOSYS_SW_RST;
            __I uint16_t RESERVED;
        } WDT_RST2_CELLS;
        __IO uint32_t WDT_RST2;
    } WDT_RST2_UNION;                  /* 0x6C */
    union {
        struct {
            __IO uint16_t USB_SW_RST;
            __IO uint16_t USBSIF_SW_RST;
        } WDT_RST3_CELLS;
        __IO uint32_t WDT_RST3;
    } WDT_RST3_UNION;                  /* 0x70 */

    __I uint32_t RESERVED_3[3];        /* 0x74 - 0x7C */

    union {
        struct {
            __IO uint16_t JTAG_RESET;
            __IO uint16_t AIRCR_RESET;
        } WDT_MASK0_CELLS;
        __IO uint32_t WDT_MASK0;
    } WDT_MASK0_UNION;                 /* 0x80 */
    union {
        struct {
            __IO uint16_t PCM_RESET;
            __IO uint16_t PMU_RESET;
        } WDT_MASK1_CELLS;
        __IO uint32_t WDT_MASK1;
    } WDT_MASK1_UNION;                 /* 0x84 */

    __I uint32_t RESERVED_4[2];        /* 0x88 - 0x8C */

    union {
        struct {
            __IO uint16_t AIRCR_RST_INTERVAL;
            __I uint16_t RESERVED1;
        } WDT_FUNC_CONT0_CELLS;
        __IO uint32_t WDT_FUNC_CONT0;
    } WDT_FUNC_CONT0_UNION;            /* 0x90 */
    union {
        struct {
            __IO uint16_t PMU_RST_INV_EN;
            __IO uint16_t PMIC_IO_RST_INV_EN;
        } WDT_FUNC_CONT1_CELLS;
        __IO uint32_t WDT_FUNC_CONT1;
    } WDT_FUNC_CONT1_UNION;            /* 0x94 */

    __I uint32_t RESERVED_5[2];        /* 0x98 - 0x9C */

    __IO uint32_t WDT_RETN_FLAG0;      /* 0xA0 */
    __IO uint32_t WDT_RETN_FLAG1;      /* 0xA4 */
    __IO uint32_t WDT_RETN_FLAG2;      /* 0xA8 */
    __IO uint32_t WDT_RETN_FLAG3;      /* 0xAC */
    __IO uint32_t WDT_RETN_FLAG4;      /* 0xB0 */
    __IO uint32_t WDT_RETN_FLAG5;      /* 0xB4 */

    __I uint32_t RESERVED_6[2];        /* 0xB8 - 0xBC */

    __IO uint32_t WDT_RETN_DAT0;       /* 0xC0 */
    __IO uint32_t WDT_RETN_DAT1;       /* 0xC4 */
    __IO uint32_t WDT_RETN_DAT2;       /* 0xC8 */
    __IO uint32_t WDT_RETN_DAT3;       /* 0xCC */
    __IO uint32_t WDT_RETN_DAT4;       /* 0xD0 */
    __IO uint32_t WDT_RETN_DAT5;       /* 0xD4 */
} WDT_REGISTER_T;

#define WDT_REGISTER  ((WDT_REGISTER_T *)(WDT_BASE))

/* Bit definition for WDT_EN Register */
#define WDT_EN_TST_MODE_OFFSET              (24)
#define WDT_EN_TST_MODE_MASK                (1 << WDT_EN_TST_MODE_OFFSET)
#define WDT_EN_TST_MODE_KEY_OFFSET          (16)
#define WDT_EN_TST_MODE_KEY_MASK            (0x11 << WDT_EN_TST_MODE_KEY_OFFSET)

#define WDT_EN_OFFSET                       (8)
#define WDT_EN_MASK                         (1 << WDT_EN_OFFSET)
#define WDT_EN_KEY_OFFSET                   (0)
#define WDT_EN_KEY                          (0x10)
#define WDT1_EN_KEY                         (0x40)

/* Bit definition for WDT_LENGTH Register */
#define WDT_LENGTH_OFFSET                   (16)
#define WDT_LENGTH_MASK                     (0xFFFF << WDT_LENGTH_OFFSET)
#define WDT_LENGTH_KEY_OFFSET               (0)
#define WDT_LENGTH_KEY                      (0x12)
#define WDT1_LENGTH_KEY                     (0x42)

/* Bit definition for WDT_INTERVAL Register */
#define WDT_INTERVAL_OFFSET                 (16)
#define WDT_INTERVAL_MASK                   (0xFFFF << WDT_INTERVAL_OFFSET)
#define WDT_INTERVAL_KEY_OFFSET             (0)
#define WDT_INTERVAL_KEY                    (0x13)
#define WDT1_INTERVAL_KEY                   (0x43)

/* Bit definition for WDT_SW_RESTART Register */
#define WDT_SW_RESTART_KEY                  (0x1456789a)
#define WDT1_SW_RESTART_KEY                 (0x4456789a)

/* Bit definition for WDT_SW_RST Register */
#define WDT_SW_RST_KEY                      (0x156789ab)
#define WDT1_SW_RST_KEY                     (0x456789ab)

/* Bit definition for WDT_AUTO_RESTART_EN Register */
#define WDT_AUTO_RESTART_EN_OFFSET          (8)
#define WDT_AUTO_RESTART_EN_MASK            (1 << WDT_AUTO_RESTART_EN_OFFSET)
#define WDT_AUTO_RESTART_EN_KEY_OFFSET      (0)
#define WDT_AUTO_RESTART_EN_KEY             (0x16)
#define WDT1_AUTO_RESTART_EN_KEY            (0x46)

/* Bit definition for WDT_STA Register */
#define WDT_SW_STA_OFFSET                   (0)
#define WDT_SW_STA_MASK                     (1 << WDT_SW_STA_OFFSET)
#define WDT_HW_STA_OFFSET                   (1)
#define WDT_HW_STA_MASK                     (1 << WDT_HW_STA_OFFSET)

/* Bit definition for WDT_IE Register */
#define WDT_IRQ_IE_OFFSET                   (24)
#define WDT_IRQ_IE_MASK                     (1 << WDT_IRQ_IE_OFFSET)
#define WDT_IRQ_IE_KEY_OFFSET               (16)
#define WDT_IRQ_IE_KEY                      (0x18)
#define WDT1_IRQ_IE_KEY                     (0x48)
#define WDT_NMI_IE_OFFSET                   (8)
#define WDT_NMI_IE_MASK                     (1 << WDT_NMI_IE_OFFSET)
#define WDT_NMI_IE_KEY_OFFSET               (0)
#define WDT_NMI_IE_KEY                      (0x17)
#define WDT1_NMI_IE_KEY                     (0x47)

/* Bit definition for WDT_INT Register */
#define WDT_INT_OFFSET                      (0)
#define WDT_INT_MASK                        (1 << WDT_INT_OFFSET)

/* Bit definition for WDT_WAKEUP_EN Register */
#define WDT_WAKEUP_EN_OFFSET                (8)
#define WDT_WAKEUP_EN_MASK                  (1 << WDT_WAKEUP_EN_OFFSET)
#define WDT_WAKEUP_EN_KEY_OFFSET            (0)
#define WDT_WAKEUP_EN_KEY                   (0x19)
#define WDT1_WAKEUP_EN_KEY                  (0x49)

/* Bit definition for WDT_WAKEUP Register */
#define WDT_WAKEUP_OFFSET                   (0)
#define WDT_WAKEUP_MASK                     (1 << WDT_WAKEUP_OFFSET)

/* Bit definition for WDT_SW_RSTSYS Register */
#define WDT_SW_RSTSYS_OFFSET                 (8)
#define WDT_SW_RSTSYS_MASK                   (1 << WDT_SW_RSTSYS_OFFSET)
#define WDT_SW_RSTSYS_KEY_OFFSET             (0)
#define WDT_SW_RSTSYS_KEY                    (0x1A)

/* Bit definition for WDT_SW_RST0 Register */
#define WDT_DSPSYS_DEBUG_SW_RST_OFFSET       (8)
#define WDT_DSPSYS_DEBUG_SW_RST_MASK         (1 << WDT_DSPSYS_DEBUG_SW_RST_OFFSET)
#define WDT_DSPSYS_DEBUG_SW_RST_KEY_OFFSET   (0)
#define WDT_DSPSYS_DEBUG_SW_RST_KEY          (0x1C)
#define WDT_DSPSYS_CORE_SW_RST_OFFSET        (8)
#define WDT_DSPSYS_CORE_SW_RST_MASK          (1 << WDT_DSPSYS_CORE_SW_RST_OFFSET)
#define WDT_DSPSYS_CORE_SW_RST_KEY_OFFSET    (0)
#define WDT_DSPSYS_CORE_SW_RST_KEY           (0x1B)

/* Bit definition for WDT_SW_RST1 Register */
#define WDT_CONNSYS_SW_RST_OFFSET            (8)
#define WDT_CONNSYS_SW_RST_MASK              (1 << WDT_CONNSYS_SW_RST_OFFSET)
#define WDT_CONNSYS_SW_RST_KEY_OFFSET        (0)
#define WDT_CONNSYS_SW_RST_KEY               (0x1D)

/* Bit definition for WDT_SW_RST2 Register */
#define WDT_AUDIOSYS_SW_RST_OFFSET           (8)
#define WDT_AUDIOSYS_SW_RST_MASK             (1 << WDT_AUDIOSYS_SW_RST_OFFSET)
#define WDT_AUDIOSYS_SW_RST_KEY_OFFSET       (0)
#define WDT_AUDIOSYS_SW_RST_KEY              (0x1F)

/* Bit definition for WDT_SW_RST3 Register */
#define WDT_USBIF_SW_RST_OFFSET              (8)
#define WDT_USBIF_SW_RST_MASK                (1 << WDT_USBIF_SW_RST_OFFSET)
#define WDT_USBIF_SW_RST_KEY_OFFSET          (0)
#define WDT_USBIF_SW_RST_KEY                 (0x22)
#define WDT_USB_SW_RST_OFFSET                (8)
#define WDT_USB_SW_RST_MASK                  (1 << WDT_USB_SW_RST_OFFSET)
#define WDT_USB_SW_RST_KEY_OFFSET            (0)
#define WDT_USB_SW_RST_KEY                   (0x21)

/* Bit definition for WDT_RST_MASK0 Register */
#define WDT_AIRCR_MASK_SW_RST_OFFSET         (24)
#define WDT_AIRCR_MASK_SW_RST_MASK           (1 << WDT_AIRCR_MASK_SW_RST_OFFSET)
#define WDT_AIRCR_MASK_SW_RST_KEY_OFFSET     (16)
#define WDT_AIRCR_MASK_SW_RST_KEY            (0x24)
#define WDT_JTAG_MASK_SW_RST_OFFSET          (8)
#define WDT_JTAG_MASK_SW_RST_MASK            (1 << WDT_JTAG_MASK_SW_RST_OFFSET)
#define WDT_JTAG_MASK_SW_RST_KEY_OFFSET      (0)
#define WDT_JTAG_MASK_SW_RST_KEY             (0x23)

/* Bit definition for WDT_RST_MASK1 Register */
#define WDT_PMU_MASK_SW_RST_OFFSET           (8)
#define WDT_PMU_MASK_SW_RST_MASK             (1 << WDT_PMU_MASK_SW_RST_OFFSET)
#define WDT_PMU_MASK_SW_RST_KEY_OFFSET       (0)
#define WDT_PMU_MASK_SW_RST_KEY              (0x26)
#define WDT_PCM_MASK_SW_RST_OFFSET           (8)
#define WDT_PCM_MASK_SW_RST_MASK             (1 << WDT_PCM_MASK_SW_RST_OFFSET)
#define WDT_PCM_MASK_SW_RST_KEY_OFFSET       (0)
#define WDT_PCM_MASK_SW_RST_KEY              (0x25)

/* Bit definition for WDT_RST_FUNC_CONT0 Register */
#define WDT_AIRCR_SW_RST_INTERVAL_OFFSET     (8)
#define WDT_AIRCR_SW_RST_INTERVAL_MASK       (0xFF << WDT_AIRCR_SW_RST_INTERVAL_OFFSET)
#define WDT_AIRCR_SW_RST_INTERVAL_KEY_OFFSET (0)
#define WDT_AIRCR_SW_RST_INTERVAL_KEY        (0x27)

/* Bit definition for WDT_RST_FUNC_CONT1 Register */
#define WDT_PMIC_SW_RST_POLARITY_OFFSET      (8)
#define WDT_PMIC_SW_RST_POLARITY_MASK        (1 << WDT_PMIC_SW_RST_POLARITY_OFFSET)
#define WDT_PMIC_SW_RST_POLARITY_KEY_OFFSET  (0)
#define WDT_PMIC_SW_RST_POLARITY_KEY         (0x29)
#define WDT_PMU_SW_RST_POLARITY_OFFSET       (8)
#define WDT_PMU_SW_RST_POLARITY_MASK         (1 << WDT_PMU_SW_RST_POLARITY_OFFSET)
#define WDT_PMU_SW_RST_POLARITY_KEY_OFFSET   (0)
#define WDT_PMU_SW_RST_POLARITY_KEY          (0x28)

/* Bit definition for WDT_RETN_FLAG0 ~ 5 Register */
#define WDT_RETENTION_FALG_OFFSET            (16)
#define WDT_RETENTION_FALG_MASK              (0xFFFF << WDT_RETENTION_FALG_OFFSET)
#define WDT_RETENTION_FALG_KEY_OFFSET        (0)
#define WDT_RETENTION_FALG0_KEY              (0x2A)
#define WDT_RETENTION_FALG1_KEY              (0x2B)
#define WDT_RETENTION_FALG2_KEY              (0x2C)
#define WDT_RETENTION_FALG3_KEY              (0x2D)
#define WDT_RETENTION_FALG4_KEY              (0x2E)
#define WDT_RETENTION_FALG5_KEY              (0x2F)

/************************ WDT register definition end line  ********************************/



 /* *************************pwm hardware definition start line**********************************
*/

/*pwm register structure definition*/
typedef struct {
    __IO uint32_t PWM_CTRL;         /*!<  PWM control register Address offset: 0x00 */
    __IO uint32_t PWM_COUNT;        /*!< PWM counter register Address offset: 0x04 */
    __IO uint32_t PWM_THRESH;      /*!<  pwm thresh up registerAddress offset:   0x08 */
    __IO uint32_t PWM_THRESH_DOWN;      /*!<  pwm thresh down registerAddress offset:   0x0c */
} PWM_REGISTER_T;

#define PWM0                  ((PWM_REGISTER_T *) (PWM0_BASE))
#define PWM1                  ((PWM_REGISTER_T *) (PWM1_BASE))
#define PWM2                  ((PWM_REGISTER_T *) (PWM2_BASE))
#define PWM3                  ((PWM_REGISTER_T *) (PWM3_BASE))
#define PWM4                  ((PWM_REGISTER_T *) (PWM4_BASE))
#define PWM5                  ((PWM_REGISTER_T *) (PWM5_BASE))
#define PWM6                  ((PWM_REGISTER_T *) (PWM6_BASE))
#define PWM7                  ((PWM_REGISTER_T *) (PWM7_BASE))
#define PWM8                  ((PWM_REGISTER_T *) (PWM8_BASE))
#define PWM9                  ((PWM_REGISTER_T *) (PWM9_BASE))
#define PWM10                 ((PWM_REGISTER_T *) (PWM10_BASE))
#define PWM11                 ((PWM_REGISTER_T *) (PWM11_BASE))
#define PWM12                 ((PWM_REGISTER_T *) (PWM12_BASE))
#define PWM13                 ((PWM_REGISTER_T *) (PWM13_BASE))





/*the bit value in PWM  control  register*/
#define PWM_CLK_SEL_OFFSET            (2)
#define PWM_CLK_SEL_MASK              (0x3<< PWM_CLK_SEL_OFFSET)

#define PWM_CLK_SEL_13M_MASK          (0x0<< PWM_CLK_SEL_OFFSET)

#define PWM_CLK_SEL_32K_MASK          (0x1<< PWM_CLK_SEL_OFFSET)

#define PWM_CLK_SEL_40M_MASK          (0x2<< PWM_CLK_SEL_OFFSET)

/*the bit mask definition in  PWM control  register */
#define PWM_CLK_DIV_OFFSET            (0)
#define PWM_CLK_DIV_MASK                   (0x3<< PWM_CLK_DIV_OFFSET)



/* *************************pwm hardware definition end line**********************************
*/

/*************************** MSDC register definition start line  *******************************
 */
 /*MSDC register structure definition*/
typedef struct {
    __IO uint32_t MSDC_CFG;      /* base+0x00h, MSDC configuration register. */
    __IO uint32_t MSDC_IOCON;     /* base+0x04h, MSDC IO configuration register. */
    __IO uint32_t MSDC_PS;         /* base+0x08h, MSDC Pin Status register. */
    __IO uint32_t MSDC_INT;      /* base+0x0ch, MSDC Interrupt Register. */
    __IO uint32_t MSDC_INTEN;     /* base+0x10h, MSDC Interrupt Enable Register. */
    __IO uint32_t MSDC_FIFOCS;     /* base+0x14h, MSDC FIFO Control and Status Register. */
    __IO uint32_t MSDC_TXDATA;     /* base+0x18h, MSDC TX Data Port Register. */
    __IO uint32_t MSDC_RXDATA;     /* base+0x1ch, MSDC RX Data Port Register. */
    __IO uint32_t rsv1[4];
    __IO uint32_t SDC_CFG;         /* base+0x30h, SD Configuration Register. */
    __IO uint32_t SDC_CMD;         /* base+0x34h, SD Command Register. */
    __IO uint32_t SDC_ARG;         /* base+0x38h, SD Argument Register. */
    __I  uint32_t SDC_STS;         /* base+0x3ch, SD Status Register. */
    __I  uint32_t SDC_RESP0;     /* base+0x40h, SD Response Register 0. */
    __I  uint32_t SDC_RESP1;     /* base+0x44h, SD Response Register 1. */
    __I  uint32_t SDC_RESP2;     /* base+0x48h, SD Response Register 2. */
    __I  uint32_t SDC_RESP3;     /* base+0x4ch, SD Response Register 3. */
    __IO uint32_t SDC_BLK_NUM;     /* base+0x50h, SD Block Number Register. */
    __IO uint32_t rsv2[1];
    __IO uint32_t SDC_CSTS;      /* base+0x58h, SD Card Status Register. */
    __IO uint32_t SDC_CSTS_EN;     /* base+0x5ch, SD Card Status Enable Register. */
    __IO uint32_t SDC_DATCRC_STS;/* base+0x60h, SD Card Data CRC Status Register. */
    __IO uint32_t rsv3[7];
    __IO uint32_t SDC_ACMD_RESP; /* base+0x80h, SD ACMD Response Register. */
    __IO uint32_t rsv4[3];
    __IO uint32_t DMA_SA;         /* base+0x90h, DMA Start Address Register.*/
    __IO uint32_t DMA_CA;         /* base+0x94h, DMA Current Address Register. */
    __IO uint32_t DMA_CTRL;      /* base+0x98h, DMA Control Register. */
    __IO uint32_t DMA_CFG;         /* base+0x9ch, DMA Configuration Register. */
    __IO uint32_t SW_DBG_DEL;     /* base+0xa0h, MSDC S/W Debug Selection Register. */
    __IO uint32_t SW_DBG_OUT;     /* base+0xa4h, MSDC S/W Debug Output Register. */
    __IO uint32_t DMA_LENGTH;     /* base+0xa8h, DMA Length Register. */
    __IO uint32_t rsv5[1];
    __IO uint32_t PATCH_BIT0;     /* base+0xb0h, MSDC Patch Bit Register 0. */
    __IO uint32_t PATCH_BIT1;     /* base+0xb4h, MSDC Patch Bit Register 1. */
    __IO uint32_t rsv6[13];
    __IO uint32_t PAD_TUNE;      /* base+0xech, MSDC Pad Tuning Register. */
    __IO uint32_t DAT_RD_DLY0;     /* base+0xf0h, MSDC Data Delay Line Register 0. */
    __IO uint32_t DAT_RD_DLY1;     /* base+0xf4h, MSDC Data Delay Line Register 1. */
    __IO uint32_t HW_DBG_SEL;     /* base+0xf8h, MSDC H/W Debug Selection Register. */
    __IO uint32_t rsv8[1];
    __IO uint32_t MAIN_VER;      /* base+0x100h, MSDC Main Version Register. */
    __IO uint32_t ECO_VER;         /* base+0x104h, MSDC ECO Version Register. */
} MSDC_REGISTER_T;

/*the bit value in MSDC configuration register*/
#define MSDC_CFG_MODE_OFFSET    (0)
#define MSDC_CFG_CKPDN_OFFSET   (1)
#define MSDC_CFG_RST_OFFSET     (2)
#define MSDC_CFG_PIO_OFFSET     (3)
#define MSDC_CFG_CKDRVEN_OFFSET (4)
#define MSDC_CFG_BV18SDT_OFFSET (5)
#define MSDC_CFG_BV18PSS_OFFSET (6)
#define MSDC_CFG_CKSTB_OFFSET   (7)
#define MSDC_CFG_CKDIV_OFFSET   (8)
#define MSDC_CFG_CCKMD_OFFSET   (16)

#define MSDC_CFG_MODE_MASK           (0x1  << MSDC_CFG_MODE_OFFSET)     /* RW */
#define MSDC_CFG_CKPDN_MASK          (0x1  << MSDC_CFG_CKPDN_OFFSET)     /* RW */
#define MSDC_CFG_RST_MASK            (0x1  << MSDC_CFG_RST_OFFSET)     /* RW */
#define MSDC_CFG_PIO_MASK            (0x1  << MSDC_CFG_PIO_OFFSET)     /* RW */
#define MSDC_CFG_CKDRVEN_MASK        (0x1  << MSDC_CFG_CKDRVEN_OFFSET)     /* RW */
#define MSDC_CFG_BV18SDT_MASK        (0x1  << MSDC_CFG_BV18SDT_OFFSET)     /* RW */
#define MSDC_CFG_BV18PSS_MASK        (0x1  << MSDC_CFG_BV18PSS_OFFSET)     /* R  */
#define MSDC_CFG_CKSTB_MASK          (0x1  << MSDC_CFG_CKSTB_OFFSET)     /* R  */
#define MSDC_CFG_CKDIV_MASK          (0xfff << MSDC_CFG_CKDIV_OFFSET)    /* RW */
#define MSDC_CFG_CCKMD_MASK          (1 << MSDC_CFG_CCKMD_OFFSET)

/*the bit value in MSDC IO configuration register*/
#define MSDC_IOCON_SDR104CKS_OFFSET (0)
#define MSDC_IOCON_RSPL_OFFSET      (1)
#define MSDC_IOCON_DSPL_OFFSET      (2)
#define MSDC_IOCON_DDLSEL_OFFSET    (3)
#define MSDC_IOCON_DDR50CKD_OFFSET  (4)
#define MSDC_IOCON_DSPLSEL_OFFSET   (5)
#define MSDC_IOCON_WDSPL_OFFSET     (8)
#define MSDC_IOCON_D0SPL_OFFSET     (16)
#define MSDC_IOCON_D1SPL_OFFSET     (17)
#define MSDC_IOCON_D2SPL_OFFSET     (18)
#define MSDC_IOCON_D3SPL_OFFSET     (19)
#define MSDC_IOCON_D4SPL_OFFSET     (20)
#define MSDC_IOCON_D5SPL_OFFSET     (21)
#define MSDC_IOCON_D6SPL_OFFSET     (22)
#define MSDC_IOCON_D7SPL_OFFSET     (23)
#define MSDC_IOCON_RISCSZ_OFFSET    (24)

#define MSDC_IOCON_SDR104CKS_MASK    (0x1  << MSDC_IOCON_SDR104CKS_OFFSET)     /* RW */
#define MSDC_IOCON_RSPL_MASK         (0x1  << MSDC_IOCON_RSPL_OFFSET)     /* RW */
#define MSDC_IOCON_DSPL_MASK         (0x1  << MSDC_IOCON_DSPL_OFFSET)     /* RW */
#define MSDC_IOCON_DDLSEL_MASK       (0x1  << MSDC_IOCON_DDLSEL_OFFSET)     /* RW */
#define MSDC_IOCON_DDR50CKD_MASK     (0x1  << MSDC_IOCON_DDR50CKD_OFFSET)     /* RW */
#define MSDC_IOCON_DSPLSEL_MASK      (0x1  << MSDC_IOCON_DSPLSEL_OFFSET)     /* RW */
#define MSDC_IOCON_WDSPL_MASK        (0x1  << MSDC_IOCON_WDSPL_OFFSET)     /* RW */
#define MSDC_IOCON_D0SPL_MASK        (0x1  << MSDC_IOCON_D0SPL_OFFSET)    /* RW */
#define MSDC_IOCON_D1SPL_MASK        (0x1  << MSDC_IOCON_D1SPL_OFFSET)    /* RW */
#define MSDC_IOCON_D2SPL_MASK        (0x1  << MSDC_IOCON_D2SPL_OFFSET)    /* RW */
#define MSDC_IOCON_D3SPL_MASK        (0x1  << MSDC_IOCON_D3SPL_OFFSET)    /* RW */
#define MSDC_IOCON_D4SPL_MASK        (0x1  << MSDC_IOCON_D4SPL_OFFSET)    /* RW */
#define MSDC_IOCON_D5SPL_MASK        (0x1  << MSDC_IOCON_D5SPL_OFFSET)    /* RW */
#define MSDC_IOCON_D6SPL_MASK        (0x1  << MSDC_IOCON_D6SPL_OFFSET)    /* RW */
#define MSDC_IOCON_D7SPL_MASK        (0x1  << MSDC_IOCON_D7SPL_OFFSET)    /* RW */
#define MSDC_IOCON_RISCSZ_MASK       (0x3  << MSDC_IOCON_RISCSZ_OFFSET)    /* RW */

/*the bit value in MSDC Pin Status register*/
#define MSDC_PS_CDEN_OFFSET       (0)
#define MSDC_PS_CDSTS_OFFSET      (1)
#define MSDC_PS_CDDEBOUNCE_OFFSET (12)
#define MSDC_PS_DAT_OFFSET        (16)
#define MSDC_PS_CMD_OFFSET        (24)
#define MSDC_PS_WP_OFFSET         (31)

#define MSDC_PS_CDEN_MASK            (0x1  << MSDC_PS_CDEN_OFFSET)     /* RW */
#define MSDC_PS_CDSTS_MASK           (0x1  << MSDC_PS_CDSTS_OFFSET)     /* R  */
#define MSDC_PS_CDDEBOUNCE_MASK      (0xf  << MSDC_PS_CDDEBOUNCE_OFFSET)    /* RW */
#define MSDC_PS_DAT_MASK             (0xff << MSDC_PS_DAT_OFFSET)    /* R  */
#define MSDC_PS_CMD_MASK             (0x1  << MSDC_PS_CMD_OFFSET)    /* R  */
#define MSDC_PS_WP_MASK              (0x1UL<< MSDC_PS_WP_OFFSET)    /* R  */

/*the bit value in MSDC Interrupt Register*/
#define MSDC_INT_MMCIRQ_OFFSET      (0)
#define MSDC_INT_CDSC_OFFSET        (1)
#define MSDC_INT_ACMDRDY_OFFSET     (3)
#define MSDC_INT_ACMDTMO_OFFSET     (4)
#define MSDC_INT_ACMDCRCERR_OFFSET  (5)
#define MSDC_INT_DMAQ_EMPTY_OFFSET  (6)
#define MSDC_INT_SDIOIRQ_OFFSET     (7)
#define MSDC_INT_CMDRDY_OFFSET      (8)
#define MSDC_INT_CMDTMO_OFFSET      (9)
#define MSDC_INT_RSPCRCERR_OFFSET   (10)
#define MSDC_INT_CSTA_OFFSET        (11)
#define MSDC_INT_XFER_COMPL_OFFSET  (12)
#define MSDC_INT_DXFER_DONE_OFFSET  (13)
#define MSDC_INT_DATTMO_OFFSET      (14)
#define MSDC_INT_DATCRCERR_OFFSET   (15)
#define MSDC_INT_ACMD19_DONE_OFFSET (16)

#define MSDC_INT_MMCIRQ_MASK         (0x1  << MSDC_INT_MMCIRQ_OFFSET)     /* W1C */
#define MSDC_INT_CDSC_MASK           (0x1  << MSDC_INT_CDSC_OFFSET)     /* W1C */
#define MSDC_INT_ACMDRDY_MASK        (0x1  << MSDC_INT_ACMDRDY_OFFSET)     /* W1C */
#define MSDC_INT_ACMDTMO_MASK        (0x1  << MSDC_INT_ACMDTMO_OFFSET)     /* W1C */
#define MSDC_INT_ACMDCRCERR_MASK     (0x1  << MSDC_INT_ACMDCRCERR_OFFSET)     /* W1C */
#define MSDC_INT_DMAQ_EMPTY_MASK     (0x1  << MSDC_INT_DMAQ_EMPTY_OFFSET)     /* W1C */
#define MSDC_INT_SDIOIRQ_MASK        (0x1  << MSDC_INT_SDIOIRQ_OFFSET)     /* W1C */
#define MSDC_INT_CMDRDY_MASK         (0x1  << MSDC_INT_CMDRDY_OFFSET)     /* W1C */
#define MSDC_INT_CMDTMO_MASK         (0x1  << MSDC_INT_CMDTMO_OFFSET)     /* W1C */
#define MSDC_INT_RSPCRCERR_MASK      (0x1  << MSDC_INT_RSPCRCERR_OFFSET)    /* W1C */
#define MSDC_INT_CSTA_MASK           (0x1  << MSDC_INT_CSTA_OFFSET)    /* R */
#define MSDC_INT_XFER_COMPL_MASK     (0x1  << MSDC_INT_XFER_COMPL_OFFSET)    /* W1C */
#define MSDC_INT_DXFER_DONE_MASK     (0x1  << MSDC_INT_DXFER_DONE_OFFSET)    /* W1C */
#define MSDC_INT_DATTMO_MASK         (0x1  << MSDC_INT_DATTMO_OFFSET)    /* W1C */
#define MSDC_INT_DATCRCERR_MASK      (0x1  << MSDC_INT_DATCRCERR_OFFSET)    /* W1C */
#define MSDC_INT_ACMD19_DONE_MASK    (0x1  << MSDC_INT_ACMD19_DONE_OFFSET)    /* W1C */

/*the bit value in MSDC Interrupt Enable Register*/
#define MSDC_INTEN_MMCIRQ_OFFSET      (0)
#define MSDC_INTEN_CDSC_OFFSET        (1)
#define MSDC_INTEN_ACMDRDY_OFFSET     (3)
#define MSDC_INTEN_ACMDTMO_OFFSET     (4)
#define MSDC_INTEN_ACMDCRCERR_OFFSET  (5)
#define MSDC_INTEN_DMAQ_EMPTY_OFFSET  (6)
#define MSDC_INTEN_SDIOIRQ_OFFSET     (7)
#define MSDC_INTEN_CMDRDY_OFFSET      (8)
#define MSDC_INTEN_CMDTMO_OFFSET      (9)
#define MSDC_INTEN_RSPCRCERR_OFFSET   (10)
#define MSDC_INTEN_CSTA_OFFSET        (11)
#define MSDC_INTEN_XFER_COMP_OFFSETL  (12)
#define MSDC_INTEN_DXFER_DONE_OFFSET  (13)
#define MSDC_INTEN_DATTMO_OFFSET      (14)
#define MSDC_INTEN_DATCRCERR_OFFSET   (15)
#define MSDC_INTEN_ACMD19_DONE_OFFSET (16)

#define MSDC_INTEN_MMCIRQ_MASK       (0x1  << MSDC_INTEN_MMCIRQ_OFFSET)     /* RW */
#define MSDC_INTEN_CDSC_MASK         (0x1  << MSDC_INTEN_CDSC_OFFSET)     /* RW */
#define MSDC_INTEN_ACMDRDY_MASK      (0x1  << MSDC_INTEN_ACMDRDY_OFFSET)     /* RW */
#define MSDC_INTEN_ACMDTMO_MASK      (0x1  << MSDC_INTEN_ACMDTMO_OFFSET)     /* RW */
#define MSDC_INTEN_ACMDCRCERR_MASK   (0x1  << MSDC_INTEN_ACMDCRCERR_OFFSET)     /* RW */
#define MSDC_INTEN_DMAQ_EMPTY_MASK   (0x1  << MSDC_INTEN_DMAQ_EMPTY_OFFSET)     /* RW */
#define MSDC_INTEN_SDIOIRQ_MASK      (0x1  << MSDC_INTEN_SDIOIRQ_OFFSET)     /* RW */
#define MSDC_INTEN_CMDRDY_MASK       (0x1  << MSDC_INTEN_CMDRDY_OFFSET)     /* RW */
#define MSDC_INTEN_CMDTMO_MASK       (0x1  << MSDC_INTEN_CMDTMO_OFFSET)     /* RW */
#define MSDC_INTEN_RSPCRCERR_MASK    (0x1  << MSDC_INTEN_RSPCRCERR_OFFSET)    /* RW */
#define MSDC_INTEN_CSTA_MASK         (0x1  << MSDC_INTEN_CSTA_OFFSET)    /* RW */
#define MSDC_INTEN_XFER_COMPL_MASK   (0x1  << MSDC_INTEN_XFER_COMP_OFFSETL)    /* RW */
#define MSDC_INTEN_DXFER_DONE_MASK   (0x1  << MSDC_INTEN_DXFER_DONE_OFFSET)    /* RW */
#define MSDC_INTEN_DATTMO_MASK       (0x1  << MSDC_INTEN_DATTMO_OFFSET)    /* RW */
#define MSDC_INTEN_DATCRCERR_MASK    (0x1  << MSDC_INTEN_DATCRCERR_OFFSET)    /* RW */
#define MSDC_INTEN_ACMD19_DONE_MASK  (0x1  << MSDC_INTEN_ACMD19_DONE_OFFSET)    /* RW */

/*the bit value in MSDC FIFO Control and Status Register*/
#define MSDC_FIFOCS_RXCNT_OFFSET  (0)
#define MSDC_FIFOCS_TXCNT_OFFSET  (16)
#define MSDC_FIFOCS_CLR_OFFSET    (31)

#define MSDC_FIFOCS_RXCNT_MASK       (0xff << MSDC_FIFOCS_RXCNT_OFFSET)     /* R */
#define MSDC_FIFOCS_TXCNT_MASK       (0xff << MSDC_FIFOCS_TXCNT_OFFSET)    /* R */
#define MSDC_FIFOCS_CLR_MASK         (0x1UL<< MSDC_FIFOCS_CLR_OFFSET)    /* RW */

/*the bit value in SD Configuration Register*/
#define SDC_CFG_SDIOINTWKUP_OFFSET (0)
#define SDC_CFG_INSWKUP_OFFSET     (1)
#define SDC_CFG_BUSWIDTH_OFFSET    (16)
#define SDC_CFG_SDIO_OFFSET        (19)
#define SDC_CFG_SDIOIDE_OFFSET     (20)
#define SDC_CFG_INTATGAP_OFFSET    (21)
#define SDC_CFG_DTOC_OFFSET        (24)

#define SDC_CFG_SDIOINTWKUP_MASK     (0x1  << SDC_CFG_SDIOINTWKUP_OFFSET)     /* RW */
#define SDC_CFG_INSWKUP_MASK         (0x1  << SDC_CFG_INSWKUP_OFFSET)     /* RW */
#define SDC_CFG_BUSWIDTH_MASK        (0x3  << SDC_CFG_BUSWIDTH_OFFSET)    /* RW */
#define SDC_CFG_SDIO_MASK            (0x1  << SDC_CFG_SDIO_OFFSET)    /* RW */
#define SDC_CFG_SDIOIDE_MASK         (0x1  << SDC_CFG_SDIOIDE_OFFSET)    /* RW */
#define SDC_CFG_INTATGAP_MASK        (0x1  << SDC_CFG_INTATGAP_OFFSET)    /* RW */
#define SDC_CFG_DTOC_MASK            (0xffUL << SDC_CFG_DTOC_OFFSET)  /* RW */

/*the bit value in SD Command Register*/
#define SDC_CMD_OPC_OFFSET      (0)
#define SDC_CMD_BRK_OFFSET      (6)
#define SDC_CMD_RSPTYP_OFFSET   (7)
#define SDC_CMD_DTYP_OFFSET     (11)
#define SDC_CMD_RW_OFFSET       (13)
#define SDC_CMD_STOP_OFFSET     (14)
#define SDC_CMD_GOIRQ_OFFSET    (15)
#define SDC_CMD_LEN_OFFSET      (16)
#define SDC_CMD_AUTOCMD_OFFSET  (28)
#define SDC_CMD_VOLSWTH_OFFSET  (30)

#define SDC_CMD_OPC_MASK             (0x3f << SDC_CMD_OPC_OFFSET)     /* RW */
#define SDC_CMD_BRK_MASK             (0x1  << SDC_CMD_BRK_OFFSET)     /* RW */
#define SDC_CMD_RSPTYP_MASK          (0x7  << SDC_CMD_RSPTYP_OFFSET)     /* RW */
#define SDC_CMD_DTYP_MASK            (0x3  << SDC_CMD_DTYP_OFFSET)    /* RW */
#define SDC_CMD_RW_MASK              (0x1  << SDC_CMD_RW_OFFSET)    /* RW */
#define SDC_CMD_STOP_MASK            (0x1  << SDC_CMD_STOP_OFFSET)    /* RW */
#define SDC_CMD_GOIRQ_MASK           (0x1  << SDC_CMD_GOIRQ_OFFSET)    /* RW */
#define SDC_CMD_BLKLEN_MASK          (0xfff<< SDC_CMD_LEN_OFFSET)    /* RW */
#define SDC_CMD_AUTOCMD_MASK         (0x3  << SDC_CMD_AUTOCMD_OFFSET)    /* RW */
#define SDC_CMD_VOLSWTH_MASK         (0x1  << SDC_CMD_VOLSWTH_OFFSET)    /* RW */

/*the bit value in SD Status Register*/
#define SDC_STS_SDCBUSY_OFFSET   (0)
#define SDC_STS_CMDBUSY_OFFSET   (1)
#define SDC_STS_SWR_COMPL_OFFSET (31)

#define SDC_STS_SDCBUSY_MASK         (0x1  << SDC_STS_SDCBUSY_OFFSET)     /* RW */
#define SDC_STS_CMDBUSY_MASK         (0x1  << SDC_STS_CMDBUSY_OFFSET)     /* RW */
#define SDC_STS_SWR_COMPL_MASK       (0x1  << SDC_STS_SWR_COMPL_OFFSET)    /* RW */

/*the bit value in SD Card Data CRC Status Register*/
#define SDC_DCRC_STS_POS_OFFSET (0)
#define SDC_DCRC_STS_NEG_OFFSET (8)

#define SDC_DCRC_STS_POS_MASK        (0xff << SDC_DCRC_STS_POS_OFFSET)     /* RO */
#define SDC_DCRC_STS_NEG_MASK        (0xf  << SDC_DCRC_STS_NEG_OFFSET)     /* RO */

/*the bit value in DMA Control Register*/
#define MSDC_DMA_CTRL_START_OFFSET   (0)
#define MSDC_DMA_CTRL_STOP_OFFSET    (1)
#define MSDC_DMA_CTRL_RESUME_OFFSET  (2)
#define MSDC_DMA_CTRL_MODE_OFFSET    (8)
#define MSDC_DMA_CTRL_LASTBUF_OFFSET (10)
#define MSDC_DMA_CTRL_BRUSTSZ_OFFSET (12)
#define MSDC_DMA_CTRL_XFERSZ_OFFSET  (16)

#define MSDC_DMA_CTRL_START_MASK     (0x1  << MSDC_DMA_CTRL_START_OFFSET)     /* W */
#define MSDC_DMA_CTRL_STOP_MASK      (0x1  << MSDC_DMA_CTRL_STOP_OFFSET)     /* W */
#define MSDC_DMA_CTRL_RESUME_MASK    (0x1  << MSDC_DMA_CTRL_RESUME_OFFSET)     /* W */
#define MSDC_DMA_CTRL_MODE_MASK      (0x1  << MSDC_DMA_CTRL_MODE_OFFSET)     /* RW */
#define MSDC_DMA_CTRL_LASTBUF_MASK   (0x1  << MSDC_DMA_CTRL_LASTBUF_OFFSET)    /* RW */
#define MSDC_DMA_CTRL_BRUSTSZ_MASK   (0x7  << MSDC_DMA_CTRL_BRUSTSZ_OFFSET)    /* RW */
#define MSDC_DMA_CTRL_XFERSZ_MASK    (0xffffUL << MSDC_DMA_CTRL_XFERSZ_OFFSET)/* RW */

/*the bit value in DMA Configuration Register*/
#define MSDC_DMA_CFG_STS_OFFSET       (0)
#define MSDC_DMA_CFG_DECSEN_OFFSET    (1)
#define MSDC_DMA_CFG_BDCSERR_OFFSET   (4)
#define MSDC_DMA_CFG_GPDCSERR_OFFSET  (5)

#define MSDC_DMA_CFG_STS_MASK        (0x1  << MSDC_DMA_CFG_STS_OFFSET)     /* R */
#define MSDC_DMA_CFG_DECSEN_MASK     (0x1  << MSDC_DMA_CFG_DECSEN_OFFSET)     /* RW */
#define MSDC_DMA_CFG_BDCSERR_MASK    (0x1  << MSDC_DMA_CFG_BDCSERR_OFFSET)     /* R */
#define MSDC_DMA_CFG_GPDCSERR_MASK   (0x1  << MSDC_DMA_CFG_GPDCSERR_OFFSET)     /* R */

/*the bit value in MSDC Patch Bit Register 0*/
#define MSDC_PATCH_BIT_WFLSMODE_OFFSET (0)
#define MSDC_PATCH_BIT_ODDSUPP_OFFSET  (1)
#define MSDC_PATCH_BIT_CKGEN_CK_OFFSET (6)
#define MSDC_PATCH_BIT_IODSSEL_OFFSET  (16)
#define MSDC_PATCH_BIT_IOINTSEL_OFFSET (17)
#define MSDC_PATCH_BIT_BUSYDLY_OFFSET  (18)
#define MSDC_PATCH_BIT_WDOD_OFFSET     (22)
#define MSDC_PATCH_BIT_IDRTSEL_OFFSET  (26)
#define MSDC_PATCH_BIT_CMDFSEL_OFFSET  (27)
#define MSDC_PATCH_BIT_INTDLSEL_OFFSET (28)
#define MSDC_PATCH_BIT_SPCPUSH_OFFSET  (29)
#define MSDC_PATCH_BIT_DECRCTMO_OFFSET (30)

#define MSDC_PATCH_BIT_WFLSMODE_MASK (0x1  << MSDC_PATCH_BIT_WFLSMODE_OFFSET)     /* RW */
#define MSDC_PATCH_BIT_ODDSUPP_MASK  (0x1  << MSDC_PATCH_BIT_ODDSUPP_OFFSET)     /* RW */
#define MSDC_PATCH_BIT_CKGEN_CK_MASK (0x1  << MSDC_PATCH_BIT_CKGEN_CK_OFFSET)     /* E2: Fixed to 1 */
#define MSDC_PATCH_BIT_IODSSEL_MASK  (0x1  << MSDC_PATCH_BIT_IODSSEL_OFFSET)    /* RW */
#define MSDC_PATCH_BIT_IOINTSEL_MASK (0x1  << MSDC_PATCH_BIT_IOINTSEL_OFFSET)    /* RW */
#define MSDC_PATCH_BIT_BUSYDLY_MASK  (0xf  << MSDC_PATCH_BIT_BUSYDLY_OFFSET)    /* RW */
#define MSDC_PATCH_BIT_WDOD_MASK     (0xf  << MSDC_PATCH_BIT_WDOD_OFFSET)    /* RW */
#define MSDC_PATCH_BIT_IDRTSEL_MASK  (0x1  << MSDC_PATCH_BIT_IDRTSEL_OFFSET)    /* RW */
#define MSDC_PATCH_BIT_CMDFSEL_MASK  (0x1  << MSDC_PATCH_BIT_IDRTSEL_OFFSET)    /* RW */
#define MSDC_PATCH_BIT_INTDLSEL_MASK (0x1  << MSDC_PATCH_BIT_INTDLSEL_OFFSET)    /* RW */
#define MSDC_PATCH_BIT_SPCPUSH_MASK  (0x1  << MSDC_PATCH_BIT_SPCPUSH_OFFSET)    /* RW */
#define MSDC_PATCH_BIT_DECRCTMO_MASK (0x1  << MSDC_PATCH_BIT_DECRCTMO_OFFSET)    /* RW */

/*the bit value in MSDC Patch Bit Register 1*/
#define MSDC_PATCH_BIT1_WRDAT_CRCS_OFFSET (0)
#define MSDC_PATCH_BIT1_CMD_RSP_OFFSET    (3)

#define MSDC_PATCH_BIT1_WRDAT_CRCS_MASK  (0x7 << MSDC_PATCH_BIT1_WRDAT_CRCS_OFFSET)
#define MSDC_PATCH_BIT1_CMD_RSP_MASK     (0x7 << MSDC_PATCH_BIT1_CMD_RSP_OFFSET)

/*the bit value in MSDC Pad Tuning Register*/
#define MSDC_PAD_TUNE_DATWRDLY_OFFSET (0)
#define MSDC_PAD_TUNE_DATRRDLY_OFFSET (8)
#define MSDC_PAD_TUNE_CMDRDLY_OFFSET  (16)
#define MSDC_PAD_TUNE_CMDRRDLY_OFFSET (22)
#define MSDC_PAD_TUNE_CLKTXDLY_OFFSET (27)

#define MSDC_PAD_TUNE_DATWRDLY_MASK  (0x1F << MSDC_PAD_TUNE_DATWRDLY_OFFSET)     /* RW */
#define MSDC_PAD_TUNE_DATRRDLY_MASK  (0x1F << MSDC_PAD_TUNE_DATRRDLY_OFFSET)     /* RW */
#define MSDC_PAD_TUNE_CMDRDLY_MASK   (0x1F << MSDC_PAD_TUNE_CMDRDLY_OFFSET)    /* RW */
#define MSDC_PAD_TUNE_CMDRRDLY_MASK  (0x1FUL << MSDC_PAD_TUNE_CMDRRDLY_OFFSET)  /* RW */
#define MSDC_PAD_TUNE_CLKTXDLY_MASK  (0x1FUL << MSDC_PAD_TUNE_CLKTXDLY_OFFSET)  /* RW */

/*the bit value in MSDC Data Delay Line Register 0*/
#define MSDC_DAT_RDDLY0_D0_OFFSET (0)
#define MSDC_DAT_RDDLY0_D1_OFFSET (8)
#define MSDC_DAT_RDDLY0_D2_OFFSET (16)
#define MSDC_DAT_RDDLY0_D3_OFFSET (24)

#define MSDC_DAT_RDDLY0_D0_MASK      (0x1F << MSDC_DAT_RDDLY0_D0_OFFSET)     /* RW */
#define MSDC_DAT_RDDLY0_D1_MASK      (0x1F << MSDC_DAT_RDDLY0_D1_OFFSET)     /* RW */
#define MSDC_DAT_RDDLY0_D2_MASK      (0x1F << MSDC_DAT_RDDLY0_D2_OFFSET)    /* RW */
#define MSDC_DAT_RDDLY0_D3_MASK      (0x1F << MSDC_DAT_RDDLY0_D3_OFFSET)    /* RW */

/*the bit value in MSDC Data Delay Line Register 1*/
#define MSDC_DAT_RDDLY1_D4_OFFSET (0)
#define MSDC_DAT_RDDLY1_D5_OFFSET (8)
#define MSDC_DAT_RDDLY1_D6_OFFSET (16)
#define MSDC_DAT_RDDLY1_D7_OFFSET (24)

#define MSDC_DAT_RDDLY1_D4_MASK      (0x1F << MSDC_DAT_RDDLY1_D4_OFFSET)     /* RW */
#define MSDC_DAT_RDDLY1_D5_MASK      (0x1F << MSDC_DAT_RDDLY1_D5_OFFSET)     /* RW */
#define MSDC_DAT_RDDLY1_D6_MASK      (0x1F << MSDC_DAT_RDDLY1_D6_OFFSET)    /* RW */
#define MSDC_DAT_RDDLY1_D7_MASK      (0x1F << MSDC_DAT_RDDLY1_D7_OFFSET)    /* RW */

/*the bit value in MSDC patch bit Register 0*/
#define MSDC_CKGEN_MSDC_DLY_SEL_OFFSET   (10)
#define MSDC_INT_DAT_LATCH_CK_SEL_OFFSET (7)
#define MSDC_CKGEN_MSDC_CK_SEL_OFFSET    (6)
#define MSDC_CKGEN_RX_SDCLKO_SEL_OFFSET  (0)

#define MSDC_CKGEN_MSDC_DLY_SEL_MASK   (0x1F << MSDC_CKGEN_MSDC_DLY_SEL_OFFSET)
#define MSDC_INT_DAT_LATCH_CK_SEL_MASK (0x7 << MSDC_INT_DAT_LATCH_CK_SEL_OFFSET)
#define MSDC_CKGEN_MSDC_CK_SEL_MASK    (0x1 << MSDC_CKGEN_MSDC_CK_SEL_OFFSET)
#define MSDC_CKGEN_RX_SDCLKO_SEL_MASK  (0x1 << MSDC_CKGEN_RX_SDCLKO_SEL_OFFSET)

/*the bit value in MSDC patch bit Register 1*/
#define MSDC_SINGLEBURST_SEL_OFFSET (16)

#define MSDC_SINGLEBURST_SEL_MASK   (0x1 << MSDC_SINGLEBURST_SEL_OFFSET)


/* *************************  MSDC  definition end line**********************************
*/

/*************************** IRRX register definition start line  *******************************
 */
 typedef struct {
    __IO uint32_t PDREG_IRH;                       /*IR COUNT HIGH REGISTER*/
    __IO uint32_t PDREG_IRM;                       /*IR COUNT MIDIUM REGISTER*/
    __IO uint32_t PDREG_IRL;                       /*IR COUNT LOW REGISTER*/
    __IO uint32_t PDREG_IRCFGH;                    /*IR CONFIGURATION HIGH REGISTER*/
    __IO uint32_t PDREG_IRCFGL;                    /*IR CONFIGURATION LOW REGISTER*/
    __IO uint32_t PDREG_IRTHD;                     /*IR THRESHOLD REGISTER*/
    __IO uint32_t PDREG_IRRCM_THD;                 /*RCMM THRESHOLD REGISTER*/
    __IO uint32_t PDREG_IRRCM_THD_0;               /*RCMM THRESHOLD REGISTER*/
    __IO uint32_t PDREG_IRCLR;                     /*IR CLEAR REGISTER*/
    __IO uint32_t PDREG_IREXP_EN;                  /*IR EXPECTATION REGISTER*/
    __IO uint32_t PDREG_EXP_BCNT;                  /*BITCNT EXPECTED VALUE REGISTER*/
    __IO uint32_t PDREG_ENEXP_IRM;                 /*IRM EXPECT VALUE BIT MASK REGISTER*/
    __IO uint32_t PDREG_ENEXP_IRL;                 /*IRM EXPECT VALUE BIT MASK REGISTER*/
    __IO uint32_t PDREG_EXP_IRL0;                  /*IRL EXPECT VALUE 0 REGISTER*/
    __IO uint32_t PDREG_EXP_IRL1;                  /*IRL EXPECT VALUE 1 REGISTER*/
    __IO uint32_t PDREG_EXP_IRL2;                  /*IRL EXPECT VALUE 2 REGISTER*/
    __IO uint32_t PDREG_EXP_IRL3;                  /*IRL EXPECT VALUE 3 REGISTER*/
    __IO uint32_t PDREG_EXP_IRL4;                  /*IRL EXPECT VALUE 4 REGISTER*/
    __IO uint32_t PDREG_EXP_IRL5;                  /*IRL EXPECT VALUE 5 REGISTER*/
    __IO uint32_t PDREG_EXP_IRL6;                  /*IRL EXPECT VALUE 6 REGISTER*/
    __IO uint32_t PDREG_EXP_IRL7;                  /*IRL EXPECT VALUE 7 REGISTER*/
    __IO uint32_t PDREG_EXP_IRL8;                  /*IRL EXPECT VALUE 8 REGISTER*/
    __IO uint32_t PDREG_EXP_IRL9;                  /*IRL EXPECT VALUE 9 REGISTER*/
    __IO uint32_t PDREG_EXP_IRM0;                  /*IRM EXPECT VALUE 0 REGISTE*/
    __IO uint32_t PDREG_EXP_IRM1;                  /*IRM EXPECT VALUE 1 REGISTE*/
    __IO uint32_t PDREG_EXP_IRM2;                  /*IRM EXPECT VALUE 2 REGISTE*/
    __IO uint32_t PDREG_EXP_IRM3;                  /*IRM EXPECT VALUE 3 REGISTE*/
    __IO uint32_t PDREG_EXP_IRM4;                  /*IRM EXPECT VALUE 4 REGISTE*/
    __IO uint32_t PDREG_EXP_IRM5;                  /*IRM EXPECT VALUE 5 REGISTE*/
    __IO uint32_t PDREG_EXP_IRM6;                  /*IRM EXPECT VALUE 6 REGISTE*/
    __IO uint32_t PDREG_EXP_IRM7;                  /*IRM EXPECT VALUE 7 REGISTE*/
    __IO uint32_t PDREG_EXP_IRM8;                  /*IRM EXPECT VALUE 8 REGISTE*/
    __IO uint32_t PDREG_EXP_IRM9;                  /*IRM EXPECT VALUE 9 REGISTE*/
    __IO uint32_t PDREG_IRINT_EN;                  /*IR INTERRUPT ENABLE REGISTER*/
    __IO uint32_t PDREG_IR_INTCLR;                 /*PDWNC INTERRUPT CLEAR REGISTER*/
    __IO uint32_t PDREG_WAKEEN;                    /*WAKE UP ENABLE REGISTER*/

    __IO uint32_t PDREG_WAKECLR;                   /*WAKE UP CLEAR REGISTER*/
    __IO uint32_t RESERVER;
    __IO uint32_t PDREG_SOFTEN;                    /*SOFTWARE MODE ENABLE REGISTER*/
    __IO uint32_t PDREG_SELECT;                    /*INTERRUPT AND WAKEUP SELECT REGISTE*/

    __IO uint32_t PDREG_CHK_DATA0;                 /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA1;                 /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA2;                 /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA3;                 /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA4;                 /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA5;                 /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA6;                 /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA7;                 /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA8;                 /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA9;                 /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA10;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA11;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA12;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA13;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA14;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA15;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA16;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA17;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA18;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA19;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA20;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA21;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA22;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA23;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA24;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA25;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA26;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA27;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA28;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA29;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA30;                /*IR Pulse Width Length Value*/
    __IO uint32_t PDREG_CHK_DATA31;                /*IR Pulse Width Length Value*/
}IRRX_REGISTER_T;

/*the bit value in PDREG_IRH register*/
#define PDREG_IRH_BIT_CNT_OFFSET   (0)
#define PDREG_IRH_RES_ONE_OFFSET   (8)
#define PDREG_IRH_RES_TWO_OFFSET   (16)
#define PDREG_IRH_RES_THREE_OFFSET (24)

#define PDREG_IRH_BIT_CNT_MASK (0x3f << PDREG_IRH_BIT_CNT_OFFSET)
#define PDREG_IRH_RES_ONE_MASK (0xff << PDREG_IRH_RES_ONE_OFFSET)
#define PDREG_IRH_RES_TWO_MASK (0xff << PDREG_IRH_RES_TWO_OFFSET)
#define PDREG_IRH_RES_THREE_MASK (0xff << PDREG_IRH_RES_THREE_OFFSET)

/*the bit value in PDREG_IRM register*/
#define PDREG_IRM_BIT_REG0_OFFSET (0)
#define PDREG_IRM_BIT_REG1_OFFSET (8)
#define PDREG_IRM_BIT_REG2_OFFSET (16)
#define PDREG_IRM_BIT_REG3_OFFSET (24)

#define PDREG_IRM_BIT_REG0_MASK (0xff << PDREG_IRM_BIT_REG0_OFFSET)
#define PDREG_IRM_BIT_REG1_MASK (0xff << PDREG_IRM_BIT_REG1_OFFSET)
#define PDREG_IRM_BIT_REG2_MASK (0xff << PDREG_IRM_BIT_REG2_OFFSET)
#define PDREG_IRM_BIT_REG3_MASK (0xff << PDREG_IRM_BIT_REG3_OFFSET)

/*the bit value in PDREG_IRL register*/
#define PDREG_IRL_BIT_REG4_OFFSET (0)
#define PDREG_IRL_BIT_REG5_OFFSET (8)
#define PDREG_IRL_BIT_REG6_OFFSET (16)

#define PDREG_IRM_BIT_REG4_MASK (0xff << PDREG_IRL_BIT_REG4_OFFSET)
#define PDREG_IRM_BIT_REG5_MASK (0xff << PDREG_IRL_BIT_REG5_OFFSET)
#define PDREG_IRM_BIT_REG6_MASK (0xff << PDREG_IRL_BIT_REG6_OFFSET)

/*the bit value in PDREG_IRCFGH register*/
#define PDREG_IRCFGH_IREN_OFFSET (0)
#define PDREG_IRCFGH_IRINV_OFFSET (1)
#define PDREG_IRCFGH_RC5_OFFSET (2)
#define PDREG_IRCFGH_RC5_1ST_OFFSET (3)
#define PDREG_IRCFGH_ORDINV_OFFSET (4)
#define PDREG_IRCFGH_IGN_1ST_OFFSET (5)
#define PDREG_IRCFGH_DISL_OFFSET (6)
#define PDREG_IRCFGH_DISH_OFFSET (7)
#define PDREG_IRCFGH_CHK_EN_OFFSET (13)
#define PDREG_IRCFGH_IGB0_OFFSET (14)
#define PDREG_IRCFGH_DISPD_OFFSET (15)
#define PDREG_IRCFGH_OK_PERIOD_OFFSET (16)

#define PDREG_IRCFGH_IREN_MASK (0x01 << PDREG_IRCFGH_IREN_OFFSET)
#define PDREG_IRCFGH_IRINV_MASK (0x01 << PDREG_IRCFGH_IRINV_OFFSET)
#define PDREG_IRCFGH_RC5_MASK (0x01 << PDREG_IRCFGH_RC5_OFFSET)
#define PDREG_IRCFGH_RC5_1ST_MASK (0x01 << PDREG_IRCFGH_RC5_1ST_OFFSET)
#define PDREG_IRCFGH_ORDINV_MASK (0x01 << PDREG_IRCFGH_ORDINV_OFFSET)
#define PDREG_IRCFGH_IGN_1ST_MASK (0x01 << PDREG_IRCFGH_IGN_1ST_OFFSET)
#define PDREG_IRCFGH_DISL_MASK (0x01 << PDREG_IRCFGH_DISL_OFFSET)
#define PDREG_IRCFGH_DISH_MASK (0x01 << PDREG_IRCFGH_DISH_OFFSET)
#define PDREG_IRCFGH_CHK_EN_MASK (0x01 << PDREG_IRCFGH_CHK_EN_OFFSET)
#define PDREG_IRCFGH_IGB0_MASK (0x01 << PDREG_IRCFGH_IGB0_OFFSET)
#define PDREG_IRCFGH_DISPD_MASK (0x01 << PDREG_IRCFGH_DISPD_OFFSET)
#define PDREG_IRCFGH_OK_PERIOD_MASK (0x7f << PDREG_IRCFGH_OK_PERIOD_OFFSET)

/*the bit value in PDREG_IRCFGL register*/
#define PDREG_IRCFGL_SAPERIOD_OFFSET (0)
#define PDREG_IRCFGL_CHECKPERIOD_OFFSET (8)

#define PDREG_IRCFGL_SAPERIOD_MASK (0xff << PDREG_IRCFGL_SAPERIOD_OFFSET)
#define PDREG_IRCFGL_CHECKPERIOD_MASK (0x1fff << PDREG_IRCFGL_CHECKPERIOD_OFFSET)

/*the bit value in PDREG_IRTHD register*/
#define PDREG_IRTHD_IRTHD_OFFSET (0)
#define PDREG_IRTHD_ICLR_OFFSET (7)
#define PDREG_IRTHD_DE_SEL_OFFSET (8)

#define PDREG_IRTHD_IRTHD_MASK (0x7f << PDREG_IRTHD_IRTHD_OFFSET)
#define PDREG_IRTHD_ICLR_MASK (0x01 << PDREG_IRTHD_ICLR_OFFSET)
#define PDREG_IRTHD_DE_SEL_MASK (0x1f << PDREG_IRTHD_DE_SEL_OFFSET)

/*the bit value in PDREG_IRRCM_THD register*/
#define PDREG_IRRCM_THD_IRRCM_THD_00_OFFSET (0)
#define PDREG_IRRCM_THD_IRRCM_THD_01_OFFSET (7)
#define PDREG_IRRCM_THD_IRRCM_THD_10_OFFSET (14)
#define PDREG_IRRCM_THD_IRRCM_THD_11_OFFSET (21)
#define PDREG_IRRCM_THD_IRRCM_OFFSET (31)

#define PDREG_IRRCM_THD_IRRCM_THD_00_MASK (0x7f << PDREG_IRRCM_THD_IRRCM_THD_00_OFFSET)
#define PDREG_IRRCM_THD_IRRCM_THD_01_MASK (0x7f << PDREG_IRRCM_THD_IRRCM_THD_01_OFFSET)
#define PDREG_IRRCM_THD_IRRCM_THD_10_MASK (0x7f << PDREG_IRRCM_THD_IRRCM_THD_10_OFFSET)
#define PDREG_IRRCM_THD_IRRCM_THD_11_MASK (0x7f << PDREG_IRRCM_THD_IRRCM_THD_11_OFFSET)
#define PDREG_IRRCM_THD_IRRCM_MASK (0x01 << PDREG_IRRCM_THD_IRRCM_OFFSET)

/*the bit value in PDREG_IRRCM_THD_0 register*/
#define PDREG_IRRCM_THD_0_IRRCM_THD_20_OFFSET (0)
#define PDREG_IRRCM_THD_0_IRRCM_THD_21_OFFSET (7)

#define PDREG_IRRCM_THD_0_IRRCM_THD_20_MASK (0x7f << PDREG_IRRCM_THD_0_IRRCM_THD_20_OFFSET)
#define PDREG_IRRCM_THD_0_IRRCM_THD_21_MASK (0x7f << PDREG_IRRCM_THD_0_IRRCM_THD_21_OFFSET)

/*the bit value in PDREG_IRCLR register*/
#define PDREG_IRCLR_IRCLR_OFFSET (0)

#define PDREG_IRCLR_IRCLR_MASK (0x01 << PDREG_IRCLR_IRCLR_OFFSET)

/*the bit value in PDREG_IREXP_EN register*/
#define PDREG_IREXP_EN_IREXPEN_OFFSET (0)
#define PDREG_IREXP_EN_BCEPEN_OFFSET (8)
#define PDREG_IREXP_EN_IRPDWNEN_OFFSET (9)
#define PDREG_IREXP_EN_PD_IREXPEN_OFFSET (10)

#define PDREG_IREXP_EN_IREXPEN_MASK (0xff << PDREG_IREXP_EN_IREXPEN_OFFSET)
#define PDREG_IREXP_EN_BCEPEN_MASK (0x01 << PDREG_IREXP_EN_BCEPEN_OFFSET)
#define PDREG_IREXP_EN_IRPDWNEN_MASK (0x01 << PDREG_IREXP_EN_IRPDWNEN_OFFSET)
#define PDREG_IREXP_EN_PD_IREXPEN_MASK (0x03 << PDREG_IREXP_EN_PD_IREXPEN_OFFSET)

/*the bit value in PDREG_XP_BCNT register*/
#define PDREG_XP_BCNT_EXP_BITCNT_OFFSET (0)
#define PDREG_XP_BCNT_CHK_CNT_OFFSET (6)

#define PDREG_XP_BCNT_EXP_BITCNT_MASK (0x3d << PDREG_XP_BCNT_EXP_BITCNT_OFFSET)
#define PDREG_XP_BCNT_CHK_CNT_MASK (0x3d << PDREG_XP_BCNT_CHK_CNT_OFFSET)

/*the bit value in PDREG_PDWNCNT register*/
#define PDREG_PDWNCNT_PDWNCNT_OFFSET (0)

#define PDREG_PDWNCNT_PDWNCNT_MASK (0xff << PDREG_PDWNCNT_PDWNCNT_OFFSET)

/*the bit value in PDREG_IRINT_EN register*/
#define PDREG_IRINT_EN_IRINT_EN_OFFSET (0)

#define PDREG_IRINT_EN_IRINT_EN_MASK (0x01 << PDREG_IRINT_EN_IRINT_EN_OFFSET)

/*the bit value in PDREG_IR_INTCLR register*/
#define PDREG_IR_INTCLR_IR_INTCLR_OFFSET (0)

#define PDREG_IR_INTCLR_IR_INTCLR_MASK (0x01 << PDREG_IR_INTCLR_IR_INTCLR_OFFSET)

/*the bit value in PDREG_WDTSET register*/
#define PDREG_WDTSET_WDT_EN_OFFSET (0)
#define PDREG_WDTSET_DBG_STOP_OFFSET (1)
#define PDREG_WDTSET_WDTMODE_OFFSET (4)

#define PDREG_WDTSET_WDT_EN_MASK (0x01 << PDREG_WDTSET_WDT_EN_OFFSET)
#define PDREG_WDTSET_DBG_STOP_MASK (0x01 << PDREG_WDTSET_DBG_STOP_OFFSET)
#define PDREG_WDTSET_WDTMODE_MASK (0x07 << PDREG_WDTSET_WDTMODE_OFFSET)



/* *************************  IRRX register definition end line**********************************
*/

/************** Define a virtual address for external flash with SPIM start line  ***************
 */

#ifdef BSP_EXTERNAL_SERIAL_FLASH_ENABLED
#define SPI_SERIAL_FLASH_ADDRESS   (0x0C000000)
#endif

/************** Define a virtual address for external flash with SPIM end line  *****************
 */


/*************************** GPIO register definition start line  *******************************
 */

typedef struct {
    __IO uint32_t RW[3];                                    /*!< GPIO RW register */
    uint32_t  RESERVED1;                                    /*!< reserved */
    __IO uint32_t SET[3];                                   /*!< GPIO set register */
    uint32_t  RESERVED2;                                    /*!< reserved */
    __IO uint32_t CLR[3];                                   /*!< GPIO clear register */
    uint32_t  RESERVED3;                                    /*!< reserved */
} GPIO_REGISTER_T;

typedef struct {
    __IO uint32_t RW;                                       /*!< GPIO RW register */
    __IO uint32_t SET;                                      /*!< GPIO set register */
    __IO uint32_t CLR;                                      /*!< GPIO clear register */
    uint32_t  RESERVED1;                                    /*!< reserved */
} GPIO_CFG_REGISTER_T;

typedef struct {
    __IO uint32_t R[3];                                     /*!< GPIO input data register */
    uint32_t      RESERVED1[37];                            /*!< reserved */
} GPIO_DIN_REGISTER_T;



typedef struct {
    __IO uint32_t RW[9];                                    /*!< GPIO RW register */
    uint32_t      RESERVED1[55];                            /*!< reserved */
    __IO uint32_t SET[9];                                   /*!< GPIO set register */
    uint32_t      RESERVED2[55];                            /*!< reserved */
    __IO uint32_t CLR[9];                                   /*!< GPIO clear register */
} GPIO_MODE_REGISTER_T;


typedef struct {
    __IO uint32_t DUMMY;
    __IO uint32_t DUMMY_SET;
    __IO uint32_t DUMMY_CLR;
    uint32_t  GPIO_OFFSET;
} GPIO_DUMMY_REGISTER_T;


typedef struct {
    GPIO_REGISTER_T         GPIO_DIR;               /*!< GPIO direction register */
    GPIO_REGISTER_T         GPIO_DOUT;              /*!< GPIO output data register */
    GPIO_DIN_REGISTER_T     GPIO_DIN;               /*!< GPIO input data register */
    GPIO_MODE_REGISTER_T    GPIO_MODE;              /*!< GPIO mode register */
} GPIO_BASE_REGISTER_T;

typedef struct {
    GPIO_CFG_REGISTER_T     GPIO_BDSEL;                 /*!< GPIO BDSEL register */
    GPIO_CFG_REGISTER_T     GPIO_BSEL;                  /*!< GPIO BSEL register */
    GPIO_CFG_REGISTER_T     GPIO_DRV[2];                /*!< GPIO DRV register */
    GPIO_CFG_REGISTER_T     GPIO_G;                     /*!< GPIO analog control register */
    GPIO_CFG_REGISTER_T     GPIO_IES;                   /*!< GPIO input buffer control register */
    GPIO_CFG_REGISTER_T     GPIO_PD;                    /*!< GPIO pull down register */
    GPIO_CFG_REGISTER_T     GPIO_PU;                    /*!< GPIO pull up register */
    GPIO_CFG_REGISTER_T     GPIO_RDSEL;                 /*!< GPIO RDSEL register */
    GPIO_CFG_REGISTER_T     GPIO_SMT;                   /*!< GPIO schimtt trigger register */
    GPIO_CFG_REGISTER_T     GPIO_SR;                    /*!< GPIO slew rate register */
    GPIO_CFG_REGISTER_T     GPIO_TDSEL[2];              /*!< GPIO TDSEL register */
    GPIO_CFG_REGISTER_T     GPIO_DUMMY;                 /*!< GPIO dummy register */
} GPIO_CFG0_REGISTER_T;

typedef struct {
    GPIO_CFG_REGISTER_T     GPIO_BDSEL;                 /*!< GPIO BDSEL register */
    GPIO_CFG_REGISTER_T     GPIO_BSEL;                  /*!< GPIO BSEL register */
    GPIO_CFG_REGISTER_T     GPIO_DRV[6];                /*!< GPIO DRV register */
    GPIO_CFG_REGISTER_T     GPIO_EH;                    /*!< GPIO EH register */
    GPIO_CFG_REGISTER_T     GPIO_G[2];                  /*!< GPIO analog control register */
    GPIO_CFG_REGISTER_T     GPIO_IES[2];                /*!< GPIO input buffer control register */
    GPIO_CFG_REGISTER_T     GPIO_PD[2];                 /*!< GPIO pull down register */
    GPIO_CFG_REGISTER_T     GPIO_PUPD;                  /*!< GPIO pull up & pull down control register */
    GPIO_CFG_REGISTER_T     GPIO_PU[2];                 /*!< GPIO pull up register */
    GPIO_CFG_REGISTER_T     GPIO_R0;                    /*!< GPIO resistance 0 register */
    GPIO_CFG_REGISTER_T     GPIO_R1;                    /*!< GPIO resistance 1 register */
    GPIO_CFG_REGISTER_T     GPIO_RDSEL[5];              /*!< GPIO RDSEL register */
    GPIO_CFG_REGISTER_T     GPIO_RSEL;                  /*!< GPIO RSEL register */
    GPIO_CFG_REGISTER_T     GPIO_SMT[2];                /*!< GPIO schimtt trigger register */
    GPIO_CFG_REGISTER_T     GPIO_SR[2];                 /*!< GPIO slew rate register */
    GPIO_CFG_REGISTER_T     GPIO_TDSEL_CFG[7];          /*!< GPIO TDSEL register */
    GPIO_CFG_REGISTER_T     GPIO_TDSEL_CFG_S;           /*!< GPIO TDSEL register */
    GPIO_CFG_REGISTER_T     GPIO_DUMMY;                 /*!< GPIO dummy register */
} GPIO_CFG1_REGISTER_T;


#define GPIO_BASE_REGISTER  ((GPIO_BASE_REGISTER_T*)GPIO_BASE)
#define GPIO_CFG0_REGISTER  ((GPIO_CFG0_REGISTER_T*)IO_CFG_0_BASE)
#define GPIO_CFG1_REGISTER  ((GPIO_CFG1_REGISTER_T*)IO_CFG_1_BASE)

/*************************** GPIO register definition end line  *******************************
 */


/*************************** captouch register definition stat line  *******************************
 */

typedef union {
    struct {
        uint32_t  TOUCH_CH_EN             :4;  /*Touch channel sensing.*/
        uint32_t  TOUCH_CH_RST_EN         :4;  /*TOUCH channel reset.*/
        uint32_t  TOUCH_CLK_EN            :1;  /*TOUCH clock.*/
        uint32_t  TOUCH_INT_EN            :1;  /*TOUCH interrupt.*/
        uint32_t  TOUCH_WAKE_EN           :1;  /*TOUCH wakeup from rtc mode.*/
        uint32_t  RESERVE0                :5;
        uint32_t  TOUCH_CAL_EN            :4;  /*TOUCH auto calibration.*/
        uint32_t  RESERVE1                :4;
        uint32_t  TOUCH_CAL_AUTO_SUP      :4;  /*TOUCH suspend autok when press.*/
        uint32_t  TOUCH_CAL_AUTO_SUPN     :4;  /*TOUCH suspend autok when adc>nthr_h.*/
    } CELLS;
    uint32_t CON0;
} CAPTOUCH_CON0_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_CLK_SEL           :3;  /*Touch clock selection.*/
        uint32_t  RESERVE0                :1;
        uint32_t  TOUCH_CLK_SEL_LPM       :3;  /*Touch clock selection.*/
        uint32_t  RESERVE1                :1;
        uint32_t  TOUCH_CTRL_MAN          :4;  /*TOUCH internal capacitance value.*/
        uint32_t  RESERVE2                :4;
        uint32_t  TOUCH_EXT_CLK_EN        :1;  /*TOUCH external clock.*/
        uint32_t  TOUCH_B2B_CH            :1;  /*TOUCH time slot of disabled sensing.*/
        uint32_t  TOUCH_AUTO_DISABLE_CH   :1;  /*TOUCH auto disable ch1~3 when rtc mode.*/
        uint32_t  TOUCH_CLK_INV_SEL       :1;  /*TOUCH invert touch clock.*/
        uint32_t  TOUCH_CH_GATING         :1;  /*TOUCH channel power saving.*/
        uint32_t  RESERVE3                :3;
        uint32_t  TOUCH_SW_DBG_SEL        :2;  /*TOUCH channel for debug.*/
        uint32_t  RESERVE4                :2;
        uint32_t  TOUCH_HW_DBG_SEL        :3;  /*TOUCH hw debug.*/
    } CELLS;
    uint32_t CON1;
} CAPTOUCH_CON1_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_AVG_S0            :4;  /*Touch sample weighting ch0.*/
        uint32_t  TOUCH_AVG_S1            :4;  /*Touch sample weighting ch1.*/
        uint32_t  TOUCH_AVG_S2            :4;  /*Touch sample weighting ch2.*/
        uint32_t  TOUCH_AVG_S3            :4;  /*Touch sample weighting ch3.*/
        uint32_t  TOUCH_AVG_S0_LPM        :4;  /*Touch sample weighting ch0 at lpm.*/
        uint32_t  TOUCH_AVG_R0_LPM        :4;  /*Touch sample avergaed ch0 at lpm.*/
    } CELLS;
    uint32_t CON2;
} CAPTOUCH_CON2_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_MAVG_R0           :5;  /*Touch sample avergaed ch0.*/
        uint32_t  RESERVE0                :3;
        uint32_t  TOUCH_MAVG_R1           :5;  /*Touch sample avergaed ch1.*/
        uint32_t  RESERVE1                :3;
        uint32_t  TOUCH_MAVG_R2           :5;  /*Touch sample avergaed ch2.*/
        uint32_t  RESERVE2                :3;
        uint32_t  TOUCH_MAVG_R3           :5;  /*Touch sample avergaed ch3.*/
    } CELLS;
    uint32_t CON3;
} CAPTOUCH_CON3_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_THRL              :9;  /*Touch low level threshold.*/
        uint32_t  RESERVE0                :7;
        uint32_t  TOUCH_THRH              :9;  /*Touch high level threshold.*/
    } CELLS;
    uint32_t THR;
} CAPTOUCH_THR_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_CAL_OUT_MAN0      :8;  /*Touch fine tune value ch0.*/
        uint32_t  TOUCH_CAL_OUT_MAN1      :8;  /*Touch fine tune value ch1.*/
        uint32_t  TOUCH_CAL_OUT_MAN2      :8;  /*Touch fine tune value ch2.*/
        uint32_t  TOUCH_CAL_OUT_MAN3      :8;  /*Touch fine tune value ch3.*/
    } CELLS;
    uint32_t FINECAP;
} CAPTOUCH_FINECAP_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_CH_WAKEUP         :8;  /*Touch wakeup event.*/
        uint32_t  TOUCH_CH_INT            :8;  /*Touch intr event.*/
    } CELLS;
    uint32_t CHMASK;
} CAPTOUCH_CHMASK_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_BASELINE_LOW      :9;  /*Touch baseline range lower bound.*/
        uint32_t  RESERVE0                :7;
        uint32_t  TOUCH_BASELINE_HIGH     :9;  /*Touch baseline range upper bound.*/
        uint32_t  RESERVE1                :3;
        uint32_t  TOUCH_THRL_EN           :1;  /*Touch dynamic threshold for thrH.*/
        uint32_t  TOUCH_THRH_EN           :1;  /*Touch dynamic threshold for thrL.*/
        uint32_t  TOUCH_NTHRL_EN          :1;  /*Touch dynamic threshold for nthrH.*/
        uint32_t  TOUCH_NTHRH_EN          :1;  /*Touch dynamic threshold for nthrL.*/
    } CELLS;
    uint32_t BASERANGE;
} CAPTOUCH_BASELINE_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_LPWU_SWRST        :1;  /*Touch reset count.*/
        uint32_t  RESERVE0                :7;
        uint32_t  TOUCH_LPWU_CHKMODE      :1;  /*Touch lpwu counter in rtc mode.*/
        uint32_t  TOUCH_LPWU_INT_EN       :1;  /*Touch lpwu intr enable.*/
        uint32_t  TOUCH_LPWU_WAKE_EN      :1;  /*Touch lpwu wakeup rtc.*/
        uint32_t  RESERVE1                :5;
        uint32_t  TOUCH_LPWU_MASK         :4;  /*Touch lpwu mask.*/
        uint32_t  TOUCH_LPSD_MASK         :4;  /*Touch lpsd mask.*/
    } CELLS;
    uint32_t LPWUCON;
} CAPTOUCH_LPWUCON_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_LPWU_TARCNT       :21;  /*Touch lpsd time.*/
    } CELLS;
    uint32_t LPWUTAR;
} CAPTOUCH_LPWUTAR_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_LPWU_INTFLAG      :1;  /*Touch lpsd flag.*/
    } CELLS;
    uint32_t LPWUINT;
} CAPTOUCH_LPWUINT_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_LPWU_INTFLAG_CLR  :1;  /*Touch clear lpsd flag.*/
    } CELLS;
    uint32_t LPWUIN_CLR;
} CAPTOUCH_LPWUINT_CLR_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_LPM_EN            :1;  /*Touch lpm mode enable.*/
        uint32_t  RESERVE0                :7;
        uint32_t  TOUCH_LPM_AUTO_EN       :1;  /*Touch auto switch to lpm.*/
        uint32_t  RESERVE1                :7;
        uint32_t  TOUCH_LPM_SWRST         :1;  /*Touch reset lpm counter.*/
        uint32_t  RESERVE2                :7;
        uint32_t  TOUCH_LPM_CHOFF_EN      :1;
    } CELLS;
    uint32_t LPMCON;
} CAPTOUCH_LPMCON_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_LPM_N2LDLY        :21;  /*Touch normal to lowpower mode delay.*/
    } CELLS;
    uint32_t LPM_N2LDLY;
} CAPTOUCH_LPMN2L_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_LPM_L2NDLY        :21;  /*Touch lowpower to normal mode delay.*/
    } CELLS;
    uint32_t LPM_L2NDLY;
} CAPTOUCH_LPML2N_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_TRIM_ADC          :9;  /*Touch adc value.*/
        uint32_t  RESERVE0                :7;
        uint32_t  TOUCH_TRIM_ADC_GAIN     :9;  /*Touch adc value.*/
        uint32_t  RESERVE1                :3;
        uint32_t  TOUCH_TRIM_ADC_EN       :1;  /*Touch trim adc value enable.*/
    } CELLS;
    uint32_t TRIMCFG;
} CAPTOUCH_TRIMCFG_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_CRS_CH0           :3;  /*Touch coarse value ch0.*/
        uint32_t  TOUCH_CRS_CH1           :3;  /*Touch coarse value ch1.*/
        uint32_t  TOUCH_CRS_CH2           :3;  /*Touch coarse value ch2.*/
        uint32_t  TOUCH_CRS_CH3           :3;  /*Touch coarse value ch3.*/
    } CELLS;
    uint32_t CRSCFG;
} CAPTOUCH_CRSCFG_UNION;

typedef union {
    struct {
        uint32_t  OPLP_EN_TOUCH           :2;  /*Touch lowpower op.*/
        uint32_t  OPLP_EN_TOUCH_LPM       :2;  /*Touch lowpower op.*/
        uint32_t  LDOLP_TOUCH             :1;  /*Touch lowpower ldo.*/
        uint32_t  LDOLP_TOUCH_LPM         :1;  /*Touch lowpower ldo.*/
        uint32_t  TOUCH_CRS_TEST          :1;
        uint32_t  RESERVE0                :9;
        uint32_t  DA_TOUCH_RSTN           :1;  /*Touch reset analog circuit.*/
    } CELLS;
    uint32_t ANACFG0;
} CAPTOUCH_ANACFG0_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_ASAT_CTRL         :8;  /*Touch analog configure1.*/
        uint32_t  TOUCH_ADC_EN            :1;  /*Touch analog configure1.*/
        uint32_t  TOUCH_CAL_EN            :1;  /*Touch analog configure1.*/
        uint32_t  TOUCH_DLY_EN            :1;  /*Touch analog configure1.*/
        uint32_t  TOUCH_CK_SEL            :1;  /*Touch analog configure1.*/
        uint32_t  TOUCH_OP_EN             :1;  /*Touch analog configure1.*/
        uint32_t  TOUCH_LDO_EN            :1;  /*Touch analog configure1.*/
        uint32_t  RESERVE0                :2;
        uint32_t  TOUCH_LDO_SEL           :3;  /*Touch analog configure1.*/
        uint32_t  TOUCH_DLY_SRC           :2;  /*Touch analog configure1.*/
        uint32_t  TOUCH_DLY_CAP           :3;  /*Touch analog configure1.*/
        uint32_t  TOUCH_REV_DMY           :8;  /*Touch analog configure1.*/
    } CELLS;
    uint32_t ANACFG1;
} CAPTOUCH_ANACFG1_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_RTC_LEAK_OUT_EN     :1;  /*Touch analog configure2.*/
        uint32_t  TOUCH_LEAK_OUT_EN         :1;  /*Touch analog configure2.*/
        uint32_t  TOUCH_RSV_ISEL            :2;  /*Touch analog configure2.*/
        uint32_t  TOUCH_ADC_ISEL            :2;  /*Touch analog configure2.*/
        uint32_t  TOUCH_OP_ISEL             :2;  /*Touch analog configure2.*/
        uint32_t  TOUCH_LDO045_ISEL         :2;  /*Touch analog configure2.*/
        uint32_t  TOUCH_LDO_09_ISEL         :2;  /*Touch analog configure2.*/
        uint32_t  TOUCH_BIAS_SEL            :2;
        uint32_t  TOUCH_BIAS_LP_EN          :1;  /*Touch analog configure2.*/
        uint32_t  TOUCH_BIAS_EN             :1;  /*Touch analog configure2.*/
        uint32_t  TOUCH_LDO_09_SEL          :3;  /*Touch analog configure2.*/
        uint32_t  TOUCH_LDO_09              :1;  /*Touch analog configure2.*/
        uint32_t  TOUCH_LDO_09_EN           :1;  /*Touch analog configure2.*/
    } CELLS;
    uint32_t ANACFG2;
} CAPTOUCH_ANACFG2_UNION;

typedef union {
    struct {
        uint32_t  TOUCH_LPWU_CNT          :21;  /*Touch lpwu counter trigger.*/
    } CELLS;
    uint32_t LPWUCNT;
} CAPTOUCH_LPWUCNT_UNION;

typedef union {
    struct {
        uint16_t  TOUCH_AVG_DBG           :9;  /*Touch averaged adc output value.*/
        uint16_t  RESERVE0                :7;
        uint16_t  TOUCH_TRIG              :4;  /*Touch channel trigger status.*/
        uint16_t  RESERVE1                :4;
        uint16_t  TOUCH_NTRIG             :4;  /*Touch trigger status based on nthr.*/
    } CELLS;
    uint32_t DBG0;
} CAPTOUCH_DBG0_UNION;

typedef union {
    struct {
        uint16_t  TOUCH_MAVG_DBG          :9;  /*Touch moving averaged adc output value.*/
        uint16_t  RESERVE0                :7;
        uint16_t  TOUCH_CAL_DBG           :8;  /*Touch channel fine cap calibration value.*/
        uint16_t  TOUCH_BASELINE_FLAG     :4;  /*Touch baseline flag.*/
    } CELLS;
    uint32_t DBG1;
} CAPTOUCH_DBG1_UNION;

typedef union {
    struct {
        uint16_t  TOUCH_VADC_DBG        :9;     /*Touch adc output value.*/
        uint16_t  RESERVE0              :7;
        uint16_t  TOUCH_VADC_12B_DBG    :12;    /*Touch adc 12b output value.*/
    } CELLS;
    uint32_t DBG2;
} CAPTOUCH_DBG2_UNION;

typedef union {
    struct {
        uint8_t  TOUCH_CAL_INIT0        :8;  /*Channel 0 capacitance fine tune value init value*/
        uint8_t  TOUCH_CAL_INIT1        :8;  /*Channel 1 capacitance fine tune value init value*/
        uint8_t  TOUCH_CAL_INIT2        :8;  /*Channel 2 capacitance fine tune value init value*/
        uint8_t  TOUCH_CAL_INIT3        :8;  /*Channel 3 capacitance fine tune value init value*/
    } CELLS;
    uint32_t FINECAP_INIT;
} CAPTOUCH_FINECAP_INIT_UNION;

typedef union {
    struct {
        uint8_t  TOUCH_CAL_INIT_EN0     :1;  /*Write touch_cal_init0 value*/
        uint8_t  RESERVE0               :7;
        uint8_t  TOUCH_CAL_INIT_EN1     :1;  /*Write touch_cal_init1 value*/
        uint8_t  RESERVE1               :7;
        uint8_t  TOUCH_CAL_INIT_EN2     :1;  /*Write touch_cal_init2 value*/
        uint8_t  RESERVE2               :7;
        uint8_t  TOUCH_CAL_INIT_EN3     :1;  /*Write touch_cal_init3 value*/
        uint8_t  RESERVE3               :7;
    } CELLS;
    uint32_t FINECAP_INIT_EN;
} CAPTOUCH_FINECAP_INIT_EN_UNION;

typedef union {
    struct {
        uint8_t  TOUCH_MAVG_E0          :4;  /*Number of samples to be averaged for ch0 (in-ear detection)*/
        uint8_t  TOUCH_MAVG_E1          :4;  /*Number of samples to be averaged for ch0 (in-ear detection)*/
        uint8_t  TOUCH_MAVG_E2          :4;  /*Number of samples to be averaged for ch0 (in-ear detection)*/
        uint8_t  TOUCH_MAVG_E3          :4;  /*Number of samples to be averaged for ch0 (in-ear detection)*/
        uint8_t  TOUCH_MAVG_E0_LPM      :4;  /*Number of samples to be averaged for ch0 @low power mode (hw switch mode)*/
    } CELLS;
    uint32_t CON4;
} CAPTOUCH_CON4_UNION;

typedef union {
    struct {
        uint8_t  TOUCH_EIN_INT_EN       :4;  /*Enable in-ear interrupt*/
        uint8_t  TOUCH_EOFF_INT_EN      :4;  /*Enable off-ear interrupt*/
        uint8_t  RESERVE0               :8;
        uint8_t  TOUCH_E_EN             :4;  /*Enable in-ear detection*/
    } CELLS;
    uint32_t ECON;
} CAPTOUCH_ECON_UNION;

typedef union {
    struct {
        uint8_t  TOUCH_EIN_INT      :4;  /*In-ear interrupt flag*/
        uint8_t  TOUCH_EOFF_INT     :4;  /*OFF-ear interrupt flag*/
    } CELLS;
    uint32_t EINT;
} CAPTOUCH_EINT_UNION;

typedef union {
    struct {
        uint8_t  TOUCH_EIN_INT_CLR      :4;  /*Clear in-ear interrupt flag*/
        uint8_t  TOUCH_EOFF_INT_CLR     :4;  /*Clear off-ear interrupt flag*/
    } CELLS;
    uint32_t EINT_CLR;
} CAPTOUCH_EINT_CLR_UNION;

typedef union {
    struct {
        uint8_t  TOUCH_DEB_EN       :4;  /*debounce enable*/
        uint8_t  RESERVE0           :4;
        uint8_t  TOUCH_DEB_1MS_EN   :4;  /*0: debounce time step = 1*32k_cycle, 1: debounce time step = 31*32k_cycle (1ms)*/
    } CELLS;
    uint32_t DEBCON;
} CAPTOUCH_DEBCON_UNION;

typedef union {
    struct {
        uint16_t  TOUCH_DEBTIME_R       :12;  /*debounce time for rising*/
        uint8_t   RESERVE0               :4;
        uint16_t  TOUCH_DEBTIME_F       :12;  /*debounce time for falling*/
    } CELLS;
    uint32_t DEBTIME;
} CAPTOUCH_DEBTIME_UNION;

typedef union {
    struct {
        uint16_t TOUCH_EMAVG_DBG        :9;
        uint8_t  RESERVE0               :7;
        uint8_t  TOUCH_TRIG_DEB         :4;     /*Touch channel debounce trigger status.*/
    } CELLS;
    uint32_t DBG3;
} CAPTOUCH_DBG3_UNION;


typedef union {
    struct {
        uint32_t  TOUCH_MAVG_CNT_DBG    :20;
    } CELLS;
    uint32_t DBG4;
} CAPTOUCH_DBG4_UNION;

typedef union {
    struct {
        uint32_t  RESERVE0;
    } CELLS;
    uint32_t RESERVED;
} CAPTOUCH_RESERVED_UNION;

typedef union {
    struct {
        uint8_t  APB2SER_CKSEL;        /*The apb2ser sdat delay*/
        uint8_t  APB2SER_DLY_EN;       /*The apb2ser clock selection*/
        uint16_t RESERVE;
    } CELLS;
    uint32_t CON;
} CAPTOUCH_APB2SER_CON0_UNION;

typedef union {
    struct {
        uint8_t  HIF_SOFT_RST;  /*!< Write 1 to issue SW reset to FIFO.*/
        uint8_t  FIFO_OVERFLOW; /*!< 0: Touch FIFO overflow did not happen.
                                     1: Touch FIFO overflow event occurred and FIFO data may be incorrect*/
        uint16_t TIMESTAMP_EN;  /*!< 0: enable timestamp function
                                     1: disable timestamp function*/
    } CELLS;
    uint32_t CON;
} CAPTOUCH_HIF_CON0_UNION;

typedef union {
    struct {
        uint16_t EVENT_TYPE;    /*!< Touch event*/
        uint8_t  EVENT_PENDING; /*!< 1: Indicate there is an touch event pending
                                     0: Indicate there is no touch event.*/
        uint8_t  EVENT_POP;     /*!< Write 1 to this bit will pop touch event*/
    } CELLS;
    uint32_t CON;
} CAPTOUCH_HIF_CON1_UNION;

typedef struct {
    __IO CAPTOUCH_CON0_UNION          TOUCH_CON0;            /*0x0*/
    __IO CAPTOUCH_CON1_UNION          TOUCH_CON1;            /*0x4*/
    __IO CAPTOUCH_CON2_UNION          TOUCH_CON2;            /*0x8*/
    __IO CAPTOUCH_CON3_UNION          TOUCH_CON3;            /*0xc*/
    __IO CAPTOUCH_THR_UNION           TOUCH_THR0;            /*0x10*/
    __IO CAPTOUCH_THR_UNION           TOUCH_THR1;            /*0x14*/
    __IO CAPTOUCH_THR_UNION           TOUCH_THR2;            /*0x18*/
    __IO CAPTOUCH_THR_UNION           TOUCH_THR3;            /*0x1c*/
    __IO CAPTOUCH_THR_UNION           TOUCH_NTHR0;           /*0x20*/
    __IO CAPTOUCH_THR_UNION           TOUCH_NTHR1;           /*0x24*/
    __IO CAPTOUCH_THR_UNION           TOUCH_NTHR2;           /*0x28*/
    __IO CAPTOUCH_THR_UNION           TOUCH_NTHR3;           /*0x2c*/
    __IO CAPTOUCH_FINECAP_UNION       TOUCH_FINECAP;         /*0x30*/
    __IO CAPTOUCH_CHMASK_UNION        TOUCH_CHMASK;          /*0x34*/
    __IO CAPTOUCH_BASELINE_UNION      TOUCH_BASELINE;        /*0x38*/
    __IO CAPTOUCH_LPWUCON_UNION       TOUCH_LPWUCON;         /*0x3c*/
    __IO CAPTOUCH_LPWUTAR_UNION       TOUCH_LPWUTAR;         /*0x40*/
    __I  CAPTOUCH_LPWUINT_UNION       TOUCH_LPWUFLAG;        /*0x44*/
    __IO CAPTOUCH_LPWUINT_CLR_UNION   TOUCH_LPWUCLR;         /*0x48*/
    __IO CAPTOUCH_LPMCON_UNION        TOUCH_LPMCFG;          /*0x4c*/
    __IO CAPTOUCH_LPMN2L_UNION        TOUCH_LPMN2L;          /*0x50*/
    __IO CAPTOUCH_LPML2N_UNION        TOUCH_LPML2N;          /*0x54*/
    __IO CAPTOUCH_TRIMCFG_UNION       TOUCH_TRIMCFG;         /*0x58*/
    __IO CAPTOUCH_CRSCFG_UNION        TOUCH_CRSCFG;          /*0x5c*/
    __IO CAPTOUCH_ANACFG0_UNION       TOUCH_ANACFG0;         /*0x60*/
    __IO CAPTOUCH_ANACFG1_UNION       TOUCH_ANACFG1;         /*0x64*/
    __IO CAPTOUCH_ANACFG2_UNION       TOUCH_ANACFG2;         /*0x68*/
    __I  CAPTOUCH_LPWUCNT_UNION       TOUCH_LPWUCNT;         /*0x6c*/
    __I  CAPTOUCH_DBG0_UNION          TOUCH_DBG0;            /*0x70*/
    __I  CAPTOUCH_DBG1_UNION          TOUCH_DBG1;            /*0x74*/
    __I  CAPTOUCH_DBG2_UNION          TOUCH_DBG2;            /*0x78*/
    __IO CAPTOUCH_FINECAP_INIT_UNION  TOUCH_FINECAP_INIT;    /*0x7C*/
    __IO CAPTOUCH_FINECAP_INIT_EN_UNION     TOUCH_FINECAP_INIT_EN;  /*0x80*/
    __IO CAPTOUCH_CON4_UNION          TOUCH_CON4;            /*0x84*/
    __IO CAPTOUCH_ECON_UNION          TOUCH_E_CON;           /*0x88*/
    __I  CAPTOUCH_EINT_UNION          TOUCH_E_INTFLAG;       /*0x8C*/
    __IO CAPTOUCH_EINT_CLR_UNION      TOUCH_E_INTFLAG_CLR;   /*0x90*/
    __IO CAPTOUCH_DEBCON_UNION        TOUCH_DEBCON;          /*0x94*/
    __IO CAPTOUCH_DEBTIME_UNION       TOUCH_DEBTIME;         /*0x98*/
    __I  CAPTOUCH_DBG3_UNION          TOUCH_DBG3;            /*0x9C*/
    __I  CAPTOUCH_DBG4_UNION          TOUCH_DBG4;            /*0xA0*/
    __IO CAPTOUCH_RESERVED_UNION      TOUCH_RSVD;            /*0xA4*/
} CAPTOUCH_REGISTER_T;

typedef struct {
    __IO CAPTOUCH_APB2SER_CON0_UNION  TOUCH_APB2SER;     /*!< APB2SER control register */
    __IO CAPTOUCH_HIF_CON0_UNION      TOUCH_HIF_CON0;    /*!< Host interface control register 0 */
    __IO CAPTOUCH_HIF_CON1_UNION      TOUCH_HIF_CON1;    /*!< Host interface control register 1 */
    __I  uint32_t                     TOUCH_HIF_CON2;    /*!< Host interface control register 2 */
    __IO uint32_t                     TOUCH_HIF_RES;     /*!< Host interface control reserved */
    __I  uint32_t                     TOUCH_HIF_DBG;     /*!< Host interface control delay output data*/
} CAPTOUCH_REGISTERHIF_T;


/*************************** captouch register definition end line  *******************************
 */



/*************************** AIRO I2C register definition end line  *******************************
 */
typedef struct {
    __IO    uint32_t    DATA_PORT;              /*0x0*/
    __IO    uint32_t    RESERV_0;               /*0x4*/
    __IO    uint32_t    INTR_MASK;              /*0x8*/
    __IO    uint32_t    INTR_STA;               /*0xC*/

    __IO    uint32_t    CONTROL;                /*0x10*/
    __IO    uint32_t    TRANSFER_LEN;           /*0x14*/
    __IO    uint32_t    TRANSAC_LEN;            /*0x18*/
    __IO    uint32_t    DELAY_LEN;              /*0x1C*/

    __IO    uint32_t    H_TIMING;               /*0x20*/
    __IO    uint32_t    START;                  /*0x24*/
    __IO    uint32_t    EXT_CONF;               /*0x28*/
    __IO    uint32_t    L_TIMING;               /*0x2C*/

    __IO    uint32_t    HS;                     /*0x30*/
    __IO    uint32_t    IO_CONFIG;              /*0x34*/
    __IO    uint32_t    FIFO_ADDR_CLR;          /*0x38*/
    __IO    uint32_t    DATA_TIMING;            /*0x3C*/

    __IO    uint32_t    MCU_ONTR;               /*0x40*/
    __IO    uint32_t    TRANSFER_LEN_AUX;       /*0x44*/
    __IO    uint32_t    CLOCK_DIV;              /*0x48*/
    __IO    uint32_t    TIMEOUT_DIV;            /*0x4C*/

    __IO    uint32_t    SOFT_RESET;             /*0x50*/

    __IO    uint32_t    TRAFFIC;                /*0x54*/
    __IO    uint32_t    COMMAND;                /*0x58*/
    __IO    uint32_t    CRC_CODE;               /*0x5C*/

    __IO    uint32_t    TERNARY;                /*0x60*/
    __IO    uint32_t    IBI_TIMING;             /*0x64*/
    __IO    uint32_t    DEF_DA;                 /*0x68*/
    __IO    uint32_t    SHAPE;                  /*0x6C*/

    __IO    uint32_t    HFIFO_DATA;             /*0x70*/
    __IO    uint32_t    RESERVE_0[3];           /*RSRV*/

    __IO    uint32_t    SYSTIME_LATCH;          /*0x80*/
    __IO    uint32_t    AED_PATCH;              /*0x84*/
    __IO    uint32_t    RESERVE_88;             /*0x88*/
    __IO    uint32_t    RESERVE_8C;             /*0x8C*/

    __IO    uint32_t    RESERVE_90;             /*0x90*/
    __IO    uint32_t    SLAVE_ADDR;             /*0x94*/
    __IO    uint32_t    RESERVE_98;             /*0x98*/
    __IO    uint32_t    RESERVE_9C;             /*0x9C*/


    __IO    uint32_t    RESERVE_2[12];          /*0xAx, 0xBx,0xCx*/

    __IO    uint32_t    ERROR;                  /*0xD0*/
    __IO    uint32_t    DELAY_STEP;             /*0xD4*/
    __IO    uint32_t    DELAY_SAMPLE;           /*0xD8*/
    __IO    uint32_t    DMA_INFO;               /*0xDC*/

    __IO    uint32_t    IRQ_INFO;               /*0xE0*/
    __IO    uint32_t    DEBUG_STAT;             /*0xE4*/
    __IO    uint32_t    DEBUG_CTRL;             /*0xE8*/
    __IO    uint32_t    DMA_FSM_DEBUG;          /*0xEC*/


    __IO    uint32_t    MULTIMAS;               /*0xF0*/
    __IO    uint32_t    FIFO_STAT;              /*0xF4*/
    __IO    uint32_t    FIFO_THRESH;            /*0xF8*/
    __IO    uint32_t    HFIFO_STAT;             /*0xFC*/

}airo_i2c_register_t;


/*************************** I3C register definition end line  *******************************
 */
typedef struct {
    __IO    uint8_t     DATA_PORT;              /*0x0*/
    __IO    uint8_t     RESERVE_0x[3];
    __IO    uint32_t    RESERV_04;              /*0x4*/
    __IO    uint32_t    INTR_MASK;              /*0x8*/
    __IO    uint32_t    INTR_STA;               /*0xC*/

    __IO    uint32_t    CONTROL;                /*0x10*/
    __IO    uint32_t    TRANSFER_LEN;           /*0x14*/
    __IO    uint32_t    TRANSAC_LEN;            /*0x18*/
    __IO    uint32_t    DELAY_LEN;              /*0x1C*/

    __IO    uint32_t    H_TIMING;               /*0x20*/
    __IO    uint32_t    START;                  /*0x24*/
    __IO    uint32_t    EXT_CONF;               /*0x28*/
    __IO    uint32_t    L_TIMING;               /*0x2C*/

    __IO    uint32_t    HS;                     /*0x30*/
    __IO    uint32_t    IO_CONFIG;              /*0x34*/
    __IO    uint32_t    FIFO_ADDR_CLR;          /*0x38*/
    __IO    uint32_t    DATA_TIMING;            /*0x3C*/

    __IO    uint32_t    MCU_INTR;               /*0x40*/
    __IO    uint32_t    TRANSFER_LEN_AUX;       /*0x44*/
    __IO    uint32_t    CLOCK_DIV;              /*0x48*/
    __IO    uint32_t    TIMEOUT_DIV;            /*0x4C*/

    __IO    uint32_t    SOFT_RESET;             /*0x50*/
    __IO    uint32_t    TRAFFIC;                /*0x54*/
    __IO    uint32_t    COMMAND;                /*0x58*/
    __IO    uint32_t    CRC_CODE;               /*0x5C*/

    __IO    uint32_t    TERNARY;                /*0x60*/
    __IO    uint32_t    IBI_TIMING;             /*0x64*/
    __IO    uint32_t    DEF_DA;                 /*0x68*/
    __IO    uint32_t    SHAPE;                  /*0x6C*/

    __IO    uint32_t    HFIFO_DATA;             /*0x70*/
    __IO    uint32_t    RESERVE_7X[3];          /*RSRV*/

    __IO    uint32_t    SYSTIME_LATCH;          /*0x80*/
    __IO    uint32_t    AED_PATCH;              /*0x84*/
    __IO    uint32_t    RESERVE_88;             /*0x88*/
    __IO    uint32_t    RESERVE_8C;             /*0x8C*/

    __IO    uint32_t    RESERVE_90;             /*0x90*/
    __IO    uint32_t    SLAVE_ADDR;             /*0x94*/
    __IO    uint32_t    RESERVE_98;             /*0x98*/
    __IO    uint32_t    RESERVE_9C;             /*0x9C*/

    __IO    uint32_t    RESERVE_2[12];          /*0xAx, 0xBx,0xCx*/

    __IO    uint32_t    ERROR;                  /*0xD0*/
    __IO    uint32_t    DELAY_STEP;             /*0xD4*/
    __IO    uint32_t    DELAY_SAMPLE;           /*0xD8*/
    __IO    uint32_t    DMA_INFO;               /*0xDC*/

    __IO    uint32_t    IRQ_INFO;               /*0xE0*/
    __IO    uint32_t    DEBUG_STAT;             /*0xE4*/
    __IO    uint32_t    DEBUG_CTRL;             /*0xE8*/
    __IO    uint32_t    DMA_FSM_DEBUG;          /*0xEC*/

    __IO    uint32_t    MULTIMAS;               /*0xF0*/
    __IO    uint32_t    FIFO_STAT;              /*0xF4*/
    __IO    uint32_t    FIFO_THRESH;            /*0xF8*/
    __IO    uint32_t    HFIFO_STAT;             /*0xFC*/

}I3C_MASTER_REGISTER_T;

/* 0x08: INTR_MASK */
#define     I3C_INTR_MASK_TRANS_COMP_OFFSET     (0)
#define     I3C_INTR_MASK_ACKERR_OFFSET         (1)
#define     I3C_INTR_MASK_HS_NACKERR_OFFSET     (2)
#define     I3C_INTR_MASK_ARB_LOST_OFFSET       (3)
#define     I3C_INTR_MASK_RS_MULTI_OFFSET       (4)
#define     I3C_INTR_MASK_TIMEOUT_OFFSET        (5)
#define     I3C_INTR_MASK_MDA_ERR_OFFSET        (6)
#define     I3C_INTR_MASK_IBI_OFFSET            (7)
#define     I3C_INTR_MASK_ERR_OFFSET            (8)

#define     I3C_INTR_MASK_TRANS_COMP_MASK       (0x1<<0)
#define     I3C_INTR_MASK_ACKERR_MASK           (0x1<<1)
#define     I3C_INTR_MASK_HS_NACKERR_MASK       (0x1<<2)
#define     I3C_INTR_MASK_ARB_LOST_MASK         (0x1<<3)
#define     I3C_INTR_MASK_RS_MULTI_MASK         (0x1<<4)
#define     I3C_INTR_MASK_TIMEOUT_MASK          (0x1<<5)
#define     I3C_INTR_MASK_MDA_ERR_MASK          (0x1<<6)
#define     I3C_INTR_MASK_IBI_MASK              (0x1<<7)
#define     I3C_INTR_MASK_ERR_MASK              (0x1<<8)



/* 0x10: CONTROL */
#define     I3C_CONTROL_RS_STOP_OFFSET          (1)
#define     I3C_CONTROL_DMA_EN_OFFSET           (2)
#define     I3C_CONTROL_CLK_EXT_EN_OFFSET       (3)
#define     I3C_CONTROL_DIR_CHANGE_OFFSET       (4)
#define     I3C_CONTROL_ACKERR_DET_EN_OFFSET    (5)
#define     I3C_CONTROL_TRANSFER_LEN_CHANGE_OFFSER (6)

#define     I3C_CONTROL_RS_STOP_MASK            (1<<1)
#define     I3C_CONTROL_DMA_EN_MASK             (1<<2)
#define     I3C_CONTROL_CLK_EXT_EN_MASK         (1<<3)
#define     I3C_CONTROL_DIR_CHANGE_MASK         (1<<4)
#define     I3C_CONTROL_ACKERR_DET_EN_MASK      (1<<5)
#define     I3C_CONTROL_TRANSFER_LEN_CHANGE_MASK (1<<6)


/* 0x20: HTIMING */
#define     I3C_HTIMING_HSTEP_CNT_DIV_OFFSET    (0)   /*for 6 bits*/
#define     I3C_HTIMING_DELAY_TIME_DET_OFFSET   (6)   /*for 2 bits */
#define     I3C_HTIMING_HSAMPLE_CNT_DIV_OFFSET  (8)   /*for 3 bits*/
#define     I3C_HTIMING_TIMEOUT_EN_OFFSET       (11)  /*for 1 bit*/

#define     I3C_HTIMING_HSTEP_CNT_DIV_MASK      (0x3F<<I3C_HTIMING_HSTEP_CNT_DIV_OFFSET)
#define     I3C_HTIMING_DELAY_TIME_DET_MASK     (0x3<<I3C_HTIMING_DELAY_TIME_DET_OFFSET)
#define     I3C_HTIMING_HSAMPLE_CNT_DIV_MASK    (0x7<<I3C_HTIMING_HSAMPLE_CNT_DIV_OFFSET)
#define     I3C_HTIMING_TIMEOUT_EN_MASK         (0x1<<I3C_HTIMING_TIMEOUT_EN_OFFSET)

/* 0x24: START */
#define     I3C_START_EN_START_OFFSET           (0)
#define     I3C_START_MUL_TRIG_CLR_OFFSET       (13)
#define     I3C_START_MUL_TRIG_EN_OFFSET        (14)
#define     I3C_START_MUL_CONF_OFFSET           (15)

#define     I3C_START_EN_START_MASK             (0x1)
#define     I3C_START_MUL_HS_CNF_MASK           (0x1<<12)
#define     I3C_START_MUL_TRIG_CLR_MASK         (0x1<<13)
#define     I3C_START_MUL_TRIG_EN_MASK          (0x1<<14)
#define     I3C_START_MUL_CONF_MASK             (0x1<<15)

/* 0x28: EXT_CONFIG */
#define     I3C_EXT_CONFIG_EXT_EN_OFFSET        (0)
#define     I3C_EXT_CONFIG_HS_EXT_TIME_OFFSET   (1)
#define     I3C_EXT_CONFIG_EXT_TIME_OFFSET      (8)

#define     I3C_EXT_CONFIG_EXT_EN_MASK          (0x1)
#define     I3C_EXT_CONFIG_HS_EXT_TIME_MASK     (0x3F<<1)
#define     I3C_EXT_CONFIG_EXT_TIME_MASK        (0xFF<<8)



/* 0x2C: LTIMING */
#define     I3C_LTIMING_LSTEP_CNT_DIV_OFFSET    (0)
#define     I3C_LTIMING_LSMAPLE_CNT_DIV_OFFSET  (6)
#define     I3C_LTIMING_LHS_STEP_CNT_DIV_OFFSET (9)
#define     I3C_LTIMING_LHS_SAMPLE_CNT_DIV_OFFSET (12)

#define     I3C_LTIMING_LSTEP_CNT_DIV_MASK      (0x3F<<I3C_LTIMING_LSTEP_CNT_DIV_OFFSET)
#define     I3C_LTIMING_LSMAPLE_CNT_DIV_MASK    (0x7<<I3C_LTIMING_LSMAPLE_CNT_DIV_OFFSET)
#define     I3C_LTIMING_LHS_STEP_CNT_DIV_MASK   (0x7<<I3C_LTIMING_LHS_STEP_CNT_DIV_OFFSET)
#define     I3C_LTIMING_LHS_SAMPLE_CNT_DIV_MASK (0x7<<I3C_LTIMING_LHS_SAMPLE_CNT_DIV_OFFSET)


/* 0x30: LTIMING */
#define     I3C_HS_EN_OFFSET                    (0)
#define     I3C_HS_NACKERR_DET_EN_OFFSET        (1)
#define     I3C_HS_HOLD_TIME_OFFSET             (2)
#define     I3C_HS_FC_STEP_OFFSET               (4)
#define     I3C_HS_SPEED_OFFSET                 (7)
#define     I3C_HS_STEP_CNT_DIV_OFFSET          (8)
#define     I3C_HS_SAMPLE_CNT_DIV_OFFSET        (12)
#define     I3C_HS_HOLD_EN_OFFSET               (15)

#define     I3C_HS_EN_MASK                      (0x1<<0)
#define     I3C_HS_NACKERR_DET_EN_MASK          (0x1<<1)
#define     I3C_HS_HOLD_TIME_MASK               (0x3<<2)
#define     I3C_HS_FC_STEP_MASK                 (0x7<<4)
#define     I3C_HS_SPEED_MASK                   (0x1<<7)
#define     I3C_HS_STEP_CNT_DIV_MASK            (0x7<<8)
#define     I3C_HS_SAMPLE_CNT_DIV_MASK          (0x7<<12)
#define     I3C_HS_HOLD_EN_MASK                 (0x1<<15)

/* 0x34: IO_CONFIG */
#define     I3C_IO_CONFIG_SCL_OFFSET            (0)
#define     I3C_IO_CONFIG_SDA_OFFSET            (1)
#define     I3C_IO_CONFIG_IO_SYNC_EN_OFFSET     (2)
#define     I3C_IO_CONFIF_IDLE_PE_EN_OFFSET     (3)
#define     I3C_IO_CONFIG_AED_DIV_OFFSET        (4)
#define     I3C_IO_CONFIG_AED_EN_OFFSET         (5)
#define     I3C_IO_CONFIG_AED_STEP_ADJ_OFFSET   (9)
#define     I3C_IO_CONFIG_AED_STEP_EN_OFFSET    (15)


#define     I3C_IO_CONFIG_SCL_MASK              (1<<0)
#define     I3C_IO_CONFIG_SDA_MASK              (1<<1)
#define     I3C_IO_CONFIG_IO_SYNC_EN_MASK       (1<<2)
#define     I3C_IO_CONFIF_IDLE_OE_EN_MASK       (1<<3)
#define     I3C_IO_CONFIG_AED_DIV_MASK          (1<<4)
#define     I3C_IO_CONFIG_AED_EN_MASK           (0x3<<5)
#define     I3C_IO_CONFIG_AED_STEP_ADJ_MASK     (0x3F<<9)
#define     I3C_IO_CONFIG_AED_STEP_EN_MASK      (0x1<<15)





/* 0x48: CLOCK_DIV */
#define     I3C_CLOCK_DIV_SMP_DIV_OFFFSET       (0)
#define     I3C_CLOCK_DIV_HS_DIV_OFFFSET        (8)
#define     I3C_CLOCK_DIV_SMP_DIV_MASK          (0x1F)
#define     I3C_CLOCK_DIV_HS_DIV_MASK           (0x1F<<8)

/* 0x50: SOFTRESET */
#define     I3C_SOFTRESET_SOFT_RESET_OFFSET     (0)
#define     I3C_SOFTRESET_FSM_RESET_OFFSET      (1)
#define     I3C_SOFTRESET_GRAIN_RESET_OFFSET    (2)
#define     I3C_SOFTRESET_ERROR_RESET_OFFSET    (3)

#define     I3C_SOFTRESET_SOFT_RESET_MASK       (1<<0)
#define     I3C_SOFTRESET_FSM_RESET_MASK        (1<<1)
#define     I3C_SOFTRESET_GRAIN_RESET_MASK      (1<<2)
#define     I3C_SOFTRESET_ERROR_RESET_MASK      (1<<3)

/* 0x54: TRAFFIC */
#define     I3C_TRAFFIC_HDR_DECODE_OFFSET       (0)
#define     I3C_TRAFFIC_DAA_EN_OFFSET           (4)
#define     I3C_TRAFFIC_TERNARY_EXIT_OFFSET     (5)
#define     I3C_TRAFFIC_PRE_AMBLE_EXIT_OFFSET   (6)
#define     I3C_TRAFFIC_TBIT_EN_OFFSET          (7)
#define     I3C_TRAFFIC_PARITY_EXIT_OFFSET      (8)
#define     I3C_TRAFFIC_HEAD_ONLY_OFFSET        (9)
#define     I3C_TRAFFIC_SKIP_LV_ADDR_OFFSET     (10)
#define     I3C_TRAFFIC_FORCE_RW_OFFSET         (11)
#define     I3C_TRAFFIC_IBI_EN_OFFSET           (13)
#define     I3C_TRAFFIC_HANDOFF_OFFSET          (14)
#define     I3C_TRAFFIC_DDR_FLEX_OFFSET         (15)

#define     I3C_TRAFFIC_HDR_DECODE_MASK         (0xF<<0)
#define     I3C_TRAFFIC_DAA_EN_MASK             (0x1<<4)
#define     I3C_TRAFFIC_TERNARY_EXIT_MASK       (0x1<<5)
#define     I3C_TRAFFIC_PRE_AMBLE_EXIT_MASK     (0x1<<6)
#define     I3C_TRAFFIC_TBIT_EN_MASK            (0x1<<7)
#define     I3C_TRAFFIC_PARITY_EXIT_MASK        (0x1<<8)
#define     I3C_TRAFFIC_HEAD_ONLY_MASK          (0x1<<9)
#define     I3C_TRAFFIC_SKIP_LV_ADDR_MASK       (0x1<<10)
#define     I3C_TRAFFIC_FORCE_RW_MASK           (0x3<<11)
#define     I3C_TRAFFIC_IBI_EN_MASK             (0x1<<13)
#define     I3C_TRAFFIC_HANDOFF_MASK            (0x1<<14)
#define     I3C_TRAFFIC_DDR_FLEX_MASK           (0x1<<15)

/* 0x6C: SHAPE */
#define     I3C_SHAPE_TBIT_FILL_OFFSET          (0)
#define     I3C_SHAPE_TBIT_STALL_OFFSET         (1)
#define     I3C_SHAPE_TBIT_PARITY_OFFSET        (2)
#define     I3C_SHAPE_TBIT_RS_OFFSET            (3)
#define     I3C_SHAPE_HDR_STRETCH_OFFSET        (4)

#define     I3C_SHAPE_TBIT_FILL_MASK            (1<<0)
#define     I3C_SHAPE_TBIT_STALL_MASK           (1<<1)
#define     I3C_SHAPE_TBIT_PARITY_MASK          (1<<2)
#define     I3C_SHAPE_TBIT_RS_MASK              (1<<3)
#define     I3C_SHAPE_HDR_STRETCH_MASK          (0xF<<4)

/* 0x70: HFIFO_DATA */
#define     I3C_HFIFO_DATA_INST_DATA_OFFSET     (0)
#define     I3C_HFIFO_DATA_NINTH_BIT_OFFSET     (8)
#define     I3C_HFIFO_DATA_INST_SPEED_OFFSET    (10)

#define     I3C_HFIFO_DATA_INST_DATA_MASK       (0xFF<<0)
#define     I3C_HFIFO_DATA_NINTH_BIT_MASK       (0x3<<8)
#define     I3C_HFIFO_DATA_INST_SPEED_MASK      (0x1<<10)

enum    {
    I3C_HFIFO_DATA_NINTH_BIT_IGNORE = 0,
    I3C_HFIFO_DATA_NINTH_BIT_ACK,
    I3C_HFIFO_DATA_NINTH_BIT_NACK,
    I3C_HFIFO_DATA_NINTH_BIT_PARITY
};


/* 0x71: HFIFO */
#define     I3C_HFIFO_WR_EN_MASK                (1 << 15)
#define     I3C_HFIFO_HS_SPEED_MASK             (1 << 10)

#define     I3C_HFIFO_9BIT_IGNORE               (0)
#define     I3C_HFIFO_9BIT_ACK                  (1 << 8)
#define     I3C_HFIFO_9BIT_NACK                 (2 << 8)
#define     I3C_HFIFO_9BIT_PARITY               (3 << 8)

/* 0xF4: FIFO_STAT */
#define     I3C_FIFO_STAT_FIFO_OFS_OFFSET       (0)
#define     I3C_FIFO_STAT_WR_ADDR_OFFSET        (5)
#define     I3C_FIFO_STAT_RD_ADDR_OFFSET        (10)

#define     I3C_FIFO_STAT_FIFO_OFS_MASK         (0x1F)
#define     I3C_FIFO_STAT_WR_ADDR_MASK          (0x1F<<5)
#define     I3C_FIFO_STAT_RD_ADDR_MASK          (0x1F<<10)




/*************************** I3C register definition end line  *******************************
 */
    typedef struct {
        __IO    uint32_t    GLB_STA;                /* 0x0000 */
        __IO    uint32_t    RESERVE_0004;           /* 0x0004 */
        __IO    uint32_t    GLB_CPU0_CFG;           /* 0x0008 */
        __IO    uint32_t    GLB_CPU0_SET;           /* 0x000C */

        __IO    uint32_t    GLB_CPU0_CLR;           /* 0x0010 */
        __IO    uint32_t    GLB_CPU1_CFG;           /* 0x0014 */
        __IO    uint32_t    GLB_CPU1_SET;           /* 0x0018 */
        __IO    uint32_t    GLB_CPU1_CLR;           /* 0x001C */

        __IO    uint32_t    GLB_SWRST;              /* 0x0020 */
        __IO    uint32_t    RESERVE_24;             /* 0x0024 */
        __IO    uint32_t    GLB_LIMITER;            /* 0x0028 */
        __IO    uint32_t    RESERVE_002C;           /* 0x002C */

        __IO    uint32_t    GLB_DBG;                /* 0x0030 */
        __IO    uint32_t    RESERVE_003X[3];        /* 0x003x */

        __IO    uint32_t    GLB_BUSY;               /* 0x0040 */
        __IO    uint32_t    GLB_INTR;               /* 0x0044 */
        __IO    uint32_t    RESERVE_004X[2];        /* 0x003x */

        __IO    uint32_t    GLB_CPU2_CFG;           /* 0x0050 */
        __IO    uint32_t    GLB_CPU2_SET;           /* 0x0054 */
        __IO    uint32_t    GLB_CPU2_CLR;           /* 0x0058 */
        __IO    uint32_t    RESERVE_005C;           /* 0x005C */

        __IO    uint32_t    GLB_CPU3_CFG;           /* 0x0060 */
        __IO    uint32_t    GLB_CPU3_SET;           /* 0x0064 */
        __IO    uint32_t    GLB_CPU3_CLR;           /* 0x0068 */
        __IO    uint32_t    RESERVE_006C;           /* 0x006C */

        __IO    uint32_t    GLB_CLK_CFG;            /* 0x0070 */
        __IO    uint32_t    GLB_CLK_SET;            /* 0x0074 */
        __IO    uint32_t    GLB_CLK_CLR;            /* 0x0078 */
        __IO    uint32_t    RESERVE_007C;           /* 0x007C */

        __IO    uint32_t    RESERVE_008X[4];        /* 0x008X */
        __IO    uint32_t    RESERVE_009X[4];        /* 0x009X */
        __IO    uint32_t    RESERVE_00AX[4];        /* 0x00AX */
        __IO    uint32_t    RESERVE_00BX[4];        /* 0x00BX */
        __IO    uint32_t    RESERVE_00CX[4];        /* 0x00CX */
        __IO    uint32_t    RESERVE_00DX[4];        /* 0x00DX */
        __IO    uint32_t    RESERVE_00EX[4];        /* 0x00EX */
        __IO    uint32_t    RESERVE_00FX[4];        /* 0x00F0 */

        __IO    uint32_t    RESERVE_010X[2];        /* 0x010x */
        __IO    uint32_t    PDMA1_WPPT;             /* 0x0108 */
        __IO    uint32_t    PDMA1_WPTO;             /* 0x010C */

        __IO    uint32_t    PDMA1_COUNT;            /* 0x0110 */
        __IO    uint32_t    PDMA1_CON;              /* 0x0114 */
        __IO    uint32_t    PDMA1_START;            /* 0x0118 */
        __IO    uint32_t    PDMA1_INTSTA;           /* 0x011C */

        __IO    uint32_t    PDMA1_ACKINT;           /* 0x0120 */
        __IO    uint32_t    PDMA1_RLCT;             /* 0x0124 */
        __IO    uint32_t    PDMA1_LIMITER;          /* 0x0128 */
        __IO    uint32_t    PDMA1_PGMADDR;          /* 0x012C */

        __IO    uint32_t    RESERVE_01XX[52];       /* 0x01xx */

        __IO    uint32_t    RESERVE_02XX[2];        /* 0x02xx */
        __IO    uint32_t    PDMA2_WPPT;             /* 0x0208 */
        __IO    uint32_t    PDMA2_WPTO;             /* 0x020C */

        __IO    uint32_t    PDMA2_COUNT;            /* 0x0210 */
        __IO    uint32_t    PDMA2_CON;              /* 0x0214 */
        __IO    uint32_t    PDMA2_START;            /* 0x0218 */
        __IO    uint32_t    PDMA2_INTSTA;           /* 0x021C */

        __IO    uint32_t    PDMA2_ACKINT;           /* 0x0220 */
        __IO    uint32_t    PDMA2_RLCT;             /* 0x0224 */
        __IO    uint32_t    PDMA2_LIMITER;          /* 0x0228 */
        __IO    uint32_t    PDMA2_PGMADDR;          /* 0x022C */

    }I2C_PDMA_REGISTER_T;



#ifdef AIR_CPU_IN_SECURITY_MODE

/* ----------------------------------------------------------------------------
-- SECURITY_TOP Peripheral Access Layer START LINE
---------------------------------------------------------------------------- */

#define MAS_SEC                          (*(volatile uint32_t *)(0x422E0000)) /*!< SECURITY_TOP MAS_SEC base  address.*/
#define MAS_SEC_SET                      (*(volatile uint32_t *)(0x422E0010)) /*!< SECURITY_TOP MAS_SEC set   address.*/
#define MAS_SEC_CLR                      (*(volatile uint32_t *)(0x422E0020)) /*!< SECURITY_TOP MAS_SEC clear address.*/

#define SFC_MRAC_REGION_0_START          (*(volatile uint32_t *)(0x422E0100)) /*!< SECURITY_TOP SFC region base address.*/
#define SFC_MRAC_REGION_0_END            (*(volatile uint32_t *)(0x422E0104)) /*!< SECURITY_TOP SFC region base address.*/
#define SFC_MRAC_REGION_1_START          (*(volatile uint32_t *)(0x422E0108)) /*!< SECURITY_TOP SFC region base address.*/
#define SFC_MRAC_REGION_1_END            (*(volatile uint32_t *)(0x422E010C)) /*!< SECURITY_TOP SFC region base address.*/
#define SFC_MRAC_REGION_2_START          (*(volatile uint32_t *)(0x422E0110)) /*!< SECURITY_TOP SFC region base address.*/
#define SFC_MRAC_REGION_2_END            (*(volatile uint32_t *)(0x422E0114)) /*!< SECURITY_TOP SFC region base address.*/
#define SFC_MRAC_REGION_3_START          (*(volatile uint32_t *)(0x422E0118)) /*!< SECURITY_TOP SFC region base address.*/
#define SFC_MRAC_REGION_3_END            (*(volatile uint32_t *)(0x422E011C)) /*!< SECURITY_TOP SFC region base address.*/

#define DSPRAM_MRAC_REGION_0_START       (*(volatile uint32_t *)(0x422E0200)) /*!< SECURITY_TOP DSPRAM  region base address.*/
#define DSPRAM_MRAC_REGION_0_END         (*(volatile uint32_t *)(0x422E0204)) /*!< SECURITY_TOP DSPRAM  region base address.*/
#define DSPRAM_MRAC_REGION_1_START       (*(volatile uint32_t *)(0x422E0208)) /*!< SECURITY_TOP DSPRAM  region base address.*/
#define DSPRAM_MRAC_REGION_1_END         (*(volatile uint32_t *)(0x422E020C)) /*!< SECURITY_TOP DSPRAM  region base address.*/
#define DSPRAM_MRAC_REGION_2_START       (*(volatile uint32_t *)(0x422E0210)) /*!< SECURITY_TOP DSPRAM  region base address.*/
#define DSPRAM_MRAC_REGION_2_END         (*(volatile uint32_t *)(0x422E0214)) /*!< SECURITY_TOP DSPRAM  region base address.*/
#define DSPRAM_MRAC_REGION_3_START       (*(volatile uint32_t *)(0x422E0218)) /*!< SECURITY_TOP DSPRAM  region base address.*/
#define DSPRAM_MRAC_REGION_3_END         (*(volatile uint32_t *)(0x422E021C)) /*!< SECURITY_TOP DSPRAM  region base address.*/

#define ESC_MRAC_REGION_0_START          (*(volatile uint32_t *)(0x422E0300)) /*!< SECURITY_TOP ESC  region base address.*/
#define ESC_MRAC_REGION_0_END            (*(volatile uint32_t *)(0x422E0304)) /*!< SECURITY_TOP ESC  region base address.*/
#define ESC_MRAC_REGION_1_START          (*(volatile uint32_t *)(0x422E0308)) /*!< SECURITY_TOP ESC  region base address.*/
#define ESC_MRAC_REGION_1_END            (*(volatile uint32_t *)(0x422E030C)) /*!< SECURITY_TOP ESC  region base address.*/
#define ESC_MRAC_REGION_2_START          (*(volatile uint32_t *)(0x422E0310)) /*!< SECURITY_TOP ESC  region base address.*/
#define ESC_MRAC_REGION_2_END            (*(volatile uint32_t *)(0x422E0314)) /*!< SECURITY_TOP ESC  region base address.*/
#define ESC_MRAC_REGION_3_START          (*(volatile uint32_t *)(0x422E0318)) /*!< SECURITY_TOP ESC  region base address.*/
#define ESC_MRAC_REGION_3_END            (*(volatile uint32_t *)(0x422E031C)) /*!< SECURITY_TOP ESC  region base address.*/

#define TCM_MRAC_REGION_0_START          (*(volatile uint32_t *)(0x422E0400)) /*!< SECURITY_TOP TCM  region base address.*/
#define TCM_MRAC_REGION_0_END            (*(volatile uint32_t *)(0x422E0404)) /*!< SECURITY_TOP TCM  region base address.*/
#define TCM_MRAC_REGION_1_START          (*(volatile uint32_t *)(0x422E0408)) /*!< SECURITY_TOP TCM  region base address.*/
#define TCM_MRAC_REGION_1_END            (*(volatile uint32_t *)(0x422E040C)) /*!< SECURITY_TOP TCM  region base address.*/
#define TCM_MRAC_REGION_2_START          (*(volatile uint32_t *)(0x422E0410)) /*!< SECURITY_TOP TCM  region base address.*/
#define TCM_MRAC_REGION_2_END            (*(volatile uint32_t *)(0x422E0414)) /*!< SECURITY_TOP TCM  region base address.*/
#define TCM_MRAC_REGION_3_START          (*(volatile uint32_t *)(0x422E0418)) /*!< SECURITY_TOP TCM  region base address.*/
#define TCM_MRAC_REGION_3_END            (*(volatile uint32_t *)(0x422E041C)) /*!< SECURITY_TOP TCM  region base address.*/

#define AUDIO_MRAC_REGION_0_START        (*(volatile uint32_t *)(0x422E0500)) /*!< SECURITY_TOP ADUIO  region base address.*/
#define AUDIO_MRAC_REGION_0_END          (*(volatile uint32_t *)(0x422E0504)) /*!< SECURITY_TOP ADUIO  region base address.*/
#define AUDIO_MRAC_REGION_1_START        (*(volatile uint32_t *)(0x422E0508)) /*!< SECURITY_TOP ADUIO  region base address.*/
#define AUDIO_MRAC_REGION_1_END          (*(volatile uint32_t *)(0x422E050C)) /*!< SECURITY_TOP ADUIO  region base address.*/
#define AUDIO_MRAC_REGION_2_START        (*(volatile uint32_t *)(0x422E0510)) /*!< SECURITY_TOP ADUIO  region base address.*/
#define AUDIO_MRAC_REGION_2_END          (*(volatile uint32_t *)(0x422E0514)) /*!< SECURITY_TOP ADUIO  region base address.*/
#define AUDIO_MRAC_REGION_3_START        (*(volatile uint32_t *)(0x422E0518)) /*!< SECURITY_TOP ADUIO  region base address.*/
#define AUDIO_MRAC_REGION_3_END          (*(volatile uint32_t *)(0x422E051C)) /*!< SECURITY_TOP ADUIO  region base address.*/

#define CUSTOMER_MRAC_REGION_0_START         (*(volatile uint32_t *)(0x422E0600)) /*!< SECURITY_TOP CUSTOMER  region base address.*/
#define CUSTOMER_MRAC_REGION_0_END           (*(volatile uint32_t *)(0x422E0604)) /*!< SECURITY_TOP CUSTOMER  region base address.*/
#define CUSTOMER_MRAC_REGION_1_START         (*(volatile uint32_t *)(0x422E0608)) /*!< SECURITY_TOP CUSTOMER  region base address.*/
#define CUSTOMER_MRAC_REGION_1_END           (*(volatile uint32_t *)(0x422E060C)) /*!< SECURITY_TOP CUSTOMER  region base address.*/
#define CUSTOMER_MRAC_REGION_2_START         (*(volatile uint32_t *)(0x422E0610)) /*!< SECURITY_TOP CUSTOMER  region base address.*/
#define CUSTOMER_MRAC_REGION_2_END           (*(volatile uint32_t *)(0x422E0614)) /*!< SECURITY_TOP CUSTOMER  region base address.*/
#define CUSTOMER_MRAC_REGION_3_START         (*(volatile uint32_t *)(0x422E0618)) /*!< SECURITY_TOP CUSTOMER  region base address.*/
#define CUSTOMER_MRAC_REGION_3_END           (*(volatile uint32_t *)(0x422E061C)) /*!< SECURITY_TOP CUSTOMER  region base address.*/

/* SYSRAM MBAC config (R/W) */
#define SYSRAM_MBAC_SECURE_B_0          (*(volatile uint32_t *)(0x422E0700)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM_MBAC_SECURE_B_1          (*(volatile uint32_t *)(0x422E0704)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM_MBAC_SECURE_B_2          (*(volatile uint32_t *)(0x422E0708)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM_MBAC_SECURE_B_3          (*(volatile uint32_t *)(0x422E070C)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM_MBAC_SECURE_B_4          (*(volatile uint32_t *)(0x422E0710)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM_MBAC_SECURE_B_5          (*(volatile uint32_t *)(0x422E0714)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM_MBAC_SECURE_B_6          (*(volatile uint32_t *)(0x422E0718)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM_MBAC_SECURE_B_7          (*(volatile uint32_t *)(0x422E071C)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_0_SET     (*(volatile uint32_t *)(0x422E0720)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_1_SET     (*(volatile uint32_t *)(0x422E0724)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_2_SET     (*(volatile uint32_t *)(0x422E0728)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_3_SET     (*(volatile uint32_t *)(0x422E072C)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_4_SET     (*(volatile uint32_t *)(0x422E0730)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_5_SET     (*(volatile uint32_t *)(0x422E0734)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_6_SET     (*(volatile uint32_t *)(0x422E0738)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_7_SET     (*(volatile uint32_t *)(0x422E073C)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_0_CLR     (*(volatile uint32_t *)(0x422E0740)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_1_CLR     (*(volatile uint32_t *)(0x422E0744)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_2_CLR     (*(volatile uint32_t *)(0x422E0748)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_3_CLR     (*(volatile uint32_t *)(0x422E074C)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_4_CLR     (*(volatile uint32_t *)(0x422E0750)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_5_CLR     (*(volatile uint32_t *)(0x422E0754)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_6_CLR     (*(volatile uint32_t *)(0x422E0758)) /*!< SECURITY_TOP SYSRAM1 region base address.*/
#define SYSRAM0_MBAC_SECURE_B_7_CLR     (*(volatile uint32_t *)(0x422E075C)) /*!< SECURITY_TOP SYSRAM1 region base address.*/

/*Peripherial*/
#define PAC_SECURE_B_0                   (*(volatile uint32_t *)(0x422E0800)) /*!< SECURITY_TOP peripherial region base address.*/
#define PAC_SECURE_B_1                   (*(volatile uint32_t *)(0x422E0804)) /*!< SECURITY_TOP peripherial region base address.*/
#define PAC_SECURE_B_2                   (*(volatile uint32_t *)(0x422E0808)) /*!< SECURITY_TOP peripherial region base address.*/
#define PAC_SECURE_B_0_SET               (*(volatile uint32_t *)(0x422E0810)) /*!< SECURITY_TOP peripherial region base address.*/
#define PAC_SECURE_B_1_SET               (*(volatile uint32_t *)(0x422E0814)) /*!< SECURITY_TOP peripherial region base address.*/
#define PAC_SECURE_B_2_SET               (*(volatile uint32_t *)(0x422E0818)) /*!< SECURITY_TOP peripherial region base address.*/
#define PAC_SECURE_B_0_CLR               (*(volatile uint32_t *)(0x422E0820)) /*!< SECURITY_TOP peripherial region base address.*/
#define PAC_SECURE_B_1_CLR               (*(volatile uint32_t *)(0x422E0824)) /*!< SECURITY_TOP peripherial region base address.*/
#define PAC_SECURE_B_2_CLR               (*(volatile uint32_t *)(0x422E0828)) /*!< SECURITY_TOP peripherial region base address.*/

/*GPIO*/
#define GPIO_SECURE_B_0                  (*(volatile uint32_t *)(0x422E0900)) /*!< SECURITY_TOP GPIO region base address.*/
#define GPIO_SECURE_B_1                  (*(volatile uint32_t *)(0x422E0904)) /*!< SECURITY_TOP GPIO region base address.*/
#define GPIO_SECURE_B_2                  (*(volatile uint32_t *)(0x422E0908)) /*!< SECURITY_TOP GPIO region base address.*/
#define GPIO_SECURE_B_0_SET              (*(volatile uint32_t *)(0x422E0910)) /*!< SECURITY_TOP GPIO region base address.*/
#define GPIO_SECURE_B_1_SET              (*(volatile uint32_t *)(0x422E0914)) /*!< SECURITY_TOP GPIO region base address.*/
#define GPIO_SECURE_B_2_SET              (*(volatile uint32_t *)(0x422E0918)) /*!< SECURITY_TOP GPIO region base address.*/
#define GPIO_SECURE_B_0_CLR              (*(volatile uint32_t *)(0x422E0920)) /*!< SECURITY_TOP GPIO region base address.*/
#define GPIO_SECURE_B_1_CLR              (*(volatile uint32_t *)(0x422E0924)) /*!< SECURITY_TOP GPIO region base address.*/
#define GPIO_SECURE_B_2_CLR              (*(volatile uint32_t *)(0x422E0928)) /*!< SECURITY_TOP GPIO region base address.*/

/*EINT*/
#define EINT_SECURE_B_0                  (*(volatile uint32_t *)(0x422E0A00)) /*!< SECURITY_TOP EINT region base address.*/
#define EINT_SECURE_B_1                  (*(volatile uint32_t *)(0x422E0A04)) /*!< SECURITY_TOP EINT region base address.*/
#define EINT_SECURE_B_0_SET              (*(volatile uint32_t *)(0x422E0A10)) /*!< SECURITY_TOP EINT region base address.*/
#define EINT_SECURE_B_1_SET              (*(volatile uint32_t *)(0x422E0A14)) /*!< SECURITY_TOP EINT region base address.*/
#define EINT_SECURE_B_0_CLR              (*(volatile uint32_t *)(0x422E0A20)) /*!< SECURITY_TOP EINT region base address.*/
#define EINT_SECURE_B_1_CLR              (*(volatile uint32_t *)(0x422E0A24)) /*!< SECURITY_TOP EINT region base address.*/
/*configuration lock*/
#define SEC_CFG_LOCK                     (*(volatile uint32_t *)(0x422E0B00)) /*!< SECURITY_TOP lock all security configuration.*/

/*configuration sysram*/
#define SPECIAL_SYSRAM_SEL               (*(volatile uint32_t *)(0x422E0E00)) /*!< SECURITY_TOP sysram sel configuration.*/

#define SECURITY_TOP_MASK(offset)        (1UL << offset)
/* ----------------------------------------------------------------------------
-- SECURITY_TOP Peripheral Access Layer  END LINE
---------------------------------------------------------------------------- */

#endif /* AIR_CPU_IN_SECURITY_MODE */

#endif


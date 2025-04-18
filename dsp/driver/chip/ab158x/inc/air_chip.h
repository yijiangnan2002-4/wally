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

#include <stdint.h>

#if defined(CORE_CM4)
//#define __CM4_REV                 0x0001  /*!< Core revision r0p1                            */
#define __MPU_PRESENT             1       /*!< MT2811 provides an MPU                        */
#define __NVIC_PRIO_BITS          5       /*!< MT2811 uses 5 Bits for the Priority Levels    */
//#define __Vendor_SysTickConfig    0       /*!< Set to 1 if different SysTick Config is used  */
#define __FPU_PRESENT             1       /*!< FPU present                                   */

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

#include "core_cm4.h"                  /* Core Peripheral Access Layer */
#include "system_mt2822.h"

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
    IRQ_NUMBER_MAX
} IRQn_Type;

typedef IRQn_Type hal_nvic_irq_t;
#else
#error "must be have a Core define!!!"
#endif

/* ================================================== */

#if defined(AIR_CPU_IN_SECURITY_MODE) 
#define I2C_0_BASE           0x400D0000 /*I2C_0*/
#define I2C_1_BASE           0x430D0000 /*I2C_1*/
#define I2C_2_BASE           0x430c0000 /*I2C_1*/
#define I2C0_PDMA_BASE       0x400D0000 /* I2C0 PDMA*/
#define I2C1_PDMA_BASE       0x43110000 /* I2C1 PDMA*/
#define I2C2_PDMA_BASE       0x43120000 /* I2C2 PDMA*/
#define I3C0_PDMA_BASE       0x43130000 /* I3C0 PDMA */
#define I3C1_PDMA_BASE       0x43140000 /* I3C1 PDMA */
#define I2C_AO_BASE          0x42270000 /*I2C_AO*/
/* PD APB Peripheral */
#define AESOTF_BASE          0xA0000000 /*AES_OnTheFly*/
#define AESOTF_ESC_BASE      0xA3000000 /*AES_OnTheFly_ESC*/
#define TRNG_BASE            0x43030000 /*TRNG*/
#define DMA_0_BASE           0x40020000 /*DMA Controller*/
#define DMA_1_BASE           0x40030000 /*DMA Controller*/
#define UART_DMA_0_BASE      0x40080000 /*UART_0 DMA Controller*/
#define UART_DMA_1_BASE      0x40090000 /*UART_1 DMA Controller*/
#define UART_DMA_2_BASE      0x400A0000 /*UART_2 DMA Controller*/
#define I2S_DMA_BASE         0xC9000000 /*I2S DMA Controller*/
//#define INFRA_MBIST_BASE     0xA0030000 /*INFRA MBIST CONFIG*/
#define SFC_BASE             0x40040000 /*Serial Flash Contoller*/
#define EMI_BASE             0xA3040000 /*External Memory Interface*/
#define CRYPTO_BASE          0xA3010000 /*Crypto_Engine*/
//#define AUDIO_BASE           0xA0070000 /*ADUIO TOP*/
//#define ASYS_BASE            0xA0080000 /*ASYS TOP*/
//#define BTIF_BASE            0xA0090000 /*Bluetooth interface*/
#define SPI_MASTER_0_BASE      0x43050000 /*SPI MASTER 0*/
#define UART0_BASE           0x40050000 /*UART 0*/
#define UART1_BASE           0x40060000 /*UART 1*/
#define UART2_BASE           0x40070000 /*UART 2*/

/* PD AHB Peripheral */
//#define DMA_AHB_BASE         0xA1000000 /*PD DMA AHB*/
//#define AUDIO_AHB_BASE       0xA1010000 /*PD ADUIO AHB*/
//#define ASYS_AHB_BASE        0xA1020000 /*PD ASYS AHB*/
//#define SDIO_MASTER_BASE     0xA1030000 /*SDIO MASTER*/
//#define SDIO_SLAVE_BASE      0xA1040000 /*SDIO SLAVE*/

/* PD CMSYS AHB Peripheral */
//#define CMSYS_CFG_BASE       0xE0100000 /*cmsys_cfg*/
//#define CMSYS_CFG_EXT_BASE   0xE00FE000 /*cmsys_cfg_ext*/
//#define CMSYS_MBIST_BASE     0xE0110000 /*cmsys_mbist*/
//#define CM4_MCU_BASE         0xE0130000 /*Reserved for MDM*/
#define CMSYS_L1CACHE_BASE   0xE0180000 /*cmsys_l1cache*/

/* AO APB Peripheral */
#define CONFIG_BASE          0xA2010000 /*Configuration Registers(Clock, Power Down, Version and Reset)*/
#define TOP_MISC_CFG_BASE    0xA2010000 /*Configuration Registers(Clock, Power Down, Version and Reset)*/
#define EMI_AO_BASE          0xA2010400
//#define CKSYS_BASE           0xA2020000 /*BBPLL control*/
//#define CKSYS_XO_CLK_BASE    0xA2030000 /*XPLL control*/
//#define BBPLL_CTRL_BASE      0xA2040000 /*26M, clk switch control (clk divider related setting)*/
#define XPLL_CTRL_BASE       0xA2050000 /*26M, top clk control (dcm, cg)*/
//#define XTAL_CTRL_BASE       0xA2060000 /*26M, RF Xtal control register*/
//#define PMU_BASE             0xA2070000 /*PMU configure*/
//#define RTC_BASE             0xA2080000 /*Real Time Clock*/
#define RGU_BASE             0xA2090000 /*Reset Generation Unit*/
//#define EFUSE_BASE           0xA20A0000 /*EFUSE*/
#define GPIO_BASE            0x420D0000 /*General Purpose Inputs/Outputs*/
#define IO_CFG_0_BASE        0x420E0000 /*io_cfg_0*/
#define IO_CFG_1_BASE        0x420F0000 /*io_cfg_1*/
//#define SPM_BASE             0xA20F0000 /*System Power Manager */
#define EINT_BASE            0x42150300 /*External Interrupts*/
#define GPT_BASE             0x42160000 /*General Purpose Timer */
#define OS_GPT_BASE          0x42180000 /*OS General Purpose Timer */
//#define PWM0_BASE            0xA2120000 /*Pulse-Width Modulation Outputs 0*/
//#define PWM1_BASE            0xA2130000 /*Pulse-Width Modulation Outputs 1*/
//#define PWM2_BASE            0xA2140000 /*Pulse-Width Modulation Outputs 2*/
//#define PWM3_BASE            0xA2150000 /*Pulse-Width Modulation Outputs 3*/
//#define PWM4_BASE            0xA2160000 /*Pulse-Width Modulation Outputs 4*/
//#define PWM5_BASE            0xA2170000 /*Pulse-Width Modulation Outputs 5*/
//#define CKSYS_BUS_CLK_BASE   0xA21D0000 /*96M, top clk control (dcm, cg)*/
#define MCU_CFG_PRI_BASE     0x422C0000 /*CM4 always-on CFG*/
//#define INFRA_CFG_BASE       0xA21F0000 /*bus configuration registers*/
#define SEMAPHORE_BASE       0x42280000 /* HareWare Semaphore */
#define CM4_CCNI_BASE      (0x42290000)
#define DSP0_CCNI_BASE     (0x422A0000)

//CCNI message address, should align with layout
#define USB_BASE             0xA1020000 /*USB*/
#define USB_SIFSLV_BASE      0xA1030000 /*USB PHY, SIFSLV*/

#else /* AIR_CPU_IN_SECURITY_MODE */

#define I2C_0_BASE           0x500D0000 /*I2C_0*/
#define I2C_1_BASE           0x530D0000 /*I2C_1*/
#define I2C_2_BASE           0x530c0000 /*I2C_1*/
#define I2C0_PDMA_BASE       0x500D0000 /* I2C0 PDMA*/
#define I2C1_PDMA_BASE       0x53110000 /* I2C1 PDMA*/
#define I2C2_PDMA_BASE       0x53120000 /* I2C2 PDMA*/
#define I3C0_PDMA_BASE       0x53130000 /* I3C0 PDMA */
#define I3C1_PDMA_BASE       0x53140000 /* I3C1 PDMA */
#define I2C_AO_BASE          0x52270000 /*I2C_AO*/
/* PD APB Peripheral */
#define AESOTF_BASE          0xA0000000 /*AES_OnTheFly*/
#define AESOTF_ESC_BASE      0xA3000000 /*AES_OnTheFly_ESC*/
#define TRNG_BASE            0x53030000 /*TRNG*/
#define DMA_0_BASE           0x50020000 /*DMA Controller*/
#define DMA_1_BASE           0x50030000 /*DMA Controller*/
#define UART_DMA_0_BASE      0x50080000 /*UART_0 DMA Controller*/
#define UART_DMA_1_BASE      0x50090000 /*UART_1 DMA Controller*/
#define UART_DMA_2_BASE      0x500A0000 /*UART_2 DMA Controller*/
#define I2S_DMA_BASE         0xD9000000 /*I2S DMA Controller*/
//#define INFRA_MBIST_BASE     0xA0030000 /*INFRA MBIST CONFIG*/
#define SFC_BASE             0x50040000 /*Serial Flash Contoller*/
#define EMI_BASE             0xA3040000 /*External Memory Interface*/
#define CRYPTO_BASE          0xA3010000 /*Crypto_Engine*/
//#define AUDIO_BASE           0xA0070000 /*ADUIO TOP*/
//#define ASYS_BASE            0xA0080000 /*ASYS TOP*/
//#define BTIF_BASE            0xA0090000 /*Bluetooth interface*/
#define SPI_MASTER_0_BASE      0x53050000 /*SPI MASTER 0*/
#define UART0_BASE           0x50050000 /*UART 0*/
#define UART1_BASE           0x50060000 /*UART 1*/
#define UART2_BASE           0x50070000 /*UART 2*/

/* PD AHB Peripheral */
//#define DMA_AHB_BASE         0xA1000000 /*PD DMA AHB*/
//#define AUDIO_AHB_BASE       0xA1010000 /*PD ADUIO AHB*/
//#define ASYS_AHB_BASE        0xA1020000 /*PD ASYS AHB*/
//#define SDIO_MASTER_BASE     0xA1030000 /*SDIO MASTER*/
//#define SDIO_SLAVE_BASE      0xA1040000 /*SDIO SLAVE*/

/* PD CMSYS AHB Peripheral */
//#define CMSYS_CFG_BASE       0xE0100000 /*cmsys_cfg*/
//#define CMSYS_CFG_EXT_BASE   0xE00FE000 /*cmsys_cfg_ext*/
//#define CMSYS_MBIST_BASE     0xE0110000 /*cmsys_mbist*/
//#define CM4_MCU_BASE         0xE0130000 /*Reserved for MDM*/
#define CMSYS_L1CACHE_BASE   0xE0180000 /*cmsys_l1cache*/

/* AO APB Peripheral */
#define CONFIG_BASE          0xA2010000 /*Configuration Registers(Clock, Power Down, Version and Reset)*/
#define TOP_MISC_CFG_BASE    0xA2010000 /*Configuration Registers(Clock, Power Down, Version and Reset)*/
#define EMI_AO_BASE          0xA2010400
//#define CKSYS_BASE           0xA2020000 /*BBPLL control*/
//#define CKSYS_XO_CLK_BASE    0xA2030000 /*XPLL control*/
//#define BBPLL_CTRL_BASE      0xA2040000 /*26M, clk switch control (clk divider related setting)*/
#define XPLL_CTRL_BASE       0xA2050000 /*26M, top clk control (dcm, cg)*/
//#define XTAL_CTRL_BASE       0xA2060000 /*26M, RF Xtal control register*/
//#define PMU_BASE             0xA2070000 /*PMU configure*/
//#define RTC_BASE             0xA2080000 /*Real Time Clock*/
#define RGU_BASE             0xA2090000 /*Reset Generation Unit*/
//#define EFUSE_BASE           0xA20A0000 /*EFUSE*/
#define GPIO_BASE            0x520D0000 /*General Purpose Inputs/Outputs*/
#define IO_CFG_0_BASE        0x520E0000 /*io_cfg_0*/
#define IO_CFG_1_BASE        0x520F0000 /*io_cfg_1*/
//#define SPM_BASE             0xA20F0000 /*System Power Manager */
#define EINT_BASE            0x52150300 /*External Interrupts*/
#define GPT_BASE             0x52160000 /*General Purpose Timer */
#define OS_GPT_BASE          0x52180000 /*OS General Purpose Timer */
//#define PWM0_BASE            0xA2120000 /*Pulse-Width Modulation Outputs 0*/
//#define PWM1_BASE            0xA2130000 /*Pulse-Width Modulation Outputs 1*/
//#define PWM2_BASE            0xA2140000 /*Pulse-Width Modulation Outputs 2*/
//#define PWM3_BASE            0xA2150000 /*Pulse-Width Modulation Outputs 3*/
//#define PWM4_BASE            0xA2160000 /*Pulse-Width Modulation Outputs 4*/
//#define PWM5_BASE            0xA2170000 /*Pulse-Width Modulation Outputs 5*/
//#define CKSYS_BUS_CLK_BASE   0xA21D0000 /*96M, top clk control (dcm, cg)*/
#define MCU_CFG_PRI_BASE     0x522C0000 /*CM4 always-on CFG*/
//#define INFRA_CFG_BASE       0xA21F0000 /*bus configuration registers*/
#define SEMAPHORE_BASE       0x52280000 /* HareWare Semaphore */

#define CM4_CCNI_BASE      (0x52290000)
#define DSP0_CCNI_BASE     (0x522A0000)

//CCNI message address, should align with layout
#define USB_BASE             0xA1020000 /*USB*/
#define USB_SIFSLV_BASE      0xA1030000 /*USB PHY, SIFSLV*/

#endif /* !AIR_CPU_IN_SECURITY_MODE */

#define I2C_AO_CLOCK_FREQUENCY  26000000/* 26Mhz */
#define I2C_CLOCK_FREQUENCY     104000000//104000000 /* 104Mhz */
#define I3C_CLOCK_FREQUENCY     104000000//104000000 /* 104Mhz */

/* structure type of CCNI
 */
typedef struct {
    __IO uint32_t IRQ0_MASK;
    __IO uint32_t IRQ0_SET;
    __IO uint32_t IRQ0_CLR;
    __IO uint32_t IRQ0_STATUS;
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
    uint32_t RESERVED1[1];    // have been modified.
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

#define SMPH    ((SMPH_REGISTER_T *) SEMAPHORE_BASE)

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



#define UART_IIR_NONE      (0x1<<UART_IIR_ID_OFFSET) /* No interrupt pending */
#define UART_IIR_LSR       (0x6<<UART_IIR_ID_OFFSET) /* Line Status Interrupt */
#define UART_IIR_RDT       (0xC<<UART_IIR_ID_OFFSET) /* RX Data Timeout */
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
#define SFC_PM              ((SFC_PM_REGISTER_T *) (SFC_PM_BASE))
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
#define SFC_GENERIC_FLASH_BANK_MASK  (0x8000000)
#define SFC_GENERIC_DPD_SW_MASK      (0x000F0F00)
#define SFC_GENERIC_DPD_SW_IO_MASK   (0x0F0F0F0F)


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
    __I  uint32_t AUXADC_DATA7;
    uint32_t RESERVED3[17];
    __IO  uint32_t MACRO_CON2;
    __IO  uint32_t ANA_EN_CON;
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


/* memory mapping of MT2811
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
    __IO uint32_t RTC_BBPU;     /* Baseband power up,                           Address offset: 0x00 */
    __I  uint32_t RTC_IRQ_STA;  /* RTC IRQ status,                              Address offset: 0x04 */
    union {
        struct {
            __IO uint8_t LP_EN;   /*  */
            __IO uint8_t ONESHOT; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_IRQ_EN_CELLS;
        __IO uint32_t RTC_IRQ_EN; /* RTC IRQ enable,                                Address offset: 0x08 */
    } RTC_IRQ_EN_UNION;
    __IO uint32_t RTC_CII_EN;       /* Counter increment IRQ enable,                    Address offset: 0x0C */
    union {
        struct {
            __IO uint8_t AL_MASK;   /*  */
            __IO uint8_t AL_EN; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_AL_MASK_CELLS;
        __IO uint32_t RTC_AL_MASK; /* RTC alarm mask,                               Address offset: 0x10 */
    } RTC_AL_MASK_UNION;
    union {
        struct {
            __IO uint8_t TC_SECOND;   /*  */
            __IO uint8_t TC_MINUTE; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_TC0_CELLS;
        __IO uint32_t RTC_TC0; /* RTC time counter register0,               Address offset: 0x14 */
    } RTC_TC0_UNION;
    union {
        struct {
            __IO uint8_t TC_HOUR;   /*  */
            __IO uint8_t TC_DOM; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_TC1_CELLS;
        __IO uint32_t RTC_TC1; /* RTC time counter register1,               Address offset: 0x18 */
    } RTC_TC1_UNION;
    union {
        struct {
            __IO uint8_t TC_DOW;   /*  */
            __IO uint8_t TC_MONTH; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_TC2_CELLS;
        __IO uint32_t RTC_TC2; /* RTC time counter register2,               Address offset: 0x1C */
    } RTC_TC2_UNION;
    __IO uint32_t RTC_TC3;      /* RTC time counter register3,          Address offset: 0x20 */
    union {
        struct {
            __IO uint8_t AL_SECOND;   /*  */
            __IO uint8_t AL_MINUTE; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_AL0_CELLS;
        __IO uint32_t RTC_AL0; /* RTC alarm setting register0,          Address offset: 0x24 */
    } RTC_AL0_UNION;
    union {
        struct {
            __IO uint8_t AL_HOUR;   /*  */
            __IO uint8_t AL_DOM; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_AL1_CELLS;
        __IO uint32_t RTC_AL1; /* RTC alarm setting register1,              Address offset: 0x28 */
    } RTC_AL1_UNION;
    union {
        struct {
            __IO uint8_t AL_DOW;   /*  */
            __IO uint8_t AL_MONTH; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_AL2_CELLS;
        __IO uint32_t RTC_AL2; /* RTC alarm setting register2,                  Address offset: 0x2C */
    } RTC_AL2_UNION;
    __IO uint32_t RTC_AL3;      /* RTC alarm setting register3,             Address offset: 0x30 */
    union {
        struct {
            __IO uint8_t LPD_CON;   /*  */
            __IO uint8_t RTC_LPD_OPT; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_LPD_CON_CELLS;
        __IO uint32_t RTC_LPD_CON; /* RTC LPD control register,             Address offset: 0x34 */
    } RTC_LPD_CON_UNION;
    union {
        struct {
            __IO uint8_t RTC_NEW_SPAR0_0;   /*  */
            __IO uint8_t RTC_NEW_SPAR0_1; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_NEW_SPAR0_CELLS;
        __IO uint32_t RTC_NEW_SPAR0; /* RTC new spare register0,                Address offset: 0x38 */
    } RTC_NEW_SPAR0_UNION;
    __IO uint32_t RTC_EINT;     /* RTC EINT control register,           Address offset: 0x3C */
    union {
        struct {
            __IO uint8_t RTC_32K_SEL;   /*  */
            __IO uint8_t RTC_TIMER_CG_EN; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_OSC32CON0_CELLS;
        __IO uint32_t RTC_OSC32CON0; /* RTC OSC32 control register0,            Address offset: 0x40 */
    } RTC_OSC32CON0_UNION;
    __IO uint32_t RTC_OSC32CON1;        /* RTC OSC32 control register1,             Address offset: 0x44 */
    union {
        struct {
            __IO uint8_t OSC32_PW_DB;   /*  */
            __IO uint8_t SRAM_ISO_EN; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_OSC32CON2_CELLS;
        __IO uint32_t RTC_OSC32CON2; /* RTC OSC32 control register2,                Address offset: 0x48 */
    } RTC_OSC32CON2_UNION;
    union {
        struct {
            __IO uint8_t SRAM_PD;   /*  */
            __IO uint8_t SRAM_SLEEPB; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_SRAM_CON_CELLS;
        __IO uint32_t RTC_SRAM_CON; /* Retention SRAM control,                              Address offset: 0x4C */
    } RTC_SRAM_CON_UNION;
    __IO uint32_t RTC_POWERKEY0;    /* RTC_POWERKEY0 register,                      Address offset: 0x50 */
    __IO uint32_t RTC_POWERKEY1;    /* RTC_POWERKEY1 register,                      Address offset: 0x54 */
    union {
        struct {
            __IO uint8_t RTC_PDN0_0;   /*  */
            __IO uint8_t RTC_PDN0_1; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_PDN0_CELLS;
        __IO uint32_t RTC_PDN0; /* PDN0,                                        Address offset: 0x58 */
    } RTC_PDN0_UNION;
    union {
        struct {
            __IO uint8_t RTC_PDN1_0;   /*  */
            __IO uint8_t RTC_PDN1_1; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_PDN1_CELLS;
        __IO uint32_t RTC_PDN1; /* PDN1,                                        Address offset: 0x5C */
    } RTC_PDN1_UNION;
    union {
        struct {
            __IO uint8_t RTC_SPAR0_0;   /*  */
            __IO uint8_t RTC_SPAR0_1; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_SPAR0_CELLS;
        __IO uint32_t RTC_SPAR0; /* Spare register for specific purpose,                Address offset: 0x60 */
    } RTC_SPAR0_UNION;
    union {
        struct {
            __IO uint8_t RTC_SPAR1_0;   /*  */
            __IO uint8_t RTC_SPAR1_1; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_SPAR1_CELLS;
        __IO uint32_t RTC_SPAR1; /* Spare register for specific purpose,                Address offset: 0x64 */
    } RTC_SPAR1_UNION;
    __IO uint32_t RTC_PROT;     /* Lock/unlock scheme to prevent RTC miswriting,        Address offset: 0x68 */
    __IO uint32_t RTC_DIFF;     /* One-time calibration offset,                 Address offset: 0x6C */
    __IO uint32_t RTC_CALI;     /* Repeat calibration offset,                       Address offset: 0x70 */
    union {
        struct {
            __IO uint8_t WRTGR;   /*  */
            __IO uint8_t RTC_STA; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_WRTGR_CELLS;
        __IO uint32_t RTC_WRTGR; /*  Enable the transfers from core to RTC in the queue,    Address offset: 0x74 */
    } RTC_WRTGR_UNION;
    union {
        struct {
            __IO uint8_t PAD_EINT_CON;   /*  */
            __IO uint8_t GPO_CON; /*  */
            __IO uint8_t RESERVED[2];
        } RTC_GPIO_CON_CELLS;
        __IO uint32_t RTC_GPIO_CON; /* GPIO control register,                   Address offset: 0x78 */
    } RTC_GPIO_CON_UNION;
} RTC_REGISTER_T;

/************************ RTC register definition end line  *******************************
 */

/* ************************* CRYPTO hardware definition start line**********************************
*/
#define CRYPTO_ENGINE_CTRL_BASE (volatile uint32_t *)(0xA3010004)
#define ENGINE_CTRL_START_OFFSET (4)

#define CRYPTO_ENGINE_STA_BASE (volatile uint32_t *)(0xA3010008)
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

/************************ WDT register definition start line  *******************************
 */
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

#define WDT_REGISTER  ((WDT_REGISTER_T *)(RGU_BASE)) /* CM4_WDT_BASE */

/* Bit definition for WDT_EN Register */
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


/************************ WDT register definition end line  *******************************
 */
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


/************************ SDIO_SLAVE register definition start line  *******************************
 */

typedef struct {
    /****************************************************************/
    /*************This part define FW global register****************/
    /****************************************************************/
    __IO uint32_t HGFCR;          /*offset:0x00, HIF Global Firmware Configuration Register.*/
    __IO uint32_t HGFISR;         /*offset:0x04, HIF Global Firmware Interrupt Status Register.*/
    __IO uint32_t HGFIER;         /*offset:0x08, HIF Global Firmware Interrupt Enable Register.*/
    __IO uint32_t rsv0;
    __IO uint32_t HSDBDLSR;       /*offset:0x10, HIF SDIO Bus Delay Selection Register.*/
    __IO uint32_t HSDLSR;         /*offset:0x14, HIF SRAM Delay Selection Register.*/
    __IO uint32_t HCSDCR;         /*offset:0x18, HIF Clock Stop Detection register.*/
    __IO uint32_t HGH2DR;         /*offset:0x1c, HIF Global Host to Device Register.*/
    __IO uint32_t HDBGCR;         /*offset:0x20, HIF Debug Control Register.*/
    __IO uint32_t rsv1[2];
    __IO uint32_t FWDSIOCR;   /*offset:0x2c, DS Pad Macro IO Control Register.*/
    __IO uint32_t HGTMTCR;    /*offset:0x30, Test Mode Trigger Control Register.*/
    __IO uint32_t HGTMCR;     /*offset:0x34, Test Mode Control Register.*/
    __IO uint32_t HGTMDPCR0;     /*offset:0x38, Test Mode Data Pattern Control Register 0.*/
    __IO uint32_t HGTMDPCR1;     /*offset:0x3c, Test Mode Data Pattern Control Register 1.*/
    __IO uint32_t FWCLKIOCR_T28LP;     /*offset:0x40, Clock Pad Macro IO Control Register.*/
    __IO uint32_t FWCMDIOCR_T28LP;     /*offset:0x44, Command Pad Macro IO Control Register.*/
    __IO uint32_t FWDAT0IOCR_T28LP;     /*offset:0x48, Data 0 Pad Macro IO Control Register.*/
    __IO uint32_t FWDAT1IOCR_T28LP;     /*offset:0x4c, Data 1 Pad Macro IO Control Register.*/
    __IO uint32_t FWDAT2IOCR_T28LP;     /*offset:0x50, Data 2 Pad Macro IO Control Register.*/
    __IO uint32_t FWDAT3IOCR_T28LP;     /*offset:0x54, Data 3 Pad Macro IO Control Register.*/
    __IO uint32_t FWCLKDLYCR;     /*offset:0x58, Clock Pad Macro Delay Chain Control Register.*/
    __IO uint32_t FWCMDDLYCR;     /*offset:0x5c, Command Pad Macro Delay Chain Control Register.*/
    __IO uint32_t FWODATDLYCR;     /*offset:0x60, SDIO Output Data Delay Chain Control Register.*/
    __IO uint32_t FWIDATDLYCR1;     /*offset:0x64, SDIO Input Data Delay Chain Control Register 1.*/
    __IO uint32_t FWIDATDLYCR2;     /*offset:0x68, SDIO Input Data Delay Chain Control Register 2.*/
    __IO uint32_t FWILCHCR;     /*offset:0x6c, SDIO Input Data Latch Time Control Register.*/
    __IO uint32_t rsv2[36];

    /****************************************************************/
    /*************This part define FW DMA register*******************/
    /****************************************************************/
    __IO uint32_t HWFISR;     /*offset:0x100, HIF WLAN Firmware Interrupt Status Register.*/
    __IO uint32_t HWFIER;     /*offset:0x104, HIF WLAN Firmware Interrupt Enable Register.*/
    __IO uint32_t HWFISR1;     /*offset:0x108, Reserve for HWFISR1.*/
    __IO uint32_t HWFIER1;     /*offset:0x10c, Reserve for HWFIER1.*/
    __IO uint32_t HWFTE0SR;     /*offset:0x110, HIF WLAN Firmware TX Event 0 Status Register.*/
    __IO uint32_t HWFTE1SR;     /*offset:0x114, Reserve for HWFTE1SR.*/
    __IO uint32_t HWFTE2SR;     /*offset:0x118, Reserve for HWFTE2SR.*/
    __IO uint32_t HWFTE3SR;     /*offset:0x11c, Reserve for HWFTE3SR.*/
    __IO uint32_t HWFTE0ER;     /*offset:0x120, HIF WLAN Firmware TX Event 0 Enable Register.*/
    __IO uint32_t HWFTE1ER;     /*offset:0x124, Reserve for HWFTE1ER.*/
    __IO uint32_t HWFTE2ER;     /*offset:0x128, Reserve for HWFTE2ER.*/
    __IO uint32_t HWFTE3ER;     /*offset:0x12c, Reserve for HWFTE3ER.*/
    __IO uint32_t HWFRE0SR;     /*offset:0x130, HIF WLAN Firmware RX Event 0 Status Register.*/
    __IO uint32_t HWFRE1SR;     /*offset:0x134, HIF WLAN Firmware RX Event 1 Status Register.*/
    __IO uint32_t HWFRE2SR;     /*offset:0x138, Reserve for HWFRE2SR.*/
    __IO uint32_t HWFRE3SR;     /*offset:0x13c, Reserve for HWFRE3SR.*/
    __IO uint32_t HWFRE0ER;     /*offset:0x140, HIF WLAN Firmware RX Event 0 Enable Register.*/
    __IO uint32_t HWFRE1ER;     /*offset:0x144, HIF WLAN Firmware RX Event 1 Enable Register.*/
    __IO uint32_t HWFRE2ER;     /*offset:0x148, Reserve for HWFRE2ER.*/
    __IO uint32_t HWFRE3ER;     /*offset:0x14c, Reserve for HWFRE3ER.*/
    __IO uint32_t HWFICR;       /*offset:0x150, HIF WLAN Firmware Interrupt Control Register.*/
    __IO uint32_t HWFCR;        /*offset:0x154, HIF WLAN Firmware Control Register.*/
    __IO uint32_t HWTDCR;       /*offset:0x158, HIF WLAN TX DMA Control Register.*/
    __IO uint32_t HWTPCCR;       /*offset:0x15c, HIF WLAN TX Packet Count Control Register.*/
    __IO uint32_t HWFTQ0SAR;       /*offset:0x160, HIF WLAN Firmware TX Queue 0 Start Address Register.*/
    __IO uint32_t HWFTQ1SAR;       /*offset:0x164, HIF WLAN Firmware TX Queue 1 Start Address Register.*/
    __IO uint32_t HWFTQ2SAR;       /*offset:0x168, HIF WLAN Firmware TX Queue 2 Start Address Register.*/
    __IO uint32_t HWFTQ3SAR;       /*offset:0x16c, HIF WLAN Firmware TX Queue 3 Start Address Register.*/
    __IO uint32_t HWFTQ4SAR;       /*offset:0x170, HIF WLAN Firmware TX Queue 4 Start Address Register.*/
    __IO uint32_t HWFTQ5SAR;       /*offset:0x174, Reserve for HIF WLAN Firmware TX Queue 5 Start Address Register.*/
    __IO uint32_t HWFTQ6SAR;       /*offset:0x178, Reserve for HIF WLAN Firmware TX Queue 6 Start Address Register.*/
    __IO uint32_t HWFTQ7SAR;       /*offset:0x17c, Reserve for HIF WLAN Firmware TX Queue 7 Start Address Register.*/
    __IO uint32_t HWFRQ0SAR;       /*offset:0x180, HIF WLAN Firmware RX Queue 0 Start Address Register.*/
    __IO uint32_t HWFRQ1SAR;       /*offset:0x184, HIF WLAN Firmware RX Queue 1 Start Address Register.*/
    __IO uint32_t HWFRQ2SAR;       /*offset:0x188, HIF WLAN Firmware RX Queue 2 Start Address Register.*/
    __IO uint32_t HWFRQ3SAR;       /*offset:0x18c, HIF WLAN Firmware RX Queue 3 Start Address Register.*/
    __IO uint32_t HWFRQ4SAR;       /*offset:0x190, Reserve for HIF WLAN Firmware RX Queue 4 Start Address Register.*/
    __IO uint32_t HWFRQ5SAR;       /*offset:0x194, Reserve for HIF WLAN Firmware RX Queue 5 Start Address Register.*/
    __IO uint32_t HWFRQ6SAR;       /*offset:0x198, Reserve for HIF WLAN Firmware RX Queue 6 Start Address Register.*/
    __IO uint32_t HWFRQ7SAR;       /*offset:0x19c, Reserve for HIF WLAN Firmware RX Queue 7 Start Address Register.*/
    __IO uint32_t H2DRM0R;       /*offset:0x1a0, Host to Device Receive Mailbox 0 Register.*/
    __IO  uint32_t H2DRM1R;       /*offset:0x1a4, Host to Device Receive Mailbox 1 Register.*/
    __IO  uint32_t D2HSM0R;       /*offset:0x1a8, Device to Host Send Mailbox 0 Register.*/
    __IO uint32_t D2HSM1R;       /*offset:0x1ac, Device to Host Send Mailbox 1 Register.*/
    __IO uint32_t D2HSM2R;       /*offset:0x1b0, Device to Host Send Mailbox 2 Register.*/
    __IO uint32_t rsv3[3];
    __IO uint32_t HWRQ0CR;       /*offset:0x1c0, HIF WLAN RX Queue 0 Control Register.*/
    __IO uint32_t HWRQ1CR;       /*offset:0x1c4, HIF WLAN RX Queue 1 Control Register.*/
    __IO uint32_t HWRQ2CR;       /*offset:0x1c8, HIF WLAN RX Queue 2 Control Register.*/
    __IO uint32_t HWRQ3CR;       /*offset:0x1cc, HIF WLAN RX Queue 3 Control Register.*/
    __IO uint32_t HWRQ4CR;       /*offset:0x1d0, Reserve for HWRQ4CR.*/
    __IO uint32_t HWRQ5CR;       /*offset:0x1d4, Reserve for HWRQ5CR.*/
    __IO uint32_t HWRQ6CR;       /*offset:0x1d8, Reserve for HWRQ6CR.*/
    __IO uint32_t HWRQ7CR;       /*offset:0x1dc, Reserve for HWRQ7CR.*/
    __I  uint32_t HWRLFACR;       /*offset:0x1e0, HIF WLAN RX Length FIFO Available Count Register.*/
    __I  uint32_t HWRLFACR1;       /*offset:0x1e4, Reserve for HWRLFACR1.*/
    __IO uint32_t HWDMACR;       /*offset:0x1e8, HIF WLAN DMA Control Register.*/
    __IO uint32_t HWFIOCDR;       /*offset:0x1ec, HIF WLAN Firmware GPD IOC bit Disable Register.*/
    __IO uint32_t HSDIOTOCR;       /*offset:0x1f0, HIF SDIO Time-Out Control Register.*/
    __IO uint32_t rsv4[3];
    __I  uint32_t HWFTSR0;       /*offset:0x200, HIF WLAN Firmware TX Status Register 0.*/
    __I  uint32_t HWFTSR1;       /*offset:0x204, HIF WLAN Firmware TX Status Register 1.*/
    __IO uint32_t rsv5[2];
    __IO uint32_t HWDBGCR;       /*offset:0x210, HIF WLAN Debug Control Register.*/
    __IO uint32_t HWDBGPLR;       /*offset:0x214, HIF WLAN Debug Packet Length Register.*/
    __IO uint32_t HSPICSR;       /*offset:0x218, WLAN SPI Control Status Register (SPI Only).*/
} sdio_slv_register_t;



#define SDIO_SLAVE_REG     ((sdio_slv_register_t *)SDIO_SLAVE_BASE)


/*********************SDIO FW HGFCR*****************************/
#define HGFCR_DB_HIF_SEL_OFFSET        (0)
#define HGFCR_DB_HIF_SEL_MASK          (0x07 << HGFCR_DB_HIF_SEL_OFFSET)
#define HGFCR_SPI_MODE_OFFSET          (4)
#define HGFCR_SPI_MODE_MASK            (1 << HGFCR_SPI_MODE_OFFSET)
#define HGFCR_EHPI_MODE_OFFSET         (5)
#define HGFCR_EHPI_MODE_MASK           (1 << HGFCR_EHPI_MODE_OFFSET)
#define HGFCR_SDIO_PIO_SEL_OFFSET      (6)
#define HGFCR_SDIO_PIO_SEL_MASK        (1 << HGFCR_SDIO_PIO_SEL_OFFSET)
#define HGFCR_HINT_AS_FW_OB_OFFSET     (8)
#define HGFCR_HINT_AS_FW_OB_MASK       (1 << HGFCR_HINT_AS_FW_OB_OFFSET)
#define HGFCR_CARD_IS_18V_OFFSET       (9)
#define HGFCR_CARD_IS_18V_MASK         (1 << HGFCR_CARD_IS_18V_OFFSET)
#define HGFCR_SDCTL_BUSY_OFFSET        (10)
#define HGFCR_SDCTL_BUSY_MASK          (1 << HGFCR_SDCTL_BUSY_OFFSET)
#define HGFCR_INT_TER_CYC_MASK_OFFSET  (16)
#define HGFCR_INT_TER_CYC_MASK_MASK    (1 << HGFCR_INT_TER_CYC_MASK_OFFSET)
#define HGFCR_HCLK_NO_GATED_OFFSET     (17)
#define HGFCR_HCLK_NO_GATED_MASK       (1 << HGFCR_HCLK_NO_GATED_OFFSET)
#define HGFCR_FORCE_SD_HS_OFFSET       (18)
#define HGFCR_FORCE_SD_HS_MASK         (1 << HGFCR_FORCE_SD_HS_OFFSET)
#define HGFCR_SDIO_HCLK_DIS_OFFSET     (24)
#define HGFCR_SDIO_HCLK_DIS_MASK       (1 << HGFCR_SDIO_HCLK_DIS_OFFSET)
#define HGFCR_SPI_HCLK_DIS_OFFSET      (25)
#define HGFCR_SPI_HCLK_DIS_MASK        (1 << HGFCR_SPI_HCLK_DIS_OFFSET)
#define HGFCR_EHPI_HCLK_DIS_OFFSET     (26)
#define HGFCR_EHPI_HCLK_DIS_MASK       (1 << HGFCR_EHPI_HCLK_DIS_OFFSET)
#define HGFCR_PB_HCLK_DIS_OFFSET       (27)
#define HGFCR_PB_HCLK_DIS_MASK         (1 << HGFCR_PB_HCLK_DIS_OFFSET)
#define HGFCR_PAD_CR_SET_BY_FW_OFFSET  (28)
#define HGFCR_PAD_CR_SET_BY_FW_MASK    (1 << HGFCR_PAD_CR_SET_BY_FW_OFFSET)


/*********************SDIO FW HGFISR*****************************/
#define HGFISR_DRV_CLR_DB_IOE_OFFSET        (0)
#define HGFISR_DRV_CLR_DB_IOE_MASK          (1 << HGFISR_DRV_CLR_DB_IOE_OFFSET)
#define HGFISR_DRV_CLR_PB_IOE_OFFSET        (1)
#define HGFISR_DRV_CLR_PB_IOE_MASK          (1 << HGFISR_DRV_CLR_PB_IOE_OFFSET)
#define HGFISR_DRV_SET_DB_IOE_OFFSET        (2)
#define HGFISR_DRV_SET_DB_IOE_MASK          (1 << HGFISR_DRV_SET_DB_IOE_OFFSET)
#define HGFISR_DRV_SET_PB_IOE_OFFSET        (3)
#define HGFISR_DRV_SET_PB_IOE_MASK          (1 << HGFISR_DRV_SET_PB_IOE_OFFSET)
#define HGFISR_SDIO_SET_RES_OFFSET          (4)
#define HGFISR_SDIO_SET_RES_MASK            (1 << HGFISR_SDIO_SET_RES_OFFSET)
#define HGFISR_SDIO_SET_ABT_OFFSET          (5)
#define HGFISR_SDIO_SET_ABT_MASK            (1 << HGFISR_SDIO_SET_ABT_OFFSET)
#define HGFISR_DB_INT_OFFSET                (6)
#define HGFISR_DB_INT_MASK                  (1 << HGFISR_DB_INT_OFFSET)
#define HGFISR_PB_INT_OFFSET                (7)
#define HGFISR_PB_INT_MASK                  (1 << HGFISR_PB_INT_OFFSET)
#define HGFISR_CRC_ERROR_INT_OFFSET         (8)
#define HGFISR_CRC_ERROR_INT_MASK           (1 << HGFISR_CRC_ERROR_INT_OFFSET)
#define HGFISR_CHG_TO_18V_REQ_INT_OFFSET    (9)
#define HGFISR_CHG_TO_18V_REQ_INT_MASK      (1 << HGFISR_CHG_TO_18V_REQ_INT_OFFSET)
#define HGFISR_SD1_SET_XTAL_UPD_INT_OFFSET  (10)
#define HGFISR_SD1_SET_XTAL_UPD_INT_MASK    (1 << HGFISR_SD1_SET_XTAL_UPD_INT_OFFSET)
#define HGFISR_SD1_SET_DS_INT_OFFSET        (11)
#define HGFISR_SD1_SET_DS_INT_MASK          (1 << HGFISR_SD1_SET_DS_INT_OFFSET)


/*********************SDIO FW HWFCR*****************************/
#define HWFCR_W_FUNC_RDY_OFFSET            (0)
#define HWFCR_W_FUNC_RDY_MASK              (1 << HWFCR_W_FUNC_RDY_OFFSET)
#define HWFCR_TRX_DESC_CHKSUM_EN_OFFSET    (1)
#define HWFCR_TRX_DESC_CHKSUM_EN_MASK      (1 << HWFCR_TRX_DESC_CHKSUM_EN_OFFSET)
#define HWFCR_TRX_DESC_CHKSUM_12B_OFFSET   (2)
#define HWFCR_TRX_DESC_CHKSUM_12B_MASK     (1 << HWFCR_TRX_DESC_CHKSUM_12B_OFFSET)
#define HWFCR_TX_CS_OFLD_EN_OFFSET         (3)
#define HWFCR_TX_CS_OFLD_EN_MASK           (1 << HWFCR_TX_CS_OFLD_EN_OFFSET)
#define HWFCR_TX_NO_HEADER_OFFSET          (8)
#define HWFCR_TX_NO_HEADER_MASK            (1 << HWFCR_TX_NO_HEADER_OFFSET)
#define HWFCR_RX_NO_TAIL_OFFSET            (9)
#define HWFCR_RX_NO_TAIL_MASK              (1 << HWFCR_RX_NO_TAIL_OFFSET)


/*********************SDIO FW HWFISR*****************************/
#define HWFISR_DRV_SET_FW_OWN_OFFSET     (0)
#define HWFISR_DRV_SET_FW_OWN_MASK       (1 << HWFISR_DRV_SET_FW_OWN_OFFSET)
#define HWFISR_DRV_CLR_FW_OWN_OFFSET     (1)
#define HWFISR_DRV_CLR_FW_OWN_MASK       (1 << HWFISR_DRV_CLR_FW_OWN_OFFSET)
#define HWFISR_D2HSM2R_RD_INT_OFFSET     (2)
#define HWFISR_D2HSM2R_RD_INT_MASK       (1 << HWFISR_D2HSM2R_RD_INT_OFFSET)
#define HWFISR_RD_TIMEOUT_INT_OFFSET     (3)
#define HWFISR_RD_TIMEOUT_INT_MASK       (1 << HWFISR_RD_TIMEOUT_INT_OFFSET)
#define HWFISR_WR_TIMEOUT_INT_OFFSET     (4)
#define HWFISR_WR_TIMEOUT_INT_MASK       (1 << HWFISR_WR_TIMEOUT_INT_OFFSET)
#define HWFISR_TX_EVENT_0_OFFSET         (8)
#define HWFISR_TX_EVENT_0_MASK           (1 << HWFISR_TX_EVENT_0_OFFSET)
#define HWFISR_RX_EVENT_0_OFFSET         (12)
#define HWFISR_RX_EVENT_0_MASK           (1 << HWFISR_RX_EVENT_0_OFFSET)
#define HWFISR_RX_EVENT_1_OFFSET         (13)
#define HWFISR_RX_EVENT_1_MASK           (1 << HWFISR_RX_EVENT_1_OFFSET)
#define HWFISR_H2D_SW_INT_OFFSET         (16)
#define HWFISR_H2D_SW_INT_MASK           (0xffff << HWFISR_H2D_SW_INT_OFFSET)

/*********************SDIO FW HWFIER*****************************/
#define HWFIER_DRV_SET_FW_OWN_INT_EN_OFFSET     (0)
#define HWFIER_DRV_SET_FW_OWN_INT_EN_MASK       (1 << HWFIER_DRV_SET_FW_OWN_INT_EN_OFFSET)
#define HWFIER_DRV_CLR_FW_OWN_INT_EN_OFFSET     (1)
#define HWFIER_DRV_CLR_FW_OWN_INT_EN_MASK       (1 << HWFIER_DRV_CLR_FW_OWN_INT_EN_OFFSET)
#define HWFIER_D2HSM2R_RD_INT_EN_OFFSET         (2)
#define HWFIER_D2HSM2R_RD_INT_EN_MASK           (1 << HWFIER_D2HSM2R_RD_INT_EN_OFFSET)
#define HWFIER_RD_TIMEOUT_INT_EN_OFFSET         (3)
#define HWFIER_RD_TIMEOUT_INT_EN_MASK           (1 << HWFIER_RD_TIMEOUT_INT_EN_OFFSET)
#define HWFIER_WR_TIMEOUT_INT_EN_OFFSET         (4)
#define HWFIER_WR_TIMEOUT_INT_EN_MASK           (1 << HWFIER_WR_TIMEOUT_INT_EN_OFFSET)
#define HWFIER_TX_EVENT_0_INT_EN_OFFSET         (8)
#define HWFIER_TX_EVENT_0_INT_EN_MASK           (1 << HWFIER_TX_EVENT_0_INT_EN_OFFSET)
#define HWFIER_RX_EVENT_0_INT_EN_OFFSET         (12)
#define HWFIER_RX_EVENT_0_INT_EN_MASK           (1 << HWFIER_RX_EVENT_0_INT_EN_OFFSET)
#define HWFIER_RX_EVENT_1_INT_EN_OFFSET         (13)
#define HWFIER_RX_EVENT_1_INT_EN_MASK           (1 << HWFIER_RX_EVENT_1_INT_EN_OFFSET)
#define HWFIER_H2D_SW_INT_EN_OFFSET             (16)
#define HWFIER_H2D_SW_INT_EN_MASK               (0xffff << HWFIER_H2D_SW_INT_EN_OFFSET)


/*********************SDIO FW HWFICR*****************************/
#define HWFICR_FW_OWN_BACK_INT_SET_OFFSET   (4)
#define HWFICR_FW_OWN_BACK_INT_SET_MASK     (1 << HWFICR_FW_OWN_BACK_INT_SET_OFFSET)
#define HWFICR_D2H_SW_INT_SET_OFFSET        (8)
#define HWFICR_D2H_SW_INT_SET_MASK          (0xffffff << HWFICR_D2H_SW_INT_SET_OFFSET)


/*********************SDIO FW HWDMACR*****************************/
#define HWFICR_DEST_BST_TYP_OFFSET   (1)
#define HWFICR_DEST_BST_TYP_MASK     (1 << HWFICR_DEST_BST_TYP_OFFSET)
#define HWFICR_DMA_BST_SIZE_OFFSET   (6)
#define HWFICR_DMA_BST_SIZE_MASK     (0x03 << HWFICR_DMA_BST_SIZE_OFFSET)


/*********************SDIO FW HWRQ0CR*****************************/
#define HWRQ0CR_RXQ0_PACKET_LENGTH_OFFSET   (0)
#define HWRQ0CR_RXQ0_PACKET_LENGTH_MASK     (0xffff << HWRQ0CR_RXQ0_PACKET_LENGTH_OFFSET)
#define HWRQ0CR_RXQ0_DMA_STOP_OFFSET        (16)
#define HWRQ0CR_RXQ0_DMA_STOP_MASK          (1 << HWRQ0CR_RXQ0_DMA_STOP_OFFSET)
#define HWRQ0CR_RXQ0_DMA_START_OFFSET       (17)
#define HWRQ0CR_RXQ0_DMA_START_MASK         (1 << HWRQ0CR_RXQ0_DMA_START_OFFSET)
#define HWRQ0CR_RXQ0_DMA_RUM_OFFSET         (18)
#define HWRQ0CR_RXQ0_DMA_RUM_MASK           (1 << HWRQ0CR_RXQ0_DMA_RUM_OFFSET)
#define HWRQ0CR_RXQ0_DMA_STATUS_OFFSET      (19)
#define HWRQ0CR_RXQ0_DMA_STATUS_MASK        (1 << HWRQ0CR_RXQ0_DMA_STATUS_OFFSET)


/*********************SDIO FW HWRQ1CR*****************************/
#define HWRQ1CR_RXQ1_PACKET_LENGTH_OFFSET   (0)
#define HWRQ1CR_RXQ1_PACKET_LENGTH_MASK     (0xffff << HWRQ1CR_RXQ1_PACKET_LENGTH_OFFSET)
#define HWRQ1CR_RXQ1_DMA_STOP_OFFSET        (16)
#define HWRQ1CR_RXQ1_DMA_STOP_MASK          (1 << HWRQ1CR_RXQ1_DMA_STOP_OFFSET)
#define HWRQ1CR_RXQ1_DMA_START_OFFSET       (17)
#define HWRQ1CR_RXQ1_DMA_START_MASK         (1 << HWRQ1CR_RXQ1_DMA_START_OFFSET)
#define HWRQ1CR_RXQ1_DMA_RUM_OFFSET         (18)
#define HWRQ1CR_RXQ1_DMA_RUM_MASK           (1 << HWRQ1CR_RXQ1_DMA_RUM_OFFSET)
#define HWRQ1CR_RXQ1_DMA_STATUS_OFFSET      (19)
#define HWRQ1CR_RXQ1_DMA_STATUS_MASK        (1 << HWRQ1CR_RXQ1_DMA_STATUS_OFFSET)

/*********************SDIO FW HWTPCCR*****************************/
#define HWTPCCR_INC_TQ_CNT_OFFSET     (0)
#define HWTPCCR_INC_TQ_CNT_MASK       (0xff << HWTPCCR_INC_TQ_CNT_OFFSET)
#define HWTPCCR_TQ_INDEX_OFFSET       (12)
#define HWTPCCR_TQ_INDEX_MASK         (0x0f << HWTPCCR_TQ_INDEX_OFFSET)
#define HWTPCCR_TQ_CNT_RESET_OFFSET   (16)
#define HWTPCCR_TQ_CNT_RESET_MASK     (1 << HWTPCCR_TQ_CNT_RESET_OFFSET)

/************************ SDIO_SLAVE register definition end line  *******************************
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

#endif


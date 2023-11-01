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

#ifndef __HAL_PLATFORM_H__
#define __HAL_PLATFORM_H__


#include "hal_define.h"
#include "hal_feature_config.h"
#include "air_chip.h"
#include "hal_core_status.h"

#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************
* Defines for Audio Register.
*****************************************************************************/
#define AUDIO_TOP_CON0                                      (0xC0000000)
#define AFE_ADDA_UL_SRC_CON0                                (0xC0000114)
#define AFE_ADDA_DL_SRC2_CON0                               (0xC0000108)
#define AFE_APLL1_TUNER_CFG                                 (0xC00003F0)
#define AFE_APLL2_TUNER_CFG                                 (0xC00003F4)
#define AFE_VOW_TOP_CON1                                    (0x44000004)
#define AFE_VOW_TOP_CON2                                    (0x44000008)
#define AFE_SPDIFIN_CFG0                                    (0xC9011A00)

/*****************************************************************************
* Defines for module subfeatures.
* All the subfeatures described below are mandatory for the driver operation. No change is recommended.
*****************************************************************************/
#ifdef HAL_DVFS_MODULE_ENABLED
#define HAL_DVFS_LOCK_CTRL_ENABLED       /*Enable dvfs lock control relative api*/
#define HAL_CLOCK_METER_ENABLE           /*Enable clock meter feature */
#endif
#ifdef HAL_CACHE_MODULE_ENABLED
#define HAL_CACHE_WITH_REMAP_FEATURE     /* Enable CACHE setting with remap feature. */
#define HAL_CACHE_REGION_CONVERT         /* Enable mutual conversion between cacheable and non-cacheable addresses*/
#endif

#ifdef HAL_AUDIO_MODULE_ENABLED
#define HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT   /*Enable support multiple audio stream out feature.*/
#define HAL_AUDIO_SUPPORT_DEBUG_DUMP            /*Enable support dump audio register for debug.*/
#define HAL_AUDIO_SUPPORT_APLL                  /*Enable support apll feature.*/
//#define HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE   /*Enable support multiple microphone.*/
//#define HAL_AUDIO_LEAKAGE_COMPENSATION_FEATURE   /*Enable support leakage compensation.*/
#endif

#ifdef HAL_ADC_MODULE_ENABLED
#define HAL_ADC_CALIBRATION_ENABLE               /*Enable ADC calibration */
#define HAL_ADC_SUPPORT_AVERAGE_ENABLE             /*Enable ADC support average    */
#endif

#ifdef HAL_I2C_MASTER_MODULE_ENABLED
#define HAL_I2C_MASTER_FEATURE_HIGH_SPEED       /* Enable I2C high speed 2M&3.25M. */
#define HAL_I2C_MASTER_FEATURE_SEND_TO_RECEIVE  /* Enable I2C master send to receive feature. */
#define HAL_I2C_MASTER_FEATURE_EXTENDED_DMA     /* Enable I2C master extend DMA feature.*/
#define HAL_I2C_MASTER_FEATURE_CONFIG_IO        /* Enable I2C master config IO mode feature.*/
#ifdef HAL_I2C_MASTER_FEATURE_EXTENDED_DMA
#define HAL_I2C_MASTER_FRATURE_NONE_BLOCKING    /* Enable I2C master software fifo */
#endif
#endif


#ifdef HAL_WDT_MODULE_ENABLED
#define HAL_WDT_FEATURE_SECOND_CHANNEL          /* Supports the second WDT */
#endif

#ifdef HAL_SPI_MASTER_MODULE_ENABLED
#define HAL_SPI_MASTER_FEATURE_ADVANCED_CONFIG       /* Enable SPI master advanced configuration feature. For more details, please refer to hal_spi_master.h.*/
#define HAL_SPI_MASTER_FEATURE_DEASSERT_CONFIG       /* Enable SPI master deassert configuration feature to deassert the chip select signal after each byte data transfer is complete. For more details, please refer to hal_spi_master.h.*/
#define HAL_SPI_MASTER_FEATURE_CHIP_SELECT_TIMING    /* Enable SPI master chip select timing configuration feature to set timing value for chip select signal. For more details, please refer to hal_spi_master.h.*/
#define HAL_SPI_MASTER_FEATURE_DMA_MODE              /* Enable SPI master DMA mode feature to do data transfer. For more details, please refer to hal_spi_master.h.*/
#define HAL_SPI_MASTER_FEATURE_DUAL_QUAD_MODE        /* Enable SPI master to use dual mode and quad mode. For more details, please refer to hal_spi_master.h. */
#define HAL_SPI_MASTER_FEATURE_NO_BUSY               /* Enable SPI master no busy API to support  multithreaded access. For more details, please refer to hal_spi_master.h. */
#endif

#ifdef HAL_ISINK_MODULE_ENABLED
#define HAL_ISINK_FEATURE_ADVANCED_CONFIG           /*Enable ISINK advance config feature*/
#define HAL_ISINK_FEATURE_HW_PMIC2562               /*Enable New ISINK  hardware feature*/
#endif


#ifdef HAL_GPIO_MODULE_ENABLED
#define HAL_GPIO_FEATURE_PUPD               /* Pull state of the pin can be configured with different resistors through different combinations of GPIO_PUPD_x,GPIO_RESEN0_x and GPIO_RESEN1_x. For more details, please refer to hal_gpio.h. */
#define HAL_GPIO_FEATURE_CLOCKOUT           /* The pin can be configured as an output clock. For more details, please refer to hal_gpio.h. */
#define HAL_GPIO_FEATURE_HIGH_Z             /* The pin can be configured to provide high impedance state to prevent possible electric leakage. For more details, please refer to hal_gpio.h. */
#define HAL_GPIO_FEATURE_SET_DRIVING        /* The pin can be configured to enhance driving. For more details, please refer to hal_gpio.h. */
#define HAL_GPIO_FEATURE_SET_SCHMITT        /* The pin can be configured to enhance schmitt trigger hysteresis. */
#define HAL_GPIO_FEATURE_SET_SLEW_RATE      /* The pin can be configured to enhance slew rate. */
#define HAL_GPIO_FEATURE_SET_CAPACITANCE    /* The pin can be configured to enhance capacitance. For more details, please refer to hal_gpio.h. */
#endif

#ifdef HAL_EINT_MODULE_ENABLED
#define HAL_EINT_FEATURE_MASK                /* Supports EINT mask interrupt. */
#define HAL_EINT_FEATURE_SW_TRIGGER_EINT     /* Supports software triggered EINT interrupt. */
// #define HAL_EINT_FEATURE_MUX_MAPPING         /* Supports EINT number mux to different EINT GPIO pin. */
#endif

#ifdef HAL_ESC_MODULE_ENABLED
#define HAL_ESC_SUPPORT_FLASH                  /* Supports ESC with Flash. */
#define HAL_ESC_SUPPORT_PSRAM                  /* Supports ESC with PSRAM. */
#endif

#ifdef HAL_GPT_MODULE_ENABLED
#define HAL_GPT_FEATURE_US_TIMER               /* Supports a microsecond timer. */
#define HAL_GPT_SW_GPT_FEATURE                 /* Supports software GPT timer. */
#define HAL_GPT_PORT_ALLOCATE                  /* Allocates GPT communication port. */
#define HAL_GPT_SW_GPT_US_FEATURE              /* Supports software GPT us timer. */
#define HAL_GPT_SW_FEATURE_ABSOLUTE_COUNT       /* Supports software GPT absolute count in software GPT timeline */
#endif

#ifdef HAL_PWM_MODULE_ENABLED
#define HAL_PWM_FEATURE_ADVANCED_CONFIG        /* Supports PWM advanced configuration. */
#endif

#ifdef HAL_RTC_MODULE_ENABLED
#define HAL_RTC_FEATURE_TIME_CALLBACK           /* Supports time change notification callback. */
#define HAL_RTC_FEATURE_RTC_MODE                /* Supports enter RTC mode. */
#define HAL_RTC_FEATURE_GPIO                    /* Supports RTC GPIO configuration. */
#define HAL_RTC_FEATURE_GPIO_EINT               /* Supports RTC GPIO and EINT configuration. */
#define HAL_RTC_FEATURE_CAPTOUCH                /* Supports CAPTOUCH configuration. */
#define HAL_RTC_FEATURE_EINT                    /* Supports EINT configuration. */
#define HAL_RTC_FEATURE_ALARM_BY_SECOND         /* Supports set rtc alarm by second. */
#define HAL_RTC_FEATURE_POWER_REASON            /* Supports get rtc power on reason. */
#ifdef HAL_CAPTOUCH_MODULE_ENABLED
#define HAL_CPT_FEATURE_4CH                     /* Supports CAPTOUCH 4channel configuration. */
//#define HAL_CPT_FEATURE_8CH                     /* Supports CAPTOUCH 8channel configuration. */
#endif

#endif

#ifdef HAL_PWM_MODULE_ENABLED
#define HAL_PWM_FEATURE_FREQUENCY_DUTY         /* Supports PWM set frequency & duty with one api. */
#define HAL_PWM_FEATURE_ADVANCED_CONFIG        /* Supports PWM advanced configuration. */
#define HAL_PWM_CLOCK_32K_SUPPORTED            /* Supports 32K PWM source clock. */
#define HAL_PWM_CLOCK_26M_SUPPORTED            /* Supports 26M PWM source clock. */
#define HAL_PWM_CLOCK_39M_SUPPORTED            /* Supports 39M PWM source clock. */
#define HAL_PWM_CLOCK_86M_SUPPORTED            /* Supports 86.6M PWM source clock. */
#endif


#ifdef HAL_SPI_SLAVE_MODULE_ENABLED
#define HAL_SPI_SLAVE_FEATURE_SW_CONTROL        /* Supports SPI slave to communicate with SPI master using software control. */
#define HAL_SPI_SLAVE_FEATURE_DIRECT_MODE       /* Supports SPI slave to communicate with SPI master without using software control. */
#define HAL_SPI_SLAVE_FEATURE_BYPASS            /* Supports SPI slave bypass feature. */
#endif


#ifdef HAL_UART_MODULE_ENABLED
#define HAL_UART_FEATURE_VFIFO_DMA_TIMEOUT        /* Supports configurable timeout value setting */
#define HAL_UART_FEATURE_3M_BAUDRATE              /* Supports UART 3M baudrate setting */
#define HAL_UART_FEATURE_6M_BAUDRATE              /* Supports UART 6M baudrate setting */
#define HAL_UART_FEATURE_8_6M_BAUDRATE            /* Supports UART 8.6M baudrate setting */
#define HAL_UART_FEATURE_FLOWCONTROL_CALLBACK      /* Supports enable UART flow control interrupt setting */
#endif

#ifdef HAL_AES_MODULE_ENABLED
#define HAL_AES_USE_PHYSICAL_MEMORY_ADDRESS        /* Notify caller must use physical memory */
#define HAL_AES_FEATURE_HWKEY_ENABLED              /* Supports HWKEY in AES module */
#define HAL_AES_FEATURE_CKDFKEY_ENABLED              /* Supports HWKEY in AES module */
#endif

#ifdef HAL_SHA_MODULE_ENABLED
#define HAL_SHA_USE_PHYSICAL_MEMORY_ADDRESS        /* Notify caller must use physical memory */
#endif

#ifdef HAL_SDIO_MODULE_ENABLED
#define HAL_SDIO_FEATURE_DATA1_IRQ                                  /*enable SDIO DAT1 IRQ feature*/
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
#define HAL_SECURITY_OTP_FEATURE_ENABLE            /* Supports OTP feature read*/
#define HAL_SECURITY_OTP_WRITE_FEATURE_ENABLE      /* Supports OTP feature write. */
#endif

#ifdef HAL_SLEEP_MANAGER_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup SLEEP_MANAGER
 * @{
 * @addtogroup hal_sleep_manager_enum Enum
 * @{
 */
/*****************************************************************************
 * Enum
 *****************************************************************************/
/** @brief Sleep modes */
typedef enum {
    HAL_SLEEP_MODE_NONE = 0,        /**< No sleep. */
    HAL_SLEEP_MODE_IDLE,            /**< Idle state. */
    HAL_SLEEP_MODE_SLEEP,           /**< Sleep state. */
    HAL_SLEEP_MODE_NUMBER           /**< To support range detection. */
}
hal_sleep_mode_t;
/** @brief sleep_manager wake up source */
typedef enum {
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_GPT                  = 0,           /**< General purpose timer. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_GPT_SEC              = 1,           /**< Secure general purpose timer. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_EINT                 = 2,           /**< External interrupt. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_IRQGEN               = 3,           /**< IRQGEN. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_OST                  = 4,           /**< OST. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_AUDIO                = 5,           /**< AUDIO. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_HW_SRC1              = 6,           /**< HW_SRC1. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_HW_SRC2              = 7,           /**< HW_SRC2. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_HW_SRC3              = 8,           /**< HW_SRC3. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_VOW_SNR              = 9,           /**< VOW_SNR. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_VOW_DMA              = 10,          /**< VOW_DMA. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_VAD                  = 11,          /**< VAD. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ANC_RAMP_DOWN        = 12,          /**< ANC_RAMP_DOWN. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ANC_DMA              = 13,          /**< ANC_DMA */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_I2S                  = 14,          /**< I2S. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_CAPTOUCH             = 15,          /**< CAP_TOUCH. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_SPI_SLA              = 16,          /**< SPI_SLAVE. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_RGU                  = 17,          /**< RGU. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_INFRA_SEC_VIO        = 18,          /**< INFRA_SEC_VIO. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_CONNSYS_GPT          = 19,          /**< CONNSYS_GPT. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_CONNSYS_BT_TIMER     = 20,          /**< CONNSYS_BT_TIMER. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_CONNSYS_AURX         = 21,          /**< CONNSYS_AURX. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_CONNSYS_AUTX         = 22,          /**< CONNSYS_AUTX. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_CONNSYS_BT_INT       = 23,          /**< CONNSYS_BT_INT. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_COLIASSYS_MBX_TX       = 24,          /**< COLIASSYS_MBX_TX. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_COLIASSYS_MBX_RX       = 25,          /**< COLIASSYS_MBX_RX. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_SYSRAM_DRAM          = 26,          /**< SYSRAM_DRAM. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_I3C0                 = 27,          /**< I3C0. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_I3C1                 = 28,          /**< I3C1. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_INFRA_BUS_ERR        = 29,          /**< INFRA_BUS_ERR. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_CONNSYS_BT_PLAYEN    = 30,          /**< CONNSYS_BT_PLAYEN. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ANC                  = 31,          /**< ANC. */
    HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ALL                  = 32,          /**< All wakeup source. */
} hal_sleep_manager_wakeup_source_t;
/**
 * @}
 * @}
 * @}
 */
#endif

#ifdef HAL_UART_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup UART
 * @{
 * @addtogroup hal_uart_enum Enum
 * @{
 */
/*****************************************************************************
* UART
*****************************************************************************/
/** @brief UART port index
 * There are total of four UART ports. Only UART0 and UART1 support hardware flow control.
 * | UART port | Hardware Flow Control |
 * |-----------|-----------------------|
 * |  UART0    |           V           |
 * |  UART1    |           V           |
 * |  UART2    |           X           |
 * |  UART3    |           X           |
 */
typedef enum {
    HAL_UART_0 = 0,                            /**< UART port 0. */
    HAL_UART_1 = 1,                            /**< UART port 1. */
    HAL_UART_2 = 2,                            /**< UART port 2. */
    HAL_UART_MAX                               /**< The total number of UART ports (invalid UART port number). */
} hal_uart_port_t;

/**
  * @}
  */

/**@addtogroup hal_uart_define Define
 * @{
  */

/** @brief  The maximum timeout value for UART timeout interrupt, unit is ms.
  */
#define HAL_UART_TIMEOUT_VALUE_MAX  (2500)

/**
  * @}
  */

/**
 * @}
 * @}
 */
#endif


#ifdef HAL_I2C_MASTER_MODULE_ENABLED
#ifdef HAL_I2C_MASTER_FEATURE_EXTENDED_DMA
/**
 * @addtogroup HAL
 * @{
 * @addtogroup I2C_MASTER
 * @{
 * @section HAL_I2C_Transaction_Pattern_Chapter API transaction length and packets
 *  The Transaction packet is a packet sent by the I2C master using SCL and SDA.
 *  Different APIs have different transaction packets, as shown below.
 * - \b Transaction \b length \b supported \b by \b the \b APIs \n
 *  The total transaction length is determined by 4 parameters:\n
 *  send_packet_length(Ns), which indicates the number of sent packet.\n
 *  send_bytes_in_one_packet(Ms).\n
 *  receive_packet_length(Nr).\n
 *  receive_bytes_in_one_packet(Mr).\n
 *  Next, the relationship between transaction packet and these 4 parameters is introduced.
 *  - Total transaction length = Ns * Ms + Nr * Mr.
 *   - Ns is the packet length to be sent by the I2C master.
 *   - Ms is the total number of bytes in a sent packet.
 *   - Nr is the packet length to be received by the I2C master.
 *   - Mr is the total number of bytes in a received packet.
 *  - NA means the related parameter should be ignored.
 *  - 1~8 specifies the parameter range is from 1 to 8. 1~65535 specifies the parameter range is from 1 to 65535.
 *  - 1 means the parameter value can only be 1.
 *  - Note1: functions with the suffix "_ex" have these 4 parameters. Other functions only have the "size" parameter and the driver splits the "size" into these 4 parameters.
 *  - Note2: The maximum of total transaction length is 4G.\n
 *    #hal_i2c_master_send_polling() for example, the "size" will be divided like this: send_packet_length = 1, send_bytes_in_one_packet = size.
 *          As a result, the total size should be: send_packet_length * send_bytes_in_one_packet = 1 * size = size. The range of "size" should be from 1 to 8.
 * |API                                         |send_packet_length(Ns) | send_bytes_in_one_packet(Ms) | receive_packet_length(Nr) | receive_bytes_in_one_packet(Mr) |
 * |--------------------------------------------|-----------------------|------------------------------|---------------------------|---------------------------------|
 * |hal_i2c_master_send_polling                 |          1            |            1~8               |            NA             |                NA               |
 * |hal_i2c_master_receive_polling              |          NA           |            NA                |            1              |                1~8              |
 * |hal_i2c_master_send_to_receive_polling      |          1            |            1~8               |            1              |                1~8              |
 * |hal_i2c_master_send_dma                     |          1            |            1~65535           |            NA             |                NA               |
 * |hal_i2c_master_receive_dma                  |          NA           |            NA                |            1              |                1~65535          |
 * |hal_i2c_master_send_to_receive_dma          |          1            |            1~65535           |            1              |                1~65534          |
 * |hal_i2c_master_send_dma_ex                  |          1~65535      |            1~65535           |            NA             |                NA               |
 * |hal_i2c_master_receive_dma_ex               |          NA           |            NA                |            1~65535        |                1~65535          |
 * |hal_i2c_master_send_to_receive_dma_ex       |          1            |            1~65535           |            1              |                1~65534          |
 *
 * - \b Waveform \b pattern \b supported \b by \b the \b APIs \n
 *  The 4 parameters (send_packet_length(Ns), send_bytes_in_one_packet(Ms), receive_packet_length(Nr), receive_bytes_in_one_packet(Mr) will also affect the transaction packet.
 *  The relationship between transaction packet and these 4 parameters is shown below.
 *  - Ns is the send_packet_length.
 *  - Ms is the send_bytes_in_one_packet.
 *  - Nr is the receive_packet_length.
 *  - Mr is the receive_bytes_in_one_packet.
 * |API                                          |transaction packet format                                 |
 * |---------------------------------------------|----------------------------------------------------------|
 * | hal_i2c_master_send_polling                 |  @image html hal_i2c_send_poling_waveform.png            |
 * | hal_i2c_master_receive_polling              |  @image html hal_i2c_receive_poling_waveform.png         |
 * | hal_i2c_master_send_to_receive_polling      |  @image html hal_i2c_send_to_receive_poling_waveform.png |
 * | hal_i2c_master_send_dma                     |  @image html hal_i2c_send_dma_waveform.png            |
 * | hal_i2c_master_receive_dma                  |  @image html hal_i2c_receive_dma_waveform.png         |
 * | hal_i2c_master_send_to_receive_dma          |  @image html hal_i2c_send_to_receive_dma_waveform.png |
 * | hal_i2c_master_send_dma_ex                  |  @image html hal_i2c_send_dma_ex_waveform.png            |
 * | hal_i2c_master_receive_dma_ex               |  @image html hal_i2c_receive_dma_ex_waveform.png         |
 * | hal_i2c_master_send_to_receive_dma_ex       |  @image html hal_i2c_send_to_receive_dma_ex_waveform.png |
 *
 *
 *
 *
 */
#endif

/** @defgroup hal_i2c_master_define Define
 * @{
  */

/** @brief  The maximum polling mode transaction size.
  */
#define HAL_I2C_MAXIMUM_POLLING_TRANSACTION_SIZE  8

/** @brief  The maximum DMA mode transaction size.
  */
#define HAL_I2C_MAXIMUM_DMA_TRANSACTION_SIZE   65535

/**
  * @}
  */

/** @addtogroup hal_i2c_master_enum Enum
  * @{
  */

/*****************************************************************************
* I2C master
*****************************************************************************/
/** @brief This enum defines the I2C port.
 *  The platform supports 4 I2C masters. Three of them support polling mode and DMA mode,
 *  while the other only supports polling mode. For more information about the polling mode,
 *  DMA mode, queue mode, please refer to @ref HAL_I2C_Features_Chapter. The details
 *  are shown below:
 *  - Supported features of I2C masters \n
 *    V : supported;  X : not supported.
 * |I2C Master   | Polling mode | DMA mode | Extended DMA mode |
 * |-------------|--------------|----------|-------------------|
 * |I2C0         |      V       |    V     |         V         |
 * |I2C1         |      V       |    V     |         V         |
 * |I2C2         |      V       |    V     |         V         |
 * |I2CAO        |      X       |    X     |         X         |
 *
 *
*/
typedef enum {
    HAL_I2C_MASTER_0 = 0,                /**< I2C master 0. */
    HAL_I2C_MASTER_1 = 1,                /**< I2C master 1. */
    HAL_I2C_MASTER_2 = 2,                /**< I2C master 2. */
    HAL_I2C_MASTER_AO = 3,               /**< I2C master AO. */
    HAL_I2C_MASTER_MAX                   /**< The total number of I2C masters (invalid I2C master number). */
} hal_i2c_port_t;

/**
  * @}
  */

/**
 * @}
 * @}
 */
#endif


#ifdef HAL_I3C_MASTER_MODULE_ENABLED
/**
  * @addtogroup HAL
  * @{
  * @addtogroup I3C_MASTER
  * @{
*/
/** @addtogroup hal_i3c_master_enum Enum
  * @{
  */

/*****************************************************************************
* I3C master
*****************************************************************************/
/** @brief This enum defines the I3C port.
 *
 *
*/
typedef enum {
    HAL_I3C_MASTER_0 = 0,                /**< I3C master 0. */
    HAL_I3C_MASTER_1 = 1,                /**< I3C master 1. */
    HAL_I3C_MASTER_MAX                   /**< The total number of I3C masters (invalid I3C master number). */
} hal_i3c_master_port_t;

/**
  * @}
  */

/**
 * @}
 * @}
 */

#endif




#ifdef HAL_GPIO_MODULE_ENABLED
/**
* @addtogroup HAL
* @{
* @addtogroup GPIO
* @{
*
* @addtogroup hal_gpio_enum Enum
* @{
*/

/*****************************************************************************
* GPIO
*****************************************************************************/
/** @brief This enum defines the GPIO port.
 *  The platform supports a total of 49 GPIO pins with various functionality.
 *
*/

typedef enum {
    HAL_GPIO_0  = 0,    /**< GPIO pin0. */
    HAL_GPIO_1  = 1,    /**< GPIO pin1. */
    HAL_GPIO_2  = 2,    /**< GPIO pin2. */
    HAL_GPIO_3  = 3,    /**< GPIO pin3. */
    HAL_GPIO_4  = 4,    /**< GPIO pin4. */
    HAL_GPIO_5  = 5,    /**< GPIO pin5. */
    HAL_GPIO_6  = 6,    /**< GPIO pin6. */
    HAL_GPIO_7  = 7,    /**< GPIO pin7. */
    HAL_GPIO_8  = 8,    /**< GPIO pin8. */
    HAL_GPIO_9  = 9,    /**< GPIO pin9. */
    HAL_GPIO_10 = 10,   /**< GPIO pin10. */
    HAL_GPIO_11 = 11,   /**< GPIO pin11. */
    HAL_GPIO_12 = 12,   /**< GPIO pin12. */
    HAL_GPIO_13 = 13,   /**< GPIO pin13. */
    HAL_GPIO_14 = 14,   /**< GPIO pin14. */
    HAL_GPIO_15 = 15,   /**< GPIO pin15. */
    HAL_GPIO_16 = 16,   /**< GPIO pin16. */
    HAL_GPIO_17 = 17,   /**< GPIO pin17. */
    HAL_GPIO_18 = 18,   /**< GPIO pin18. */
    HAL_GPIO_19 = 19,   /**< GPIO pin19. */
    HAL_GPIO_20 = 20,   /**< GPIO pin20. */
    HAL_GPIO_21 = 21,   /**< GPIO pin21. */
    HAL_GPIO_22 = 22,   /**< GPIO pin22. */
    HAL_GPIO_23 = 23,   /**< GPIO pin23. */
    HAL_GPIO_24 = 24,   /**< GPIO pin24. */
    HAL_GPIO_25 = 25,   /**< GPIO pin25. */
    HAL_GPIO_26 = 26,   /**< GPIO pin26. */
    HAL_GPIO_27 = 27,   /**< GPIO pin27. */
    HAL_GPIO_28 = 28,   /**< GPIO pin28. */
    HAL_GPIO_29 = 29,   /**< GPIO pin29. */
    HAL_GPIO_30 = 30,   /**< GPIO pin30. */
    HAL_GPIO_31 = 31,   /**< GPIO pin31. */
    HAL_GPIO_32 = 32,   /**< GPIO pin32. */
    HAL_GPIO_33 = 33,   /**< GPIO pin33. */
    HAL_GPIO_34 = 34,   /**< GPIO pin34. */
    HAL_GPIO_35 = 35,   /**< GPIO pin35. */
    HAL_GPIO_36 = 36,   /**< GPIO pin36. */
    HAL_GPIO_37 = 37,   /**< GPIO pin37. */
    HAL_GPIO_38 = 38,   /**< GPIO pin38. */
    HAL_GPIO_39 = 39,   /**< GPIO pin39. */
    HAL_GPIO_40 = 40,   /**< GPIO pin40. */
    HAL_GPIO_41 = 41,   /**< GPIO pin41. */
    HAL_GPIO_42 = 42,   /**< GPIO pin42. */
    HAL_GPIO_43 = 43,   /**< GPIO pin43. */
    HAL_GPIO_44 = 44,   /**< GPIO pin44. */
    HAL_GPIO_45 = 45,   /**< GPIO pin45. */
    HAL_GPIO_46 = 46,   /**< GPIO pin46. */
    HAL_GPIO_47 = 47,   /**< GPIO pin47. */
    HAL_GPIO_48 = 48,   /**< GPIO pin48. */
    HAL_GPIO_49 = 49,   /**< GPIO pin49. */
    HAL_GPIO_50 = 50,   /**< GPIO pin50. */
    HAL_GPIO_51 = 51,   /**< GPIO pin51. */
    HAL_GPIO_52 = 52,   /**< GPIO pin52. */
    HAL_GPIO_53 = 53,   /**< GPIO pin53. */
    HAL_GPIO_54 = 54,   /**< GPIO pin54. */
    HAL_GPIO_55 = 55,   /**< GPIO pin55. */
    HAL_GPIO_56 = 56,   /**< GPIO pin56. */
    HAL_GPIO_57 = 57,   /**< GPIO pin57. */
    HAL_GPIO_MAX                               /**< The total number of GPIO pins (invalid GPIO pin number). */
} hal_gpio_pin_t;

/**
  * @}
  */

/**
 * @}
 * @}
 */
#endif

#ifdef HAL_GPIO_FEATURE_CLOCKOUT
/**
 * @addtogroup HAL
 * @{
 * @addtogroup GPIO
 * @{
 * @addtogroup hal_gpio_enum Enum
 * @{
 */
/*****************************************************************************
* CLKOUT
*****************************************************************************/
/** @brief  This enum defines output clock number of GPIO */
typedef enum {
    HAL_GPIO_CLOCK_0   = 0,              /**< define GPIO output clock 0 */
    HAL_GPIO_CLOCK_1   = 1,              /**< define GPIO output clock 1 */
    HAL_GPIO_CLOCK_2   = 2,              /**< define GPIO output clock 2 */
    HAL_GPIO_CLOCK_3   = 3,              /**< define GPIO output clock 3 */
    HAL_GPIO_CLOCK_MAX                   /**< define GPIO output clock max number(invalid) */
} hal_gpio_clock_t;


/** @brief This enum defines output clock mode of GPIO */
typedef enum {
    HAL_GPIO_CLOCK_MODE_FRTC_32K  = 1,        /**< define GPIO output clock mode as 32KHz */
    HAL_GPIO_CLOCK_MODE_FXO_26M   = 2,        /**< define GPIO output clock mode as 26MHz */
    HAL_GPIO_CLOCK_MODE_FXOD2_13M = 3,        /**< define GPIO output clock mode as 13MHz */
    HAL_GPIO_CLOCK_MODE_FCHOP     = 8,        /**< define GPIO output clock mode FCHOP, the specific frequency is decided by chop divider. */
    HAL_GPIO_CLOCK_MODE_MAX             /**< define GPIO output clock mode of max number(invalid) */
} hal_gpio_clock_mode_t;
/**
  * @}
  */

/**
 * @}
 * @}
 */
#endif

#ifdef HAL_GPIO_FEATURE_SET_DRIVING
/**
 * @addtogroup HAL
 * @{
 * @addtogroup GPIO
 * @{
 * @addtogroup hal_gpio_enum Enum
 * @{
 */
/** @brief This enum defines driving current. */
typedef enum {
    HAL_GPIO_DRIVING_CURRENT_2MA  = 0,   /**< Defines GPIO driving current as 2mA.  */
    HAL_GPIO_DRIVING_CURRENT_4MA  = 1,   /**< Defines GPIO driving current as 4mA.  */
    HAL_GPIO_DRIVING_CURRENT_6MA  = 2,   /**< Defines GPIO driving current as 6mA. */
    HAL_GPIO_DRIVING_CURRENT_8MA  = 3,   /**< Defines GPIO driving current as 8mA. */
    HAL_GPIO_DRIVING_CURRENT_10MA = 4,   /**< Defines GPIO driving current as 10mA. */
    HAL_GPIO_DRIVING_CURRENT_12MA = 5,   /**< Defines GPIO driving current as 12mA. */
    HAL_GPIO_DRIVING_CURRENT_14MA = 6,   /**< Defines GPIO driving current as 14mA. */
    HAL_GPIO_DRIVING_CURRENT_16MA = 7,   /**< Defines GPIO driving current as 16mA. */
    HAL_GPIO_DRIVING_CURRENT_MAX = 8,   /**< The total number of GPIO driving current level (invalid GPIO GPIO driving current level). */
} hal_gpio_driving_current_t;
/**
  * @}
  */

/**
 * @}
 * @}
 */
#endif

#ifdef HAL_GPIO_FEATURE_SET_CAPACITANCE
/**
 * @addtogroup HAL
 * @{
 * @addtogroup GPIO
 * @{
 * @addtogroup hal_gpio_enum Enum
 * @{
 */

/** @brief This enum defines capacitance level for some specific GPIOs. */
typedef enum {
    HAL_GPIO_CAPACITANCE_0, /**< Defines GPIO capacitance level 0. */
    HAL_GPIO_CAPACITANCE_1, /**< Defines GPIO capacitance level 1. */
    HAL_GPIO_CAPACITANCE_2, /**< Defines GPIO capacitance level 2. */
    HAL_GPIO_CAPACITANCE_3, /**< Defines GPIO capacitance level 3. */
    HAL_GPIO_CAPACITANCE_MAX, /**< The total number of GPIO capacitance level (invalid GPIO capacitance level). */
} hal_gpio_capacitance_t;

/**
  * @}
  */

/**
 * @}
 * @}
 */

#endif

#ifdef HAL_ADC_MODULE_ENABLED

/**
* @addtogroup HAL
* @{
* @addtogroup ADC
* @{
*
* @addtogroup hal_adc_enum Enum
* @{
*/

/*****************************************************************************
* ADC
*****************************************************************************/
/** @brief adc channel */
typedef enum {
    HAL_ADC_CHANNEL_0 = 0,                        /**< ADC channel 0. */
    HAL_ADC_CHANNEL_1 = 1,                        /**< ADC channel 1. */
    HAL_ADC_CHANNEL_2 = 2,                        /**< ADC channel 2. */
    HAL_ADC_CHANNEL_3 = 3,                        /**< ADC channel 3. */
    HAL_ADC_CHANNEL_4 = 4,                        /**< ADC channel 4. */
    HAL_ADC_CHANNEL_5 = 5,                        /**< ADC channel 5. */
    HAL_ADC_CHANNEL_6 = 6,                        /**< ADC channel 6. */
    HAL_ADC_CHANNEL_7 = 7,                        /**< ADC channel 7. */
    HAL_ADC_CHANNEL_MAX                           /**< The total number of ADC channels (invalid ADC channel).*/
} hal_adc_channel_t;

/**
  * @}
  */


/**
 * @}
 * @}
 */
#endif



#ifdef HAL_I2S_MODULE_ENABLED
/**
* @addtogroup HAL
* @{
* @addtogroup I2S
* @{
*
* @addtogroup hal_i2s_enum Enum
* @{
*/


/*****************************************************************************
* I2S
*****************************************************************************/
#ifdef HAL_I2S_FEATURE_MULTI_I2S
/** @brief This enum defines the I2S port.
 *
 *  The platform supports 2 I2S HW interface. Two of them support master mode and slave mode.
 *  User can drive 2 I2S HW simultaneously by the extended APIs. The basic APIs are only for I2S0.
 *  User should not use basic APIs and extension APIs simultaneously. The details are shown below:
 *
 *  - I2S supported feature table \n
 *    V : means support;  X : means not support.
 *
 * |I2S PORT   |SAMPLE WIDTH    | FS | I2S TDM  |
 * |--------- |--------------|-------------|-------------------|
 * |I2S0         | 16 bits                 |11.025 / 16 / 22.05 / 24 / 44.1 /48 /96 / 192 KHZ     | V        |
 * |I2S1         | 16 / 24 bits         |11.025 / 16 / 22.05 / 24 / 44.1 /48 /96 / 192 KHZ     | X        |
 *
*/
typedef enum {
    HAL_I2S_0  = 0,   /**< I2S interfeace 0. */
    HAL_I2S_1  = 1,    /**< I2S interfeace 1. */
    HAL_I2S_MAX
} hal_i2s_port_t;
#endif


#ifdef HAL_I2S_FEATURE_TDM
/** @brief Channels per frame sync. Number of channels in each frame sync.*/
typedef enum {
    HAL_I2S_TDM_2_CHANNEL  = 0,   /**< 2 channels. */
    HAL_I2S_TDM_4_CHANNEL  = 1    /**< 4 channels. */
} hal_i2s_tdm_channel_t;


/** @brief Polarity of BCLK.*/
typedef enum {
    HAL_I2S_BCLK_INVERSE_DISABLE  = 0, /**< Normal mode. (Invalid)*/
    HAL_I2S_BCLK_INVERSE_EABLE  = 1    /**< Invert BCLK. (Invalid)*/
} hal_i2s_bclk_inverse_t;
#endif

#ifdef HAL_I2S_EXTENDED
/** @brief I2S sample widths.  */
typedef enum {
    HAL_I2S_SAMPLE_WIDTH_8BIT  = 0,   /**< I2S sample width is 8bit. (Invalid)*/
    HAL_I2S_SAMPLE_WIDTH_16BIT = 1,   /**< I2S sample width is 16bit. (HAL_I2S_0)*/
    HAL_I2S_SAMPLE_WIDTH_24BIT = 2    /**< I2S sample width is 24bit. (HAL_I2S_0/HAL_I2S_1)*/
} hal_i2s_sample_width_t;


/** @brief Number of bits per frame sync(FS). This parameter determines the bits of a complete sample of both left and right channels.*/
typedef enum {
    HAL_I2S_FRAME_SYNC_WIDTH_32  = 0,   /**< 32 bits per frame. */
    HAL_I2S_FRAME_SYNC_WIDTH_64  = 1,   /**< 64 bits per frame. */
    HAL_I2S_FRAME_SYNC_WIDTH_128  = 2   /**< 128 bits per frame. */
} hal_i2s_frame_sync_width_t;


/** @brief Enable or disable right channel of the I2S TX to send the same data as on the left channel of the I2S TX.\n
        This function only works when the sample width of the I2S is 16 bits.*/
typedef enum {
    HAL_I2S_TX_MONO_DUPLICATE_DISABLE = 0,  /**< Keep data to its channel. */
    HAL_I2S_TX_MONO_DUPLICATE_ENABLE  = 1   /**< Assume input is mono data for left channel, the data is duplicated to right channel.*/
} hal_i2s_tx_mode_t;


/** @brief Enable or disable twice the downsampling rate mode in the I2S RX.
                 In this mode the sampling rate of the I2S TX is 48kHz while the sampling rate of the I2S RX is 24kHz. The I2S RX automatically drops 1 sample in each 2 input samples received. */
typedef enum {
    HAL_I2S_RX_DOWN_RATE_DISABLE = 0,  /**< Actual sampling rate of the I2S RX = sampling rate. (Default)*/
    HAL_I2S_RX_DOWN_RATE_ENABLE  = 1   /**< Actual sampling rate of the I2S RX is half of the original sampling rate. (Invalid)*/
} hal_i2s_rx_down_rate_t;
#endif //  #ifdef HAL_I2S_EXTENDED


/** @brief Enable or disable data swapping between right and left channels of the I2S link.\n
        This function only works when the sample width of the I2S is 16 bits.*/
typedef enum {
    HAL_I2S_LR_SWAP_DISABLE = 0,  /**< Disable the data swapping. */
    HAL_I2S_LR_SWAP_ENABLE  = 1   /**< Enable the data swapping.  */
} hal_i2s_lr_swap_t;


/** @brief Enable or disable word select clock inverting of the I2S link. */
typedef enum {
    HAL_I2S_WORD_SELECT_INVERSE_DISABLE = 0,  /**< Disable the word select clock inverting. */
    HAL_I2S_WORD_SELECT_INVERSE_ENABLE  = 1   /**< Enable the word select clock inverting.  */
} hal_i2s_word_select_inverse_t;

/** @brief This enum defines initial type of the I2S.
 */

typedef enum {
    HAL_I2S_TYPE_EXTERNAL_MODE          = 0,        /**< External mode. I2S mode with external codec.*/
    HAL_I2S_TYPE_EXTERNAL_TDM_MODE      = 1,        /**< External TDM mode. I2S TDM mode with external codec*/
    HAL_I2S_TYPE_INTERNAL_MODE          = 2,        /**< Internal mode. I2S mode with internal codec. (Invalid)*/
    HAL_I2S_TYPE_INTERNAL_LOOPBACK_MODE = 3,        /**< I2S internal loopback mode. Data loopback mode.*/
    HAL_I2S_TYPE_INTERNAL_TDM_LOOPBACK_MODE = 4,    /**< TDM internal loopback mode. Data loopback mode.*/
    HAL_I2S_TYPE_MAX = 5
} hal_i2s_initial_type_t;


/** @brief I2S event */
typedef enum {
    HAL_I2S_EVENT_ERROR               = -1, /**<  An error occurred during the function call. */
    HAL_I2S_EVENT_NONE                =  0, /**<  No error occurred during the function call. */
    HAL_I2S_EVENT_OVERFLOW            =  1, /**<  RX data overflow. */
    HAL_I2S_EVENT_UNDERFLOW           =  2, /**<  TX data underflow. */
    HAL_I2S_EVENT_DATA_REQUEST        =  3, /**<  Request for user-defined data. */
    HAL_I2S_EVENT_DATA_NOTIFICATION   =  4  /**<  Notify user the RX data is ready. */
} hal_i2s_event_t;


/** @brief I2S sampling rates */
typedef enum {
    HAL_I2S_SAMPLE_RATE_8K        = 0,  /**<  8000Hz  */
    HAL_I2S_SAMPLE_RATE_11_025K   = 1,  /**<  11025Hz */
    HAL_I2S_SAMPLE_RATE_12K       = 2,  /**<  12000Hz */
    HAL_I2S_SAMPLE_RATE_16K       = 3,  /**<  16000Hz */
    HAL_I2S_SAMPLE_RATE_22_05K    = 4,  /**<  22050Hz */
    HAL_I2S_SAMPLE_RATE_24K       = 5,  /**<  24000Hz */
    HAL_I2S_SAMPLE_RATE_32K       = 6,  /**<  32000Hz */
    HAL_I2S_SAMPLE_RATE_44_1K     = 7,  /**<  44100Hz */
    HAL_I2S_SAMPLE_RATE_48K       = 8,  /**<  48000Hz */
    HAL_I2S_SAMPLE_RATE_88_2K     = 9,  /**<  88200Hz */
    HAL_I2S_SAMPLE_RATE_96K       = 10, /**<  96000Hz */
    HAL_I2S_SAMPLE_RATE_176_4K    = 11, /**<  176400Hz */
    HAL_I2S_SAMPLE_RATE_192K      = 12  /**<  192000Hz */
} hal_i2s_sample_rate_t;

/**
  * @}
  */

/**
 * @}
 * @}
 */
#endif


#ifdef HAL_SPI_MASTER_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup SPI_MASTER
 * @{
 * @defgroup hal_spi_master_define Define
 * @{
 */

/** @brief  The maximum polling mode transaction size in bytes.
 */
#define HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE  32

/** @brief  The maximum transaction size in bytes when configuration is not single mode.
 */
#define HAL_SPI_MAXIMUM_NON_SINGLE_MODE_TRANSACTION_SIZE  15

/** @brief  The minimum clock frequency.
 */
#define  HAL_SPI_MASTER_CLOCK_MIN_FREQUENCY  30000

/** @brief  The maximum clock frequency.
 */
#define  HAL_SPI_MASTER_CLOCK_MAX_FREQUENCY  52000000

/**
 * @}
 */

/**
 * @addtogroup hal_spi_master_enum Enum
 * @{
 */

/*****************************************************************************
* SPI master
*****************************************************************************/
/** @brief This enum defines the SPI master port.
 *  The chip supports total of 4 SPI master ports, each of them supports polling mode
 *  and DMA mode. For more details about polling mode and DMA mode, please refer to @ref
 *  HAL_SPI_MASTER_Features_Chapter.
 */
typedef enum {
    HAL_SPI_MASTER_0 = 0,                              /**< SPI master port 0. */
    HAL_SPI_MASTER_1 = 1,                              /**< SPI master port 1. */
    HAL_SPI_MASTER_2 = 2,                              /**< SPI master port 2. */
    HAL_SPI_MASTER_MAX                                 /**< The total number of SPI master ports (invalid SPI master port). */
} hal_spi_master_port_t;

/** @brief This enum defines the options to connect the SPI slave device to the SPI master's CS pins. */
typedef enum {
    HAL_SPI_MASTER_SLAVE_0 = 0,                       /**< The SPI slave device is connected to the SPI master's CS0 pin. */
    HAL_SPI_MASTER_SLAVE_1 = 1,                       /**< The SPI slave device is connected to the SPI master's CS1 pin. */
    HAL_SPI_MASTER_SLAVE_2 = 2,                       /**< The SPI slave device is connected to the SPI master's CS2 pin. */
    HAL_SPI_MASTER_SLAVE_3 = 3,                       /**< The SPI slave device is connected to the SPI master's CS3 pin. */
    HAL_SPI_MASTER_SLAVE_MAX                          /**< The total number of SPI master CS pins (invalid SPI master CS pin). */
} hal_spi_master_slave_port_t;

/** @brief SPI master transaction bit order definition. */
typedef enum {
    HAL_SPI_MASTER_LSB_FIRST = 0,                       /**< Both send and receive data transfer LSB first. */
    HAL_SPI_MASTER_MSB_FIRST = 1                        /**< Both send and receive data transfer MSB first. */
} hal_spi_master_bit_order_t;

/** @brief SPI master clock polarity definition. */
typedef enum {
    HAL_SPI_MASTER_CLOCK_POLARITY0 = 0,                     /**< Clock polarity is 0. */
    HAL_SPI_MASTER_CLOCK_POLARITY1 = 1                      /**< Clock polarity is 1. */
} hal_spi_master_clock_polarity_t;

/** @brief SPI master clock format definition. */
typedef enum {
    HAL_SPI_MASTER_CLOCK_PHASE0 = 0,                         /**< Clock format is 0. */
    HAL_SPI_MASTER_CLOCK_PHASE1 = 1                          /**< Clock format is 1. */
} hal_spi_master_clock_phase_t;

/** @brief This enum defines the mode of the SPI master. */
typedef enum {
    HAL_SPI_MASTER_SINGLE_MODE = 0,                      /**< Single mode. */
    HAL_SPI_MASTER_3_WIRE_MODE = 1,                      /**< Normal mode. */
    HAL_SPI_MASTER_DUAL_MODE = 2,                        /**< Dual mode. */
    HAL_SPI_MASTER_QUAD_MODE = 3,                        /**< Quad mode. */
} hal_spi_master_mode_t;

/**
 * @}
 */

/**
 * @}
 * @}
 */
#endif

#ifdef HAL_SPI_SLAVE_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup SPI_SLAVE
 * @{
 * @addtogroup hal_spi_slave_enum Enum
 * @{
 */

/*****************************************************************************
* SPI slave
*****************************************************************************/
/** @brief This enum defines the SPI slave port. This chip supports only one
 *  SPI slave port.
 */
typedef enum {
    HAL_SPI_SLAVE_0 = 0,                             /**< SPI slave port 0. */
    HAL_SPI_SLAVE_MAX                                /**< The total number of SPI slave ports (invalid SPI slave port number). */
} hal_spi_slave_port_t;

/** @brief SPI slave transaction bit order definition. */
typedef enum {
    HAL_SPI_SLAVE_LSB_FIRST = 0,                       /**< Both send and receive data transfer is the LSB first. */
    HAL_SPI_SLAVE_MSB_FIRST = 1                        /**< Both send and receive data transfer is the MSB first. */
} hal_spi_slave_bit_order_t;

/** @brief SPI slave clock polarity definition. */
typedef enum {
    HAL_SPI_SLAVE_CLOCK_POLARITY0 = 0,                 /**< Clock polarity is 0. */
    HAL_SPI_SLAVE_CLOCK_POLARITY1 = 1                  /**< Clock polarity is 1. */
} hal_spi_slave_clock_polarity_t;

/** @brief SPI slave clock format definition. */
typedef enum {
    HAL_SPI_SLAVE_CLOCK_PHASE0 = 0,                    /**< Clock format is 0. */
    HAL_SPI_SLAVE_CLOCK_PHASE1 = 1                     /**< Clock format is 1. */
} hal_spi_slave_clock_phase_t;

/** @brief This enum defines the SPI slave event when an interrupt occurs. */
typedef enum {
    HAL_SPI_SLAVE_EVENT_POWER_ON = SPIS_INT_POWER_ON_MASK,         /**< Power on command is received. */
    HAL_SPI_SLAVE_EVENT_POWER_OFF = SPIS_INT_POWER_OFF_MASK,       /**< Power off command is received. */
    HAL_SPI_SLAVE_EVENT_CRD_FINISH = SPIS_INT_RD_CFG_FINISH_MASK,  /**< Configure read command is received. */
    HAL_SPI_SLAVE_EVENT_RD_FINISH = SPIS_INT_RD_TRANS_FINISH_MASK, /**< Read command is received. */
    HAL_SPI_SLAVE_EVENT_CWR_FINISH = SPIS_INT_WR_CFG_FINISH_MASK,  /**< Configure write command is received. */
    HAL_SPI_SLAVE_EVENT_WR_FINISH = SPIS_INT_WR_TRANS_FINISH_MASK, /**< Write command is received. */
    HAL_SPI_SLAVE_EVENT_RD_ERR = SPIS_INT_RD_DATA_ERR_MASK,        /**< An error occurred during a read command. */
    HAL_SPI_SLAVE_EVENT_WR_ERR = SPIS_INT_WR_DATA_ERR_MASK,        /**< An error occurred during a write command. */
    HAL_SPI_SLAVE_EVENT_TIMEOUT_ERR = SPIS_INT_TMOUT_ERR_MASK,     /**< A timeout is detected between configure read command and read command or configure write command and write command. */
    HAL_SPI_SLAVE_EVENT_IDLE_TIMEOUT = SPIS_INT_IDLE_TMOUT_MASK,   /**< A timeout is detected as CS pin is inactive in direct mode. */
    HAL_SPI_SLAVE_EVENT_TX_FIFO_EMPTY = SPIS_INT_TX_FIFO_EMPTY_MASK,      /**< The data in TX FIFO is less than or equal to tx threshold in FIFO mode. */
    HAL_SPI_SLAVE_EVENT_RX_FIFO_FULL = SPIS_INT_RX_FIFO_FULL_MASK,        /**< The data in RX FIFO is larger than rx threshold in FIFO mode. */
    HAL_SPI_SLAVE_EVENT_TX_DMA_EMPTY = SPIS_INT_TX_DMA_EMPTY_MASK,      /**< The data in VFIFO TX buffer is less than or equal to tx threshold in DMA direct mode. */
    HAL_SPI_SLAVE_EVENT_RX_DMA_FULL = SPIS_INT_RX_DMA_FULL_MASK,        /**< The data in RX fifo is larger than rx threshold in DMA direct mode. */
    HAL_SPI_SLAVE_EVENT_RX_OVERRUN = SPIS_INT_RX_OVERRUN_MASK,          /**< A data received when the data in VFIFO RX buffer is equal to the avilable fifo space in direct mode. */
} hal_spi_slave_callback_event_t;

/** @brief This enum defines the SPI slave commands. */
typedef enum {
    HAL_SPI_SLAVE_CMD_WS        = 0,       /**< Write Status command. */
    HAL_SPI_SLAVE_CMD_RS        = 1,       /**< Read Status command. */
    HAL_SPI_SLAVE_CMD_WR        = 2,       /**< Write Data command. */
    HAL_SPI_SLAVE_CMD_RD        = 3,       /**< Read Data command. */
    HAL_SPI_SLAVE_CMD_POWEROFF  = 4,       /**< POWER OFF command. */
    HAL_SPI_SLAVE_CMD_POWERON   = 5,       /**< POWER ON command. */
    HAL_SPI_SLAVE_CMD_CW        = 6,       /**< Configure Write command. */
    HAL_SPI_SLAVE_CMD_CR        = 7,        /**< Configure Read command. */
    HAL_SPI_SLAVE_CMD_CT        = 8        /**< Configure Type command. */
} hal_spi_slave_command_type_t;

#ifdef HAL_SPI_SLAVE_FEATURE_BYPASS
/** @brief This enum defines the SPI slave bypass master port. */
typedef enum {
    HAL_SPI_SLAVE_BYPASS_MASTER_0 = 0,      /**< SPI slave bypass master port 0. */
    HAL_SPI_SLAVE_BYPASS_MASTER_1 = 1,      /**< SPI slave bypass master port 1. */
    HAL_SPI_SLAVE_BYPASS_MASTER_2 = 2,      /**< SPI slave bypass master port 2. */
    HAL_SPI_SLAVE_BYPASS_MASTER_MAX         /**< The total number of SPI slave bypass destination ports (invalid SPI slave bypass destination port number). */
} hal_spi_slave_bypass_master_port_t;

/** @brief This enum defines the SPI slave bypass master port chip selection channels.
 */
typedef enum {
    HAL_SPI_SLAVE_BYPASS_MASTER_CS_0 = 0,     /**< bypass SPI slave chip selection channel to master chip selection channel 0. */
    HAL_SPI_SLAVE_BYPASS_MASTER_CS_1 = 1,     /**< bypass SPI slave chip selection channel to master chip selection channel 1. */
    HAL_SPI_SLAVE_BYPASS_MASTER_CS_2 = 2,     /**< bypass SPI slave chip selection channel to master chip selection channel 2. */
    HAL_SPI_SLAVE_BYPASS_MASTER_CS_3 = 3,     /**< bypass SPI slave chip selection channel to master chip selection channel 3. */
    HAL_SPI_SLAVE_BYPASS_MASTER_CS_MAX,       /**< The total number of SPI slave bypass chip selection channel */
} hal_spi_slave_bypass_master_cs_t;

/** @brief This enum defines the SPI slave bypass mode. */
typedef enum {
    HAL_SPI_SLAVE_BYPASS_MODE_W  = 0,       /**< The signal MOSI/MISO/SIO2/SIO3 of SPI slave bypass to SPI Master MOSI/MISO/SIO2/SIO3. @image html hal_spi_slave_bypass_mode_w.png*/
    HAL_SPI_SLAVE_BYPASS_MODE_R  = 1,       /**< The signal MOSI/MISO/SIO2/SIO3 of SPI master bypass to SPI slave MOSI/MISO/SIO2/SIO3. @image html hal_spi_slave_bypass_mode_r.png*/
    HAL_SPI_SLAVE_BYPASS_MODE_RW = 2,       /**< The signal bypass bi-directionally. The signal MOSI of SPI slave bypass to SPI master, and the signal MISO of SPI master bypass to SPI slave. @image html hal_spi_slave_bypass_mode_rw.png */
    HAL_SPI_SLAVE_BYPASS_MODE_MAX,          /**< The total number of SPI slave bypass mode. */
} hal_spi_slave_bypass_mode_t;
#endif

/**
 * @}
 */


/**
 * @}
 * @}
 */
#endif


#ifdef HAL_RTC_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup RTC
 * @{
 * @addtogroup hal_rtc_define Define
 * @{
 */

/** @brief  This macro defines a maximum number for backup data that used in #hal_rtc_set_data(),
  * #hal_rtc_get_data(), #hal_rtc_clear_data functions.
  */
#define HAL_RTC_BACKUP_BYTE_NUM_MAX     (12)

/**
 * @}
 */

/**
 * @defgroup hal_rtc_enum Enum
 * @{
 */

/** @brief RTC current time change notification period selections. */
typedef enum {
    HAL_RTC_TIME_NOTIFICATION_NONE = 0,                     /**< No need for a time notification. */
    HAL_RTC_TIME_NOTIFICATION_EVERY_SECOND = 1,             /**< Execute a callback function set by #hal_rtc_set_time_notification_period() for every second. */
    HAL_RTC_TIME_NOTIFICATION_EVERY_MINUTE = 2,             /**< Execute a callback function set by #hal_rtc_set_time_notification_period() for every minute. */
    HAL_RTC_TIME_NOTIFICATION_EVERY_HOUR = 3,               /**< Execute a callback function set by #hal_rtc_set_time_notification_period() for every hour. */
    HAL_RTC_TIME_NOTIFICATION_EVERY_DAY = 4,                /**< Execute a callback function set by #hal_rtc_set_time_notification_period() for every day. */
    HAL_RTC_TIME_NOTIFICATION_EVERY_MONTH = 5,              /**< Execute a callback function set by #hal_rtc_set_time_notification_period() for every month. */
    HAL_RTC_TIME_NOTIFICATION_EVERY_YEAR = 6,               /**< Execute a callback function set by #hal_rtc_set_time_notification_period() for every year. */
    HAL_RTC_TIME_NOTIFICATION_EVERY_SECOND_1_2 = 7,         /**< Execute a callback function set by #hal_rtc_set_time_notification_period() for every one-half of a second. */
    HAL_RTC_TIME_NOTIFICATION_EVERY_SECOND_1_4 = 8,         /**< Execute a callback function set by #hal_rtc_set_time_notification_period() for every one-fourth of a second. */
    HAL_RTC_TIME_NOTIFICATION_EVERY_SECOND_1_8 = 9,         /**< Execute a callback function set by #hal_rtc_set_time_notification_period() for every one-eighth of a second. */
    HAL_RTC_TIME_NOTIFICATION_MAX,                          /**< Max enum item  */
} hal_rtc_time_notification_period_t;
/** @brief This enum defines the type of RTC GPIO. */
typedef enum {
    HAL_RTC_GPIO_0 = 0,     /**< RTC GPIO 0. */
    HAL_RTC_GPIO_1 = 1,     /**< RTC GPIO 1. */
    HAL_RTC_GPIO_2 = 2,     /**< RTC GPIO 2. */
    HAL_RTC_GPIO_3 = 3,     /**< RTC GPIO 3. */
    HAL_RTC_GPIO_MAX
} hal_rtc_gpio_t;
/** @brief This enum defines the data type of RTC GPIO. */
typedef enum {
    HAL_RTC_GPIO_DATA_LOW  = 0,                     /**< RTC GPIO data low. */
    HAL_RTC_GPIO_DATA_HIGH = 1                      /**< RTC GPIO data high. */
} hal_rtc_gpio_data_t;
/**
 * @}
 */

/** @defgroup hal_rtc_struct Struct
  * @{
  */

/** @brief RTC time structure definition. */
typedef struct {
    uint8_t rtc_sec;                                  /**< Seconds after minutes     - [0,59]  */
    uint8_t rtc_min;                                  /**< Minutes after the hour    - [0,59]  */
    uint8_t rtc_hour;                                 /**< Hours after midnight      - [0,23]  */
    uint8_t rtc_day;                                  /**< Day of the month          - [1,31]  */
    uint8_t rtc_mon;                                  /**< Months                    - [1,12]  */
    uint8_t rtc_week;                                 /**< Days in a week            - [0,6]   */
    uint8_t rtc_year;                                 /**< Years                     - [0,127] */
    uint16_t rtc_milli_sec;                           /**< Millisecond value, when in time API, this represents the read only register rtc_int_cnt - [0,32767]; when in alarm API, this parameter has no meaning. */
} hal_rtc_time_t;



/** @brief RTC GPIO control structure definition. */
typedef struct {
    hal_rtc_gpio_t rtc_gpio;        /**< Configure which GPIO will apply this setting. */
    bool is_enable_rtc_eint;        /**< Enable RTC EINT or not. */
    uint8_t is_falling_edge_active;    /**< Configure RTC EINT edge(0:rising edge, 1:falling edge, 2:dual edge) */
    bool is_enable_debounce;        /**< Enable RTC EINT debounce or not, if enabled, EINT debounce time is 4T*32k. */
	bool is_enable_wakeup;          /**< Enable RTC EINT wakeup system from rtc mode or not. */
} hal_rtc_eint_config_t;
/** @brief This structure defines the settings of RTC GPIO. */
typedef struct {
    hal_rtc_gpio_t rtc_gpio;        /**< Configure which GPIO will apply this setting. */
    bool is_analog;                 /**< Cinfugure RTC GPIO as analog or digtal mode. */
    bool is_input;                  /**< Cinfugure RTC GPIO as input direction or not. */
    bool is_pull_up;                /**< If RTC GPIO is pull mode, configure RTC GPIO pull up or not. */
    bool is_pull_down;              /**< If RTC GPIO is pull mode, configure RTC GPIO pull down or not. */
} hal_rtc_gpio_config_t;
/**
 * @}
 */

/**
 * @}
 * @}
 */
#endif


#ifdef HAL_EINT_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup EINT
 * @{
 * @addtogroup hal_eint_enum Enum
 * @{
 */

/*****************************************************************************
* EINT
*****************************************************************************/
/** @brief EINT pins. */
typedef enum {
    HAL_EINT_NUMBER_0 = 0,
    HAL_EINT_NUMBER_1 = 1,
    HAL_EINT_NUMBER_2 = 2,
    HAL_EINT_NUMBER_3 = 3,
    HAL_EINT_NUMBER_4 = 4,
    HAL_EINT_NUMBER_5 = 5,
    HAL_EINT_NUMBER_6 = 6,
    HAL_EINT_NUMBER_7 = 7,
    HAL_EINT_NUMBER_8 = 8,
    HAL_EINT_NUMBER_9 = 9,
    HAL_EINT_NUMBER_10 = 10,
    HAL_EINT_NUMBER_11 = 11,
    HAL_EINT_NUMBER_12 = 12,
    HAL_EINT_NUMBER_13 = 13,
    HAL_EINT_NUMBER_14 = 14,
    HAL_EINT_NUMBER_15 = 15,
    HAL_EINT_NUMBER_16 = 16,
    HAL_EINT_NUMBER_17 = 17,
    HAL_EINT_NUMBER_18 = 18,
    HAL_EINT_NUMBER_19 = 19,
    HAL_EINT_NUMBER_20 = 20,
    HAL_EINT_NUMBER_21 = 21,
    HAL_EINT_NUMBER_22 = 22,
    HAL_EINT_NUMBER_23 = 23,
    HAL_EINT_NUMBER_24 = 24,
    HAL_EINT_NUMBER_25 = 25,
    HAL_EINT_NUMBER_26 = 26,
    HAL_EINT_NUMBER_27 = 27,
    HAL_EINT_NUMBER_28 = 28,
    HAL_EINT_NUMBER_29 = 29,
    HAL_EINT_NUMBER_30 = 30,
    HAL_EINT_NUMBER_31 = 31,
    HAL_EINT_NUMBER_32 = 32,
    HAL_EINT_NUMBER_33 = 33,
    HAL_EINT_NUMBER_34 = 34,
    HAL_EINT_NUMBER_35 = 35,
    HAL_EINT_NUMBER_36 = 36,
    HAL_EINT_NUMBER_37 = 37,
    HAL_EINT_NUMBER_38 = 38,
    HAL_EINT_NUMBER_39 = 39,
    HAL_EINT_NUMBER_40 = 40,
    HAL_EINT_NUMBER_41 = 41,
    HAL_EINT_NUMBER_42 = 42,
    HAL_EINT_NUMBER_43 = 43,
    HAL_EINT_NUMBER_44 = 44,
    HAL_EINT_NUMBER_45 = 45,
    HAL_EINT_NUMBER_46 = 46,
    HAL_EINT_NUMBER_47 = 47,
    HAL_EINT_NUMBER_48 = 48,
    HAL_EINT_NUMBER_49 = 49,
    HAL_EINT_NUMBER_50 = 50,
    HAL_EINT_NUMBER_51 = 51,
    HAL_EINT_NUMBER_52 = 52,
    HAL_EINT_NUMBER_53 = 53,
    HAL_EINT_NUMBER_54 = 54,
    HAL_EINT_NUMBER_55 = 55,
    HAL_EINT_NUMBER_56 = 56,
    HAL_EINT_NUMBER_57 = 57,
    HAL_EINT_UART_0_RX = 58,    /**< EINT number 58:  UART0 RX. */
    HAL_EINT_UART_1_RX = 59,    /**< EINT number 59:  UART1 RX. */
    HAL_EINT_UART_2_RX = 60,    /**< EINT number 60:  UART2 RX. */
    HAL_EINT_RTC       = 61,    /**< EINT number 61:  RTC. */
    HAL_EINT_USB       = 62,    /**< EINT number 62:  USB. */
    HAL_EINT_PMU       = 63,    /**< EINT number 63:  PMIC. */
    HAL_EINT_IRRX      = 64,    /**< EINT number 64:  IRRX. */
    HAL_EINT_NUMBER_MAX         /**< The total number of EINT channels (invalid EINT channel). */
} hal_eint_number_t;
/**
  * @}
  */
/**
 * @}
 * @}
 */
#endif

#ifdef HAL_GPT_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup GPT
 * @{
 * @addtogroup hal_gpt_enum Enum
 * @{
 */

/*****************************************************************************
* GPT
*****************************************************************************/
/** @brief GPT port */
typedef enum {
    HAL_GPT_0 = 0,                          /**< GPT port 0. CM4 User defined. */
    HAL_GPT_1 = 1,                          /**< GPT port 1. Usee for CM4/DSP0 to set a millisecond delay and get 1/32Khz free count. The clock source is 32Khz*/
    HAL_GPT_2 = 2,                          /**< GPT port 2. Usee for CM4/DSP0 to set a microsecond delay and get microsecond free count. The clock source is 1Mhz*/
    HAL_GPT_3 = 3,                          /**< GPT port 3. Used for CM4 as software GPT. The clock source is 32Khz.*/
    HAL_GPT_4 = 4,                          /**< GPT port 4. Used for CM4 as software GPT. The clock source is 1Mhz.*/
    HAL_GPT_5 = 5,                          /**< GPT port 5. Used for DSP0 as software GPT. The clock source is 32Khz.*/
    HAL_GPT_6 = 6,                          /**< GPT port 6. Used for DSP0 as software GPT. The clock source is 1Mhz.*/
    HAL_GPT_7 = 7,                          /**< GPT port 7. CM4 User defined.*/
    HAL_GPT_8 = 8,                          /**< GPT port 8. Used for BT system wakeup*/
    HAL_GPT_MAX_PORT = 9,                  /**< The total number of GPT ports (invalid GPT port). */
    HAL_GPT_MAX = 9
} hal_gpt_port_t;

/** @brief GPT clock source  */
typedef enum {
    HAL_GPT_CLOCK_SOURCE_32K = 0,            /**< Set the GPT clock source to 32kHz, 1 tick = 1/32768 seconds. */
    HAL_GPT_CLOCK_SOURCE_1M  = 1             /**< Set the GPT clock source to 1MHz, 1 tick = 1 microsecond.*/
} hal_gpt_clock_source_t;

/** @brief  The maximum time of millisecond timer.
  */
#define HAL_GPT_MAXIMUM_MS_TIMER_TIME   (130150523)

/** @brief  The max user number for sw gpt,include ms user and us user.
  */
#define HAL_SW_GPT_MAX_USERS 20

/**
  * @}
  */
/**
 * @}
 * @}
 */
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup FLASH
 * @{
 */

/*****************************************************************************
* Flash
*****************************************************************************/

/** @defgroup hal_flash_define Define
 * @{
  */

/** @brief  This macro defines the Flash base address.
  */
#define HAL_FLASH_BASE_ADDRESS    (SFC_GENERIC_FLASH_BANK_MASK)
/**
  * @}
  */
/**
 * @}
 * @}
 */
#endif


#ifdef HAL_ESC_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup ESC
 * @{
 */

/*****************************************************************************
* ESC
*****************************************************************************/

/** @defgroup hal_esc_define Define
 * @{
  */

/** @brief  This macro defines the ESC base address.
  */
#define HAL_ESC_MEMORY_BASE_ADDRESS    (0x10000000)
/**
  * @}
  */
/**
 * @}
 * @}
 */
#endif


#ifdef HAL_PWM_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup PWM
 * @{
 * @addtogroup hal_pwm_enum Enum
 * @{
 */
/*****************************************************************************
* PWM
*****************************************************************************/
/** @brief The PWM channels */
typedef enum {
    HAL_PWM_0 = 0,                            /**< PWM channel 0. */
    HAL_PWM_1 = 1,                            /**< PWM channel 1. */
    HAL_PWM_2 = 2,                            /**< PWM channel 2. */
    HAL_PWM_3 = 3,                            /**< PWM channel 3. */
    HAL_PWM_4 = 4,                            /**< PWM channel 4. */
    HAL_PWM_5 = 5,                            /**< PWM channel 5. */
    HAL_PWM_6 = 6,                            /**< PWM channel 6. */
    HAL_PWM_7 = 7,                            /**< PWM channel 7. */
    HAL_PWM_8 = 8,                            /**< PWM channel 8. */
    HAL_PWM_9 = 9,                            /**< PWM channel 9. */
    HAL_PWM_10 = 10,                          /**< PWM channel 10. */
    HAL_PWM_11 = 11,                          /**< PWM channel 11. */
    HAL_PWM_12 = 12,                          /**< PWM channel 12. */
    HAL_PWM_13 = 13,                          /**< PWM channel 13. */
    HAL_PWM_MAX_CHANNEL                     /**< The total number of PWM channels (invalid PWM channel).*/
} hal_pwm_channel_t;

/** @brief PWM clock source options */
typedef enum {
    HAL_PWM_CLOCK_26MHZ = 0,                /**< PWM clock source 26MHz(FXO). */
    HAL_PWM_CLOCK_32KHZ = 1,                /**< PWM clock srouce 32kHz. */
    HAL_PWM_CLOCK_39MHZ = 2,                /**< PWM clock source 39MHz. */
    HAL_PWM_CLOCK_86MHZ = 3,                /**< PWM clock srouce 86.6MHz. */
    HAL_PWM_CLOCK_MAX,
} hal_pwm_source_clock_t ;

/**
  * @}
  */
/**
 * @}
 * @}
 */
#endif

#ifdef HAL_WDT_MODULE_ENABLED

/**
 * @addtogroup HAL
 * @{
 * @addtogroup WDT
 * @{
*/

/* @addtogroup hal_wdt_define Define
 * @{
 */
/** @brief  This enum define the max timeout value of WDT.  */
#define HAL_WDT_MAX_TIMEOUT_VALUE (1000)
/**
 * @}
 */

/**
 * @}
 * @}
 */

#endif

#ifdef HAL_CACHE_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup CACHE
 * @{
 */

/*****************************************************************************
* Cache
*****************************************************************************/
/* NULL */

/**
 * @}
 * @}
 */
#endif

#define audio_message_type_t hal_audio_message_type_t
#ifdef HAL_AUDIO_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup AUDIO
 * @{
 * @addtogroup hal_audio_enum Enum
 * @{
 */

/** @brief Audio message type */
typedef enum {
    AUDIO_MESSAGE_TYPE_COMMON,            /**< Audio basic scenario. */
    AUDIO_MESSAGE_TYPE_BT_AUDIO_UL  = 1,  /**< BT audio UL scenario. */
    AUDIO_MESSAGE_TYPE_BT_AUDIO_DL  = 2,  /**< BT audio DL scenario. */
    AUDIO_MESSAGE_TYPE_BT_VOICE_UL  = 3,  /**< BT aoice UL scenario. */
    AUDIO_MESSAGE_TYPE_BT_VOICE_DL  = 4,  /**< BT aoice DL scenario. */
    AUDIO_MESSAGE_TYPE_PLAYBACK     = 5,  /**< Local playback scenario. */
    AUDIO_MESSAGE_TYPE_RECORD       = 6,  /**< Mic record scenario. */
    AUDIO_MESSAGE_TYPE_PROMPT       = 7,  /**< Voice prompt scenario. */
    AUDIO_MESSAGE_TYPE_LINEIN       = 8,  /**< LineIN & loopback scenario. */
    AUDIO_MESSAGE_TYPE_BLE_AUDIO_UL = 9,  /**< BLE audio UL scenario. */
    AUDIO_MESSAGE_TYPE_BLE_AUDIO_DL = 10, /**< BLE audio DL scenario. */
    AUDIO_MESSAGE_TYPE_SIDETONE,          /**< Sidetone scenario. */
    AUDIO_MESSAGE_TYPE_ANC,               /**< ANC scenario. */
    AUDIO_MESSAGE_TYPE_AFE,               /**< DSP AFE dummy type. */
    AUDIO_MESSAGE_TYPE_AUDIO_TRANSMITTER= 14, /**< audio transmitter scenario.**/
    AUDIO_MESSAGE_TYPE_SPDIF_DUMP,        /**< DSP AFE dummy type. */
    AUDIO_MESSAGE_TYPE_ADAPTIVE,          /**< Adaptive control */
    AUDIO_MESSAGE_TYPE_MCLK,              /**< DSP AFE dummy type. */
    AUDIO_MESSAGE_TYPE_AUDIO_NVDM,        /**< audio nvdm relative. */
    AUDIO_MESSAGE_TYPE_BT_A2DP_DL,        /**< audio a2dp sync action scenario.**/
    AUDIO_MESSAGE_TYPE_DCHS_DL,              /*DCHS DL type*/
    AUDIO_MESSAGE_TYPE_DCHS_UL,              /*DCHS UL type*/
    AUDIO_MESSAGE_TYPE_LLF,              /**< PSAP scenario. */
    AUDIO_MESSAGE_TYPE_VOLUME_MONITOR,    /**< Audio Volume Monitor .*/
    AUDIO_MESSAGE_TYPE_MAX,               /**< Audio scenario type MAX. */

    AUDIO_RESERVE_TYPE_QUERY_RCDC,        /**< Query Message: RCDC. Different from above audio main scenario messages. Only for query purpose.*/
    AUDIO_RESERVE_TYPE_ULL_QUERY_RCDC,    /**< Query Message: ULL RCDC. Different from above audio main scenario messages. Only for query purpose.*/
    AUDIO_MESSAGE_TYPE_BLE_AUDIO_SUB_UL,  /**< BLE audio SUB UL scenario. */
    AUDIO_MESSAGE_TYPE_BLE_AUDIO_SUB_DL,  /**< BLE audio SUB DL scenario. */

    AUDIO_MESSAGE_TYPE_ANC_MONITOR_ADAPTIVE_ANC, /**< Adaptive ANC scenario. */
    AUDIO_MESSAGE_TYPE_AEQ_MONITOR_ADAPTIVE_EQ, /**< Adaptive AEQ scenario. */
    AUDIO_MESSAGE_TYPE_3RD_PARTY_AUDIO_PLATFORM,/**< Audio 3rd party platform integration. */


} audio_message_type_t;

/** @brief Audio clock setting */
typedef enum {
    /* clock */
    AUDIO_CLOCK_INT        = 0,                                /**< Audio Clock for Audio internal bus */
    AUDIO_CLOCK_ENGINE     = 1,                                /**< Audio Clock for Memory agent engine */
    AUDIO_CLOCK_GPSRC      = 2,                                /**< Audio Clock for HWSRC */
    AUDIO_CLOCK_UPLINK     = 3,                                /**< Audio Clock for DL hi-res */
    AUDIO_CLOCK_DWLINK     = 4,                                /**< Audio Clock for UL hi-res */
    AUDIO_CLOCK_SPDIF      = 5,                                /**< Audio Clock for SPDIF */
    AUDIO_CLOCK_INTF0_IN   = 6,                                /**< Audio Clock for I2S IN APLL2 48KHz, APLL2:49.152M */
    AUDIO_CLOCK_INTF1_IN   = 7,                                /**< Audio Clock for I2S IN APLL1 44.1KHz, APLL1:45.1584MHZ */
    AUDIO_CLOCK_INTF0_OUT  = 8,                                /**< Audio Clock for I2S OUT APLL2 48KHz, APLL2:49.152M */
    AUDIO_CLOCK_INTF1_OUT  = 9,                                /**< Audio Clock for I2S OUT APLL1 44.1KHz, APLL1:45.1584MHZ */
    AUDIO_CLOCK_TEST       = 10,                               /**< Audio Clock for TEST */
    AUDIO_CLOCK_ANC        = 11,                               /**< Audio Clock for ANC */
    AUDIO_CLOCK_CLD        = 12,                               /**< Audio Clock for CLD */
    AUDIO_CLOCK_VOW        = 13,                               /**< Audio Clock for VOW */
    /* power */
    AUDIO_POWER_MICBIAS_0_HP  = 14,                            /**< Audio Power for Micbias 1 high performance mode*/
    AUDIO_POWER_MICBIAS_1_HP  = 15,                            /**< Audio Power for Micbias 2 high performance mode*/
    AUDIO_POWER_MICBIAS_2_HP  = 16,                            /**< Audio Power for Micbias 3 high performance mode*/
    AUDIO_POWER_MICBIAS_0_NM  = 17,                            /**< Audio Power for Micbias 1 normal mode*/
    AUDIO_POWER_MICBIAS_1_NM  = 18,                            /**< Audio Power for Micbias 2 normal mode*/
    AUDIO_POWER_MICBIAS_2_NM  = 19,                            /**< Audio Power for Micbias 3 normal mode*/
    AUDIO_POWER_MICBIAS_0_LP  = 20,                            /**< Audio Power for Micbias 1 low power mode*/
    AUDIO_POWER_MICBIAS_1_LP  = 21,                            /**< Audio Power for Micbias 2 low power mode*/
    AUDIO_POWER_MICBIAS_2_LP  = 22,                            /**< Audio Power for Micbias 3 low power mode*/
    AUDIO_POWER_MICBIAS_END = AUDIO_POWER_MICBIAS_2_LP,        /**< Audio Power for Micbias End */
    AUDIO_POWER_MICBIAS_SHARE = 23,                            /**< Audio Power for Micbias Share LDO0 */
    AUDIO_POWER_DAC            ,                               /**< Audio Power for DAC Out(vitual), for amp lock */
    AUDIO_POWER_I2S            ,                               /**< Audio Power for I2S MST Out(vitual), for amp lock */
    /* SPM STATE */
    AUDIO_DSP_SPM_STATE1       ,                               /**< Audio DSP Low power state1, dsp can't sleep */
    AUDIO_DSP_SPM_STATE3       ,                               /**< Audio DSP Low power state3, audio won't use, reserved */
    AUDIO_DSP_SPM_STATE4       ,                               /**< Audio DSP Low power state4, dsp can sleep */
    AUDIO_POWER_END            ,                               /**< Audio Power End Number */
    AUDIO_CLOCK_MAX = 0xFFFFFFFF,                              /**< Audio Clock Max Number */
} audio_clock_setting_type_t;

/*****************************************************************************
* Audio setting
*****************************************************************************/


/** @brief Hal audio dmic selection. */
typedef enum {
    HAL_AUDIO_DMIC_GPIO_DMIC0   = 0x0,              /**<  for dmic selection */
    HAL_AUDIO_DMIC_GPIO_DMIC1,                      /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC0,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC1,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC2,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC3,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC4,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC5,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_DUMMY        = 0xFFFFFFFF,           /**<  for DSP structrue alignment */
} hal_audio_dmic_selection_t;

/** @brief i2s clk source define */
typedef enum {
    I2S_CLK_SOURCE_APLL                         = 0, /**< Low jitter mode. */
    I2S_CLK_SOURCE_DCXO                         = 1, /**< Normal mode. */
    I2S_CLK_SOURCE_TYPE_DUMMY                   = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} I2S_CLK_SOURCE_TYPE;

/** @brief micbias source define */
typedef enum {
    MICBIAS_SOURCE_0                            = 1, /**< Open micbias0. */
    MICBIAS_SOURCE_1                            = 2, /**< Open micbias1. */
    MICBIAS_SOURCE_2                            = 3, /**< Open micbias2. */
    MICBIAS_SOURCE_3                            = 4, /**< Open micbias3. */
    MICBIAS_SOURCE_4                            = 5, /**< Open micbias4. */
    MICBIAS_SOURCE_ALL                          = 6, /**< Open micbias0 to micbias4. */
    MICBIAS_SOURCE_TYPE_DUMMY                   = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} MICBIAS_SOURCE_TYPE;

/** @brief micbias out voltage define */
typedef enum {
    MICBIAS3V_OUTVOLTAGE_1p8v                   = 1 << 2,   /**< 1.8V */
    MICBIAS3V_OUTVOLTAGE_1p85v                  = 1 << 3,   /**< 1.85V (Default) */
    MICBIAS3V_OUTVOLTAGE_1p9v                   = 1 << 4,   /**< 1.9V */
    MICBIAS3V_OUTVOLTAGE_2p0v                   = 1 << 5,   /**< 2.0V */
    MICBIAS3V_OUTVOLTAGE_2p1v                   = 1 << 6,   /**< 2.1V */
    MICBIAS3V_OUTVOLTAGE_2p2v                   = 1 << 7,   /**< 2.2V (Not support in 2V) */
    MICBIAS3V_OUTVOLTAGE_2p4v                   = 1 << 8,   /**< 2.4V (Not support in 2V) */
    MICBIAS3V_OUTVOLTAGE_2p55v                  = 1 << 9,   /**< 2.55V (Not support in 2V) */
    MICBIAS3V_OUTVOLTAGE_VCC                    = 0x7f << 2, /**< BYPASSEN  */
    MICBIAS3V_OUTVOLTAGE_TYPE_DUMMY             = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} MICBIAS3V_OUTVOLTAGE_TYPE;

/** @brief micbias0 amic type define */
typedef enum {
    MICBIAS0_AMIC_MEMS                          = 0 << 10,  /**< MEMS (Default)*/
    MICBIAS0_AMIC_ECM_DIFFERENTIAL              = 1 << 10,  /**< ECM Differential*/
    MICBIAS0_AMIC_ECM_SINGLE                    = 3 << 10,  /**< ECM Single*/
    MICBIAS0_AMIC_TYPE_DUMMY                    = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} MICBIAS0_AMIC_TYPE;

/** @brief micbias1 amic type define */
typedef enum {
    MICBIAS1_AMIC_MEMS                          = 0 << 12,  /**< MEMS (Default)*/
    MICBIAS1_AMIC_ECM_DIFFERENTIAL              = 1 << 12,  /**< ECM Differential*/
    MICBIAS1_AMIC_ECM_SINGLE                    = 3 << 12,  /**< ECM Single*/
    MICBIAS1_AMIC_TYPE_DUMMY                    = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} MICBIAS1_AMIC_TYPE;

/** @brief uplink performance type define */
typedef enum {
    UPLINK_PERFORMANCE_NORMAL                   = 0 << 13, /**< Normal mode (Default)*/
    UPLINK_PERFORMANCE_HIGH                     = 1 << 13, /**< High performance mode*/
    UPLINK_PERFORMANCE_TYPE_DUMMY               = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} UPLINK_PERFORMANCE_TYPE;

/** @brief amic mic type define */
typedef enum {
    ANALOG_INPUT_MODE_DCC                                    = 0 << 14, /**< AMIC DCC mode.*/
    ANALOG_INPUT_MODE_ACC_10K                                = 1 << 14, /**< AMIC ACC 10K mode.*/
    ANALOG_INPUT_MODE_ACC_20K                                = 2 << 14, /**< AMIC ACC 10K mode.*/
    ANALOG_INPUT_MODE_DUMMY                                  = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} ANALOG_INPUT_MODE;

/** @brief dac output mode define */
typedef enum {
    ANALOG_OUTPUT_MODE_CLASSAB = 0 << 16,
    ANALOG_OUTPUT_MODE_CLASSG =  0 << 16,
    ANALOG_OUTPUT_MODE_CLASSD  = 1 << 16,
} ANALOG_OUTPUT_MODE;

/** @brief downlink performance type define */
typedef enum {
    DOWNLINK_PERFORMANCE_NORMAL                 = 0, /**< Normal mode (Default)*/
    DOWNLINK_PERFORMANCE_HIGH                   = 1, /**< High performance mode*/
    DOWNLINK_PERFORMANCE_TYPE_DUMMY             = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} DOWNLINK_PERFORMANCE_TYPE;

/** @brief audio MCLK pin select define */
typedef enum {
    AFE_MCLK_PIN_FROM_I2S0 = 0,     /**< MCLK from I2S0's mclk pin */
    AFE_MCLK_PIN_FROM_I2S1,         /**< MCLK from I2S1's mclk pin */
    AFE_MCLK_PIN_FROM_I2S2,         /**< MCLK from I2S2's mclk pin */
    AFE_MCLK_PIN_FROM_I2S3,         /**< MCLK from I2S3's mclk pin */
} afe_mclk_out_pin_t;

/** @brief audio APLL define */
typedef enum {
    AFE_APLL_NONE = 0,
    AFE_APLL1 = 1,                  /**< APLL1:45.1584M, 44.1K base */
    AFE_APLL2 = 2,                  /**< APLL2:49.152M, 48K base */
} afe_apll_source_t;

/** @brief audio MCLK status define */
typedef enum {
    MCLK_DISABLE = 0,               /**< Turn off MCLK */
    MCLK_ENABLE  = 1,               /**< Turn on MCLK */
} afe_mclk_status_t;

/** @brief amp performance define */
typedef enum {
    AUDIO_AMP_PERFORMANCE_NORMAL                = 0, /**< Normal mode. */
    AUDIO_AMP_PERFORMANCE_HIGH                  = 1, /**< High performance mode. */
    AUDIO_AMP_PERFORMANCE_TYPE_DUMMY            = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} AUDIO_AMP_PERFORMANCE_TYPE;

/** @brief audio ul1 dmic data selection define */
typedef enum {
    HAL_AUDIO_UL1_DMIC_DATA_SHIFT_NUM           = 15,
    HAL_AUDIO_UL1_DMIC_DATA_GPIO_DMIC0          = 0 << HAL_AUDIO_UL1_DMIC_DATA_SHIFT_NUM,    /**< Select UL1 data from GPIO DMIC0 */
    HAL_AUDIO_UL1_DMIC_DATA_GPIO_DMIC1          = 1 << HAL_AUDIO_UL1_DMIC_DATA_SHIFT_NUM,    /**< Select UL1 data from GPIO DMIC1 */
    HAL_AUDIO_UL1_DMIC_DATA_ANA_DMIC0           = 2 << HAL_AUDIO_UL1_DMIC_DATA_SHIFT_NUM,    /**< Select UL1 data from ANA_DMIC0 */
    HAL_AUDIO_UL1_DMIC_DATA_ANA_DMIC1           = 3 << HAL_AUDIO_UL1_DMIC_DATA_SHIFT_NUM,    /**< Select UL1 data from ANA_DMIC1 */
    HAL_AUDIO_UL1_DMIC_DATA_MASK                = HAL_AUDIO_UL1_DMIC_DATA_ANA_DMIC1,
} hal_audio_ul1_dmic_data_selection_t;

/** @brief audio ul2 dmic data selection define */
typedef enum {
    HAL_AUDIO_UL2_DMIC_DATA_SHIFT_NUM           = 17,
    HAL_AUDIO_UL2_DMIC_DATA_GPIO_DMIC0          = 0 << HAL_AUDIO_UL2_DMIC_DATA_SHIFT_NUM,    /**< Select UL2 data from GPIO DMIC0 */
    HAL_AUDIO_UL2_DMIC_DATA_GPIO_DMIC1          = 1 << HAL_AUDIO_UL2_DMIC_DATA_SHIFT_NUM,    /**< Select UL2 data from GPIO DMIC1 */
    HAL_AUDIO_UL2_DMIC_DATA_ANA_DMIC0           = 2 << HAL_AUDIO_UL2_DMIC_DATA_SHIFT_NUM,    /**< Select UL2 data from ANA_DMIC0 */
    HAL_AUDIO_UL2_DMIC_DATA_ANA_DMIC1           = 3 << HAL_AUDIO_UL2_DMIC_DATA_SHIFT_NUM,    /**< Select UL2 data from ANA_DMIC1 */
    HAL_AUDIO_UL2_DMIC_DATA_MASK                = HAL_AUDIO_UL2_DMIC_DATA_ANA_DMIC1,
} hal_audio_ul2_dmic_data_selection_t;

/** @brief audio ul3 dmic data selection define */
typedef enum {
    HAL_AUDIO_UL3_DMIC_DATA_SHIFT_NUM           = 19,
    HAL_AUDIO_UL3_DMIC_DATA_GPIO_DMIC0          = 0 << HAL_AUDIO_UL3_DMIC_DATA_SHIFT_NUM,    /**< Select UL3 data from GPIO DMIC0 */
    HAL_AUDIO_UL3_DMIC_DATA_GPIO_DMIC1          = 1 << HAL_AUDIO_UL3_DMIC_DATA_SHIFT_NUM,    /**< Select UL3 data from GPIO DMIC1 */
    HAL_AUDIO_UL3_DMIC_DATA_ANA_DMIC0           = 2 << HAL_AUDIO_UL3_DMIC_DATA_SHIFT_NUM,    /**< Select UL3 data from ANA_DMIC0 */
    HAL_AUDIO_UL3_DMIC_DATA_ANA_DMIC1           = 3 << HAL_AUDIO_UL3_DMIC_DATA_SHIFT_NUM,    /**< Select UL3 data from ANA_DMIC1 */
    HAL_AUDIO_UL3_DMIC_DATA_MASK                = HAL_AUDIO_UL3_DMIC_DATA_ANA_DMIC1,
} hal_audio_ul3_dmic_data_selection_t;

/** @brief DSP streaming source channel define */
typedef enum {
    AUDIO_DSP_CHANNEL_SELECTION_STEREO          = 0, /**< DSP streaming output L and R will be it own. */
    AUDIO_DSP_CHANNEL_SELECTION_MONO            = 1, /**< DSP streaming output L and R will be (L+R)/2. */
    AUDIO_DSP_CHANNEL_SELECTION_BOTH_L          = 2, /**< DSP streaming output both L. */
    AUDIO_DSP_CHANNEL_SELECTION_BOTH_R          = 3, /**< DSP streaming output both R. */
    AUDIO_DSP_CHANNEL_SELECTION_SWAP            = 8, /**< DSP streaming output L and R will be swapped. */
    AUDIO_DSP_CHANNEL_SELECTION_NUM,
} AUDIO_DSP_CHANNEL_SELECTION;

/** @brief Hal audio bias voltage. */
typedef enum {
    HAL_AUDIO_BIAS_VOLTAGE_1_80V    = 0x0,            /**<  for bias voltage setting */
    HAL_AUDIO_BIAS_VOLTAGE_1_85V,                     /**<  for bias voltage setting */
    HAL_AUDIO_BIAS_VOLTAGE_1_90V,                     /**<  for bias voltage setting */
    HAL_AUDIO_BIAS_VOLTAGE_2_00V,                     /**<  for bias voltage setting */
    HAL_AUDIO_BIAS_VOLTAGE_2_10V,                     /**<  for bias voltage setting */
    HAL_AUDIO_BIAS_VOLTAGE_2_20V,                     /**<  for bias voltage setting */
    HAL_AUDIO_BIAS_VOLTAGE_2_40V,                     /**<  for bias voltage setting */
    HAL_AUDIO_BIAS_VOLTAGE_2_55V,                     /**<  for bias voltage setting */
    HAL_AUDIO_BIAS_VOLTAGE_DUMMY    = 0xFFFFFFFF,       /**<  for DSP structrue alignment */
} hal_audio_bias_voltage_t;

/** @brief Hal audio bias selection. */
typedef enum {
    HAL_AUDIO_BIAS_SELECT_BIAS0 = 1<<0,                 /**< Open micbias0. */
    HAL_AUDIO_BIAS_SELECT_BIAS1 = 1<<1,                 /**< Open micbias1. */
    HAL_AUDIO_BIAS_SELECT_BIAS2 = 1<<2,                 /**< Open micbias2. */
    HAL_AUDIO_BIAS_SELECT_BIAS3 = 1<<3,                 /**< Open micbias3. */
    HAL_AUDIO_BIAS_SELECT_BIAS4 = 1<<4,                 /**< Open micbias4. */
    HAL_AUDIO_BIAS_SELECT_MAX   = HAL_AUDIO_BIAS_SELECT_BIAS4,
    HAL_AUDIO_BIAS_SELECT_ALL   = 0x1F,                  /**< Open micbias0 and micbias1 and micbias2. */
    HAL_AUDIO_BIAS_SELECT_NUM   = 5,
    HAL_AUDIO_BIAS_SELECT_DUMMY = 0xFFFFFFFF,           /**<  for DSP structrue alignment */
} hal_audio_bias_selection_t;

/** @brief Hal audio ul path iir filter. */
typedef enum {
    HAL_AUDIO_UL_IIR_DISABLE        = 0xF,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_SW             = 0x0,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_5HZ_AT_48KHZ   = 0x1,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_10HZ_AT_48KHZ  = 0x2,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_25HZ_AT_48KHZ  = 0x3,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_50HZ_AT_48KHZ  = 0x4,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_75HZ_AT_48KHZ  = 0x5,            /**< UL IIR setting */

    HAL_AUDIO_UL_IIR_10HZ_AT_96KHZ  = 0x1,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_20HZ_AT_96KHZ  = 0x2,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_50HZ_AT_96KHZ  = 0x3,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_100HZ_AT_96KHZ = 0x4,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_150HZ_AT_96KHZ = 0x5,            /**< UL IIR setting */
    HAL_AUDIO_UL_IIR_DUMMY          = 0xFFFFFFFF,       /**<  for DSP structrue alignment */
} hal_audio_ul_iir_t;

/** @brief Hal audio I2S data format. */
typedef enum {
    HAL_AUDIO_I2S_RJ    = 0, // Right-justified                      /**< I2S data format */
    HAL_AUDIO_I2S_LJ    = 1, // Left-justified                        /**< I2S data format */
    HAL_AUDIO_I2S_I2S   = 2,                           /**< I2S data format */
    HAL_AUDIO_I2S_DUMMY = 0xFFFFFFFF,                   /**<  for DSP structrue alignment */
} hal_audio_i2s_format_t;

/** @brief Hal audio I2S word length. */
typedef enum {
    HAL_AUDIO_I2S_WORD_LENGTH_16BIT = 0x0,              /**< I2S word length */
    HAL_AUDIO_I2S_WORD_LENGTH_32BIT = 0x1,              /**< I2S word length */
    HAL_AUDIO_I2S_WORD_LENGTH_DUMMY = 0xFFFFFFFF,       /**<  for DSP structrue alignment */
} hal_audio_i2s_word_length_t;

/** @brief Hal audio afe sample rate. */
typedef enum {
    HAL_AUDIO_FIXED_AFE_48K_SAMPLE_RATE = 48000,        /**< AFE sample rate 48K */
    HAL_AUDIO_FIXED_AFE_96K_SAMPLE_RATE = 96000,        /**< AFE sample rate 96K */
} hal_audio_afe_sample_rate_t;

/** @brief audio MCLK status structure */
typedef struct {
    bool                        status;                 /**< Audio mclk on/off status*/
    int16_t                     mclk_cntr;              /**< Audio mclk user count*/
    afe_apll_source_t           apll;                   /**< Specifies the apll of mclk source.*/
    uint8_t                     divider;                /**< Specifies the divider of mclk source, MCLK = clock_source/(1+Divider), Divider = [6:0].*/
} hal_audio_mclk_status_t;

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
/** @brief DSP input gain selection define */
typedef enum {
    HAL_AUDIO_INPUT_GAIN_SELECTION_D0_A0     = 0, /**< Setting input digital gain0 and analog gain0 . */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D0_D1     = 1, /**< Setting input digital gain0 and digital gain1 . */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D2_D3     = 2, /**< Setting input digital gain2 and digital gain3 . */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D4_D5     = 3, /**< Setting input digital gain4 and digital gain5 . */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D6_D7     = 4, /**< Setting input digital gain6 for I2S0_L and digital gain7 for I2S0_R . */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D8_D9     = 5, /**< Setting input digital gain8 for I2S1_L and digital gain9 for I2S1_R . */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D10_D11   = 6, /**< Setting input digital gain10 for I2S2_L and digital gain11 for I2S2_R . */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D12_D13   = 7, /**< Setting input digital gain12 for LININ_L and digital gain13 for I2S0_R . */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D14       = 8, /**< Setting input digital gain14 for echo path. */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D4        = 8,//temp for build error fix
    HAL_AUDIO_INPUT_GAIN_SELECTION_D18_19    = 9,  /**< Setting input digital for mic funtion. */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D20_21    = 10, /**< Setting input digital for mic funtion. */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D22_23    = 11, /**< Setting input digital for mic funtion. */
    HAL_AUDIO_INPUT_GAIN_SELECTION_D24_25    = 12, /**< Setting input digital for mic funtion. */
    HAL_AUDIO_INPUT_GAIN_SELECTION_A0_A1     = 13, /**< Setting input analog gain0 and analog gain1 . */
    HAL_AUDIO_INPUT_GAIN_SELECTION_A2_A3     = 14, /**< Setting input analog gain2 and analog gain3 . */
    HAL_AUDIO_INPUT_GAIN_SELECTION_A4_A5     = 15, /**< Setting input analog gain4 and analog gain5 . */
} hal_audio_input_gain_select_t;
#endif

/**
  * @}
  */
/**
 * @}
 * @}
 */
#endif

#ifdef HAL_GPC_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup GPC
 * @{
 * @addtogroup hal_gpc_enum Enum
 * @{
 */
/** @brief GPC port */
typedef enum {
    HAL_GPC_0 = 0,                          /**< GPC port 0. */
    HAL_GPC_MAX_PORT                        /**< The total number of GPC ports (invalid GPC port). */
} hal_gpc_port_t;


/**
  * @}
  */
/**
 * @}
 * @}
 */
#endif


#ifdef HAL_SD_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup SD
 * @{
 * @addtogroup hal_sd_enum Enum
 * @{
 */
/*****************************************************************************
* SD
*****************************************************************************/
/** @brief  This enum defines the SD/eMMC port. */
typedef enum {
    HAL_SD_PORT_0 = 0,                                             /**<  SD/eMMC port 0. */
    HAL_SD_PORT_1 = 1                                              /**<  SD/eMMC port 1. */
} hal_sd_port_t;

/**
  * @}
  */
/**
 * @}
 * @}
 */
#endif


#ifdef HAL_SDIO_SLAVE_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup SDIO
 * @{
 * @addtogroup hal_sdio_enum Enum
 * @{
 */
/*****************************************************************************
* SDIO
*****************************************************************************/
/** @brief  This enum defines the SDIO port.  */
typedef enum {
    HAL_SDIO_SLAVE_PORT_0 = 0,                                             /**< SDIO slave port 0. */
} hal_sdio_slave_port_t;

/**
  * @}
  */
/**
 * @}
 * @}
 */
#endif


#ifdef HAL_SDIO_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup SDIO
 * @{
 * @addtogroup hal_sdio_enum Enum
 * @{
 */
/*****************************************************************************
* SDIO
*****************************************************************************/
/** @brief  This enum defines the SDIO port.  */
typedef enum {
    HAL_SDIO_PORT_0 = 0,                                             /**< SDIO port 0. */
    HAL_SDIO_PORT_1 = 1                                              /**< SDIO port 1. */
} hal_sdio_port_t;


/**
  * @}
  */
/**
 * @}
 * @}
 */
#endif

#ifdef HAL_CLOCK_MODULE_ENABLED

/*****************************************************************************
* Clock
*****************************************************************************/

/**
 * @addtogroup HAL
 * @{
 * @addtogroup CLOCK
 * @{
 * @addtogroup hal_clock_enum Enum
 * @{
 *
 * @section CLOCK_CG_ID_Usage_Chapter HAL_CLOCK_CG_ID descriptions
 *
 * Each #hal_clock_cg_id is related to one CG. Please check the following parameters before controlling the clock.
 *
 * The description of API parameters for HAL_CLOCK_CG_ID is listed below:
 * | HAL_CLOCK_CG_ID            |Details                                                                            |
 * |----------------------------|-----------------------------------------------------------------------------------|
 * |\b HAL_CLOCK_CG_DMA         | The CG for DMA. It is controlled in DMA driver.|
 * |\b HAL_CLOCK_CG_SDIOMST_BUS | The CG for SDIO master bus. It is controlled in SDIO driver.|
 * |\b HAL_CLOCK_CG_SW_ASYS     | The CG for I2S1. It is controlled in I2S driver.|
 * |\b HAL_CLOCK_CG_SPISLV      | The CG for SPI slave. This CG should be enabled when it is connected to the master device if choosing a custom driver.|
 * |\b HAL_CLOCK_CG_SPIMST      | The CG for SPI master. It is controlled in SPI driver.|
 * |\b HAL_CLOCK_CG_SW_AUDIO    | The CG for I2S0. It is controlled in I2S driver.|
 * |\b HAL_CLOCK_CG_SDIOMST     | The CG for SDIO master. It is controlled in SDIO driver.|
 * |\b HAL_CLOCK_CG_UART1       | The CG for UART1. It is controlled in UART driver.|
 * |\b HAL_CLOCK_CG_UART2       | The CG for UART2. It is controlled in UART driver.|
 * |\b HAL_CLOCK_CG_I2C0        | The CG for I2C0. It is controlled in I2C driver.|
 * |\b HAL_CLOCK_CG_I2C1        | The CG for I2C1. It is controlled in I2C driver.|
 * |\b HAL_CLOCK_CG_CM_SYSROM   | The CG for system ROM. It cannot be disabled, otherwise the system will fail.|
 * |\b HAL_CLOCK_CG_SFC_SW      | The CG for serial flash controller. It cannot be disabled, otherwise the system will fail.|
 * |\b HAL_CLOCK_CG_SW_TRNG     | The CG for TRNG. It is controlled in TRNG driver.|
 * |\b HAL_CLOCK_CG_SW_XTALCTL  | The CG for crystal oscillator. It cannot be disabled, otherwise the system will fail.|
 * |\b HAL_CLOCK_CG_UART0       | The CG for UART0. It cannot be disabled, otherwise the system will fail.|
 * |\b HAL_CLOCK_CG_CRYPTO      | The CG for crypto engine. It is controlled in crypto engine driver.|
 * |\b HAL_CLOCK_CG_SDIOSLV     | The CG for SDIO slave. This CG should be enabled when it is connected to the master device if choosing a custom driver.|
 * |\b HAL_CLOCK_CG_PWM0        | The CG for PWM0. It is controlled in PWM driver.|
 * |\b HAL_CLOCK_CG_PWM1        | The CG for PWM1. It is controlled in PWM driver.|
 * |\b HAL_CLOCK_CG_PWM2        | The CG for PWM2. It is controlled in PWM driver.|
 * |\b HAL_CLOCK_CG_PWM3        | The CG for PWM3. It is controlled in PWM driver.|
 * |\b HAL_CLOCK_CG_PWM4        | The CG for PWM4. It is controlled in PWM driver.|
 * |\b HAL_CLOCK_CG_PWM5        | The CG for PWM5. It is controlled in PWM driver.|
 * |\b HAL_CLOCK_CG_SW_GPTIMER  | The CG for general purpose timer. It cannot be disabled, otherwise the system will fail.|
 * |\b HAL_CLOCK_CG_SW_AUXADC   | The CG for ADC. It is controlled in ADC driver.|
 */
/** @brief Use hal_clock_cg_id in Clock API. */
/*************************************************************************
 * Define clock gating registers and bit structure.
 * Note: Mandatory, modify clk_cg_mask in hal_clock.c source file, if hal_clock_cg_id has changed.
 *************************************************************************/

typedef enum {
    /* XO_PDN_PD_COND0  */
    HAL_CLOCK_CG_UART1                     =  0,        /* bit 0, PDN_COND0_FROM */
    HAL_CLOCK_CG_UART2                     =  1,        /* bit 1,*/
    HAL_CLOCK_CG_UART_DMA_0                =  2,        /* bit 2, */
    HAL_CLOCK_CG_UART_DMA_1                =  3,        /* bit 3, */
    HAL_CLOCK_CG_UART_DMA_2                =  4,        /* bit 4, */
    HAL_CLOCK_CG_IRRX                      =  5,        /* bit 5, */
    HAL_CLOCK_CG_IRRX_BUS                  =  6,        /* bit 6, */
    HAL_CLOCK_CG_I3C0                      =  7,        /* bit 16, */
    HAL_CLOCK_CG_I3C1                      =  8,        /* bit 8, */
    HAL_CLOCK_CG_I2C0                      =  9,        /* bit 9, */
    HAL_CLOCK_CG_I2C1                      =  10,        /* bit 10, */
    HAL_CLOCK_CG_I2C2                      =  11,        /* bit 11, */
    HAL_CLOCK_CG_ROSC                      =  12,        /* bit 12, */
    HAL_CLOCK_CG_UART0                     =  16,        /* bit 16, */
    HAL_CLOCK_CG_AUXADC                    =  17,        /* bit 17, */

    /* XO_PDN_AO_COND0 */
    HAL_CLOCK_CG_PWM0                      = (0 + 32),    /* bit 0, XO_PDN_AO_COND0 */
    HAL_CLOCK_CG_PWM1                      = (1 + 32),    /* bit 1, */
    HAL_CLOCK_CG_PWM2                      = (2 + 32),    /* bit 2, */
    HAL_CLOCK_CG_PWM3                      = (3 + 32),    /* bit 3, */
    HAL_CLOCK_CG_PWM4                      = (4 + 32),    /* bit 4, */
    HAL_CLOCK_CG_PWM5                      = (5 + 32),    /* bit 5, */
    HAL_CLOCK_CG_PWM6                      = (6 + 32),    /* bit 6, */
    HAL_CLOCK_CG_PWM7                      = (7 + 32),    /* bit 7, */
    HAL_CLOCK_CG_PWM8                      = (8 + 32),    /* bit 8, */
    HAL_CLOCK_CG_PWM9                      = (9 + 32),    /* bit 9, */
    HAL_CLOCK_CG_PWM10                     = (10 + 32),    /* bit 10, */
    HAL_CLOCK_CG_PWM11                     = (11 + 32),    /* bit 11, */
    HAL_CLOCK_CG_PWM12                     = (12 + 32),    /* bit 12, */
    HAL_CLOCK_CG_PWM13                     = (13 + 32),    /* bit 13, */
    HAL_CLOCK_CG_SPM_PCLK                  = (16 + 32),    /* bit 16, */
    HAL_CLOCK_CG_BCLK_CM33                 = (17 + 32),    /* bit 17, */
    HAL_CLOCK_CG_BCLK_DSP                  = (18 + 32),    /* bit 18, */
    HAL_CLOCK_CG_SPM_DIV                   = (19 + 32),    /* bit 19, */
    HAL_CLOCK_CG_I2C_AO                    = (20 + 32),    /* bit 20, */
    HAL_CLOCK_CG_OSTIMER                   = (21 + 32),    /* bit 21, */
    HAL_CLOCK_CG_GPTIMER0                  = (22 + 32),    /* bit 22, */
    HAL_CLOCK_CG_GPTIMER1                  = (23 + 32),    /* bit 23, */

    /* XO_PDN_TOP_COND0 */
    HAL_CLOCK_CG_SPIMST0                   = (0 + 64),    /* bit 0, XO_PDN_TOP_COND0 */
    HAL_CLOCK_CG_SPIMST1                   = (1 + 64),    /* bit 1, */
    HAL_CLOCK_CG_SPIMST2                   = (2 + 64),    /* bit 2, */
    HAL_CLOCK_CG_SDIOMST                   = (3 + 64),    /* bit 3, */
    HAL_CLOCK_CG_I3C                       = (4 + 64),    /* bit 4, */
    HAL_CLOCK_CG_BT_HOP                    = (5 + 64),    /* bit 5, */
    HAL_CLOCK_CG_AUD_GPSRC                 = (6 + 64),    /* bit 6, */
    HAL_CLOCK_CG_AUD_UPLINK                = (7 + 64),    /* bit 7, */
    HAL_CLOCK_CG_AUD_DWLINK                = (8 + 64),    /* bit 8, */
    HAL_CLOCK_CG_SPDIF                     = (9 + 64),    /* bit 9, */
    HAL_CLOCK_CG_AUD_INTF0                 = (10 + 64),   /* bit 10, */
    HAL_CLOCK_CG_AUD_INTF1                 = (11 + 64),   /* bit 11, */
    HAL_CLOCK_CG_AUD_TEST                  = (12 + 64),   /* bit 12, */
    HAL_CLOCK_CG_AUD_ANC                   = (13 + 64),   /* bit 13, */
    HAL_CLOCK_CG_AUD_CLD                   = (14 + 64),   /* bit 14, */
    HAL_CLOCK_CG_SFC                       = (17 + 64),   /* bit 17, */
    HAL_CLOCK_CG_ESC                       = (18 + 64),   /* bit 18, */
    HAL_CLOCK_CG_SPISLV                    = (19 + 64),   /* bit 19, */
    HAL_CLOCK_CG_USB                       = (20 + 64),   /* bit 20, */
    HAL_CLOCK_CG_AUD_INTBUS                = (21 + 64),   /* bit 21, */
    HAL_CLOCK_CG_DSP                       = (22 + 64),   /* bit 22, */
    HAL_CLOCK_CG_SEJ                       = (23 + 64),   /* bit 23, */
    HAL_CLOCK_CG_MIXEDSYS                  = (24 + 64),   /* bit 24, */
    HAL_CLOCK_CG_EFUSE                     = (25 + 64),   /* bit 25, */

    HAL_CLOCK_CG_OSC1_D12_D2               = (0 + 96),    /* bit 0, XO_PDN_TOP_COND1*/
    HAL_CLOCK_CG_OSC2_D10_D2               = (1 + 96),    /* bit 1, */
    HAL_CLOCK_CG_CMSYS_BUS                 = (16 + 96),   /* bit 16, */
    HAL_CLOCK_CG_RSV_BUS                   = (17 + 96),   /* bit 17, */
    HAL_CLOCK_CG_CONN_BUS                  = (18 + 96),   /* bit 18, */
    HAL_CLOCK_CG_RSV_RTC_BUS               = (19 + 96),   /* bit 19, */
    HAL_CLOCK_CG_RSV_MCLK_BUS              = (20 + 96),   /* bit 20, */
    HAL_CLOCK_CG_AUD_ENGINE_BUS            = (21 + 96),   /* bit 21, */
    HAL_CLOCK_CG_AUD_VOW_BUS               = (22 + 96),   /* bit 22, */

    HAL_CLOCK_CG_FAST_DMA0                 = (0 + 128),    /* bit 0, PDN_PD_COND0 */
    HAL_CLOCK_CG_FAST_DMA1                 = (1 + 128),    /* bit 1, */
    HAL_CLOCK_CG_SPIMST0_BUS               = (2 + 128),    /* bit 2, */
    HAL_CLOCK_CG_SPIMST1_BUS               = (3 + 128),    /* bit 3, */
    HAL_CLOCK_CG_SPIMST2_BUS               = (4 + 128),    /* bit 4, */
    HAL_CLOCK_CG_I3C_DMA0                  = (5 + 128),    /* bit 5, */
    HAL_CLOCK_CG_I3C_DMA1                  = (6 + 128),    /* bit 6, */
    HAL_CLOCK_CG_I2C_DMA0                  = (7 + 128),    /* bit 7, */
    HAL_CLOCK_CG_I2C_DMA1                  = (8 + 128),    /* bit 8, */
    HAL_CLOCK_CG_I2C_DMA2                  = (9 + 128),    /* bit 9, */
    HAL_CLOCK_CG_SECURITY_PD               = (16 + 128),   /* bit 16, */
    HAL_CLOCK_CG_AESOTF                    = (17 + 128),   /* bit 17, */
    HAL_CLOCK_CG_AESOTF_ESC                = (18 + 128),   /* bit 18, */
    HAL_CLOCK_CG_CRYPTO                    = (19 + 128),   /* bit 19, */
    HAL_CLOCK_CG_TRNG                      = (20 + 128),   /* bit 20, */
    HAL_CLOCK_CG_SPISLV_BUS                = (21 + 128),   /* bit 21, */
    HAL_CLOCK_CG_SDIOMST0                  = (22 + 128),   /* bit 22, */
    HAL_CLOCK_CG_USB_BUS                   = (23 + 128),   /* bit 23, */
    HAL_CLOCK_CG_USB_DMA                   = (24 + 128),   /* bit 24, */
    HAL_CLOCK_CG_SECURITY_PERI             = (25 + 128),   /* bit 25, */

    HAL_CLOCK_CG_BUS_ERR                   = (0 + 160),    /* bit 0, PDN_AO_CON0 */
    HAL_CLOCK_CG_SEC_MON                   = (1 + 160),    /* bit 1, */

    HAL_CLOCK_CG_CMSYS_ROM                 = (16 + 192),   /* bit 16, PDN_TOP_CON0 */
    HAL_CLOCK_CG_PHYS_NUM, //0
    /* Psuedo CGs */
    HAL_CLOCK_CG_PSUEDO_CLK_26M,
    HAL_CLOCK_CG_PSUEDO_CLK_OSC_26M,
    HAL_CLOCK_CG_PSUEDO_MCLK,
    HAL_CLOCK_CG_END,
    /* special CG control */
    /* 14 PWM channels, use bit idx 15 as PWM_MULTI_CTRL indicator */
    HAL_CLOCK_CG_PWM_MULTI_CTRL            = (0 + 0x8000) /* bit 15 indicates PWM_MULTI_CTRL request */
} hal_clock_cg_id;


/** @brief Use hal_src_clock in Clock API. */
/*************************************************************************
 * Define clock meter number.
 * Note: fill hal_src_clock into software clock meter api hal_clock_get_freq_meter as first parameter to get specific clock frequency in khz.
 *************************************************************************/
typedef enum {
    AD_26M_DBB_1P2 =1,
    NA,
    xo_ck,
    PAD_SOC_CK,
    PAD_CK,
    AD_DCXO_CLK26M,
    AD_UPLL_CLK_TEST,
    AD_APLL1_MON_CK,
    AD_APLL2_MON_CK,
    PAD_SOC_CK_,
    ROSC_CK,
    AD_UPLL_624M_CK,
    AD_UPLL_CK,
    AD_UPLL_48M_24M_CK,
    AD_OSC1_CK,
    AD_OSC1_D3_CK,
    AD_OSC1_SYNC_CK,
    AD_OSC2_CK,
    AD_OSC2_D3_CK,
    AD_OSC2_SYNC_CK,
    AD_APLL1_CK,
    AD_APLL2_CK,
    clk_pll1_d2,
    clk_pll1_d3,
    clk_pll1_d5,
    clk_osc1_d2,
    clk_osc1_d3,
    clk_osc1_d5,
    clk_osc1_d8,
    clk_osc1_d12,
    clk_osc2_d2,
    clk_osc2_d3=33,
    clk_osc2_d5,
    clk_osc2_d12,
    rtc_ck,
    hf_fsys_ck,
    hf_fdsp_ck,
    hf_fsfc_ck,
    hf_fesc_ck,
    hf_fspimst0_ck,
    hf_fspimst1_ck,
    hf_fspimst2_ck,
    hf_fspislv_ck,
    hf_fusb_ck,
    hf_fi3c_ck,
    hf_fbt_hop_ck,
    hf_faud_intbus_ck,
    hf_faud_gpsrc_ck,
    hf_faud_uplink_ck,
    hf_faud_dwlink_ck,
    hf_faud_spdif_ck,
    hf_faud_int0_ck,
    hf_faud_int1_ck,
    f_f26m_ck,
    hf_faud_engine_ck,
    hf_faud_vow_ck,
    f_fosc_26m_ck,
    _hf_fesc_ck_tmp,
    _hf_fesc_cg_ck,
    hf_faud_i2s0_m_ck,
    f_chop_ck,
    /* debug use definitions */
    FQMTR_SYS_DBG = 255, /* print CPU, DSP, SFC freqs */
}hal_src_clock;

/** @brief Use clock_mux_sel_id in Clock API. */
/*************************************************************************
 * Define clock mux number.
 * Note: fill clock_mux_sel_id into software clock mux select api clock_mux_sel as first parameter to select specific clock domain.
 *************************************************************************/


typedef enum {
    CLK_SYS_SEL,
    CLK_DSP_SEL,
    CLK_SFC_SEL,
    CLK_ESC_SEL,
    CLK_SPIMST0_SEL,
    CLK_SPIMST1_SEL,
    CLK_SPIMST2_SEL,
    CLK_SPISLV_SEL,

    CLK_SDIOMST0_SEL,
    CLK_USB_SEL,
    CLK_I3C_SEL,
    CLK_BT_HOP_SEL,
    CLK_AUD_BUS_SEL,
    CLK_AUD_GPSRC_SEL,
    CLK_AUD_ULCK_SEL,
    CLK_AUD_DLCK_SEL,
    CLK_AUD_INTERFACE0_SEL,
    CLK_AUD_INTERFACE1_SEL,
    CLK_26M_SEL,
    CLK_AUD_ENGINE_SEL,
    CLK_VOW_SEL,
    CLK_OSC_26M_SEL, //     CLK_PHYS_MUX_NUM,
    // Psuedo mux start
    //I2S_MCLK0/1/2/3
    CLK_MCLK_SEL,
    CLK_SPDIF_SEL,
    CLK_PWM0_SEL,
    CLK_PWM1_SEL,
    CLK_PWM2_SEL,
    CLK_PWM3_SEL,
    CLK_PWM4_SEL,
    CLK_PWM5_SEL,
    CLK_PWM6_SEL,
    CLK_PWM7_SEL,
    CLK_PWM8_SEL,
    CLK_PWM9_SEL,
    CLK_PWM10_SEL,
    CLK_PWM11_SEL,
    CLK_PWM12_SEL,
    CLK_PWM13_SEL,

    TOTAL_MUX_CNT //Total mux count
} clock_mux_sel_id;



#ifdef HAL_CLOCK_MODULE_ENABLED
/** @brief DVFS operating performance point (OPP) definition for driver/middleware usage.
 *  Depending on AIR_RFI_SUPPRESS_DISABLE config in chip.mk, CPU, DSP frequencies will be different.
 *  |     DVFS_OPP      | AIR_RFI_SUPPRESS_DISABLE = y | AIR_RFI_SUPPRESS_DISABLE = n    |
 *  | ----------------- | ---------------------------- | ------------------------------- |
 *  | HAL_DVFS_OPP_LOW  | CM33 104MHz, DSP 156MHz      | CM33 99.75MHz,   DSP 149.625MHz |
 *  | HAL_DVFS_OPP_MID  | CM33 156MHz, DSP 312MHz      | CM33 149.625MHz, DSP 299.25MHz  |
 *  | HAL_DVFS_OPP_HIGH | CM33 260MHz,  DSP 520MHz     | CM33 260MHz,     DSP 520MHz     |
 *  Note: Use this as the 1st parameter when calling DVFS API hal_dvfs_lock_control()
 */

typedef enum {
    HAL_DVFS_OPP_LOW = 0,
    HAL_DVFS_OPP_MID,
    HAL_DVFS_OPP_HIGH,
    HAL_DVFS_OPP_NUM,
} dvfs_frequency_t;
#endif


/**
  * @}
  */

/**
 * @}
 * @}
 */
#endif

#ifdef HAL_SW_DMA_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup SW_DMA
 * @{
 * @defgroup hal_sw_dma_define Define
 * @{
 */

/** @brief  The maximum number of SW DMA users.
 */
#define    SW_DMA_USER_NUM  (8)

/**
 * @}
 */


/**
 * @}
 * @}
 */
#endif

#ifdef __cplusplus
}
#endif

#endif /* __HAL_PLATFORM_H__ */


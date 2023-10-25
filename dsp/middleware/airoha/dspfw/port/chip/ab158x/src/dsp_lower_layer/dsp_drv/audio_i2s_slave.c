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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "source.h"
#include "stream_audio_setting.h"
#include "hal_nvic.h"
#include "FreeRTOS.h"
#include "stream_audio_driver.h"
#include "dsp_callback.h"
#include "dsp_audio_ctrl.h"
#include "hal_audio_afe_control.h"
#include "hal_audio_afe_define.h"
#include "audio_afe_common.h"
#include "hal_audio_driver.h"

extern afe_stream_channel_t connect_type[2][2];
extern afe_t afe;
extern ATTR_TEXT_IN_IRAM void i2s_slave_ul_port_interrupt_handler(vdma_event_t event, void  *user_data);

#include "hal_pdma_internal.h"
#include "memory_attribute.h"
#include "hal_gpio.h"

const vdma_channel_t g_i2s_slave_vdma_channel[] = {
    VDMA_I2S3TX, VDMA_I2S3RX,//I2S0 DMA TX(VDMA7),  I2S0 DMA RX(VDMA8)
    VDMA_I2S0TX, VDMA_I2S0RX,//I2S1 DMA TX(VDMA1),  I2S1 DMA RX(VDMA2)
    VDMA_I2S4TX, VDMA_I2S4RX,//I2S2 DMA TX(VDMA9),  I2S2 DMA RX(VDMA10)
};

static const uint32_t g_i2s_slave_infrasys_baseaddr[] = {
    AFE_I2S_SLV0_BASE,
    AFE_I2S_SLV1_BASE,
    AFE_I2S_SLV2_BASE,
};

static bool i2s_slave_port_set_flag[3] = {false, false, false};

#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_PREMIUM_G2)
typedef struct {
    hal_gpio_pin_t clk_pin;
    uint8_t clk_func;
    hal_gpio_pin_t ws_pin;
    uint8_t ws_func;
    hal_gpio_pin_t tx_pin;
    uint8_t tx_func;
    hal_gpio_pin_t rx_pin;
    uint8_t rx_func;
} i2s_slave_pin_cfg_t;

static const i2s_slave_pin_cfg_t g_i2s_slave_pin_cfg[] = {
#ifdef AIR_BTA_IC_PREMIUM_G3
    /* 0 */
    {
        HAL_GPIO_30, HAL_GPIO_30_I2S_SLV0_CK,
        HAL_GPIO_35, HAL_GPIO_35_I2S_SLV0_WS,
        HAL_GPIO_25, HAL_GPIO_25_I2S_SLV0_TX,
        HAL_GPIO_27, HAL_GPIO_27_I2S_SLV0_RX,
    },
    /* 1 */
    {
        HAL_GPIO_31, HAL_GPIO_31_I2S_SLV1_CK,
        HAL_GPIO_32, HAL_GPIO_32_I2S_SLV1_WS,
        HAL_GPIO_26, HAL_GPIO_26_I2S_SLV1_TX,
        HAL_GPIO_33, HAL_GPIO_33_I2S_SLV1_RX,
    },
    /* 2 */
    {
        HAL_GPIO_41, HAL_GPIO_41_I2S_SLV2_CK,
        HAL_GPIO_43, HAL_GPIO_43_I2S_SLV2_WS,
        HAL_GPIO_42, HAL_GPIO_42_I2S_SLV2_TX,
        HAL_GPIO_34, HAL_GPIO_34_I2S_SLV2_RX,
    },
#endif
#ifdef AIR_BTA_IC_PREMIUM_G2
    /* 0 */
    {
        HAL_GPIO_9, HAL_GPIO_9_I2S_SLV0_CK,
        HAL_GPIO_10, HAL_GPIO_10_I2S_SLV0_WS,
        HAL_GPIO_8, HAL_GPIO_8_I2S_SLV0_TX,
        HAL_GPIO_11, HAL_GPIO_11_I2S_SLV0_RX,
    },
    /* 1 */
    {
        HAL_GPIO_12, HAL_GPIO_12_I2S_SLV1_CK,
        HAL_GPIO_15, HAL_GPIO_15_I2S_SLV1_WS,
        HAL_GPIO_13, HAL_GPIO_13_I2S_SLV1_TX,
        HAL_GPIO_14, HAL_GPIO_14_I2S_SLV1_RX,
    },
    /* 2 */
    {
        HAL_GPIO_17, HAL_GPIO_17_I2S_SLV2_CK,
        HAL_GPIO_16, HAL_GPIO_16_I2S_SLV2_WS,
        HAL_GPIO_19, HAL_GPIO_19_I2S_SLV2_TX,
        HAL_GPIO_18, HAL_GPIO_18_I2S_SLV2_RX,
    },
#endif
};

static const uint32_t g_i2s_slave_pin_index[] = {0, 1, 2, 3};
#if 0
void print_reg_list(void)
{
    DSP_MW_LOG_I("GPIO mode setting", 0);
    DSP_MW_LOG_I("0xA20B0074 = 0x%08x", 1, *(volatile uint32_t *)0xA20B0074);
    DSP_MW_LOG_I("0xA20B0078 = 0x%08x", 1, *(volatile uint32_t *)0xA20B0078);

    DSP_MW_LOG_I("global PDN setting", 0);
    DSP_MW_LOG_I("0xA2270300 = 0x%08x", 1, *(volatile uint32_t *)0xA2270300);
    DSP_MW_LOG_I("0xA2270330 = 0x%08x", 1, *(volatile uint32_t *)0xA2270330);

    DSP_MW_LOG_I("global vdma setting", 0);
    DSP_MW_LOG_I("0xA3010000 = 0x%08x", 1, *(volatile uint32_t *)0xA3010000);
    DSP_MW_LOG_I("0xA3010070 = 0x%08x", 1, *(volatile uint32_t *)0xA3010070);
    DSP_MW_LOG_I("0xA3010028 = 0x%08x", 1, *(volatile uint32_t *)0xA3010028);
    DSP_MW_LOG_I("0xA3010030 = 0x%08x", 1, *(volatile uint32_t *)0xA3010030);
    DSP_MW_LOG_I("0xA3010020 = 0x%08x", 1, *(volatile uint32_t *)0xA3010020);
    DSP_MW_LOG_I("0xA3010040 = 0x%08x", 1, *(volatile uint32_t *)0xA3010040);
    DSP_MW_LOG_I("0xA3010044 = 0x%08x", 1, *(volatile uint32_t *)0xA3010044);

    DSP_MW_LOG_I("I2S slave 0 TX VDMA setting", 0);
    DSP_MW_LOG_I("0xA3010110 = 0x%08x", 1, *(volatile uint32_t *)0xA3010110);
    DSP_MW_LOG_I("0xA3010114 = 0x%08x", 1, *(volatile uint32_t *)0xA3010114);
    DSP_MW_LOG_I("0xA3010118 = 0x%08x", 1, *(volatile uint32_t *)0xA3010118);
    DSP_MW_LOG_I("0xA301011C = 0x%08x", 1, *(volatile uint32_t *)0xA301011C);
    DSP_MW_LOG_I("0xA3010120 = 0x%08x", 1, *(volatile uint32_t *)0xA3010120);
    DSP_MW_LOG_I("0xA3010128 = 0x%08x", 1, *(volatile uint32_t *)0xA3010128);
    DSP_MW_LOG_I("0xA301012C = 0x%08x", 1, *(volatile uint32_t *)0xA301012C);
    DSP_MW_LOG_I("0xA3010130 = 0x%08x", 1, *(volatile uint32_t *)0xA3010130);
    DSP_MW_LOG_I("0xA3010134 = 0x%08x", 1, *(volatile uint32_t *)0xA3010134);
    DSP_MW_LOG_I("0xA3010138 = 0x%08x", 1, *(volatile uint32_t *)0xA3010138);
    DSP_MW_LOG_I("0xA301013C = 0x%08x", 1, *(volatile uint32_t *)0xA301013C);
    DSP_MW_LOG_I("0xA3010140 = 0x%08x", 1, *(volatile uint32_t *)0xA3010140);
    DSP_MW_LOG_I("0xA3010144 = 0x%08x", 1, *(volatile uint32_t *)0xA3010144);
    DSP_MW_LOG_I("0xA3010160 = 0x%08x", 1, *(volatile uint32_t *)0xA3010160);
    DSP_MW_LOG_I("0xA3010164 = 0x%08x", 1, *(volatile uint32_t *)0xA3010164);
    DSP_MW_LOG_I("0xA3010168 = 0x%08x", 1, *(volatile uint32_t *)0xA3010168);
    DSP_MW_LOG_I("0xA301016C = 0x%08x", 1, *(volatile uint32_t *)0xA301016C);

    DSP_MW_LOG_I("I2S slave 0 RX VDMA setting", 0);
    DSP_MW_LOG_I("0xA3010210 = 0x%08x", 1, *(volatile uint32_t *)0xA3010210);
    DSP_MW_LOG_I("0xA3010214 = 0x%08x", 1, *(volatile uint32_t *)0xA3010214);
    DSP_MW_LOG_I("0xA3010218 = 0x%08x", 1, *(volatile uint32_t *)0xA3010218);
    DSP_MW_LOG_I("0xA301021C = 0x%08x", 1, *(volatile uint32_t *)0xA301021C);
    DSP_MW_LOG_I("0xA3010220 = 0x%08x", 1, *(volatile uint32_t *)0xA3010220);
    DSP_MW_LOG_I("0xA3010228 = 0x%08x", 1, *(volatile uint32_t *)0xA3010228);
    DSP_MW_LOG_I("0xA301022C = 0x%08x", 1, *(volatile uint32_t *)0xA301022C);
    DSP_MW_LOG_I("0xA3010230 = 0x%08x", 1, *(volatile uint32_t *)0xA3010230);
    DSP_MW_LOG_I("0xA3010234 = 0x%08x", 1, *(volatile uint32_t *)0xA3010234);
    DSP_MW_LOG_I("0xA3010238 = 0x%08x", 1, *(volatile uint32_t *)0xA3010238);
    DSP_MW_LOG_I("0xA301023C = 0x%08x", 1, *(volatile uint32_t *)0xA301023C);
    DSP_MW_LOG_I("0xA3010240 = 0x%08x", 1, *(volatile uint32_t *)0xA3010240);
    DSP_MW_LOG_I("0xA3010244 = 0x%08x", 1, *(volatile uint32_t *)0xA3010244);
    DSP_MW_LOG_I("0xA3010260 = 0x%08x", 1, *(volatile uint32_t *)0xA3010260);
    DSP_MW_LOG_I("0xA3010264 = 0x%08x", 1, *(volatile uint32_t *)0xA3010264);
    DSP_MW_LOG_I("0xA3010268 = 0x%08x", 1, *(volatile uint32_t *)0xA3010268);
    DSP_MW_LOG_I("0xA301026C = 0x%08x", 1, *(volatile uint32_t *)0xA301026C);

    DSP_MW_LOG_I("I2S slave 0 setting", 0);
    DSP_MW_LOG_I("[i2s slave reg] I2S1_GLOBAL_CONTROL    =0x%x",AFE_GET_REG(0xC9020000));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_DL_CONTROL        =0x%x",AFE_GET_REG(0xC9020004));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_UL_CONTROL        =0x%x",AFE_GET_REG(0xC9020008));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_SOFT_RESET        =0x%x",AFE_GET_REG(0xC902000c));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_DL_FIFO           =0x%x",AFE_GET_REG(0xC9020010));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_UL_FIFO           =0x%x",AFE_GET_REG(0xC9020014));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_DL_FIFO_STATUS    =0x%x",AFE_GET_REG(0xC9020018));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_UL_FIFO_STATUS    =0x%x",AFE_GET_REG(0xC902001c));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_SCAN_RSV          =0x%x",AFE_GET_REG(0xC9020020));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_GLOBAL_EN_CONTROL =0x%x",AFE_GET_REG(0xC9020030));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_DL_SR_EN_CONTROL  =0x%x",AFE_GET_REG(0xC9020034));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_UL_SR_EN_CONTROL  =0x%x",AFE_GET_REG(0xC9020038));
    DSP_MW_LOG_I("[i2s slave reg] I2S_MONITOR            =0x%x",AFE_GET_REG(0xC902003c));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_DL_INT_CONTROL    =0x%x",AFE_GET_REG(0xC9020040));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_UL_INT_CONTROL    =0x%x",AFE_GET_REG(0xC9020044));
    DSP_MW_LOG_I("[i2s slave reg] I2S1_INT_ACK_CONTROL   =0x%x",AFE_GET_REG(0xC9020048));
    DSP_MW_LOG_I("[i2s slave reg] I2S_SHARE_CK_CONTROL   =0x%x",AFE_GET_REG(0xC9020050));
    DSP_MW_LOG_I("[i2s slave reg] I2S_PLAY_EN_CONTROL    =0x%x",AFE_GET_REG(0xC9020054));
    DSP_MW_LOG_I("[i2s slave reg] I2S_INIT_DELAY_CNT_MON =0x%x",AFE_GET_REG(0xC9020058));
    DSP_MW_LOG_I("[i2s slave reg] I2S_SHARE_EN_CONTROL   =0x%x",AFE_GET_REG(0xC902005c));
    DSP_MW_LOG_I("====================================================================", 0);
}
#endif
#endif
static volatile uint32_t g_i2s_slave_port_ref_count = 0;

extern void i2s_slave_ul_interrupt_handler(vdma_event_t event, void  *user_data);
extern ATTR_TEXT_IN_IRAM void i2s_slave_dl_interrupt_handler(vdma_event_t event, void  *user_data);
extern uint32_t i2s_slave_port_translate(hal_audio_interface_t audio_interface);

static void internal_i2s_slave_common_init(uint32_t port)
{
#if 0
    uint32_t infrasys_baseaddr;
#endif

#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_PREMIUM_G2)
    int32_t index;
    /* GPIO configure */
    index = g_i2s_slave_pin_index[port];
    hal_gpio_init(g_i2s_slave_pin_cfg[index].tx_pin);
    hal_pinmux_set_function(g_i2s_slave_pin_cfg[index].tx_pin, g_i2s_slave_pin_cfg[index].tx_func);
    hal_gpio_init(g_i2s_slave_pin_cfg[index].clk_pin);
    hal_pinmux_set_function(g_i2s_slave_pin_cfg[index].clk_pin, g_i2s_slave_pin_cfg[index].clk_func);
    hal_gpio_init(g_i2s_slave_pin_cfg[index].ws_pin);
    hal_pinmux_set_function(g_i2s_slave_pin_cfg[index].ws_pin, g_i2s_slave_pin_cfg[index].ws_func);
    hal_gpio_init(g_i2s_slave_pin_cfg[index].rx_pin);
    hal_pinmux_set_function(g_i2s_slave_pin_cfg[index].rx_pin, g_i2s_slave_pin_cfg[index].rx_func);
#else
    UNUSED(port);
#endif
#if 0
    /* Set I2S slave PDN on */
    *(volatile uint32_t *)(0xA2270320) |= 1 << (4 + port);

    /* Set RG_SW_I2S_DMA_CG on */
    *(volatile uint32_t *)(0xA2270350) |= 1 << 12;

    /* Set Infra I2S setting */
    infrasys_baseaddr = g_i2s_slave_infrasys_baseaddr[port];
    *(volatile uint32_t *)(infrasys_baseaddr + 0x00) = 0x20028; /* default setting */
    *(volatile uint32_t *)(infrasys_baseaddr + 0x30) &= ~(1 << 24); /* PDN clear 26M */
#endif
}

static void internal_i2s_slave_common_deinit(uint32_t port)
{
    uint32_t infrasys_baseaddr;

    infrasys_baseaddr = g_i2s_slave_infrasys_baseaddr[port];

    /* PDN clear 26M */
    *(volatile uint32_t *)(infrasys_baseaddr + 0x30) |= 1 << 24;

    /* Clear PDN on */
    *(volatile uint32_t *)(0xA2030B80) = 1 << (4 + port);
}

/***************************************************
                I2 slave RX side
***************************************************/

static void ul_global_var_init(SOURCE source)
{
#if 0
    afe_block_t *afe_block = &source->param.audio.AfeBlkControl;

    memset(afe_block, 0, sizeof(afe_block_t));
#else
    UNUSED(source);
#endif
}

static int32_t i2s_slave_ul_probe(SOURCE source)
{
    hal_audio_device_t device = source->param.audio.audio_device;

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        return -1;
    }

    ul_global_var_init(source);

    return 0;
}
static hal_audio_agent_t hal_audio_agent = HAL_AUDIO_AGENT_ERROR;
i2s_slave_irq_user_data_t g_slave_vdma_irq_user_data[AUDIO_I2S_SLAVE_BLOCK_NUMBER] = {0};
static int32_t i2s_slave_ul_start(SOURCE source)
{
    uint32_t port;
#if 0
    vdma_config_t dma_config;
    vdma_status_t i2s_rx_vdma_status;
    afe_i2s_wlen_t i2s_wlen;
    afe_i2s_format_t i2s_format;
    hal_audio_memory_selection_t memory_select, fined_memory = 0, memory_search; //modify for ab1568z
#endif
    vdma_channel_t rx_dma_channel;
    uint32_t infrasys_baseaddr;
    hal_audio_interface_t audio_interface = source->param.audio.audio_interface;
    hal_audio_device_t device = source->param.audio.audio_device;
    afe_block_t *afe_block = &source->param.audio.AfeBlkControl;
    AUDIO_PARAMETER *runtime = &source->param.audio;
    hal_audio_memory_parameter_t *mem_handle = &source->param.audio.mem_handle;//modify for ab1568
    afe_mem_asrc_id_t asrc_id = MEM_ASRC_NUM;
    //uint32_t channel_index = 0;
    uint32_t mem_size = 0;
    U8 *mem_ptr;
    DSP_MW_LOG_I("DSP i2s_slave_ul_start() enter\r\n", 0);

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        configASSERT(0 && "[i2s slave] err device");
    }
    port = i2s_slave_port_translate(audio_interface);
    if (i2s_slave_port_set_flag[port] != false) {
        DSP_MW_LOG_W("[i2s slave]port %d started\r\n", 1, port);
        return -1;
    }
    internal_i2s_slave_common_init(port);
    g_i2s_slave_port_ref_count++;
    //internal_i2s_slave_common_init(port); //set i2s slave gpio
    rx_dma_channel = g_i2s_slave_vdma_channel[port * 2 + 1];
    infrasys_baseaddr = g_i2s_slave_infrasys_baseaddr[port];
    mem_handle->audio_path_rate = runtime->rate;
    mem_handle->buffer_addr = afe_block->phys_buffer_addr;
    mem_handle->buffer_length = runtime->buffer_size;
    DSP_MW_LOG_I("DSP UL start source type:%d, %d, memory select:0x%x dma_ch %d\r\n", 4,
        source->type,
        source->scenario_type,
        mem_handle->memory_select,
        rx_dma_channel
        );//modify for ab1568
    /* set irq start */
    if (!source->param.audio.AfeBlkControl.u4awsflag) {
        mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_NONE;
    } else {
        mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_SW_TRIGGER;
    }
    if (runtime->AfeBlkControl.u4asrcflag) {
        //asrc tracking source sel
        AFE_SET_REG(infrasys_baseaddr + 0x50,  port << 24, 0x3000000);
    }
    mem_size = mem_handle->buffer_length;
    if (runtime->AfeBlkControl.u4asrcflag) {
        mem_size += runtime->AfeBlkControl.u4asrc_buffer_size;
    }
#ifdef AIR_BTA_IC_PREMIUM_G2
    if (runtime->audio_interface == HAL_AUDIO_INTERFACE_1) {
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S0_SLAVE;
    } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_2) {
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S1_SLAVE;
    } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_3) {
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S2_SLAVE;
    }
#else
    hal_audio_agent = hal_device_convert_agent(device, audio_interface, false);
#endif
    afe_block->phys_buffer_addr = hal_memory_allocate_sram(mem_handle->scenario_type, hal_audio_agent, mem_size);

    mem_ptr = (U8 *)afe_block->phys_buffer_addr;

    if (source->param.audio.AfeBlkControl.u4asrcflag) {
        mem_ptr += source->param.audio.AfeBlkControl.u4asrc_buffer_size;
    }
    source->streamBuffer.BufferInfo.startaddr[0] = mem_ptr;

    hal_audio_slave_vdma_parameter_t vdma_setting;
    vdma_setting.is_start_now = true;
    vdma_setting.base_address = afe_block->phys_buffer_addr;
    vdma_setting.size = mem_handle->buffer_length >> 2;
    vdma_setting.threshold = mem_handle->buffer_length >> 2;
    vdma_setting.audio_interface = runtime->audio_interface;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    vdma_setting.tdm_channel = HAL_AUDIO_I2S_TDM_DISABLE;
    vdma_setting.enable = true;
#endif
#if defined AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE || defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
    if (runtime->AfeBlkControl.u4asrcflag) {
        afe_src_configuration_t asrc_config;
        afe_set_asrc_ul_configuration_parameters(source, &asrc_config);
        /* asrc check enable */
        for (uint8_t i = 0; i < MEM_ASRC_NUM; i ++) {
            if (AFE_READ(ASM_GEN_CONF + 0x100 * i) & (1 << ASM_GEN_CONF_ASRC_EN_POS )) {
                continue;
            } else {
                asrc_id = i;
                #ifdef AIR_BTA_IC_PREMIUM_G3
                // // For g3 chip, AFE_MEM_ASRC_3's value is strange! We should do special handle for it.
                // if (asrc_id == (MEM_ASRC_NUM - 1)) {
                //     asrc_id = AFE_MEM_ASRC_3;
                // }
                #endif
                break;
            }
        }
        if (asrc_id == MEM_ASRC_NUM) {
            AUDIO_ASSERT(0 && "i2s slave error: hwsrc block is not enough, plz check hardware");
        }
        afe_set_asrc_enable(true, asrc_id, (void *)&asrc_config);
        runtime->AfeBlkControl.u4asrcid = asrc_id;
        source->param.audio.AfeBlkControl.u4ReadIdx = AFE_GET_REG(ASM_CH01_IBUF_RDPNT + 0x100 * asrc_id) - AFE_GET_REG(ASM_IBUF_SADR + 0x100 * asrc_id);
        source->param.audio.AfeBlkControl.u4WriteIdx = AFE_GET_REG(ASM_CH01_IBUF_WRPNT + 0x100 * asrc_id) - AFE_GET_REG(ASM_IBUF_SADR + 0x100 * asrc_id);
        hal_src_set_start(asrc_id, true);
        vdma_setting.size = runtime->AfeBlkControl.u4asrc_buffer_size / 4;
        vdma_setting.threshold = 160; // 160 bytes for 8K/16K/32K/44.1K/48K/96K, the max period is 2.5ms.
    }
#else
    if (runtime->memory == HAL_AUDIO_MEM2) {
        if (runtime->AfeBlkControl.u4asrcflag) {
            afe_src_configuration_t asrc_config;
            afe_set_asrc_ul_configuration_parameters(source, &asrc_config);
            afe_set_asrc_enable(true, AFE_MEM_ASRC_2, (void *)&asrc_config);
            source->param.audio.AfeBlkControl.u4ReadIdx = AFE_GET_REG(ASM2_CH01_IBUF_RDPNT) - AFE_GET_REG(ASM2_IBUF_SADR);
            source->param.audio.AfeBlkControl.u4WriteIdx = AFE_GET_REG(ASM2_CH01_IBUF_WRPNT) - AFE_GET_REG(ASM2_IBUF_SADR);
            // afe_mem_asrc_enable(AFE_MEM_ASRC_2, true);
            hal_src_set_start(AFE_MEM_ASRC_2, true);
            asrc_id = AFE_MEM_ASRC_2;
            vdma_setting.size = runtime->AfeBlkControl.u4asrc_buffer_size / 4;
            vdma_setting.threshold = runtime->AfeBlkControl.u4asrc_buffer_size / 2;
        }
    } else {
        if (runtime->AfeBlkControl.u4asrcflag) {
            afe_src_configuration_t asrc_config;
            afe_set_asrc_ul_configuration_parameters(source, &asrc_config);
            afe_set_asrc_enable(true, AFE_MEM_ASRC_1, (void *)&asrc_config);
            source->param.audio.AfeBlkControl.u4ReadIdx = AFE_GET_REG(ASM_CH01_IBUF_RDPNT) - AFE_GET_REG(ASM_IBUF_SADR);
            source->param.audio.AfeBlkControl.u4WriteIdx = AFE_GET_REG(ASM_CH01_IBUF_WRPNT) - AFE_GET_REG(ASM_IBUF_SADR);
            // afe_mem_asrc_enable(AFE_MEM_ASRC_1, true);
            hal_src_set_start(AFE_MEM_ASRC_1, true);
            asrc_id = AFE_MEM_ASRC_1;
            vdma_setting.size = runtime->AfeBlkControl.u4asrc_buffer_size / 4;
            vdma_setting.threshold = runtime->AfeBlkControl.u4asrc_buffer_size / 2;
        }
    }
#endif
    if (runtime->audio_interface == HAL_AUDIO_INTERFACE_1) {
        #ifdef AIR_BTA_IC_PREMIUM_G2
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S0_SLAVE;
        #else
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S0_SLAVE_TX;
        #endif
        g_slave_vdma_irq_user_data[0].source       = source;
        g_slave_vdma_irq_user_data[0].vdma_channel = rx_dma_channel;
        g_slave_vdma_irq_user_data[0].asrc_id      = asrc_id;
        g_slave_vdma_irq_user_data[0].vdma_threshold_samples = vdma_setting.threshold;
        vdma_register_callback(rx_dma_channel, i2s_slave_ul_port_interrupt_handler, (void*)&g_slave_vdma_irq_user_data[0]);
    } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_2) {
        #ifdef AIR_BTA_IC_PREMIUM_G2
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S1_SLAVE;
        #else
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S1_SLAVE_TX;
        #endif
        g_slave_vdma_irq_user_data[1].source       = source;
        g_slave_vdma_irq_user_data[1].vdma_channel = rx_dma_channel;
        g_slave_vdma_irq_user_data[1].asrc_id      = asrc_id;
        g_slave_vdma_irq_user_data[1].vdma_threshold_samples = vdma_setting.threshold;
        vdma_register_callback(rx_dma_channel, i2s_slave_ul_port_interrupt_handler, (void*)&g_slave_vdma_irq_user_data[1]);
    } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_3) {
        #ifdef AIR_BTA_IC_PREMIUM_G2
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S2_SLAVE;
        #else
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S2_SLAVE_TX;
        #endif
        g_slave_vdma_irq_user_data[2].source       = source;
        g_slave_vdma_irq_user_data[2].vdma_channel = rx_dma_channel;
        g_slave_vdma_irq_user_data[2].asrc_id      = asrc_id;
        g_slave_vdma_irq_user_data[2].vdma_threshold_samples = vdma_setting.threshold;
        vdma_register_callback(rx_dma_channel, i2s_slave_ul_port_interrupt_handler, (void*)&g_slave_vdma_irq_user_data[2]);
    }
    vdma_setting.is_ul_mode = true;
    if ((source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN) ||
        (source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0) ||
        (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) ||
        ((source->scenario_type >= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0) &&
         (source->scenario_type <= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2))) {
        /* user trigger vdma */
        vdma_setting.is_start_now = false;
    }
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&vdma_setting, HAL_AUDIO_SET_SLAVE_VDMA);
#if defined AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE || defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
    DSP_MW_LOG_I("[Dongle I2S VDMA Mode] vdma addr 0x%x size %d threshold %d, src_%d in_addr 0x%x size %d, out_addr 0x%x size %d oro 0x%x owo 0x%x iro 0x%x iwo 0x%x", 12,
        vdma_setting.base_address, vdma_setting.size, vdma_setting.threshold,
        asrc_id,
        AFE_GET_REG(ASM_IBUF_SADR + asrc_id * 0x100),
        AFE_GET_REG(ASM_IBUF_SIZE + asrc_id * 0x100),
        AFE_GET_REG(ASM_OBUF_SADR + asrc_id * 0x100),
        AFE_GET_REG(ASM_OBUF_SIZE + asrc_id * 0x100),
        AFE_GET_REG(ASM_CH01_OBUF_RDPNT + asrc_id * 0x100),
        AFE_GET_REG(ASM_CH01_OBUF_WRPNT + asrc_id * 0x100),
        AFE_GET_REG(ASM_CH01_IBUF_RDPNT + asrc_id * 0x100),
        AFE_GET_REG(ASM_CH01_IBUF_WRPNT + asrc_id * 0x100)
        );
#else
    if (runtime->audio_interface == HAL_AUDIO_INTERFACE_1) {
        Source_Audio_BufferInfo_Rst(source, AFE_READ(0xC9000830) - AFE_READ(0xC900082c));
    } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_2) {
        Source_Audio_BufferInfo_Rst(source, AFE_READ(0xC9000230) - AFE_READ(0xC900022c));
    } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_3) {
        Source_Audio_BufferInfo_Rst(source, AFE_READ(0xC9000a30) - AFE_READ(0xC9000a2c));
    }
#endif /* AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
    mem_handle->memory_select = 0; // avoid sub source wrong process.
    source->param.audio.is_memory_start = true;
    i2s_slave_port_set_flag[port] = true;
    //print_reg_list();
    return 0;
}

static int32_t i2s_slave_ul_stop(SOURCE source)
{
    vdma_channel_t rx_dma_channel;
    // uint32_t irq_status;
    uint32_t port;
    hal_audio_interface_t audio_interface = source->param.audio.audio_interface;
    hal_audio_device_t device = source->param.audio.audio_device;
    AUDIO_PARAMETER *runtime = &source->param.audio;

    source->param.audio.is_memory_start = false;
    DSP_MW_LOG_I("DSP i2s_slave_ul_stop() enter\r\n", 0);

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        return -1;
    }

    port = i2s_slave_port_translate(audio_interface);

    if (i2s_slave_port_set_flag[port] != true) {
        DSP_MW_LOG_W("[i2s slave]port %d stop\r\n", 1, port);
        return -1;
    }

    rx_dma_channel = g_i2s_slave_vdma_channel[port * 2 + 1];

    // hal_nvic_save_and_set_interrupt_mask(&irq_status);

    hal_audio_device_parameter_t *device_handle = &source->param.audio.device_handle;

    /* Disable I2S out */
    // *(volatile uint32_t *)(infrasys_baseaddr + 0x30) &= ~(1 << 16);
    // *(volatile uint32_t *)(infrasys_baseaddr + 0x38) &= ~(1 << 0);
    // /* Clear FIFO status */
    // *(volatile uint32_t *)(infrasys_baseaddr + 0x14) |= 1 << 0;
    // *(volatile uint32_t *)(infrasys_baseaddr + 0x14) |= 1 << 16;
    // *(volatile uint32_t *)(infrasys_baseaddr + 0x14) &= ~(1 << 0);
    // *(volatile uint32_t *)(infrasys_baseaddr + 0x14) &= ~(1 << 16);
    // /* PDN clear I2S in clk */
    // *(volatile uint32_t *)(infrasys_baseaddr + 0x38) |= 1 << 16;


    // close hwsrc
#if defined AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE || defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
    if (runtime->AfeBlkControl.u4asrcflag) {
        afe_set_asrc_enable(false, runtime->AfeBlkControl.u4asrcid, NULL);
        // afe_mem_asrc_enable(AFE_MEM_ASRC_1, false);
        hal_src_set_start(runtime->AfeBlkControl.u4asrcid, false);
    }
#else
    if (runtime->memory == HAL_AUDIO_MEM2) {
        if (runtime->AfeBlkControl.u4asrcflag) {
            afe_set_asrc_enable(false, AFE_MEM_ASRC_2, NULL);
            // afe_mem_asrc_enable(AFE_MEM_ASRC_2, false);
            hal_src_set_start(AFE_MEM_ASRC_2, false);
        }
    } else {
        if (runtime->AfeBlkControl.u4asrcflag) {
            afe_set_asrc_enable(false, AFE_MEM_ASRC_1, NULL);
            // afe_mem_asrc_enable(AFE_MEM_ASRC_1, false);
            hal_src_set_start(AFE_MEM_ASRC_1, false);
        }
    }
#endif

    vdma_disable_interrupt(rx_dma_channel);
    vdma_stop(rx_dma_channel);
    vdma_deinit(rx_dma_channel);
    hal_audio_device_set_agent(device_handle, device, HAL_AUDIO_CONTROL_OFF); //close i2s

    //free memory
#ifdef AIR_BTA_IC_PREMIUM_G2
    if (runtime->audio_interface == HAL_AUDIO_INTERFACE_1) {
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S0_SLAVE;
    } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_2) {
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S1_SLAVE;
    } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_3) {
        hal_audio_agent = HAL_AUDIO_AGENT_DEVICE_I2S2_SLAVE;
    }
#else
    hal_audio_agent = hal_device_convert_agent(device, audio_interface, false);
#endif
    hal_memory_free_sram(source->param.audio.mem_handle.scenario_type, hal_audio_agent);
    hal_audio_agent = HAL_AUDIO_AGENT_ERROR;

    return 0;
}

static int32_t i2s_slave_ul_hw_params(SOURCE source)
{
    hal_audio_device_t device = source->param.audio.audio_device;

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        return -1;
    }

    return 0;
}

static int32_t i2s_slave_ul_open(SOURCE source)
{
    uint32_t source_ch;
    uint32_t port;
    hal_audio_interface_t audio_interface = source->param.audio.audio_interface;
    AUDIO_PARAMETER *runtime = &source->param.audio;
    hal_audio_device_t device = source->param.audio.audio_device;

    DSP_MW_LOG_I("DSP i2s_slave_ul_open() enter\r\n", 0);

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        return -1;
    }
    port = i2s_slave_port_translate(audio_interface);
    if (i2s_slave_port_set_flag[port] != false) {
        DSP_MW_LOG_W("[i2s slave]port %d started\r\n", 1, port);
        return -1;
    }
    source_ch = (runtime->channel_num > 2)
                ? 1
                : runtime->channel_num - 1;
    runtime->connect_channel_type = connect_type[source_ch][source_ch];

    hal_audio_device_parameter_t *device_handle = &source->param.audio.device_handle;
    hal_audio_device_set_agent(device_handle, device, HAL_AUDIO_CONTROL_ON);

    return 0;
}

static int32_t i2s_slave_ul_close(SOURCE source)
{
    hal_audio_device_t device = source->param.audio.audio_device;
    uint32_t port;
    hal_audio_interface_t audio_interface = source->param.audio.audio_interface;

    DSP_MW_LOG_I("DSP i2s_slave_ul_close() enter\r\n", 0);

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        return -1;
    }
    port = i2s_slave_port_translate(audio_interface);
    if (i2s_slave_port_set_flag[port] != true) {
        DSP_MW_LOG_W("[i2s slave]port %d closed\r\n", 1, port);
        return -1;
    }
    // hal_audio_path_parameter_t *path_handle = &source->param.audio.path_handle;
    // hal_audio_set_path(path_handle, HAL_AUDIO_CONTROL_OFF);
    i2s_slave_port_set_flag[port] = false;
    return 0;
}

static int32_t i2s_slave_ul_trigger(SOURCE source, int cmd)
{
    int32_t ret;

    switch (cmd) {
        case AFE_PCM_TRIGGER_START:
            ret = i2s_slave_ul_start(source);
            break;
        case AFE_PCM_TRIGGER_STOP:
            ret = i2s_slave_ul_stop(source);
            break;
        case AFE_PCM_TRIGGER_RESUME:
            ret = i2s_slave_ul_open(source);
            break;
        case AFE_PCM_TRIGGER_SUSPEND:
            ret = i2s_slave_ul_close(source);
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}

static int32_t i2s_slave_ul_copy(SOURCE source, void *dst, uint32_t count)
{
    //copy the AFE src streambuffer to sink streambuffer
    if (Source_Audio_ReadAudioBuffer(source, dst, count) == false) {
        return -1;
    }
    return 0;
}

audio_source_pcm_ops_t i2s_slave_ul_ops = {
    .probe      = i2s_slave_ul_probe,
    .open       = i2s_slave_ul_open,
    .close      = i2s_slave_ul_close,
    .hw_params  = i2s_slave_ul_hw_params,
    .trigger    = i2s_slave_ul_trigger,
    .copy       = i2s_slave_ul_copy,
};

/***************************************************
                I2 slave TX side
***************************************************/
static void dl_global_var_init(SINK sink)
{
#if 0
    afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;

    memset(afe_block, 0, sizeof(afe_block_t));
#else
    UNUSED(sink);
#endif
}

static int32_t i2s_slave_dl_probe(SINK sink)
{
    hal_audio_device_t device = sink->param.audio.audio_device;

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        return -1;
    }

    dl_global_var_init(sink);

    return 0;
}

static int32_t i2s_slave_dl_start(SINK sink)
{
    uint32_t port;
    vdma_channel_t tx_dma_channel;
    vdma_config_t dma_config;
    afe_i2s_wlen_t i2s_wlen;
    afe_i2s_format_t i2s_format;
    uint32_t infrasys_baseaddr, irq_status;
    afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;
    hal_audio_interface_t audio_interface = sink->param.audio.audio_interface;
    hal_audio_device_t device = sink->param.audio.audio_device;
    AUDIO_PARAMETER *runtime = &sink->param.audio;

    DSP_MW_LOG_I("DSP i2s_slave_dl_start() enter\r\n", 0);

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        configASSERT(0 && "[i2s slave] err device");
    }

    hal_nvic_save_and_set_interrupt_mask(&irq_status);

    DSP_MW_LOG_I("rate = %d, count = %d, phys_buffer_addr = %x, u4BufferSize = %d", 4,
                 runtime->rate,
                 runtime->count,
                 runtime->AfeBlkControl.phys_buffer_addr,
                 runtime->AfeBlkControl.u4BufferSize);

    port = i2s_slave_port_translate(audio_interface);
    infrasys_baseaddr = g_i2s_slave_infrasys_baseaddr[port];

    if (g_i2s_slave_port_ref_count == 0) {
        internal_i2s_slave_common_init(port);
    }
    g_i2s_slave_port_ref_count++;

    *(volatile uint32_t *)(0xA3010074) |= (1 << (port * 2)); /* Set Infra DMA CLK */
    *(volatile uint32_t *)(0xA301000C) |= (1 << (port * 2)); /* Set Infra DMA IRQ */

    *(volatile uint32_t *)(infrasys_baseaddr + 0x34) &= ~(1 << 16); /* pdn clear I2S out clk */

    i2s_format = AFE_I2S_SETTING_FORMAT;
    i2s_wlen = AFE_I2S_SETTING_WORD_LENGTH;
    if (i2s_format == I2S_LJ) {
        *(volatile uint32_t *)(infrasys_baseaddr + 0x04) = 0x40400A7; /* DL */
    } else if (i2s_format == I2S_RJ) {
        *(volatile uint32_t *)(infrasys_baseaddr + 0x04) = 0x40460A7; /* DL */
    } else {
        if (i2s_wlen == I2S_16BIT) {
            *(volatile uint32_t *)(infrasys_baseaddr + 0x04) = 0x0000D; /* DL */
        } else {
            *(volatile uint32_t *)(infrasys_baseaddr + 0x04) = 0x4008F; /* DL */
        }
    }

    *(volatile uint32_t *)(infrasys_baseaddr + 0x30) |= 1 << 8; /* DL FIFO EN */
    *(volatile uint32_t *)(infrasys_baseaddr + 0x34) |= 1 << 0; /* I2S OUT enable */

    *(volatile uint32_t *)(infrasys_baseaddr + 0x30) |= 1 << 0; /* ENABLE */

    tx_dma_channel = g_i2s_slave_vdma_channel[port * 2];
    vdma_init(tx_dma_channel);
    dma_config.base_address = afe_block->phys_buffer_addr;
    dma_config.size = afe_block->u4BufferSize;
    vdma_configure(tx_dma_channel, &dma_config);
    vdma_set_threshold(tx_dma_channel, 0);
    vdma_register_callback(tx_dma_channel, i2s_slave_dl_interrupt_handler, NULL);
    vdma_start(tx_dma_channel);

    /* Insert dummy audio data to avoid IRQ triggered immediately after vdma_enable_interrupt() is called. */
    /*
        for (i = 0; i < afe_block->u4BufferSize; i++) {
            vdma_push_data_4bytes(tx_dma_channel, 0x00000000);
        }

        vdma_enable_interrupt(tx_dma_channel);
    */
    hal_nvic_restore_interrupt_mask(irq_status);

    //print_reg_list();

    return 0;
}

static int32_t i2s_slave_dl_stop(SINK sink)
{
    vdma_channel_t tx_dma_channel;
    uint32_t infrasys_baseaddr, irq_status;
    uint32_t port;
    hal_audio_interface_t audio_interface = sink->param.audio.audio_interface;
    hal_audio_device_t device = sink->param.audio.audio_device;

    DSP_MW_LOG_I("DSP i2s_slave_dl_stop() enter\r\n", 0);

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        return -1;
    }

    if (g_i2s_slave_port_ref_count == 0) {
        DSP_MW_LOG_I("found g_i2s_slave_port_ref_count abnormal", 0);
        return -1;
    }

    port = i2s_slave_port_translate(audio_interface);
    tx_dma_channel = g_i2s_slave_vdma_channel[port * 2];
    infrasys_baseaddr = g_i2s_slave_infrasys_baseaddr[port];

    hal_nvic_save_and_set_interrupt_mask(&irq_status);

    /* Disable I2S out */
    *(volatile uint32_t *)(infrasys_baseaddr + 0x30) &= ~(1 << 8);
    *(volatile uint32_t *)(infrasys_baseaddr + 0x34) &= ~(1 << 0);
    /* Clear FIFO status */
    *(volatile uint32_t *)(infrasys_baseaddr + 0x10) |= 1 << 0;
    *(volatile uint32_t *)(infrasys_baseaddr + 0x10) |= 1 << 16;
    *(volatile uint32_t *)(infrasys_baseaddr + 0x10) &= ~(1 << 0);
    *(volatile uint32_t *)(infrasys_baseaddr + 0x10) &= ~(1 << 16);
    /* PDN clear I2S in clk */
    *(volatile uint32_t *)(infrasys_baseaddr + 0x34) |= 1 << 16;

    g_i2s_slave_port_ref_count--;
    if (g_i2s_slave_port_ref_count == 0) {
        internal_i2s_slave_common_deinit(port);
    }

    /* VDMA deinit */
    vdma_disable_interrupt(tx_dma_channel);
    vdma_stop(tx_dma_channel);
    vdma_deinit(tx_dma_channel);

    hal_nvic_restore_interrupt_mask(irq_status);

    return 0;
}

static int32_t i2s_slave_dl_hw_params(SINK sink)
{
    hal_audio_device_t device = sink->param.audio.audio_device;

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        return -1;
    }

    return 0;
}

static int32_t i2s_slave_dl_open(SINK sink)
{
    uint32_t source_ch;
    AUDIO_PARAMETER *runtime = &sink->param.audio;
    hal_audio_device_t device = sink->param.audio.audio_device;

    DSP_MW_LOG_I("DSP i2s_slave_dl_open() enter\r\n", 0);

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        return -1;
    }

    source_ch = (runtime->channel_num > 2)
                ? 1
                : runtime->channel_num - 1;
    runtime->connect_channel_type = connect_type[source_ch][source_ch];

    return 0;
}

static int32_t i2s_slave_dl_close(SINK sink)
{
    hal_audio_device_t device = sink->param.audio.audio_device;

    DSP_MW_LOG_I("DSP i2s_slave_dl_close() enter\r\n", 0);

    if (device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        return -1;
    }

    return 0;
}

static int32_t i2s_slave_dl_trigger(SINK sink, int cmd)
{

    switch (cmd) {
        case AFE_PCM_TRIGGER_START:
            return i2s_slave_dl_start(sink);
            break;
        case AFE_PCM_TRIGGER_STOP:
            return i2s_slave_dl_stop(sink);
            break;
        case AFE_PCM_TRIGGER_RESUME:
            return i2s_slave_dl_open(sink);
            break;
        case AFE_PCM_TRIGGER_SUSPEND:
            return i2s_slave_dl_close(sink);
            break;
        default:
            break;
    }

    return -1;
}

// src: Source Streambuffer not Sink Streambuffer
static int32_t i2s_slave_dl_copy(SINK sink, void *src, uint32_t count)
{
    // count: w/o channl, unit: bytes
    // copy the src's streambuffer to sink's streambuffer
    if (Sink_Audio_WriteBuffer(sink, src, count) == false) {
        return -1;
    }
    return 0;
}

audio_sink_pcm_ops_t i2s_slave_dl_ops = {
    .probe      = i2s_slave_dl_probe,
    .open       = i2s_slave_dl_open,
    .close      = i2s_slave_dl_close,
    .hw_params  = i2s_slave_dl_hw_params,
    .trigger    = i2s_slave_dl_trigger,
    .copy       = i2s_slave_dl_copy,
};
#if 0
void i2s_slave_dl_update_wptr(SINK sink, U32 amount)
{
    uint32_t port;
    vdma_channel_t tx_dma_channel;

    port = i2s_slave_port_translate(sink->param.audio.audio_interface);
    tx_dma_channel = g_i2s_slave_vdma_channel[port * 2];
    vdma_set_sw_move_byte(tx_dma_channel, amount);
}
#endif

/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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
#include "dsp_temp.h"
#include "bt_interface.h"
#ifdef AIR_DCHS_MODE_ENABLE
#include "mux_ll_uart.h"
#endif
#define BTCLK_LEN                   4
#define STATE_LEN                   4
#define VOICE_HEADER                (BTCLK_LEN + STATE_LEN)
#define ESCO_UL_ERROR_DETECT_THD    (8)
#define BT_FRAME_UNIT               (2500)
#define BT_NCLK_MASK                (0x0FFFFFFC)
#define BT_SLOT_UNIT                (625)
#define BIT_MASK(n)         (1UL << (n))
#define BT_CLOCK_TICK_MASK  (0x0FFFFFFC)
#define BT_CLOCK_MAX_WRAP   (0x10000000)

#ifdef AIR_DCHS_MODE_ENABLE
#define DCHS_PHASE_OFFSET_REG       (0xA0010978)
#define DCHS_CLK_OFFSET_REG         (0xA0010974)
#endif

#ifdef AIR_CPU_IN_SECURITY_MODE
VOLATILE t_hardware_baseband_registers *rBb = (VOLATILE t_hardware_baseband_registers *)0xA0010000;
#else
VOLATILE t_hardware_baseband_registers *rBb = (VOLATILE t_hardware_baseband_registers *)0xB0010000;
#endif
VOLATILE ULL_CLK_INFO_PTR rBb_ull = NULL;

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
#define FEATURE_DONGLE_IN_IRAM ATTR_TEXT_IN_IRAM
#else
#define FEATURE_DONGLE_IN_IRAM
#endif

FEATURE_DONGLE_IN_IRAM VOID MCE_BtClkPhaseSwitch(BTCLK *pCLK, BTPHASE *pPhase)
{
    if (*pPhase >= 2500) {
        *pPhase -= 2500;
        *pCLK += 4;
    }

    if (*pPhase < 1250) {
        if (*pPhase >= 625) {
            *pCLK += 0x01;
            *pPhase -= 625;
        }
    } else if (*pPhase < 1875) {
        *pCLK += 0x02;
        *pPhase -= 1250;
    } else {
        *pCLK += 0x03;
        *pPhase -= 1875;
    }
}


VOID MCE_Update_BtClkOffsetAddr(void *addr, BT_CLOCK_OFFSET_SCENARIO type)
{
    if (type == ULL_CLK_Offset) {
        rBb_ull = (ULL_CLK_INFO_PTR)addr;
    } else {
        DSP_MW_LOG_E("[MCE]Update wrong BT offset address", 0);
    }
}

FEATURE_DONGLE_IN_IRAM VOID MCE_Get_BtClkOffset(BTCLK *pClkOffset, BTPHASE *pNClkOffse, BT_CLOCK_OFFSET_SCENARIO type)
{
    BTPHASE    PhaseOffset = 0;
    BTCLK      ClockOffset = 0;
    if (type == BT_CLK_Offset) {
        PhaseOffset = rBb->rAudioCtl.rRxPhsOffset;
        ClockOffset = rBb->rAudioCtl.rRxClkOffset;
    } else if (type == ULL_CLK_Offset) {
        if (rBb_ull->rValid) {
            PhaseOffset = rBb_ull->rRxPhsOffset;
            ClockOffset = rBb_ull->rRxClkOffset;
        } else {
            DSP_MW_LOG_W("[MCE] rValid = 0,maybe get the worng clock offset", 0);
        }
    }
    #ifdef AIR_DCHS_MODE_ENABLE
    else if (type == DCHS_CLK_Offset) {
        PhaseOffset = *((volatile uint32_t *)(DCHS_PHASE_OFFSET_REG));
        ClockOffset = *((volatile uint32_t *)(DCHS_CLK_OFFSET_REG));
    }
    #endif
    else {
        DSP_MW_LOG_E("[MCE]Get wrong BT offset type", 0);
    }

    *pClkOffset = ClockOffset;
    *pNClkOffse = PhaseOffset;
}

FEATURE_DONGLE_IN_IRAM VOID MCE_GetBtClk(BTCLK *pCurrCLK, BTPHASE *pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type)
{
    BTCLK CurrCLK, NativeCLK;
    BTPHASE CurrPhase;

    BTPHASE    PhaseOffset;
    BTCLK      ClockOffset;

    MCE_Get_BtClkOffset(&ClockOffset, &PhaseOffset, type);

#ifdef AIR_DCHS_MODE_ENABLE
    do {
        NativeCLK = rBb->rClkCtl.rNativeClock & 0x0FFFFFFC; /*Native Clk*/
        CurrCLK = (NativeCLK + ClockOffset) & 0x0FFFFFFC; /*Bt Clk*/
        CurrPhase = rBb->rClkCtl.rNativePhase + PhaseOffset; /*Bt Intra Clk*/
        CurrCLK -= 4;
        CurrPhase += 2500;
        if(CurrPhase>=2500)
        {
           CurrCLK += 4*(CurrPhase / 2500);
           CurrPhase %= 2500;
        }
    } while (NativeCLK != (rBb->rClkCtl.rNativeClock & 0x0FFFFFFC));
#else
    do {
        NativeCLK = rBb->rClkCtl.rNativeClock & 0x0FFFFFFC; /*Native Clk*/
        CurrCLK = (NativeCLK + ClockOffset) & 0x0FFFFFFC; /*Bt Clk*/
        CurrPhase = rBb->rClkCtl.rNativePhase + PhaseOffset; /*Bt Intra Clk*/
    } while (NativeCLK != (rBb->rClkCtl.rNativeClock & 0x0FFFFFFC));

    MCE_BtClkPhaseSwitch(&CurrCLK, &CurrPhase);
#endif

    *pCurrCLK = CurrCLK;    /*Bt Clk*/
    *pCurrPhase = CurrPhase;/*Bt Intra Clk*/
}

VOID MCE_Get_NativeClk_from_Controller(U32 *pNClk, U32 *pAClk)
{
    U32    NativeClk = rBb->_reserved_dword_904h[2];
    U32    AudioClk = rBb->_reserved_dword_904h[3];

    *pNClk = NativeClk;
    *pAClk = AudioClk;
    //printf("NativeClk:0x%x, AudioClk:0x%x",NativeClk, AudioClk);
}

VOID MCE_Initial_Aud_Cnt_from_Controller(void)
{
    rBb->_reserved_dword_904h[3] = 0;
}

VOID MCE_TransBT2NativeClk(BTCLK CurrCLK, BTPHASE CurrPhase, BTCLK *pNativeBTCLK, BTPHASE *pNativePhase, BT_CLOCK_OFFSET_SCENARIO type)
{
    BTPHASE    PhaseOffset;
    BTCLK      ClockOffset;

    MCE_Get_BtClkOffset(&ClockOffset, &PhaseOffset, type);
#ifdef AIR_DCHS_MODE_ENABLE
    *pNativeBTCLK = (CurrCLK - ClockOffset);
    *pNativePhase = (CurrPhase - PhaseOffset);

    *pNativeBTCLK -= 4;
    *pNativePhase += 2500;
    if(*pNativePhase >= 2500)
    {
        *pNativeBTCLK += 4 * (*pNativePhase / 2500);
        *pNativePhase %= 2500;
    }
    *pNativeBTCLK &= BT_NCLK_MASK;
#else
    if (CurrPhase > BT_FRAME_UNIT) {
        CurrPhase -= (BT_FRAME_UNIT);
        CurrCLK += 4;
    }
    *pNativeBTCLK = (CurrCLK - ClockOffset);
    *pNativePhase = (CurrPhase - PhaseOffset);
    if (*pNativePhase > BT_FRAME_UNIT) {
        *pNativePhase += (BT_FRAME_UNIT);
        *pNativeBTCLK -= 4;
    }
    uint32_t remain_n = *pNativeBTCLK & 0x03;
    if (remain_n != 0) {
        uint32_t intra_2 = remain_n * BT_SLOT_UNIT + *pNativePhase;
        if (intra_2 >= BT_FRAME_UNIT) {
            intra_2 -= BT_FRAME_UNIT;
            *pNativeBTCLK += 4;
        }
        *pNativePhase = intra_2;
    }
    *pNativeBTCLK &= BT_NCLK_MASK;
#endif
}


VOID MCE_LatchSrcTiming(VOID)
{
    BTCLK BtClk;
    BTPHASE BtPhase;

    MCE_GetBtClk(&BtClk, &BtPhase, BT_CLK_Offset);
    DSP_MW_LOG_I("BtClk:%d, BtPhase:%d \r\n", 2, BtClk, BtPhase);
}

/**
 * LC_Get_Offset_FromAB
 *
 * get offset us between a and b
 *
 * @Author : Wen <wen.huang@airoha.com.tw>
 *
 * @pa : In Point a
 * @pb : In Point b
 * @Return : b-a = ? us , maximum value 134217726us
 */
FEATURE_DONGLE_IN_IRAM U32 MCE_Get_Offset_FromAB(BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb)
{
    BTCLK a_t0 = pa->period & 0xFFFFFFC;
    BTCLK b_t0 = pb->period & 0xFFFFFFC;
    BTCLK CLKOffset;
    U32 Phase;
    if (pa->period <= pb->period) {
        CLKOffset = (b_t0 - a_t0);
    } else {
        CLKOffset = (0xFFFFFFF - a_t0 + b_t0 + 1);
    }
    Phase = (CLKOffset * 625) - pa->phase + pb->phase;
    return (Phase >> 1);
}

/**
 * MCE_Compare_Val_FromAB
 *
 * Compare val between a and b
 *
 * @Author : Wen <wen.huang@airoha.com.tw>
 *
 * @pa : In Point a
 * @pb : In Point b
 * @Return : compare the pb and pa
 */
FEATURE_DONGLE_IN_IRAM S32 MCE_Compare_Val_FromAB(BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb)
{
    if (pb->period > pa->period) {
        return 1;
    } else if (pb->period == pa->period) {
        if (pb->phase > pa->phase) {
            return 1;
        } else if (pb->phase == pa->phase) {
            return 0;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

/**
 * LC_Add_us_FromA
 *
 * Add us from a => b = a + n
 *
 * @Author : Wen <wen.huang@airoha.com.tw>
 *
 * @n : In offset unit us
 * @pa : In
 * @pb : out
 * @Return : b = a + n
 */
FEATURE_DONGLE_IN_IRAM VOID MCE_Add_us_FromA(U32 n, BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb)
{
    //get cur clk and phase
    BTCLK a_t0 = pa->period & 0xFFFFFFC;
    U32 m;
    m = pa->phase + (n << 1);
    pb->period = (a_t0 + m / 625) &BT_CLOCK_TICK_MASK;
    pb->phase = m % 2500;
}

/**
 * LC_Subtract_us_Fromb
 *
 * Subtract us from b => a = b - n
 *
 * @Author : Wen <wen.huang@airoha.com.tw>
 *
 * @n : In offset unit us
 * @pa : out
 * @pb : in
 * @Return : b - n = a
 */
FEATURE_DONGLE_IN_IRAM VOID MCE_Subtract_us_Fromb(U32 n, BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb)
{
    BTCLK b_t0 = pb->period & 0xFFFFFFC;
    BTCLK a_t0;
    U32 x;

    n = n << 1; //change to unit 0.5us
    if (pb->phase >= n) {
        pa->phase = pb->phase - n;
        pa->period = (b_t0 + pa->phase / 625) &BT_CLOCK_TICK_MASK;
    } else {
        x = (n - pb->phase) % 2500;
        pa->phase = (2500 - x) % 2500;
        a_t0 = (BT_CLOCK_MAX_WRAP + b_t0 - ((((n - pb->phase) / 2500) + (pa->phase > 0)) * 4)) & BT_CLOCK_TICK_MASK;
        pa->period = a_t0 + pa->phase / 625;
    }
}

VOID AT_MCE_LatchSrcTiming(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    BTCLK BtClk;
    BTPHASE BtPhase;

    MCE_GetBtClk(&BtClk, &BtPhase, BT_CLK_Offset);
    DSP_MW_LOG_I("BtClk:%d, BtPhase:%d\r\n", 2, BtClk, BtPhase);
}

uint16_t Forwarder_Get_RX_FWD_Pattern_Size(SCO_CODEC codec_type)
{
    uint16_t pattern_framesize; /*mSBC:decode in size; CVSD:decode out size*/

    if ((codec_type == VOICE_WB) && (rBb->rAudioCtl.rRxAirMode == 0x3)) {
        if ((rBb->rAudioCtl.rRxDataLen != 60) && (rBb->rAudioCtl.rRxDataLen != 30)) {
            DSP_MW_LOG_I("[RX FWD] mSBC strange rRxDataLen:%d", 1, rBb->rAudioCtl.rRxDataLen);
            return 0;
        } else {
            pattern_framesize = rBb->rAudioCtl.rRxDataLen;
        }
    } else if ((codec_type == VOICE_NB) && (rBb->rAudioCtl.rRxAirMode == 0x2)) {
        if ((rBb->rAudioCtl.rRxDataLen != 60) && (rBb->rAudioCtl.rRxDataLen != 30) && (rBb->rAudioCtl.rRxDataLen != 20) && (rBb->rAudioCtl.rRxDataLen != 10)) {
            DSP_MW_LOG_I("[RX FWD] CVSD strange rRxDataLen:%d", 1, rBb->rAudioCtl.rRxDataLen);
            return 0;
        } else {
            pattern_framesize = rBb->rAudioCtl.rRxDataLen * 2;
        }
    } else {
        DSP_MW_LOG_I("[RX FWD] Codec type error, host codec:%d, controller codec: %d", 2, codec_type, rBb->rAudioCtl.rRxAirMode);
        return 0;
    }

    return pattern_framesize;
}

uint16_t Forwarder_Get_TX_FWD_Pattern_Size(SCO_CODEC codec_type)
{
    uint16_t pattern_framesize; /*mSBC:decode in size; CVSD:decode out size*/

    if ((codec_type == VOICE_WB) && (rBb->rAudioCtl.rTxAirMode == 0x3)) {
        if ((rBb->rAudioCtl.rTxDataLen != 60) && (rBb->rAudioCtl.rRxDataLen != 30)) {
            DSP_MW_LOG_I("[TX FWD] mSBC strange rRxDataLen:%d", 1, rBb->rAudioCtl.rTxDataLen);
            return 0;
        } else {
            pattern_framesize = rBb->rAudioCtl.rRxDataLen;
        }
    } else if ((codec_type == VOICE_NB) && (rBb->rAudioCtl.rTxAirMode == 0x2)) {
        if ((rBb->rAudioCtl.rTxDataLen != 120) && (rBb->rAudioCtl.rTxDataLen != 60) && (rBb->rAudioCtl.rTxDataLen != 30) && (rBb->rAudioCtl.rTxDataLen != 20)) {
            DSP_MW_LOG_I("[TX FWD] CVSD strange rRxDataLen:%d", 1, rBb->rAudioCtl.rTxDataLen);
            return 0;
        } else {
            pattern_framesize = rBb->rAudioCtl.rTxDataLen * 2;
        }
    } else {
        DSP_MW_LOG_I("[TX FWD] Codec type error, host codec:%d, controller codec: %d", 2, codec_type, rBb->rAudioCtl.rTxAirMode);
        return 0;
    }

    return pattern_framesize;
}

void Forwarder_Rx_Intr_HW_Handler(void)
{
    rBb->rAuRxIntFlag = 1;
    rBb->rAuRxIntFlag = 1;
    rBb->rAuRxIntFlag = 1;
}

void Forwarder_Tx_Intr_HW_Handler(void)
{
    uint32_t irq_status, loop_cnt;

    rBb->rAuTxIntFlag = 1;

    loop_cnt = 0;
    do {
        if (loop_cnt++ > 10000) {
            DSP_MW_LOG_W("[TX FWD] clear IRQ flag timeout", 0);
            break;
        }
        irq_status = hal_nvic_get_pending_irq(BT_AUTX_IRQn);
    } while (irq_status == 1);
}

uint32_t Forwarder_Rx_Status(void)
{
    return rBb->rAudioCtl.rRxDstAddrSelCurrSw;
}

uint32_t Forwarder_Tx_Status(void)
{
    return rBb->rAudioCtl.rTxSrcAddrSelNxtSw;
}

bool Forwarder_Rx_Check_Disconnect_Status(void)
{
    if ((rBb->_reserved_dword_904h[1] & (1 << 0)) != 0) {
        return true;
    } else {
        return false;
    }
}

void Forwarder_Rx_Reset_Disconnect_Status(void)
{
    rBb->_reserved_dword_904h[1] &= ~(1 << 0);
}

#ifdef AIR_HFP_SYNC_START_ENABLE
bool Forwarder_Check_Sync_Start_Status(void)
{
    if ((rBb->_reserved_dword_904h[1] & (1 << 2)) != 0) {
        return true;
    } else {
        return false;
    }
}

void Forwarder_Set_DSP_Ready_Status(void)
{
    rBb->_reserved_dword_904h[1] |= (1 << 1);
}
#endif

#ifdef AIR_HFP_SYNC_STOP_ENABLE
bool Forwarder_Check_Sync_Stop_Status(void)
{
    if ((rBb->_reserved_dword_904h[1] & (1 << 3)) != 0) {
        return true;
    } else {
        return false;
    }
}

void Forwarder_Reset_Sync_Stop_Status(void)
{
    rBb->_reserved_dword_904h[1] &= ~(1 << 3);
}
#endif

uint32_t Forwarder_Rx_AncClk(void)
{
    return rBb->rAudioCtl.rRxAncClk; //unit:312.5us
}

void Forwarder_Rx_Intr_Ctrl(bool ctrl)
{
    rBb->rAuRxIntMask = ctrl;
}

void Forwarder_Rx_Buf_Ctrl(bool ctrl)
{
    rBb->rAudioCtl.rRxBufRdy = ctrl;
}

void Forwarder_Tx_Intr_Ctrl(bool ctrl)
{
    rBb->rAuTxIntMask = ctrl;
}

void Forwarder_Tx_Buf_Ctrl(bool ctrl)
{
    rBb->rAudioCtl.rTxBufRdy = ctrl;
}


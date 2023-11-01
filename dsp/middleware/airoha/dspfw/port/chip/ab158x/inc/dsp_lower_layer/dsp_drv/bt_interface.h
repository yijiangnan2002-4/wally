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

#include "sfr_bt.h"
#include "hal_ccni.h"
#include "audio_config.h"

extern VOID MCE_BtClkPhaseSwitch(BTCLK *pCLK, BTPHASE *pPhase);
extern VOID MCE_Update_BtClkOffsetAddr(void *addr, BT_CLOCK_OFFSET_SCENARIO type);
extern VOID MCE_GetBtClk(BTCLK *pCurrCLK, BTPHASE *pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);
extern VOID MCE_Get_BtClkOffset(BTCLK *pClkOffset, BTPHASE *pNClkOffse, BT_CLOCK_OFFSET_SCENARIO type);
extern VOID MCE_Get_NativeClk_from_Controller(U32 *pNClk, U32 *pAClk);
extern VOID MCE_Initial_Aud_Cnt_from_Controller(void);
extern VOID MCE_LatchSrcTiming(VOID);
extern U32 MCE_Get_Offset_FromAB(BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb);
extern S32 MCE_Compare_Val_FromAB(BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb);
extern VOID MCE_Add_us_FromA(U32 n, BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb);
extern VOID MCE_Subtract_us_Fromb(U32 n, BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb);
extern VOID AT_MCE_LatchSrcTiming(hal_ccni_message_t msg, hal_ccni_message_t *ack);
extern VOID MCE_TransBT2NativeClk(BTCLK CurrCLK, BTPHASE CurrPhase, BTCLK *pNativeBTCLK, BTPHASE *pNativePhase, BT_CLOCK_OFFSET_SCENARIO type);

extern uint16_t Forwarder_Get_RX_FWD_Pattern_Size(SCO_CODEC codec_type);
extern uint16_t Forwarder_Get_TX_FWD_Pattern_Size(SCO_CODEC codec_type);
extern void Forwarder_Rx_Intr_HW_Handler(void);
extern void Forwarder_Tx_Intr_HW_Handler(void);
extern uint32_t Forwarder_Rx_Status(void);
extern uint32_t Forwarder_Tx_Status(void);
extern bool Forwarder_Rx_Check_Disconnect_Status(void);
extern void Forwarder_Rx_Reset_Disconnect_Status(void);
#ifdef AIR_HFP_SYNC_START_ENABLE
extern bool Forwarder_Check_Sync_Start_Status(void);
extern void Forwarder_Set_DSP_Ready_Status(void);
#endif
#ifdef AIR_HFP_SYNC_STOP_ENABLE
extern bool Forwarder_Check_Sync_Stop_Status(void);
extern void Forwarder_Reset_Sync_Stop_Status(void);
#endif
extern uint32_t Forwarder_Rx_AncClk(void);
extern void Forwarder_Rx_Intr_Ctrl(bool ctrl);
extern void Forwarder_Rx_Buf_Ctrl(bool ctrl);
extern void Forwarder_Tx_Intr_Ctrl(bool ctrl);
extern void Forwarder_Tx_Buf_Ctrl(bool ctrl);

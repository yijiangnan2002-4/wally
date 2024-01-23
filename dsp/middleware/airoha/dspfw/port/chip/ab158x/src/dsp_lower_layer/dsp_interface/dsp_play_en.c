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

#include "dsp_sdk.h"
#include "dsp_callback.h"
#include "transform_.h"
#include "transform_inter.h"
#include "dsp_play_en.h"
#include "hal_audio_driver.h"



////////////////////////////////////////////////////////////////////////////////
// Constant Definitions ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function Prototypes /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BOOL DSP_setup_play_en_internal(play_en_setup_info_t* setup_info, play_en_port_t port);

////////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static play_en_list_info_t play_en_port[PLAY_EN_PORT_MAX];
static U16 cur_port;
#define PLAY_EN_DISABLE (0xFF)
static uint32_t g_play_en_timer_handle;




////////////////////////////////////////////////////////////////////////////////
// DSPMEM FUNCTION DECLARATIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

play_en_port_t DSP_get_avaliable_play_en_port(VOID)
{
    for (int i = 0; i < PLAY_EN_PORT_MAX; i++)
    {
        if (play_en_port[i].is_using == FALSE)
        {
            return i;
        }
    }
    DSP_MW_LOG_I("[PLAY EN] get play en port failed", 0);
    return PLAY_EN_DISABLE;
}
VOID DSP_setup_play_en_port(play_en_port_t port, play_en_setup_info_t* play_info, BOOL enable)
{
    if (play_info != NULL){
        play_en_port[port].play_en_info = *play_info;
    }
    play_en_port[port].is_using = enable;
}

BOOL DSP_play_en_init(VOID)
{
    for (int i = 0; i < PLAY_EN_PORT_MAX; i++)
    {
        memset(&play_en_port[i],0,sizeof(play_en_setup_info_t));
        cur_port = PLAY_EN_DISABLE;
    }
    DSP_MW_LOG_I("[PLAY EN] init dsp play en", 0);
    return TRUE;
}
extern bool hal_memory_set_palyen_counter(hal_audio_control_status_t control);
VOID DSP_play_en_set_afe_rg(VOID)
{
    if (cur_port >= PLAY_EN_PORT_MAX){
        DSP_MW_LOG_I("[PLAY EN] play en set invalid port", 0);
        AUDIO_ASSERT(0);    
    }
    hal_audio_afe_set_play_en(play_en_port[cur_port].play_en_info.bt_time.period,play_en_port[cur_port].play_en_info.bt_time.phase);
    //afe_audio_bt_sync_con0_t dl_agent_bit = afe_get_bt_sync_enable_bit(play_en_port[cur_port].play_en_info.play_memory);
    //afe_audio_bt_sync_con0_t dl_agent_bit_org = AFE_READ(AFE_AUDIO_BT_SYNC_CON0) & AFE_AUDIO_BT_SYNC_ENABLE;

    AFE_SET_REG(AFE_AUDIO_BT_SYNC_CON0, 0x0000, 0xFFFFF);//clear AFE_AUDIO_BT_SYNC_CON0

    //AFE_SET_REG(AFE_AUDIO_BT_SYNC_CON0,  0x0600 | dl_agent_bit, 0x0600 | AFE_AUDIO_BT_SYNC_ENABLE);//Only enable certain dl agent
    AFE_SET_REG(AFE_AUDIO_BT_SYNC_CON0,  0x0600 | (play_en_port[cur_port].play_en_info.play_memory<<16), 0x0600 | AFE_AUDIO_BT_SYNC_ENABLE);//Only enable certain dl agent
    hal_memory_set_palyen_counter(HAL_AUDIO_CONTROL_ON);
}


void DSP_play_en_timer_callback(void *user_data)
{
    UNUSED(user_data);
    play_en_port[cur_port].is_using = FALSE;
    U32 NativeCLK;
    U32 NativePhase;
    U8  play_en;
    hal_audio_afe_get_play_en(&NativeCLK, &NativePhase, &play_en);

    if (play_en != 0){
        DSP_MW_LOG_E("[PLAY EN] play en timer detect playen expired current %d set %d user 0x%x", 3,rBb->rAudioCtl.rRxClk,NativeCLK,play_en_port[cur_port].play_en_info.usingPtr);
        if (play_en_port[cur_port].play_en_info.postpone_handler != NULL){
            play_en_port[cur_port].play_en_info.postpone_handler(play_en_port[cur_port].play_en_info.usingPtr);

        }
    } else {
        // hal_memory_set_enable(play_en_port[cur_port].play_en_info.play_memory, HAL_AUDIO_CONTROL_ON); // keep dl memory agent running
        // hal_memory_set_palyen(play_en_port[cur_port].play_en_info.play_memory, HAL_AUDIO_CONTROL_OFF);
        // AFE_SET_REG(AFE_AUDIO_BT_SYNC_CON0, 0x0000, 0x0003);
        for (int i = 0; i < 4; i++)
        {
            if ((play_en_port[cur_port].play_en_info.play_memory & (0x1 << i)) != 0)
            {
                hal_memory_set_enable(i, HAL_AUDIO_CONTROL_ON);
                hal_memory_set_palyen(i, HAL_AUDIO_CONTROL_OFF);
            }
        }
        AFE_SET_REG(AFE_AUDIO_BT_SYNC_CON0, 0x0000, 0x0003);
    }
    
    do{
        cur_port = PLAY_EN_DISABLE;
        for (int i = 1; i < PLAY_EN_PORT_MAX; i++)
        {
            if (play_en_port[i].is_using == TRUE)
            {
                if ((cur_port == PLAY_EN_DISABLE)||(MCE_Compare_Val_FromAB(&play_en_port[cur_port].play_en_info.bt_time,&play_en_port[i].play_en_info.bt_time) == -1)){
                    cur_port = i;
                }
            }
        }
    }while ((cur_port != PLAY_EN_DISABLE)&&(DSP_setup_play_en_internal(&play_en_port[cur_port].play_en_info,cur_port)));
    if ((cur_port == PLAY_EN_DISABLE)&&(g_play_en_timer_handle != 0))
    {
        hal_gpt_sw_free_timer(g_play_en_timer_handle);
        g_play_en_timer_handle = 0;
    }
}
BOOL DSP_setup_play_en_internal(play_en_setup_info_t* setup_info, play_en_port_t port)
{
    BTTIME_STRU current_time;
    current_time.period= rBb->rClkCtl.rNativeClock;
    current_time.phase= rBb->rClkCtl.rNativePhase;
    U32 timer_offset;
    if (port == PLAY_EN_DISABLE){
        DSP_MW_LOG_I("[PLAY EN] play en set port DISABLE", 0);
        return FALSE;
    }
    MCE_Add_us_FromA(1000, &current_time, &current_time);// reserve process time & timer wake up time
    if (MCE_Compare_Val_FromAB(&current_time,&setup_info->bt_time) == -1){
        DSP_MW_LOG_I("[PLAY EN] play en set past time fail current %d set %d ", 2,current_time.period,setup_info->bt_time.period);
        play_en_port[port].is_using = FALSE;
        return FALSE;
    }
    DSP_setup_play_en_port(port,setup_info,TRUE);
    if (cur_port == PLAY_EN_DISABLE){
        cur_port = port;
        DSP_play_en_set_afe_rg();
    }
    else
    {
        if (MCE_Compare_Val_FromAB(&play_en_port[cur_port].play_en_info.bt_time,&setup_info->bt_time) == -1){
            cur_port = port;
            DSP_play_en_set_afe_rg();
        }
    }
    DSP_MW_LOG_I("[PLAY EN] play en set port success user 0x%x", 1,play_en_port[port].play_en_info.usingPtr);
    hal_gpt_sw_get_timer(&g_play_en_timer_handle);
    current_time.period= rBb->rClkCtl.rNativeClock;
    current_time.phase= rBb->rClkCtl.rNativePhase;
    timer_offset = MCE_Get_Offset_FromAB(&current_time,&play_en_port[cur_port].play_en_info.bt_time) + 10; // 10us additional offset
    hal_gpt_sw_start_timer_us(g_play_en_timer_handle, timer_offset, DSP_play_en_timer_callback, NULL);
    return TRUE;
    
}

BOOL DSP_setup_play_en(play_en_setup_info_t* setup_info)
{
    U16 port;
    port = DSP_get_avaliable_play_en_port();
    if (port != PLAY_EN_DISABLE){
        play_en_port[port].is_using = TRUE;
    }
    else {
        DSP_MW_LOG_I("[PLAY EN] play en set port failed", 0);
        return FALSE;
    }
    DSP_setup_play_en_internal(setup_info,port);
    return TRUE;
}



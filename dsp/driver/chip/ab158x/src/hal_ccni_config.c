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

#include "hal_ccni.h"

#ifdef HAL_CCNI_MODULE_ENABLED
#include "hal_ccni_config.h"
#include "hal_log.h"
#include "exception_handler.h"
#include "memory_attribute.h"

#ifdef FREERTOS_ENABLE
#include "FreeRTOS.h"
#include "port_tick.h"
#include "timers.h"
#endif
//#ifdef AIR_ICE_DEBUG_ENABLE
#include "hal_resource_assignment.h"
//#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef FREERTOS_ENABLE
#if configGENERATE_RUN_TIME_STATS == 1
dsp_task_info_t * mcu_to_dsp_info = NULL;
#endif
#endif

void default_isr(hal_ccni_event_t event, void * msg)
{
    (void) msg;
    hal_ccni_mask_event(event);
    log_hal_msgid_error("[DSP0] default_isr event = 0x%x \r\n", 1, event);
    hal_ccni_clear_event(event);
    hal_ccni_unmask_event(event);
}

#ifdef CCCI_ENABLE
extern void ccci_dsp0_receive_msg_from_others_core(hal_ccni_event_t event, void *msg);
extern void ccci_dsp0_receive_msg_from_others_core_for_ccci_ack(hal_ccni_event_t event, void *msg);
#else
#define ccci_dsp0_receive_msg_from_others_core  default_isr
#define ccci_dsp0_receive_msg_from_others_core_for_ccci_ack  default_isr
#endif /* CCCI_ENABLE */

#ifdef MTK_DSP_AUDIO_MESSAGE_ENABLE
extern void aud_msg_rx_handler(hal_ccni_event_t event, void *msg);
#else
#define aud_msg_rx_handler  default_isr
#endif /* MTK_DSP_AUDIO_MESSAGE_ENABLE */

// It is ISR for triggered exception event.
void trigger_exception(hal_ccni_event_t event, void *msg)
{
    ATTR_LOG_STRING exp[] = "Triggered";
    ATTR_LOG_STRING file[] = __FILE__;
    light_assert(exp, file, __LINE__);
    (void) event;
    (void) msg;
}

// It is ISR for at_cmd triggered exception event.
void at_cmd_trigger_exception(hal_ccni_event_t event, void *msg)
{
    ATTR_LOG_STRING exp[] = "AT+SYSTEM=dsp,crash";
    ATTR_LOG_STRING file[] = __FILE__;
    light_assert(exp, file, __LINE__);
    (void) event;
    (void) msg;
}
#ifdef AIR_CPU_MCPS_PRIORING_ENABLE
void dsp_profiling_handler(hal_ccni_event_t event, void *msg)
{
#ifdef FREERTOS_ENABLE
    extern TimerHandle_t xTimerofTest;
    extern uint32_t g_tmp_enable_heak_leak;
    hal_ccni_status_t status;
    if (NULL != msg) {
        mcu_to_dsp_info = (dsp_task_info_t *)hal_memview_infrasys_to_dsp0(((hal_ccni_message_t *)msg)->ccni_message[0]);

        if (NULL != mcu_to_dsp_info) {
            if(mcu_to_dsp_info->task_information_period <= 1){
                g_tmp_enable_heak_leak = mcu_to_dsp_info->task_information_period;
            }
            if (mcu_to_dsp_info->isEnableTaskInforPeriod == TRUE) {
                xTimerStartFromISR(xTimerofTest, 0);
            } else {
                xTimerStopFromISR(xTimerofTest, 0);
            }

            /* set task information period */
            if ( mcu_to_dsp_info->option_selector == TASK_PERIOD_CONFIG) {
                xTimerStopFromISR(xTimerofTest, 0);
                xTimerChangePeriodFromISR(xTimerofTest, mcu_to_dsp_info->task_information_period, 0);
            }
        }
    }
#endif
    status = hal_ccni_clear_event(event);  // clear the event.
    status = hal_ccni_unmask_event(event); // unmask the event.
}
#endif

void ice_debug_dsp_test(hal_ccni_event_t event, void *msg)
{
    uint32_t *pMsg = (uint32_t *)msg;
    hal_ccni_status_t status;
    uint32_t irq_status;

    status = hal_ccni_mask_event(event);
    if (status != HAL_CCNI_STATUS_OK) {
        log_hal_msgid_info("CM4 CCNI mask event: 0x%x something wrong, return is %d\r\n", 2, event, status);
    }
    //hal_memview_infrasys_to_cm4(pMsg);

    if (pMsg[1] == 1) {
        hal_nvic_save_and_set_interrupt_mask(&irq_status);
        if (pMsg[0] > 100000) {
            assert(0);
        }
        hal_gpt_delay_ms(pMsg[0]);

        hal_nvic_restore_interrupt_mask(irq_status);
        log_hal_msgid_info("stop_sync_test_mcu, dsp stop %d ms test ok\r\n", 1, pMsg[0]);
    } else if (pMsg[1] == 2) {
        hal_nvic_save_and_set_interrupt_mask(&irq_status);
        if (pMsg[0] > 100000) {
            assert(0);
        }
        /*trigger MCU bus hang*/

        hal_gpt_delay_ms(pMsg[0]);
        hal_nvic_restore_interrupt_mask(irq_status);
        log_hal_msgid_info("stop_sync_test_dsp, dsp stop %d ms test ok\r\n", 1, pMsg[0]);
    } else if (pMsg[1] == 3) {
        log_hal_msgid_info("wake up dsp ok\r\n", 0);
    }

    status = hal_ccni_clear_event(event);  // clear the event.
    status = hal_ccni_unmask_event(event); // unmask the event.
}

#if AIR_AUDIO_TRANSMITTER_ENABLE
extern void audio_transmitter_source_ccni_handler(hal_ccni_event_t event, void *msg);
extern void audio_transmitter_source_ccni_handler1(hal_ccni_event_t event, void *msg);
extern void audio_transmitter_source_ccni_handler2(hal_ccni_event_t event, void *msg);
#else
#define audio_transmitter_source_ccni_handler default_isr
#define audio_transmitter_source_ccni_handler1 default_isr
#define audio_transmitter_source_ccni_handler2 default_isr
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

#if AIR_AUDIO_BT_COMMON_ENABLE
extern void bt_common_source_ccni_handler(hal_ccni_event_t event, void *msg);
#else
#define bt_common_source_ccni_handler default_isr
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */


#if defined(AIR_LOW_LATENCY_MUX_ENABLE)
extern void mux_ll_uart_rx_event_from_mcu_handler(hal_ccni_event_t event, void * msg);
#else
#define mux_ll_uart_rx_event_from_mcu_handler default_isr
#endif /* AIR_LOW_LATENCY_MUX_ENABLE */
//please should replace dafult_callback with user callback if the event is used
const hal_ccni_function_t ccni_cm4_to_dsp0_function_table[CCNI_CM4_TO_DSP0_EVENT_MAX] = {
    {aud_msg_rx_handler}, /*CCNI_CM4_TO_DSP0_RX_EVENT*/
    {audio_transmitter_source_ccni_handler},        /*CM4_TO_DSP0_WITH_MSG_EVENT1*/
    {bt_common_source_ccni_handler},        /*CM4_TO_DSP0_WITH_MSG_EVENT2*/
    {ice_debug_dsp_test},        /*CM4_TO_DSP0_WITH_MSG_EVENT3*/
    {mux_ll_uart_rx_event_from_mcu_handler},        /*CM4_TO_DSP0_WITH_MSG_EVENT4*/
#if configGENERATE_RUN_TIME_STATS == 1
    {dsp_profiling_handler},        /*CM4_TO_DSP0_WITH_MSG_CONFIG_PROI*/
#else
    {default_isr},
#endif
    {ccci_dsp0_receive_msg_from_others_core_for_ccci_ack},    /*CM4_TO_DSP0_WITH_MSG_EVENT6*/
    {ccci_dsp0_receive_msg_from_others_core},                 /*CM4_TO_DSP0_WITH_MSG_EVENT7*/
    {trigger_exception},  /*CM4_TO_DSP0_WITHOUT_MSG_EVENT8*/
    {at_cmd_trigger_exception},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT9*/
    {audio_transmitter_source_ccni_handler1},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT10*/
    {audio_transmitter_source_ccni_handler2},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT11*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT12*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT13*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT14*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT15*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT16*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT17*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT18*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT19*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT20*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT21*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT22*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT23*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT24*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT25*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT26*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT27*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT28*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT29*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT30*/
    {default_isr},        /*CM4_TO_DSP0_WITHOUT_MSG_EVENT31*/
};


#ifdef __cplusplus
}
#endif

#endif /* HAL_CCNI_MODULE_ENABLED */


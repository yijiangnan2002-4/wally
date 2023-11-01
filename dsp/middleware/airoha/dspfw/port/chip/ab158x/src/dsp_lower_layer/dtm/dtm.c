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

#include "config.h"
#include "dsp_task.h"
#include "dtm.h"
#include "dsp_interface.h"
#include "dtm_interface.h"
#include "audio_config.h"
#include "dsp_drv_afe.h"
#include "dsp_temp.h"
#include "dsp_memory.h"
#include "dsp_scenario.h"
#include "transform.h"
#include "audio_config.h"
#include "stream_audio.h"
#include "stream.h"
#include "dsp_feature_interface.h"
#include "queue.h"
#include "dsp_vow.h"
#if defined (AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE) && defined (AIR_GAMING_MODE_DONGLE_ENABLE)
#include "scenario_ull_audio.h"
#endif
#if 0
#if defined (AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE) && defined (AIR_WIRED_AUDIO_ENABLE)
#include "scenario_wired_audio.h"
#endif
#endif
#ifdef AIR_SILENCE_DETECTION_ENABLE
#include "silence_detection_interface.h"
#endif /* AIR_SILENCE_DETECTION_ENABLE */
#if defined (AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE) && defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
#include "scenario_ull_audio_v2.h"
#endif
#define DTM_QUEUE_MAX_NUMBER       (10)
/******************************************************************************
 * Function Definitions
 ******************************************************************************/


/******************************************************************************
 * Function Declaration
 ******************************************************************************/
VOID                    DTM_Init(VOID);
VOID                    DTM(VOID);
VOID                    DTM_ResumeDPRT(VOID);
VOID                    DTM_ResumeDAVT(VOID);
BOOL                    DTM_CheckTaskAvailable(U8 TaskID);
#if  DspIpcEnable

STATIC DSP_CMD_ACT_t    DTM_JobStatusHandler(DSP_CMD_PTR_t DspCmdPtr);
STATIC VOID             DTM_CommandHandler(DSP_CMD_PTR_t DspCmdPtr, DSP_CMD_ACT_t Action);
STATIC DSP_CMD_ACT_t    DTM_DSP_JobHandler(DSP_CMD_PTR_t DspCmdPtr);
STATIC DSP_CMD_ACT_t    DTM_AV_JobSuspend(DSP_CMD_PTR_t DspCmdPtr);
STATIC DSP_CMD_ACT_t    DTM_AV_JobInit(DSP_CMD_PTR_t DspCmdPtr);
STATIC DSP_CMD_ACT_t    DTM_AV_JobActive(DSP_CMD_PTR_t DspCmdPtr);
STATIC DSP_CMD_ACT_t    DTM_AV_JobDeInit(DSP_CMD_PTR_t DspCmdPtr);
STATIC DSP_CMD_ACT_t    DTM_PR_JobSuspend(DSP_CMD_PTR_t DspCmdPtr);
STATIC DSP_CMD_ACT_t    DTM_PR_JobInit(DSP_CMD_PTR_t DspCmdPtr);
STATIC DSP_CMD_ACT_t    DTM_PR_JobActive(DSP_CMD_PTR_t DspCmdPtr);
STATIC DSP_CMD_ACT_t    DTM_PR_JobDeInit(DSP_CMD_PTR_t DspCmdPtr);
#endif
VOID                Audio_Setup(VOID);

/******************************************************************************
 * External Function Prototypes
 ******************************************************************************/
#if (FEA_SUPP_DSP_AUDIO_VERIFICATION)
EXTERN VOID DSP_FakeAudioProcess(VOID);
#endif
#ifdef AIR_SIDETONE_ENABLE
extern afe_sidetone_param_t dsp_afe_sidetone;
extern afe_sidetone_param_extension_t dsp_afe_sidetone_extension;
#endif
#ifdef MTK_WWE_ENABLE
// extern hal_audio_device_parameter_vow_t *dsp_vow;
extern hal_audio_device_parameter_vow_t hwvad_vow_control;

#endif
/******************************************************************************
 * Variables
 ******************************************************************************/
#if DspIpcEnable
STATIC DSP_TASK_LIST_t DspJobStatus;
#endif
AUDIO_STATUS_t  Audio_Status;
AUDIO_SINK_STATUS_t Audio_Sink_Status;
static QueueHandle_t dtm_queue = NULL;

/******************************************************************************
 * DSP State Handler Vector
 ******************************************************************************/
#if DspIpcEnable
typedef void (*DSP_CMD_HDLR)(void);
typedef DSP_CMD_ACT_t (*DSP_JOB_HDLR)(DSP_CMD_PTR_t DspCmdPtr);
#endif
/* Number of DSP States */
#define NO_OF_PR_STATE_HANDLER (sizeof(DspPrJobHdlr)/sizeof(DspPrJobHdlr[0]))
#define NO_OF_AV_STATE_HANDLER (sizeof(DspAvJobHdlr)/sizeof(DspAvJobHdlr[0]))
#if DspIpcEnable
/* DSP Job State Handler */
DSP_JOB_HDLR CODE DspTmHdler = DTM_DSP_JobHandler;

DSP_JOB_HDLR CODE DspAvJobHdlr[] = {
    DTM_AV_JobSuspend,
    DTM_AV_JobInit,
    DTM_AV_JobActive,
    DTM_AV_JobDeInit,
};

DSP_JOB_HDLR CODE DspPrJobHdlr[] = {
    DTM_PR_JobSuspend,
    DTM_PR_JobInit,
    DTM_PR_JobActive,
    DTM_PR_JobDeInit,
};

#endif

#ifdef MTK_SENSOR_SOURCE_ENABLE
#include "bsp_multi_axis_sensor.h"
#endif

uint32_t DTM_query_queue_avail_number(void)
{
    if (dtm_queue == NULL) {
        return 0;
    }

    if (hal_nvic_query_exception_number() > 0) {
        return DTM_QUEUE_MAX_NUMBER - (uint32_t)uxQueueMessagesWaitingFromISR(dtm_queue);
    } else {
        return (uint32_t)uxQueueSpacesAvailable(dtm_queue);
    }
}

void DTM_enqueue(DTM_EVENT_ID_t id, U32 arg, BOOL isFromISR)
{
    DTM_QUEUE_t element;
    element.event_id = id;
    element.arg = arg;

    if (isFromISR == true) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (xQueueSendFromISR(dtm_queue, &element, &xHigherPriorityTaskWoken) != pdPASS) {
            DSP_MW_LOG_E("[DTM_enqueue] error, fail to send queue id:%d\n", 1, id);
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        if (xQueueSend(dtm_queue, &element, 0) != pdPASS) {
            DSP_MW_LOG_E("[DTM_enqueue] error, fail to send queue id:%d\n", 1, id);
        }
    }
}

/**
 * DTM_Init
 *
 * Initialization for DTM Task
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
VOID DTM_Init(VOID)
{
#if DspIpcEnable
    DspJobStatus.AV = DSP_JOB_SUSPEND;
    DspJobStatus.PR = DSP_JOB_SUSPEND;
#endif
    Audio_Sink_Status.DSP_Audio_busy = FALSE;
    Audio_Sink_Status.DSP_vp_path_busy = FALSE;
    DSPMEM_Init(DSP_TASK_ID);
    memset(&Audio_Status, 0, sizeof(AUDIO_STATUS_t));
    //DSP_CommandInit();

    if (dtm_queue == NULL) {
        dtm_queue = xQueueCreate(DTM_QUEUE_MAX_NUMBER, sizeof(DTM_QUEUE_t));
    }
}

#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
extern void full_adapt_anc_process(void);
#endif
#ifdef AIR_HW_VIVID_PT_ENABLE
extern void hw_vivid_passthru_process(void);
#endif

/**
 * DTM
 *
 * Main Manager for DSP Task
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
VOID DTM(VOID)
{
#if DspIpcEnable
    DSP_CMD_PTR_t DspCmdPtr;
    DSP_CMD_ACT_t DspCmdAction;
    DSP_CMD_PARA_t DspCmdPara;
#endif
    DTM_QUEUE_t cur_queue;

    /* Do Initialization */
    DTM_Init();

    /* Test Command */
#if (FEA_SUPP_DSP_AUDIO_VERIFICATION)
    AUDIO_ASSERT(DSP_CommandConstructAndDelivery(DSP_TEST_MODE, DSP_TASK_ID, &DspCmdPara));
#else
    //UNUSED(DspCmdPara);
#endif
    portYIELD();
    portYIELD();
#if (FEA_SUPP_DSP_AUDIO_LOOPBACK_TEST)
    portYIELD();
    DSP_SCN_STD_LINE_MUSIC();
#elif (AudioTestMode)
    Audio_Setup();
#else
    //Audio_Setup();
#endif
#ifdef AIR_DSP_TASK_PRINT_PROCESS_WARNING_LOG_ENABLE
    uint32_t start_time, end_time;
#endif
    while (1) {
#ifdef AIR_DSP_TASK_PRINT_PROCESS_WARNING_LOG_ENABLE
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &start_time);
#endif
        xQueueReceive(dtm_queue, &cur_queue, portMAX_DELAY);
        switch (cur_queue.event_id) {
#ifdef AIR_SIDETONE_ENABLE
            case DTM_EVENT_ID_SIDETONE_START:
                afe_set_sidetone_enable(true, &dsp_afe_sidetone, &dsp_afe_sidetone_extension, false);
                DSP_MW_LOG_I("receive sidetone start 0x%x\r\n", 1, dsp_afe_sidetone_extension.device_handle_in_side_tone.sidetone.audio_device);
#if 0//modify for ab1568
#ifdef ENABLE_SIDETONE_RAMP_TIMER
                fw_sidetone_set_ramp_timer(dsp_afe_sidetone.gain);
#else
                dsp_sidetone_start_volume_set();
#endif
#endif
                break;
            case DTM_EVENT_ID_SIDETONE_STOP_RAMP:
                afe_set_sidetone_enable(false, &dsp_afe_sidetone, &dsp_afe_sidetone_extension, false);
                DSP_MW_LOG_I("receive sidetone stop ramp\r\n", 0);
                break;
            case DTM_EVENT_ID_SIDETONE_STOP: {
                /*Send SIDETONE_STOP ack*/
                hal_ccni_message_t msg;
                memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
                msg.ccni_message[0] = (MSG_MCU2DSP_COMMON_SIDETONE_STOP << 16) | 0x80000000;
                aud_msg_tx_handler(msg, 0, FALSE);
                DSP_MW_LOG_I("receive sidetone stop\r\n", 0);
            }

            break;
#endif
#ifdef MTK_SENSOR_SOURCE_ENABLE
            case DTM_EVENT_ID_GSENSOR_WATERMARK_TRIGGER:
                bsp_multi_axis_read_sensor_data();
                volatile SOURCE source = Source_blks[SOURCE_TYPE_GSENSOR];
                AudioCheckTransformHandle(source->transform);
                break;
#endif
#ifdef AIR_AUDIO_DUMP_BY_SPDIF_ENABLE
            case DTM_EVENT_ID_AUDIO_DUMP:
                audio_dump_task_handler(cur_queue.arg);
                break;
#endif
#ifdef MTK_WWE_ENABLE
            case DTM_EVENT_ID_VOW_DISABLE:
                // vow_disable(dsp_vow, NULL);
                vow_disable(&hwvad_vow_control, NULL);
                DSP_MW_LOG_I("receive VOW Disable\r\n", 0);
                break;
            case DTM_EVENT_ID_VOW_ENABLE:
                vow_enable(&hwvad_vow_control, NULL);
                DSP_MW_LOG_I("receive VOW Enable\r\n", 0);
                break;
#endif
#if defined (AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE)
            case DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_ENABLE:
                switch ((audio_scenario_type_t)(cur_queue.arg))
                {
#if defined(AIR_GAMING_MODE_DONGLE_ENABLE)
                    case AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_0:
                        gaming_mode_dongle_game_chat_volume_smart_balance_enable();
                        break;
#endif /* AIR_GAMING_MODE_DONGLE_ENABLE */

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
                    case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
                        ull_audio_v2_dongle_game_chat_volume_smart_balance_enable();
                        break;
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */

                    default:
                        break;
                }
                break;

            case DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_PROCESS:
                switch ((audio_scenario_type_t)(cur_queue.arg))
                {
#if defined(AIR_GAMING_MODE_DONGLE_ENABLE)
                    case AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_0:
                        gaming_mode_dongle_game_chat_volume_smart_balance_do_process();
                        break;
#endif /* AIR_GAMING_MODE_DONGLE_ENABLE */

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
                    case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
                        ull_audio_v2_dongle_game_chat_volume_smart_balance_do_process();
                        break;
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */

                    default:
                        break;
                }
                break;

            case DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_DISABLE:
                switch ((audio_scenario_type_t)(cur_queue.arg))
                {
#if defined(AIR_GAMING_MODE_DONGLE_ENABLE)
                    case AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_0:
                        gaming_mode_dongle_game_chat_volume_smart_balance_disable();
                break;
#endif /* AIR_GAMING_MODE_DONGLE_ENABLE */

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
                    case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
                        ull_audio_v2_dongle_game_chat_volume_smart_balance_disable();
                        break;
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */

                    default:
                        break;
                }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
#if 0
#if defined (AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE) && defined (AIR_WIRED_AUDIO_ENABLE)
            case DTM_EVENT_ID_WIRED_AUDIO_USB_VOLUME_SMART_BALANCE_ENABLE:
                wired_audio_usb_in_volume_smart_balance_enable((void *)(cur_queue.arg));
                break;

            case DTM_EVENT_ID_WIRED_AUDIO_USB_VOLUME_SMART_BALANCE_PROCESS:
                wired_audio_usb_in_volume_smart_balance_do_process((void *)(cur_queue.arg));
                break;

            case DTM_EVENT_ID_WIRED_AUDIO_USB_VOLUME_SMART_BALANCE_DISABLE:
                wired_audio_usb_in_volume_smart_balance_disable((void *)(cur_queue.arg));
                break;
#endif /* AIR_WIRED_AUDIO_USB_VOLUME_SMART_BALANCE_ENABLE */
#endif
#ifdef AIR_SILENCE_DETECTION_ENABLE
            case DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_INIT:
                SilenceDetection_Scenario_Init((void *)(cur_queue.arg));
                break;

            case DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_DEINIT:
                SilenceDetection_Scenario_Deinit((void *)(cur_queue.arg));
                break;

            case DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_ENABLE:
                SilenceDetection_Scenario_Enable((void *)(cur_queue.arg));
                break;

            case DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_DISABLE:
                SilenceDetection_Scenario_Disable((void *)(cur_queue.arg));
                break;

            case DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_PROCESS:
                SilenceDetection_Scenario_Process((void *)(cur_queue.arg));
                break;
#endif /* AIR_SILENCE_DETECTION_ENABLE */
            case DTM_EVENT_ID_AUDIO_VOLUME_MONITOR_NOTIFY:
            case DTM_EVENT_ID_AUDIO_AMP_NOTIFY:
            case DTM_EVENT_ID_AUDIO_SYNC_END: {
                /* audio sync done ack to APP */
                hal_ccni_message_t msg;
                memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
                msg.ccni_message[0] = cur_queue.arg;
                while (aud_msg_tx_handler(msg, 0, FALSE) != AUDIO_MSG_STATUS_OK) {
                    DSP_MW_LOG_E("ERROR: [0x%x], event id:%d, ack fail\r\n", 2, cur_queue.arg,cur_queue.event_id);
                    vTaskDelay(pdMS_TO_TICKS(100)); // delay 100ms
                }
                if (cur_queue.event_id == DTM_EVENT_ID_AUDIO_SYNC_END) {
                    DSP_MW_LOG_I("[DSP SYNC] sync done [0x%x], ack success\r\n", 1, cur_queue.arg);
                } else if (cur_queue.event_id == DTM_EVENT_ID_AUDIO_AMP_NOTIFY) {
                    DSP_MW_LOG_I("[DSP AMP] afe_send_amp_status_ccni [0x%x], ack success\r\n", 1, cur_queue.arg);
                } else {
                    DSP_MW_LOG_I("Success: [0x%x], ack fail\r\n", 1, cur_queue.arg);
                }
            }
            break;
#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
            case DTM_EVENT_ID_FULL_ADAPT_ANC_PROCESS: {
                full_adapt_anc_process();
            }
            break;
#endif
#ifdef AIR_HW_VIVID_PT_ENABLE
            case DTM_EVENT_ID_HW_VIVID_PASSTHRU: {
                hw_vivid_passthru_process();
            }
            break;
#endif
#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
            case DTM_EVENT_ID_DAC_DEACTIVE_MODE_ENTER:
                hal_audio_device_enter_dac_deactive_mode(true);
                break;

            case DTM_EVENT_ID_DAC_DEACTIVE_MODE_EXIT:
                {
                    uint32_t old_dac_mode = hal_volume_get_analog_mode(AFE_HW_ANALOG_GAIN_OUTPUT);

                    if(old_dac_mode != cur_queue.arg) {
                        hal_volume_set_analog_mode(AFE_HW_ANALOG_GAIN_OUTPUT, cur_queue.arg);
                        hal_audio_device_enter_dac_deactive_mode(false);
                    } else {
                        DSP_MW_LOG_E("invalid DTM event id: %d\n", 1, cur_queue.event_id);
                        assert(0);
                    }
                }
                break;
#endif
            default:
                DSP_MW_LOG_E("invalid DTM event id: %d\n", 1, cur_queue.event_id);
                break;

        }
#ifdef AIR_DSP_TASK_PRINT_PROCESS_WARNING_LOG_ENABLE
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &end_time);
        if ((uint32_t)(end_time - start_time) > DTM_TASK_MAX_RUNTIME) {
            DSP_MW_LOG_W("[DTM_TASK] task process time:%d us, event_id: %d", 2, (uint32_t)(end_time - start_time), cur_queue.event_id);
        }
#endif
    }
}


/**
 * DTM_ResumeDPRT
 *
 * API to resume DPRT in DTM
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
VOID DTM_ResumeDPRT(VOID)
{
    vTaskResume(DPR_TASK_ID);
}


/**
 * DTM_ResumeDAVT
 *
 * API to resume DAVT in DTM
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
VOID DTM_ResumeDAVT(VOID)
{
    vTaskResume(DAV_TASK_ID);
}

#if DspIpcEnable

/**
 * DTM_CheckTaskAvailable
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
BOOL DTM_CheckTaskAvailable(U8 DspTaskID)
{
    switch (DspTaskID) {
        case DSP_TASK_ID:
            return TRUE;

        case DPR_TASK_ID:
            return (DspJobStatus.PR == DSP_JOB_SUSPEND);

        case DAV_TASK_ID:
            return (DspJobStatus.AV == DSP_JOB_SUSPEND);

        default:
            AUDIO_ASSERT(0);
            break;
    }
    return TRUE;
}




/**
 * DTM_JobStatusHandler
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
DSP_CMD_ACT_t DTM_JobStatusHandler(DSP_COMMAND_t *DspCmdPtr)
{
    U8 TaskID = DspCmdPtr->TaskID;
    DSP_CMD_ACT_t DspCmdAction;

    AUDIO_ASSERT(DspCmdPtr != 0);

    switch (TaskID) {
        case DSP_TASK_ID:
            DspCmdAction = DspTmHdler(DspCmdPtr);
            break;

        case DPR_TASK_ID:
            DspCmdAction = DspPrJobHdlr[DspJobStatus.PR](DspCmdPtr);
            break;

        case DAV_TASK_ID:
            DspCmdAction = DspAvJobHdlr[DspJobStatus.AV](DspCmdPtr);
            break;
        default:
            OS_TRAP();
            break;
    }

    return DspCmdAction;
}



/**
 * DTM_CommandHandler
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
VOID DTM_CommandHandler(DSP_CMD_PTR_t DspCmdPtr, DSP_CMD_ACT_t Action)
{

    switch (Action) {
        case DSP_CMD_CLEAR:
            OSMEM_Put(DspCmdPtr);
            break;

        case DSP_CMD_FORWARD_TO_DPRT:
            DSP_CommandForward(DspCmdPtr, DPR_TASK_ID);
            break;

        case DSP_CMD_FORWARD_TO_DAVT:
            DSP_CommandForward(DspCmdPtr, DAV_TASK_ID);
            break;

        case DSP_CMD_PUTBACK:
            DSP_CommandDelivery(DspCmdPtr, DSP_TASK_ID);
            break;

        default:
            OS_TRAP();
            break;
    }

}


/**
 * DTM_DSP_JobHandler
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
DSP_CMD_ACT_t DTM_DSP_JobHandler(DSP_CMD_PTR_t DspCmdPtr)
{
    DSP_CMD_MSG_t DspMsg = DspCmdPtr->DspMsg;

#if 0
    DSP_CMD_PARA_t DspReplyCmdPara;
#endif


    switch (DspMsg) {
#if (FEA_SUPP_DSP_AUDIO_VERIFICATION)
        case  DSP_TEST_MODE :
            DSP_FakeAudioProcess();
            break;
#endif
        case DSP_MSG_SCO_Tx_READY:
            SCO_Setup_Tx_Para(&(DspCmdPtr->DspCmdPara.TxPara), TRUE);
            break;
        case DSP_MSG_SCO_Rx_READY:
            SCO_Setup_Rx_Para(&(DspCmdPtr->DspCmdPara.RxPara), TRUE);
            break;
        case DSP_MSG_SCO_Tx_DISABLE:
            SCO_Setup_Tx_Para(NULL, FALSE);
            break;
        case DSP_MSG_SCO_Rx_DISABLE:
            SCO_Setup_Rx_Para(NULL, FALSE);
            break;
        default :
            OS_TRAP();
            break;
    }




    /* Check Reply */
#if 0
    if (DspCmdPtr->TxTaskID == HOST_TASK_ID) {
        AUDIO_ASSERT(DSP_CommandConstructAndDelivery(DspCmdPtr->DspMsg, HOST_TASK_ID, &DspReplyCmdPara));
    }
#endif



    return DSP_CMD_CLEAR;
}



/**
 * DTM_AV_JobSuspend
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
DSP_CMD_ACT_t DTM_AV_JobSuspend(DSP_CMD_PTR_t DspCmdPtr)
{
    if (DspCmdPtr->DspMsg == DSP_MSG_STOP_AVT) {
        return DSP_CMD_CLEAR;
    } else {
        assert((DspCmdPtr->DspMsg > DSP_MSG_AVT_START_ENUM_BEGIN)
               && (DspCmdPtr->DspMsg < DSP_MSG_AVT_START_ENUM_END));

        DspJobStatus.AV = DSP_JOB_INIT;
        return DSP_CMD_FORWARD_TO_DAVT;
    }
}


/**
 * DTM_AV_JobInit
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
DSP_CMD_ACT_t DTM_AV_JobInit(DSP_CMD_PTR_t DspCmdPtr)
{
    if (DspCmdPtr->DspMsg == DSP_MSG_AVT_READY_TO_START) {
        DspJobStatus.AV = DSP_JOB_START;
        return DSP_CMD_CLEAR;
    } else {
        return DSP_CMD_PUTBACK;
    }
}


/**
 * DTM_AV_JobActive
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
DSP_CMD_ACT_t DTM_AV_JobActive(DSP_CMD_PTR_t DspCmdPtr)
{
    if (DspCmdPtr->DspMsg == DSP_MSG_STOP_AVT) {
        DspJobStatus.AV = DSP_JOB_DEINIT;
        return DSP_CMD_FORWARD_TO_DAVT;
    } else {
        return DSP_CMD_PUTBACK;
    }
}


/**
 * DTM_AV_JobDeInit
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
DSP_CMD_ACT_t DTM_AV_JobDeInit(DSP_CMD_PTR_t DspCmdPtr)
{
    if (DspCmdPtr->DspMsg == DSP_MSG_AVT_READY_TO_STOP) {
        DspJobStatus.AV = DSP_JOB_SUSPEND;
        DSP_CommandKeepNewest(DAV_TASK_ID);
        return DSP_CMD_CLEAR;
    } else {
        return DSP_CMD_PUTBACK;
    }
}



/**
 * DTM_PR_JobSuspend
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
DSP_CMD_ACT_t DTM_PR_JobSuspend(DSP_CMD_PTR_t DspCmdPtr)
{
    if (DspCmdPtr->DspMsg == DSP_MSG_STOP_PRT) {
        return DSP_CMD_CLEAR;
    } else {
        assert((DspCmdPtr->DspMsg > DSP_MSG_PRT_START_ENUM_BEGIN)
               && (DspCmdPtr->DspMsg < DSP_MSG_PRT_START_ENUM_END));

        DspJobStatus.PR = DSP_JOB_INIT;
        return DSP_CMD_FORWARD_TO_DPRT;
    }
}


/**
 * DTM_PR_JobInit
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
DSP_CMD_ACT_t DTM_PR_JobInit(DSP_CMD_PTR_t DspCmdPtr)
{
    if (DspCmdPtr->DspMsg == DSP_MSG_PRT_READY_TO_START) {
        DspJobStatus.PR = DSP_JOB_START;
        return DSP_CMD_CLEAR;
    } else if (DspCmdPtr->DspMsg == DSP_MSG_STOP_PRT) {
        return DSP_CMD_PUTBACK;
    } else {
        return DSP_CMD_PUTBACK;
    }
}


/**
 * DTM_PR_JobActive
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
DSP_CMD_ACT_t DTM_PR_JobActive(DSP_CMD_PTR_t DspCmdPtr)
{
    DSP_CMD_PTR_t DupeCmdPtr;

    if (DspCmdPtr->DspMsg == DSP_MSG_STOP_PRT) {
        DspJobStatus.PR = DSP_JOB_DEINIT;

        if (DspCmdPtr->TxTaskID == HOST_TASK_ID) {
            DupeCmdPtr = DSP_CommandDuplicate(DspCmdPtr);
            DSP_CommandDelivery(DupeCmdPtr, DSP_TASK_ID);
        }

        return DSP_CMD_FORWARD_TO_DPRT;
    } else {
        return DSP_CMD_PUTBACK;
    }
}


/**
 * DTM_PR_JobDeInit
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
DSP_CMD_ACT_t DTM_PR_JobDeInit(DSP_CMD_PTR_t DspCmdPtr)
{
    if (DspCmdPtr->DspMsg == DSP_MSG_PRT_READY_TO_STOP) {
        DspJobStatus.PR = DSP_JOB_SUSPEND;
        DSP_CommandKeepActiveGroup(DPR_TASK_ID);
        return DSP_CMD_CLEAR;
    } else {
        return DSP_CMD_PUTBACK;
    }
}
#endif




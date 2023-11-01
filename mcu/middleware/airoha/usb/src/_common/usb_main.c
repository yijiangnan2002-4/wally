/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifdef AIR_USB_ENABLE

/* C library */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Kernel includes */
#if FREERTOS_ENABLE
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "task_def.h"
#endif /* FREERTOS_ENABLE */

/* USB Middleware includes */
#include "usb.h"
#include "usb_case.h"
#include "usb_custom.h"
#include "usb_host_detect.h"
#include "usb_main.h"
#include "usb_resource.h"
#include "usbacm_adap.h"
#include "usbacm_drv.h"
#include "usbaudio_drv.h"
#include "usbhid_drv.h"

#ifdef  AIR_USB_MSC_ENABLE
#include "usbms_state.h"
#include "usbms_adap.h"
#include "usbms_drv.h"
#endif

#ifdef AIR_USB_XBOX_ENABLE
#include "usb_xbox.h"
#endif /* AIR_USB_XBOX_ENABLE */

/* Other Middleware includes */
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#endif /* MTK_BATTERY_MANAGEMENT_ENABLE */

#ifdef AIR_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "usb_nvkey_struct.h"
#endif /* AIR_NVDM_ENABLE */

#ifdef MTK_PORT_SERVICE_ENABLE
#include "serial_port.h"
#endif /* MTK_PORT_SERVICE_ENABLE */

/* Hal includes */
#include "hal_gpio.h"
#include "hal_gpt.h"
#include "hal_nvic_internal.h"
#include "hal_pmu.h"
#include "hal_usb.h"
#include "hal_usb_internal.h"

#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_platform.h"
#endif /* HAL_SLEEP_MANAGER_ENABLED */

/* Other includes */
#include "memory_attribute.h"

#include "usb_dbg.h"

/* Syslog create module for usb_main.c */
log_create_module_variant(USB_MAIN, DEBUG_LOG_ON, PRINT_LEVEL_INFO);

#ifdef AIR_USB_HID_ENABLE
extern mux_usb_hid_callback g_mux_usb_hid_callback;
#endif

/*****************************************************************************
 * Static Variables
 *****************************************************************************/
static volatile bool usb_drven = false;
static usb_evt_callback_hdlr_t usb_evt_callback[USB_EVT_NUM][USB_USER_NUM] = {
    [0 ... (USB_EVT_NUM-1)] = {
        [0 ... (USB_USER_NUM-1)] =  {
            NULL, NULL, NULL
        },
    },
};

/*****************************************************************************
 * Static Functions
 *****************************************************************************/
static void usb_drv_init(void);
static void usb_drv_deinit(void);
static void usb_detect_vbus_change(void);
static void usb_plug_in_routine(void);
static void usb_plug_out_routine(void);

/*****************************************************************************
 * Global Variables
 *****************************************************************************/
#ifdef MTK_PORT_SERVICE_ENABLE
extern serial_port_register_callback_t g_serial_port_usb_callback[MAX_USB_PORT_NUM];
#endif

#ifdef  AIR_USB_MSC_ENABLE
msc_register_callback_t g_msc_usb_callback = NULL;
#endif

#define BOOT_TIMEOUT_CNT 900000
/* 200ms, uint:us*/
#define USB_ENUM_RESET_TIMEOUT 200000

volatile bool usb_initial = false;

#ifdef FREERTOS_ENABLE
static TaskHandle_t usb_task_handle = NULL;
static QueueHandle_t usb_queue_handle = NULL;
#endif /* FREERTOS_ENABLE */

extern void usb_hisr(hal_nvic_irq_t irq_number);

bool is_vusb_ready(void)
{
#ifndef FPGA_ENV
    return (bool)pmu_get_usb_input_status();
#else
    return 1;
#endif
}

#if FREERTOS_ENABLE
void USB_Send_Message(usb_msg_type_t msg, void *data)
{
    usb_msg_t msgs;
    BaseType_t xHigherPriorityTaskWoken;
    BaseType_t ret;

    if (usb_queue_handle == NULL) {
        LOG_MSGID_I(USB_MAIN, "USB_Send_Message usb task not initlize", 0);
        return;
    }

    // We have not woken a task at the start of the ISR.
    xHigherPriorityTaskWoken = pdFALSE;

    msgs.msg_id = msg;
    msgs.data = data;

    if (0 == HAL_NVIC_QUERY_EXCEPTION_NUMBER) {
        ret = xQueueSend(usb_queue_handle, &msgs, 0);
        LOG_MSGID_I(USB_MAIN, "USB_Send_Message in Task id:%d", 1, msgs.msg_id);
    } else {
        ret = xQueueSendFromISR(usb_queue_handle, &msgs, &xHigherPriorityTaskWoken);
        LOG_MSGID_I(USB_MAIN, "USB_Send_Message in ISR id:%d", 1, msgs.msg_id);
    }

    if (ret != pdTRUE) {
        LOG_MSGID_E(USB_MAIN, "USB_Send_Message queue size:%d ", 1, USB_QUEUE_LENGTH);
    }
    // Now the buffer is empty we can switch context if necessary.
    if (xHigherPriorityTaskWoken) {
        // Actual macro used here is port specific.
        portYIELD_FROM_ISR(pdTRUE);
    }
}

static uint32_t rmwk_stime;
static uint32_t rmwk_ctime;
static uint32_t rmwk_dtime;

#ifdef AIR_NVIDIA_REFLEX_ENABLE
/* 1ms */
#define USB_RMWK_SIGNAL_TIME 1000
#else
/* Reusme Signal time in us. Defaule is 10000 = 10ms */
#define USB_RMWK_SIGNAL_TIME 10000
#endif

static void usb_task_main(void *pvParameters)
{
    usb_msg_t msgs;

    static uint32_t stack_max = 0;
    uint32_t stack_cur = 0;
#if defined(AIR_USB_AUDIO_ENABLE)
    uint32_t temp;
#endif

    while (1) {
        if (usb_queue_handle != NULL) {
            if (xQueueReceive(usb_queue_handle, &msgs, portMAX_DELAY)) {

                LOG_MSGID_I(USB_MAIN, "usb_task_main receive queue id:%d", 1, msgs.msg_id);

                switch (msgs.msg_id) {
                    case USB_ACM_MSG:
                        LOG_MSGID_I(USB_MAIN, "usb_task_main USB_ACM_MSG", 0);
                        break;
#ifdef  AIR_USB_MSC_ENABLE
                    case USB_MSC_RX_MSG:
                        if(gUsbDevice.conf == NULL)
                            break;
                        USB_Ms_State_Main(MSG_ID_USB_MSDRV_REC_DONE_CONF, (usb_ms_rec_done_conf_struct *) msgs.data);
                        LOG_MSGID_I(USB_MAIN, "usb_task_main USB_MSC_RX_MSG: 0x%x", 1, msgs.data);
                        break;
                    case USB_MSC_TX_MSG:
                        if(gUsbDevice.conf == NULL)
                            break;
                        USB_Ms_State_Main(MSG_ID_USB_MSDRV_TRX_DONE_CONF, NULL);
                        LOG_MSGID_I(USB_MAIN, "usb_task_main USB_MSC_TX_MSG", 0);
                        break;
                    case USB_MSC_CLR_STALL_MSG:
                        USB_Ms_State_Main(MSG_ID_USB_MSDRV_CLEAR_STALL_REQ, NULL);
                        LOG_MSGID_I(USB_MAIN, "usb_task_main USB_MSC_TX_MSG", 0);
                        break;
                    case USB_MSC_RESET_IND_MSG:
                        USB_Ms_State_Main(MSG_ID_USB_MSDRV_RESET_IND, NULL);
                        LOG_MSGID_I(USB_MAIN, "usb_task_main USB_MSC_TX_MSG", 0);
                        break;
#endif
#if defined(AIR_USB_AUDIO_ENABLE)
                    case USB_AUDIO_SET_INTERFACE:
                        USB_Audio_Set_Interface_CB((uint32_t)msgs.data);
                        break;
                    case USB_AUDIO_SET_SAMPLING_FREQ:
                        LOG_MSGID_I(USB_MAIN, "usb_task_main speaker sample ep[%d],rate[%d]", 2, ((uint32_t)(msgs.data)) >> 24, ((uint32_t)(msgs.data)) & 0x00FFFFFF);
                        if((((uint32_t)(msgs.data)) >> 24) == (USB_EP_DIR_OUT | g_UsbAudio[0].stream_ep_out_id)){
                            if(USB_Audio[0].setsamplingrate_cb){
                                USB_Audio[0].setsamplingrate_cb(((uint32_t)(msgs.data)) >> 24, ((uint32_t)(msgs.data)) & 0x00FFFFFF);
                            }
                        }

                        if((((uint32_t)(msgs.data)) >> 24) == (USB_EP_DIR_OUT | g_UsbAudio[1].stream_ep_out_id)){
                            if(USB_Audio[1].setsamplingrate_cb){
                                USB_Audio[1].setsamplingrate_cb(((uint32_t)(msgs.data)) >> 24, ((uint32_t)(msgs.data)) & 0x00FFFFFF);
                            }
                        }

                        break;
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
                    case USB_MIC_SET_SAMPLING_FREQ:
                        LOG_MSGID_I(USB_MAIN, "usb_task_main mic sample ep[%d],rate[%d]", 2, ((uint32_t)(msgs.data)) >> 24, ((uint32_t)(msgs.data)) & 0x00FFFFFF);
                        if (USB_Audio[0].setsamplingrate_cb_mic) {
                            #ifndef USB_TEST_MIC 
                            USB_Audio[0].setsamplingrate_cb_mic(((uint32_t)(msgs.data)) >> 24, ((uint32_t)(msgs.data)) & 0x00FFFFFF);
                            #endif
                        }
                        break;
#endif
                    case USB_AUDIO_RX_DATA:
                        if (USB_Audio[0].rx_cb) {
                            USB_Audio[0].rx_cb();
                        }
                        temp = g_UsbAudio[0].msg_notify;
                        if (temp > 0) {
                            temp--;
                            g_UsbAudio[0].msg_notify = temp;
                        }
                        break;
                    case USB_AUDIO_UNPLUG:
                        LOG_MSGID_I(USB_MAIN, "usb_task_main usb cable disconnect", 0);
                        if (USB_Audio[0].unplug_cb) {
                            USB_Audio[0].unplug_cb();
                        }
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
                        if (USB_Audio[0].unplug_cb_mic) {
                            USB_Audio[0].unplug_cb_mic();
                        }
#endif
                        break;
                    case USB_AUDIO_SET_MUTE:
                        LOG_MSGID_I(USB_MAIN, "usb_task_main speaker mute ep[%d]mute[%d]", 2, ((uint32_t)(msgs.data)) >> 24, ((uint32_t)(msgs.data)) & 0x000000FF);
                        if (((((uint32_t)(msgs.data)) >> 24) == 0x01) && USB_Audio[0].mute_cb) {
                            USB_Audio[0].mute_cb(((uint32_t)(msgs.data)) >> 24, ((uint32_t)(msgs.data)) & 0x000000FF);
                        } else if (((((uint32_t)(msgs.data)) >> 24) == 0x02) && USB_Audio[1].mute_cb) {
                            USB_Audio[1].mute_cb(((uint32_t)(msgs.data)) >> 24, ((uint32_t)(msgs.data)) & 0x000000FF);
                        }
                        break;
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
                    case USB_MIC_SET_MUTE:
                        LOG_MSGID_I(USB_MAIN, "usb_task_main mic mute ep[%d]mute[%d]", 2, ((uint32_t)(msgs.data)) >> 24, ((uint32_t)(msgs.data)) & 0x000000FF);
                        if (USB_Audio[0].mute_cb_mic) {
                            #ifndef USB_TEST_MIC 
                            USB_Audio[0].mute_cb_mic(((uint32_t)(msgs.data)) >> 24, ((uint32_t)(msgs.data)) & 0x000000FF);
                            #endif
                        }
                        break;
#endif
                    case USB_AUDIO_SET_VOLUME: {
                            uint16_t volume_cur = (uint16_t) ((uint32_t)msgs.data);
                            uint8_t  ch_index = (uint8_t) ((uint32_t)msgs.data>> 16);
                            uint8_t  ep_addr = (uint8_t) ((uint32_t)msgs.data>> 24);
                            uint8_t  spk_index = 0;
                            AUDIO_VOLUMECHANGE_FUNC volumechange_cb = 0;

                            uint8_t  spk1_ep_addr = (USB_EP_DIR_OUT | g_UsbAudio[0].stream_ep_out_id);
                            uint8_t  spk2_ep_addr = (USB_EP_DIR_OUT | g_UsbAudio[1].stream_ep_out_id);
                            if(ep_addr == spk1_ep_addr){
                                spk_index = 0;
                                volumechange_cb = USB_Audio[spk_index].volumechange_cb;
                            }
                            else if(ep_addr == spk2_ep_addr){
                                spk_index = 1;
                                volumechange_cb = USB_Audio[spk_index].volumechange_cb;
                            }

                            uint16_t volume_0to100 = USB_Audio_Volume_0to100_Convertor(volume_cur, g_UsbAudio[spk_index].spk_min, g_UsbAudio[spk_index].spk_max);
                            int32_t  volume_dB     = USB_Audio_Volume_dB_Convertor(volume_cur);

                            LOG_MSGID_I(USB_MAIN, "usb_task_main speaker volume ep:0x%02X ch:%d vol:%d(raw:%04x) dB:%d", 5, ep_addr, ch_index, volume_0to100, volume_cur, volume_dB);
                            if( volumechange_cb){
                                volumechange_cb( ep_addr, ch_index, volume_0to100, volume_dB);
                             }
                        }
                        break;
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
                    case USB_MIC_SET_VOLUME: {
                            uint16_t volume_cur = (uint16_t) ((uint32_t)msgs.data);
                            uint8_t  ch_index = (uint8_t) ((uint32_t)msgs.data>> 16);
                            uint8_t  ep_addr = (uint8_t) ((uint32_t)msgs.data>> 24);
                            uint16_t volume_0to100 = USB_Audio_Volume_0to100_Convertor(volume_cur, g_UsbAudio[0].mic_min, g_UsbAudio[0].mic_max);
                            int32_t  volume_dB     = USB_Audio_Volume_dB_Convertor(volume_cur);

                            LOG_MSGID_I(USB_MAIN, "usb_task_main mic volume ep:0x%02X ch:%d vol:%d(raw:%04x) dB:%d", 5, ep_addr, ch_index, volume_0to100, volume_cur, volume_dB);
                            if (USB_Audio[0].volumechange_cb_mic) {
                                #ifndef USB_TEST_MIC 
                                USB_Audio[0].volumechange_cb_mic( ep_addr, ch_index, volume_0to100, volume_dB);
                                #endif
                            }
                        }
                        break;
#endif
#endif
                    case USB_ATCI_NOTIFY_UNPLUG:
                        usb_case_atci_call(false);
                        break;
                    case USB_ATCI_NOTIFY_PLUG:
                        usb_case_atci_call(true);
                        break;
                    case USB_RACE_NOTIFY_PLUG:
#ifdef AIR_USB_HID_ENABLE
                        if (g_mux_usb_hid_callback) {
                            g_mux_usb_hid_callback(USB_MUX_PORT1, HID_EVENT_USB_CONNECTION, NULL);
                            g_mux_usb_hid_callback(USB_MUX_PORT2, HID_EVENT_USB_CONNECTION, NULL);
                        }
#endif /* AIR_USB_HID_ENABLE */
                        usb_case_race_call(true);
                        break;
                    case USB_RACE_NOTIFY_UNPLUG:
                        usb_case_race_call(false);
#ifdef AIR_USB_HID_ENABLE
                        if (g_mux_usb_hid_callback) {
                            g_mux_usb_hid_callback(USB_MUX_PORT1, HID_EVENT_USB_DISCONNECTION, NULL);
                            g_mux_usb_hid_callback(USB_MUX_PORT2, HID_EVENT_USB_DISCONNECTION, NULL);
                        }
#endif /* AIR_USB_HID_ENABLE */
                        break;
                    case USB_CONFIG_DONE:
                        usb_evt_exec_cb(USB_EVT_CONFIG_DONE, NULL);
                        break;
#ifdef AIR_USB_HID_ENABLE
                    case USB_HID_SET_REPORT:
                        USB_HID_Ep0_DispatchData();
                        break;
                    case USB_HID_GET_REPORT:
                        break;
#endif
                    case USB_AP_REBOOT:
                        LOG_MSGID_I(USB_MAIN, "usb_task_main USB_AP_REBOOT AP_TYPE[%d]", 1, (int)msgs.data);
                        ap_usb_deinit();
                        ap_usb_init((usb_dev_type_t)msgs.data);
                        break;
                    case USB_REMOTE_WAKEUP:
                        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &rmwk_stime);
                        USB_Send_Message(USB_REMOTE_WAKEUP_CHK, NULL);
                        break;
                    case USB_REMOTE_WAKEUP_CHK:
                        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &rmwk_ctime);
                        hal_gpt_get_duration_count(rmwk_stime, rmwk_ctime, &rmwk_dtime);
                        if (rmwk_dtime > USB_RMWK_SIGNAL_TIME) {
                            USB_Send_Message(USB_REMOTE_WAKEUP_DONE, NULL);
                            hal_usb_rmwk_clear();
                        } else {
                            USB_Send_Message(USB_REMOTE_WAKEUP_CHK, NULL);
                        }
                        break;
                    case USB_REMOTE_WAKEUP_DONE:
                        break;
                    default:
                        break;
                }

                stack_cur = (USB_TASK_STACKSIZE - uxTaskGetStackHighWaterMark(usb_task_handle) * sizeof(portSTACK_TYPE));
                if (stack_cur > stack_max) {
                    stack_max = stack_cur;
                    LOG_MSGID_I(USB_MAIN, "task max-usage: %d", 1, stack_max);
                }

            }
        }
    }
}


bool usb_task_init(void)
{
    portBASE_TYPE xReturn = pdFALSE;

    /* Check task and queue is ready */
    if ((usb_queue_handle != NULL) || (usb_task_handle != NULL)) {
        LOG_MSGID_I(USB_MAIN, "usb_task_init task have inited, return !", 0);
        return true;
    }

    /* Queue creation */
    usb_queue_handle = xQueueCreate(USB_QUEUE_LENGTH, sizeof(usb_msg_t));

    if (usb_queue_handle == NULL) {
        LOG_MSGID_E(USB_MAIN, "usb_task_init queue create Fail!", 0);
        return false;
    }

    /* Task creation */
    xReturn = xTaskCreate(usb_task_main, USB_TASK_NAME, USB_TASK_STACKSIZE / sizeof(portSTACK_TYPE), NULL, USB_TASK_PRIO, &usb_task_handle);

    if (xReturn == pdFALSE) {
        vQueueDelete(usb_queue_handle);
        usb_queue_handle = NULL;
        LOG_MSGID_E(USB_MAIN, "usb_task_init task create Fail!", 0);
        return false;
    }

    LOG_MSGID_I(USB_MAIN, "usb_task_init success to create task and queue", 0);
    return true;
}

void usb_task_deinit(void)
{
    //for atci to notify task.
#if defined(USB_ATCI_FEATURE)

#else
    if (usb_queue_handle != NULL) {
        vQueueDelete(usb_queue_handle);
        usb_queue_handle = NULL;
    }

    if (usb_task_handle != NULL) {
        vTaskDelete(usb_task_handle);
        usb_task_handle = NULL;
    }
#endif
}
#endif /* FREERTOS_ENABLE */

void usb_dev_app_init_cdc_acm()
{
    /* Enable IAD */
    gUsbDevice.use_iad = true;

    /* USB_EVT_DEVICE_INIT callback */
    usb_evt_device_init_data_t usb_data = {
        .device_type = gUsbDevice.device_type,
    };
    usb_evt_exec_cb(USB_EVT_DEVICE_INIT, (void *)&usb_data);

#ifdef AIR_USB_CDC_ENABLE

#if (AIR_USB_CDC_PORT_NUM == 0) || (AIR_USB_CDC_PORT_NUM > 2)
#error "AIR_USB_CDC_PORT_NUM must be 1 or 2"
#endif

#if AIR_USB_CDC_PORT_NUM >= 1
    USB_Register_CreateFunc("ACM COMMU1",
                            USB_Acm1_CommIf_Create,
                            USB_Acm1_CommIf_Reset,
                            USB_Acm1_CommIf_Enable,
                            (usb_speed_if_func_ptr)USB_Acm1_CommIf_Speed_Reset,
                            NULL);

    USB_Register_CreateFunc("ACM DATA1",
                            USB_Acm1_DataIf_Create,
                            USB_Acm1_DataIf_Reset,
                            USB_Acm1_DataIf_Enable,
                            (usb_speed_if_func_ptr)USB_Acm1_DataIf_Speed_Reset,
                            NULL);
#endif

#if AIR_USB_CDC_PORT_NUM >= 2
    USB_Register_CreateFunc("ACM COMMU2",
                            USB_Acm2_CommIf_Create,
                            USB_Acm2_CommIf_Reset,
                            USB_Acm2_CommIf_Enable,
                            (usb_speed_if_func_ptr)USB_Acm2_CommIf_Speed_Reset,
                            NULL);

    USB_Register_CreateFunc("ACM DATA2",
                            USB_Acm2_DataIf_Create,
                            USB_Acm2_DataIf_Reset,
                            USB_Acm2_DataIf_Enable,
                            (usb_speed_if_func_ptr)USB_Acm2_DataIf_Speed_Reset,
                            NULL);
#endif

    USB_Init(USB_CDC_ACM);
#endif
}

void usb_dev_app_init_msc()
{
    /* Disable IAD */
    gUsbDevice.use_iad = false;

    /* USB_EVT_DEVICE_INIT callback */
    usb_evt_device_init_data_t usb_data = {
        .device_type = gUsbDevice.device_type,
    };
    usb_evt_exec_cb(USB_EVT_DEVICE_INIT, (void *)&usb_data);

#ifdef  AIR_USB_MSC_ENABLE
    g_UsbMS.ms_param = USB_GetMsParam();

    //USB_Ms_Register_DiskDriver(&USB_RAM_drv);
    USB_Ms_Register_DiskDriver(&usbms_msdc_driver);

#ifdef MTK_FATFS_ON_SERIAL_NAND
    USB_Ms_Register_DiskDriver(&usbms_spinand_driver);
#endif

    USB_Register_CreateFunc("MASS STORAGE", USB_Ms_If_Create, USB_Ms_If_Reset,
                            USB_Ms_If_Enable, (usb_speed_if_func_ptr)USB_Ms_If_Speed_Reset, USB_Ms_If_Resume);

    /* initialize MASS STORAGE MODE */
    USB_Init(USB_MASS_STORAGE);
#endif
}

void usb_dev_app_init_hid()
{
    /* Disable IAD */
    gUsbDevice.use_iad = false;

    /* USB_EVT_DEVICE_INIT callback */
    usb_evt_device_init_data_t usb_data = {
        .device_type = gUsbDevice.device_type,
    };
    usb_evt_exec_cb(USB_EVT_DEVICE_INIT, (void *)&usb_data);

#ifdef AIR_USB_HID_ENABLE
    uint8_t hid_num = 0;
    for (uint8_t i = 0; i < USB_HID_MAX_DEVICE_NUM; i++) {
        if(g_UsbHids[i].enable) {
            USB_Register_CreateFunc(
                "HID",
                usb_hid_if_create_funcs[i],
                usb_hid_if_init_funcs[i],
                usb_hid_if_enable_funcs[i],
                usb_hid_if_speed_funcs[i],
                usb_hid_if_resume_funcs[i]
            );
            hid_num++;
        }
    }
#endif

    USB_Init(USB_HID);
}

void usb_dev_app_init_audio()
{
    /* Disable IAD */
    gUsbDevice.use_iad = false;

#if defined(AIR_USB_AUDIO_ENABLE)
#ifdef AIR_NVDM_ENABLE
    USB_Audio_Setting_By_NVKey();
#endif /* AIR_NVDM_ENABLE */
#endif /* AIR_USB_AUDIO_ENABLE */

    /* USB_EVT_DEVICE_INIT callback */
    usb_evt_device_init_data_t usb_data = {
        .device_type = gUsbDevice.device_type,
    };
    usb_evt_exec_cb(USB_EVT_DEVICE_INIT, (void *)&usb_data);

#if defined(AIR_USB_AUDIO_ENABLE)
    LOG_MSGID_I(USB_MAIN, "usb_dev_app_init_audio speaker1:%d microphone:%d speaker2:%d", 3,
                g_UsbAudio[0].spk_feature_en, g_UsbAudio[0].mic_feature_en, g_UsbAudio[1].spk_feature_en);

    for (uint8_t i = 0; i < USB_DSCR_FEATURE_MAX_CONTROLS; ++i)
    {
        g_UsbAudio[0].spk_cur_channel_vol[i] = g_UsbAudio[0].spk_cur;
        g_UsbAudio[0].mic_cur_channel_vol[i] = g_UsbAudio[0].mic_cur;
        g_UsbAudio[1].spk_cur_channel_vol[i] = g_UsbAudio[1].spk_cur;
        g_UsbAudio[1].mic_cur_channel_vol[i] = g_UsbAudio[1].mic_cur;
    }

#ifdef USB_AUDIO_INVERSE_SPEAER_ENUM
#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
        if(g_UsbAudio[1].spk_feature_en) {
            USB_Register_CreateFunc("AUDIO2 Control", USB_Audio2_ControlIf_Create, USB_Audio2_ControlIf_Reset,
                                    USB_Audio2_ControlIf_Enable, (usb_speed_if_func_ptr)USB_Audio2_ControlIf_Speed_Reset, NULL);
            USB_Register_CreateFunc("AUDIO2 Stream", USB_Audio2_StreamIf_Create, USB_Audio2_StreamIf_Reset,
                                    USB_Audio2_StreamIf_Enable, (usb_speed_if_func_ptr)USB_Audio2_StreamIf_Speed_Reset, NULL);
        }
#endif
#endif

    USB_Register_CreateFunc("AUDIO1 Control", USB_Audio1_ControlIf_Create, USB_Audio1_ControlIf_Reset,
                        USB_Audio1_ControlIf_Enable, (usb_speed_if_func_ptr)USB_Audio1_ControlIf_Speed_Reset, NULL);
#ifdef AIR_USB_AUDIO_1_SPK_ENABLE
    USB_Register_CreateFunc("AUDIO1 Stream", USB_Audio1_StreamIf_Create, USB_Audio1_StreamIf_Reset,
                        USB_Audio1_StreamIf_Enable, (usb_speed_if_func_ptr)USB_Audio1_StreamIf_Speed_Reset, NULL);
#endif

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    USB_Register_CreateFunc("AUDIO Microphone", USB_Audio_StreamIf_Microphone_Create, USB_Audio_StreamIf_Microphone_Reset,
                        USB_Audio_StreamIf_Microphone_Enable, (usb_speed_if_func_ptr)USB_Audio_StreamIf_Microphone_Speed_Reset, NULL);
#endif

#ifndef USB_AUDIO_INVERSE_SPEAER_ENUM
#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
    if(g_UsbAudio[1].spk_feature_en) {
        USB_Register_CreateFunc("AUDIO2 Control", USB_Audio2_ControlIf_Create, USB_Audio2_ControlIf_Reset,
                                USB_Audio2_ControlIf_Enable, (usb_speed_if_func_ptr)USB_Audio2_ControlIf_Speed_Reset, NULL);
        USB_Register_CreateFunc("AUDIO2 Stream", USB_Audio2_StreamIf_Create, USB_Audio2_StreamIf_Reset,
                                USB_Audio2_StreamIf_Enable, (usb_speed_if_func_ptr)USB_Audio2_StreamIf_Speed_Reset, NULL);
    }
#endif
#endif

#endif

#ifdef AIR_USB_HID_ENABLE
    uint8_t hid_num = 0;
    for (uint8_t i = 0; i < USB_HID_MAX_DEVICE_NUM; i++) {
        if(g_UsbHids[i].enable) {
            USB_Register_CreateFunc(
                "HID",
                usb_hid_if_create_funcs[i],
                usb_hid_if_init_funcs[i],
                usb_hid_if_enable_funcs[i],
                usb_hid_if_speed_funcs[i],
                usb_hid_if_resume_funcs[i]
            );
            hid_num++;
        }
    }

    if (hid_num > 1) {
        gUsbDevice.use_iad = true;
    }
#endif

    USB_Init(USB_AUDIO);
}

/*******************************************************
*   XBOX APP Initialise
*******************************************************/
#ifdef AIR_USB_XBOX_ENABLE
void usb_dev_app_init_xbox()
{
    /* Disable IAD */
    gUsbDevice.use_iad = false;

    LOG_MSGID_I(USB_MAIN, "usb_dev_app_init_xbox", 0);

    /* Create interface */
    USB_Register_CreateFunc("XBOX Control",
                            USB_XBOX_ControlIf_Create,
                            USB_XBOX_ControlIf_Reset,
                            USB_XBOX_ControlIf_Enable,
                            (usb_speed_if_func_ptr)USB_XBOX_ControlIf_Speed_Reset,
                            NULL);

    USB_Register_CreateFunc("XBOX Audio",
                            USB_XBOX_AudioIf_Create,
                            USB_XBOX_AudioIf_Reset,
                            USB_XBOX_AudioIf_Enable,
                            (usb_speed_if_func_ptr)USB_XBOX_AudioIf_Speed_Reset,
                            NULL);

    /* Initilize USB_XBOX */
    USB_Init(USB_XBOX);
}
#endif


/**
 * Senior: Dongle/Headset connect on Huawei SmartPhone.
 *
 * Huawei SmartPhone's phone call need below requirements.
 * 1. Device is in full-speed.
 * 2. Microphone supports 48k Hz.
 * 3. Only one USB audio device.
 */
void usb_dev_app_init_audio_smartphone()
{
    /* Disable IAD */
    gUsbDevice.use_iad = false;

#if defined(AIR_USB_AUDIO_ENABLE)
#ifdef AIR_NVDM_ENABLE
    USB_Audio_Setting_By_NVKey();
#endif /* AIR_NVDM_ENABLE */

    for (uint8_t i = 0; i < USB_DSCR_FEATURE_MAX_CONTROLS; ++i)
    {
        g_UsbAudio[0].spk_cur_channel_vol[i] = g_UsbAudio[0].spk_cur;
        g_UsbAudio[0].mic_cur_channel_vol[i] = g_UsbAudio[0].mic_cur;
        g_UsbAudio[1].spk_cur_channel_vol[i] = g_UsbAudio[1].spk_cur;
        g_UsbAudio[1].mic_cur_channel_vol[i] = g_UsbAudio[1].mic_cur;
    }

    USB_Register_CreateFunc("AUDIO1 Control", USB_Audio1_ControlIf_Create, USB_Audio1_ControlIf_Reset,
                            USB_Audio1_ControlIf_Enable, (usb_speed_if_func_ptr)USB_Audio1_ControlIf_Speed_Reset, NULL);
    USB_Register_CreateFunc("AUDIO1 Stream", USB_Audio1_StreamIf_Create, USB_Audio1_StreamIf_Reset,
                            USB_Audio1_StreamIf_Enable, (usb_speed_if_func_ptr)USB_Audio1_StreamIf_Speed_Reset, NULL);
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    USB_Register_CreateFunc("AUDIO Microphone", USB_Audio_StreamIf_Microphone_Create, USB_Audio_StreamIf_Microphone_Reset,
                            USB_Audio_StreamIf_Microphone_Enable, (usb_speed_if_func_ptr)USB_Audio_StreamIf_Microphone_Speed_Reset, NULL);
#endif
#endif

#ifdef AIR_USB_HID_ENABLE
    uint8_t hid_num = 0;
    for (uint8_t i = 0; i < USB_HID_MAX_DEVICE_NUM; i++) {
        if(g_UsbHids[i].enable) {
            USB_Register_CreateFunc(
                "HID",
                usb_hid_if_create_funcs[i],
                usb_hid_if_init_funcs[i],
                usb_hid_if_enable_funcs[i],
                usb_hid_if_speed_funcs[i],
                usb_hid_if_resume_funcs[i]
            );
            hid_num++;
        }
    }

    if (hid_num > 1) {
        gUsbDevice.use_iad = true;
    }
#endif

    USB_Init(USB_AUDIO_SMARTPHONE);
}

/*  Refer to usb_dev_app_init_audio(), only enable HID for logging.
    Senior: headset connect with dongle by BT which can't accept USB-Audio in headset side;
             otherwise, headset disconnect with dongle.
    Becase this scenior, headset side can't enable USB-Audio(only enalbe USB-HID) for logging.
*/
void usb_dev_app_init_hid_logging()
{
    /* Disable IAD */
    gUsbDevice.use_iad = false;

#ifdef AIR_USB_HID_ENABLE
    uint8_t hid_num = 0;
    for (uint8_t i = 0; i < USB_HID_MAX_DEVICE_NUM; i++) {
        if(g_UsbHids[i].enable) {
            USB_Register_CreateFunc(
                "HID",
                usb_hid_if_create_funcs[i],
                usb_hid_if_init_funcs[i],
                usb_hid_if_enable_funcs[i],
                usb_hid_if_speed_funcs[i],
                usb_hid_if_resume_funcs[i]
            );
            hid_num++;
        }
    }

    if (hid_num > 1) {
        gUsbDevice.use_iad = true;
    }
#endif

#if (defined(AIR_USB_HID_ENABLE))
    USB_Init(USB_AUDIO);
#endif
}

void usb_dev_app_init_audio_cdc()
{
    /* Enable IAD */
    gUsbDevice.use_iad = true;

    USB_Register_CreateFunc("ACM COMMU1",
                            USB_Acm1_CommIf_Create,
                            USB_Acm1_CommIf_Reset,
                            USB_Acm1_CommIf_Enable,
                            (usb_speed_if_func_ptr)USB_Acm1_CommIf_Speed_Reset,
                            NULL);

    USB_Register_CreateFunc("ACM DATA1",
                            USB_Acm1_DataIf_Create,
                            USB_Acm1_DataIf_Reset,
                            USB_Acm1_DataIf_Enable,
                            (usb_speed_if_func_ptr)USB_Acm1_DataIf_Speed_Reset,
                            NULL);
#if defined(AIR_USB_AUDIO_ENABLE)
#ifdef AIR_NVDM_ENABLE
    USB_Audio_Setting_By_NVKey();
#endif /* AIR_NVDM_ENABLE */

    for (uint8_t i = 0; i < USB_DSCR_FEATURE_MAX_CONTROLS; ++i)
    {
        g_UsbAudio[0].spk_cur_channel_vol[i] = g_UsbAudio[0].spk_cur;
        g_UsbAudio[0].mic_cur_channel_vol[i] = g_UsbAudio[0].mic_cur;
        g_UsbAudio[1].spk_cur_channel_vol[i] = g_UsbAudio[1].spk_cur;
        g_UsbAudio[1].mic_cur_channel_vol[i] = g_UsbAudio[1].mic_cur;
    }

    /* USB_EVT_DEVICE_INIT callback */
    usb_evt_device_init_data_t usb_data = {
        .device_type = gUsbDevice.device_type,
    };
    usb_evt_exec_cb(USB_EVT_DEVICE_INIT, (void *)&usb_data);

    USB_Register_CreateFunc("AUDIO1 Control", USB_Audio1_ControlIf_Create, USB_Audio1_ControlIf_Reset,
                            USB_Audio1_ControlIf_Enable, (usb_speed_if_func_ptr)USB_Audio1_ControlIf_Speed_Reset, NULL);
    USB_Register_CreateFunc("AUDIO1 Stream", USB_Audio1_StreamIf_Create, USB_Audio1_StreamIf_Reset,
                            USB_Audio1_StreamIf_Enable, (usb_speed_if_func_ptr)USB_Audio1_StreamIf_Speed_Reset, NULL);
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    USB_Register_CreateFunc("AUDIO Microphone", USB_Audio_StreamIf_Microphone_Create, USB_Audio_StreamIf_Microphone_Reset,
                            USB_Audio_StreamIf_Microphone_Enable, (usb_speed_if_func_ptr)USB_Audio_StreamIf_Microphone_Speed_Reset, NULL);
#endif

#endif

#ifdef AIR_USB_HID_ENABLE
    uint8_t hid_num = 0;
    for (uint8_t i = 0; i < USB_HID_MAX_DEVICE_NUM; i++) {
        if(g_UsbHids[i].enable) {
            USB_Register_CreateFunc(
                "HID",
                usb_hid_if_create_funcs[i],
                usb_hid_if_init_funcs[i],
                usb_hid_if_enable_funcs[i],
                usb_hid_if_speed_funcs[i],
                usb_hid_if_resume_funcs[i]
            );
            hid_num++;
        }
    }

    if (hid_num > 1) {
        gUsbDevice.use_iad = true;
    }
#endif


    USB_Init(USB_AUDIO_CDC);
}

usb_init_status_t usb_common_get_bc12(uint8_t *bc12_type)
{
    usb_init_status_t ret = usb_common_init_bc12_fail;

#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
    *bc12_type = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_TYPE);

    ret = usb_common_init_sucess;
#endif

    /**
     * Bypass BC1.2
     */
#ifndef FPGA_ENV
    *bc12_type = SDP_CHARGER;
#else
    *bc12_type = 1;
#endif

    LOG_MSGID_I(USB_MAIN, "usb_common_get_bc12 ret[%d] type[%d]", 2, ret, *bc12_type);

    return ret;
}

bool ap_usb_init(usb_dev_type_t type)
{
    uint32_t mask_irq = 0;

    hal_nvic_save_and_set_interrupt_mask(&mask_irq);
    if (usb_initial == true) {
        hal_nvic_restore_interrupt_mask(mask_irq);
        return false;
    }
    usb_initial = true;
    hal_nvic_restore_interrupt_mask(mask_irq);

    /* Lock sleep during USB usage */
#ifdef HAL_SLEEP_MANAGER_ENABLED
#ifndef HAL_SLEEP_MANAGER_SUPPROT_DEEPSLEEP_LOCK
    hal_sleep_manager_lock_sleep(SLEEP_LOCK_USB);
#else
    hal_sleep_manager_acquire_sleeplock(usb_sleep_handler, HAL_SLEEP_LOCK_ALL);
#endif
#endif /* HAL_SLEEP_MANAGER_ENABLED */

    hal_usb_init();

    LOG_MSGID_I(USB_MAIN, "usb app init type [%d]", 1, type);

    switch (type) {
        case USB_CDC_ACM:
            usb_dev_app_init_cdc_acm();
            break;
        case USB_MASS_STORAGE:
            usb_dev_app_init_msc();
            break;
        case USB_HID:
            usb_dev_app_init_hid();
            break;
        case USB_AUDIO:
            usb_dev_app_init_audio();
            break;
        case USB_AUDIO_SMARTPHONE:
            usb_dev_app_init_audio_smartphone();
            break;
        case USB_HID_LOGGING:
            usb_dev_app_init_hid_logging();
            break;
#ifdef AIR_USB_XBOX_ENABLE
        case USB_XBOX:
            usb_dev_app_init_xbox();
            break;
#endif
        case USB_AUDIO_CDC:
            usb_dev_app_init_audio_cdc();
            break;
        default:
            LOG_MSGID_E(USB_MAIN, "ap_usb_init : Fail", 0);
            break;
    }
    return true;
}

bool ap_usb_deinit(void)
{
    uint32_t mask_irq = 0;

    hal_nvic_save_and_set_interrupt_mask(&mask_irq);
    if (usb_initial == false) {
        hal_nvic_restore_interrupt_mask(mask_irq);
        return false;
    }
    hal_nvic_restore_interrupt_mask(mask_irq);

    /* Disable USB IRQ, otherwise USB irq would be tirggered during USB plug in/out (usb deinit stage) */
    hal_nvic_disable_irq(USB_IRQn);

    USB_Release_Type();

#ifdef HAL_SLEEP_MANAGER_ENABLED
#ifndef HAL_SLEEP_MANAGER_SUPPROT_DEEPSLEEP_LOCK
    hal_sleep_manager_unlock_sleep(SLEEP_LOCK_USB);
#else
    //hal_sleep_manager_release_sleeplock(usb_sleep_handler, HAL_SLEEP_LOCK_ALL);
#endif
#endif /* HAL_SLEEP_MANAGER_ENABLED */
    /*LOG_I(USB_MAIN,"--------- usb unlcok sleep\n");*/
    hal_nvic_save_and_set_interrupt_mask(&mask_irq);
    usb_initial = false;
    hal_nvic_restore_interrupt_mask(mask_irq);

    return true;
}

bool usb_device_test_case(usb_device_case_type_t usb_case)
{
    bool ret_value = false;

    LOG_MSGID_I(USB_MAIN, "usb_device_test_case case [%d]", 1, (int)usb_case);

    uint32_t i = 0;

    switch (usb_case) {
        case usb_device_slt:
            ap_usb_init(USB_CDC_ACM);

            do {
                usb_hisr(USB_IRQn);
                if (i++ >= BOOT_TIMEOUT_CNT) {
                    break;
                }
            } while ((USB_Get_Device_State() != DEVSTATE_CONFIG));

            if (USB_Get_Device_State() == DEVSTATE_CONFIG) {
                ret_value = true;
                LOG_MSGID_I(USB_MAIN, "usb_device_test_case slt pass", 0);
            } else {
                ret_value = false;
                LOG_MSGID_I(USB_MAIN, "usb_device_test_case slt fail", 0);
            }
            ap_usb_deinit();
            break;
        default:
            break;
    }

    return ret_value;
}

#ifdef  AIR_USB_MSC_ENABLE
void ap_usb_register_msc_callback(msc_register_callback_t callback)
{
    g_msc_usb_callback = callback;
    LOG_MSGID_I(USB_MAIN, "register MSC connection callback\n", 0);

    if (g_msc_usb_callback != NULL) {
        if (is_vusb_ready()) {
            g_msc_usb_callback(MSC_EVENT_USB_CONNECTION);
            LOG_MSGID_I(USB_MAIN, "MSC_CONNECTION callback\n", 0);
        } else {
            g_msc_usb_callback(MSC_EVENT_USB_DISCONNECTION);
            LOG_MSGID_I(USB_MAIN, "MSC_DISCONNECTION callback\n", 0);
        }
    }
}
#endif

/* Set/Get USB Host Type */
USB_HOST_TYPE g_usb_host_type = USB_HOST_TYPE_UNKNOWN;

USB_HOST_TYPE Set_USB_Host_Type(USB_HOST_TYPE usb_host_type)
{
    LOG_MSGID_I(USB_MAIN, "Set_USB_Host_Type[%x]", 1, usb_host_type);
    g_usb_host_type = usb_host_type;
    return g_usb_host_type;
}

USB_HOST_TYPE Get_USB_Host_Type()
{
    LOG_MSGID_I(USB_MAIN, "Get_USB_Host_Type[%x]", 1, g_usb_host_type);
    return g_usb_host_type;
}

extern USB_HOST_TYPE Set_USB_Host_Type(USB_HOST_TYPE usb_host_type);
extern USB_HOST_TYPE Get_USB_Host_Type();

/*****************************************************************************
 * Battery Management Related Functions
 *****************************************************************************/
#if MTK_BATTERY_MANAGEMENT_ENABLE
static void usb_bm_callback(battery_management_event_t event, const void *data)
{
    if (usb_drven == false) {
        return;
    }
    if (event == BATTERY_MANAGEMENT_EVENT_CHARGER_EXIST_UPDATE) {
        usb_detect_vbus_change();
    }
}

void usb_register_bm_callback(void)
{
    static bool registered = false;
    battery_management_status_t status;

    if (registered == true) {
        /* Battery management callback has been registered. */
        return;
    }

    status = battery_management_register_callback(usb_bm_callback);

    if (status == BATTERY_MANAGEMENT_STATUS_OK) {
        registered = true;
    } else {
        LOG_MSGID_I(USB_MAIN, "Cannot register battery callback", 0);
    }
}
#endif /* MTK_BATTERY_MANAGEMENT_ENABLE */

/*****************************************************************************
 * USB Events Functions
 *****************************************************************************/
void usb_evt_exec_cb(usb_evt_t event, void *usb_data)
{
    usb_cb_func_t func;
    void* user_data;

    for (uint32_t usr = 0; usr < USB_USER_NUM; usr++) {
        func = usb_evt_callback[event][usr].func;
        user_data = usb_evt_callback[event][usr].user_data;
        LOG_MSGID_I(USB_MAIN, "usb_evt_exec_cb evt[%d], user[%d], func[0x%08X], usb_data[0x%08X], user_data[0x%08X]", 5, event, usr, func, usb_data, user_data);
        if (func) {
            func(event, usb_data, user_data);
        }
    }
}

void usb_evt_register_cb(usb_user_t user, usb_evt_t evt, usb_cb_func_t func, void* user_data)
{
    LOG_MSGID_I(USB_MAIN, "usb_evt_register_cb evt[%d], user[%d], func[0x%08X], user_data[0x%08X]", 4, evt, user, func, user_data);
    usb_evt_callback[evt][user].func = func;
    usb_evt_callback[evt][user].user_data = user_data;
}

/*****************************************************************************
 * USB Device Type Functions
 *****************************************************************************/
void usb_set_device_type(usb_dev_type_t type)
{
    if (gUsbDevice.initialized == true) {
        LOG_MSGID_W(USB, "usb_set_device_type device[%d] is initialized. Couldn't change.", 1, gUsbDevice.device_type);
        return;
    }
    LOG_MSGID_I(USB_MAIN, "usb_set_device_type before[%d], after[%d]", 2, gUsbDevice.device_type, type);
    gUsbDevice.device_type = type;
}

usb_dev_type_t usb_get_device_type(void)
{
    return gUsbDevice.device_type;
}

static usb_dev_type_t usb_device_type_read_logging_mode_nvkey(void)
{
    usb_dev_type_t type = USB_UNKOWN;
    uint8_t logging_mode_enable = 0;
#ifdef AIR_NVDM_ENABLE
    nvkey_status_t nvkey_status = NVKEY_STATUS_OK;
    uint32_t nvkey_size = sizeof(uint8_t);
    nvkey_status = nvkey_read_data(NVID_APP_USB_LOGGING_MODE, &logging_mode_enable, &nvkey_size);

    LOG_MSGID_I(USB_MAIN, "usb_device_type_read_logging_mode_nvkey nvkey_status[%d] logging_mode[%d]", 2, nvkey_status, logging_mode_enable);
    if (nvkey_status != NVKEY_STATUS_OK) {
        logging_mode_enable = 0;
    }
#endif /* AIR_NVDM_ENABLE */
    if (logging_mode_enable == 1) {
        type = USB_HID_LOGGING;
    }
    return type;
}

static usb_dev_type_t usb_device_type_read_nvkey(void)
{
    usb_dev_type_t type = USB_UNKOWN;
    /**
     * TODO: Add read nvkey flow
     */
    return type;
}

usb_dev_type_t usb_device_type_get_default(void)
{
    usb_dev_type_t type = USB_UNKOWN;

    if (type == USB_UNKOWN) {
#if defined(AIR_USB_AUDIO_ENABLE) || defined(AIR_USB_HID_ENABLE) || defined(AIR_USB_XBOX_ENABLE)
        type = USB_AUDIO;
#elif defined(AIR_USB_MSC_ENABLE)
        type = USB_MASS_STORAGE;
#else
        type = USB_CDC_ACM;
#endif
    }

    return type;
}

void usb_device_type_check_and_set(void)
{
    usb_dev_type_t type = USB_UNKOWN;

    /* Check APP layer set the device type */
    if (gUsbDevice.device_type != USB_UNKOWN) {
        return;
    }

    /* use device by NVID_APP_USB_LOGGING_MODE */
    type = usb_device_type_read_logging_mode_nvkey();
    if (type != USB_UNKOWN) {
        gUsbDevice.device_type = type;
        return;
    }

    /* use device by NVID_USB_SETTING_0 */
    type = usb_device_type_read_nvkey();
    if (type != USB_UNKOWN) {
        gUsbDevice.device_type = type;
        return;
    }

    /* use device by feature option */
    type = usb_device_type_get_default();
    if (type != USB_UNKOWN) {
        gUsbDevice.device_type = type;
        return;
    }

    /* Device type is still USB_UNKOWN */
    LOG_MSGID_E(USB, "USB device is not assigned!", 0);
    assert(0);
}

/*****************************************************************************
 * USB Device Init Flow
 *****************************************************************************/
void usb_drv_enable(void)
{
    if (usb_drven == true) {
        LOG_MSGID_W(USB_MAIN, "usb_drv_enable, usb driver has been enabled", 0);
        return;
    }
    LOG_MSGID_I(USB_MAIN, "usb_drv_enable", 0);
    usb_drven = true;

#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
    usb_register_bm_callback();
#endif /* MTK_BATTERY_MANAGEMENT_ENABLE */
#ifdef ATCI_ENABLE
    usb_atci_init();
#endif /* ATCI_ENABLE */
    usb_detect_vbus_change();
}

void usb_drv_disable(void)
{
    if (usb_drven == false) {
        LOG_MSGID_W(USB_MAIN, "usb_drv_disable, usb driver has been disabled", 0);
        return;
    }
    LOG_MSGID_I(USB_MAIN, "usb_drv_disable", 0);
    usb_drven = false;

    usb_detect_vbus_change();
}

#ifdef AIR_NVDM_ENABLE
extern uint8_t g_hal_usb_flag;
extern bool    g_USB_dynamic_fifo;

static void usb_device_cfg_nvkey_read(void)
{
    usb_nvkey_device_cfg_t devcfg;
    uint32_t nvkey_size = sizeof(devcfg);
    nvkey_status_t nvkey_status = nvkey_read_data(NVID_USB_SETTING_0, (uint8_t *)&devcfg, &nvkey_size);

    if (nvkey_status == NVKEY_STATUS_OK) {
        // this bit6 is for internal test use, it will remove it later
        if(devcfg.byte0_bit5) {
            g_USB_dynamic_fifo = !g_USB_dynamic_fifo;
        }

        // this bit6 is for internal test use, it will remove it later
        if(devcfg.byte0_bit6) {
            g_USB_Software_Speed_skip_enable = 0;
        } else {
            g_USB_Software_Speed_skip_enable = 1;
        }

        // this bit7 is for internal test use, it will remove it later
        if(devcfg.byte0_bit7) {
            g_hal_usb_flag &= ~(1<<0); // clr HAL_USB_FLAG__W1C
        } else {
            g_hal_usb_flag |= (1<<0);  // set HAL_USB_FLAG__W1C
        }
    }

    LOG_MSGID_I(USB_MAIN, "usb_device_cfg_nvkey_read, nvkey_status[%d] hs_fs[%d] devcfg[0x%02x]", 3, nvkey_status, devcfg.hs_fs, *((uint8_t*)&devcfg));

#ifdef USB_DBG
    usb_nvkey_dbg_t nvdbg;
    nvkey_size   = sizeof(nvdbg);
    nvkey_status = nvkey_read_data(NVID_USB_DBG, (uint8_t *)&nvdbg, &nvkey_size);
    if (nvkey_status == NVKEY_STATUS_OK)
    {
        g_USB_DBG_filter = nvdbg.dbg_filter_1;
        g_usb_dbg_opt[0] = nvdbg.dbg_option_1;
        g_usb_dbg_opt[1] = nvdbg.dbg_option_2;
        LOG_MSGID_I(USB_MAIN, "g_USB_DBG_filter 0x%08x, g_usb_dbg_opt[0]:0x%08x, g_usb_dbg_opt[1]:0x%08x",
            3, g_USB_DBG_filter, g_usb_dbg_opt[0], g_usb_dbg_opt[1]);
    }
#endif
}
#endif /* AIR_NVDM_ENABLE */

static void usb_drv_init(void)
{
    usb_init_status_t ret    = usb_common_init_sucess;
    uint8_t charger_type     = 0;

    LOG_MSGID_I(USB_MAIN, "usb_drv_init", 0);

#ifdef AIR_NVDM_ENABLE
    usb_device_cfg_nvkey_read();
#endif

    hal_usb_phy_preinit();
    ret &= usb_common_get_bc12(&charger_type);
    LOG_MSGID_I(USB_MAIN, "charger_type [%d]", 1, charger_type);
#ifdef USB_HOST_DETECT_ENABLE
    USB_HostDetect_ResetRecorder();
#endif /* USB_HOST_DETECT_ENABLE */

#ifdef FREERTOS_ENABLE
    usb_task_init();
#endif /* FREERTOS_ENABLE */

    usb_device_type_check_and_set();
    ap_usb_init(gUsbDevice.device_type);

#ifdef MTK_PORT_SERVICE_ENABLE
    if (g_serial_port_usb_callback[0] != NULL) {
        g_serial_port_usb_callback[0](SERIAL_PORT_DEV_USB_COM1, SERIAL_PORT_EVENT_USB_CONNECTION, NULL);
        LOG_MSGID_I(USB_MAIN, "USB1 CONNECTION callback", 0);
    }
    if (g_serial_port_usb_callback[1] != NULL) {
        g_serial_port_usb_callback[1](SERIAL_PORT_DEV_USB_COM2, SERIAL_PORT_EVENT_USB_CONNECTION, NULL);
        LOG_MSGID_I(USB_MAIN, "USB2 CONNECTION callback", 0);
    }
#endif
}

static void usb_drv_deinit(void)
{
    LOG_MSGID_I(USB_MAIN, "usb_drv_deinit", 0);

#ifdef FREERTOS_ENABLE
    USB_Send_Message(USB_ATCI_NOTIFY_UNPLUG, NULL);
    USB_Send_Message(USB_RACE_NOTIFY_UNPLUG, NULL);
#endif /* FREERTOS_ENABLE */

#ifdef MTK_PORT_SERVICE_ENABLE
    if (g_serial_port_usb_callback[0] != NULL) {
        g_serial_port_usb_callback[0](SERIAL_PORT_DEV_USB_COM1, SERIAL_PORT_EVENT_USB_DISCONNECTION, NULL);
        LOG_MSGID_I(USB_MAIN, "USB1 CONNECTION callback", 0);
    }
    if (g_serial_port_usb_callback[1] != NULL) {
        g_serial_port_usb_callback[1](SERIAL_PORT_DEV_USB_COM2, SERIAL_PORT_EVENT_USB_DISCONNECTION, NULL);
        LOG_MSGID_I(USB_MAIN, "USB2 CONNECTION callback", 0);
    }
#endif

    ap_usb_deinit();

#ifdef FREERTOS_ENABLE
    usb_task_deinit();
#endif /* FREERTOS_ENABLE */

    hal_usb_save_current();
}

void usb_cable_detect(void)
{
    LOG_MSGID_W(USB_MAIN, "usb_cable_detect, this function should be replaced by usb_drv_enable", 0);
    usb_drv_enable();
}

static void usb_detect_vbus_change(void)
{
    static bool prev_vbus  = false;
    static bool prev_drven = false;

    bool curr_vbus  = is_vusb_ready();
    bool curr_drven = usb_drven;
    bool plug_in_evt = false;
    bool plug_out_evt = false;

    LOG_MSGID_I(USB_MAIN, "usb_detect_vbus_change VBUS[%d->%d] DRVEN[%d->%d]",
                4, prev_vbus, curr_vbus, prev_drven, curr_drven);

    /* Detect vbus change */
    if (prev_vbus == true && curr_vbus == true) {
        /* Detect driver enable/disable change */
        if (prev_drven == true && curr_drven == false) {
            /* Driver enable -> disable, make a fake plug-out */
            plug_out_evt = true;
        } else if (prev_drven == false && curr_drven == true) {
            /* Driver disable -> enable, make a fake plug-in */
            plug_in_evt = true;
        }

    } else if (prev_vbus == true && curr_vbus == false && curr_drven == true) {
        /* Vbus plug out */
        plug_out_evt = true;
    } else if (prev_vbus == true && curr_vbus == false && prev_drven == true && curr_drven == false) {
        /* Vbus plug out */
        plug_out_evt = true;
    } else if (prev_vbus == false && curr_vbus == true && curr_drven == true) {
        /* Vbus plug in */
        plug_in_evt = true;
    }

    /* Record current value for next call */
    prev_vbus  = curr_vbus;
    prev_drven = curr_drven;

    /* Execute routine */
    if (plug_in_evt) {
        LOG_MSGID_I(USB_MAIN, "usb_detect_vbus_change PLUG IN", 0);
        usb_plug_in_routine();
    }
    if (plug_out_evt) {
        LOG_MSGID_I(USB_MAIN, "usb_detect_vbus_change PLUG OUT", 0);
        usb_plug_out_routine();
    }
}

void usb_plug_in_routine(void)
{
    usb_evt_exec_cb(USB_EVT_PLUG_IN, NULL);
    usb_drv_init();
}

void usb_plug_out_routine(void)
{
    usb_evt_exec_cb(USB_EVT_PLUG_OUT, NULL);
    usb_drv_deinit();
}

#endif /* AIR_USB_ENABLE */

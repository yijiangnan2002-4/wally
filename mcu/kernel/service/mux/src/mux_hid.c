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
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO ObtAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */
#ifdef AIR_MUX_BT_HID_ENABLE
#include "bt_type.h"
#include "hal_platform.h"
#include "hal_nvic.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "mux.h"
#include "mux_port_device.h"
#include "assert.h"
#include "mux_port.h"
#include "ble_air_interface.h"
#include "spp_air_interface.h"
#include "bt_utils.h"
#include "bt_hid.h"

log_create_module(mux_hid, PRINT_LEVEL_INFO);
SemaphoreHandle_t x_mux_hid_Semaphore[2];
mux_irq_handler_t g_mux_hid_callback;
virtual_read_write_point_t g_mux_hid_r_w_point[MUX_HID_END - MUX_HID_BEGIN + 1];
typedef void (*mux_bt_hid_callback)(uint8_t channel_type, uint32_t event, void *parameter, uint32_t channel);
uint32_t bt_hid_cnt_handle;
//static bool g_mux_hid_ponit_init[MUX_HID_END - MUX_HID_BEGIN + 1] = {0};
//static mux_port_config_t *p_local_setting[MUX_HID_END - MUX_HID_BEGIN + 1] = {0};
mux_bt_hid_callback g_mux_bt_hid_callback;

uint32_t g_mux_hid_phase2_send_status;
typedef uint8_t bt_hid_channel_type;
#define HID_CONRTROL_CHANNEL_TYPE        0x01
#define HID_INTERRUPT_CHANNEL_TYPE        0x02

#define BT_HID_REPORT_DATA_LENGTH         0x3B  /* HID Report size of  tool is 62-byte, Available data is 62-3 = 59 (0x3B)(Report ID, Length, Target Device)*/

#define TARGET_LOCAL_DEVICE                      0x00
#define TARGET_REMOTE_DEVICE                     0x80
#define TARGET_REMOTE_MULTIDEVICE1               0x81
#define TARGET_REMOTE_MULTIDEVICE2               0x82
#define TARGET_REMOTE_MULTIDEVICE3               0x83
#define TARGET_REMOTE_MULTIDEVICE4               0x84
#define TARGET_EXTERNAL_DEVICE                   0x90
#define TARGET_INVALID_DEVICE                    0xFF

uint8_t g_mux_hid_data_flag = 0;

uint8_t hid_mux_set_data_flag(uint8_t flag)
{
    if ( flag == TARGET_LOCAL_DEVICE ||
         flag == TARGET_REMOTE_DEVICE ||
         flag == TARGET_REMOTE_MULTIDEVICE1 ||
         flag == TARGET_REMOTE_MULTIDEVICE2 ||
         flag == TARGET_REMOTE_MULTIDEVICE3 ||
         flag == TARGET_REMOTE_MULTIDEVICE4 ||
         flag == TARGET_EXTERNAL_DEVICE)
        {
            g_mux_hid_data_flag = flag;
            LOG_MSGID_I(mux_hid, "hid_mux_check_data_flag valid target device flag[%X]", 1, flag);
            return flag;
        }
        LOG_MSGID_E(mux_hid, "hid_mux_check_data_flag invalid target device flag[%X]", 1, flag);
        return TARGET_INVALID_DEVICE;
}
uint8_t hid_mux_get_data_flag(void)
{
    return g_mux_hid_data_flag;

}
extern bt_status_t bt_hid_send_handshake(uint32_t handle, uint8_t error_code);

void mux_hid_bt_callback(uint8_t channel_type, uint32_t event, void *parameter, uint32_t channel)
{
    LOG_MSGID_I(mux_hid, "mux_hid_callback channel_type[%x] event[%x] hid_report_id[%x]", 2, channel_type, event);
    virtual_read_write_point_t *p;
    uint8_t port_index;
    if (HID_CONRTROL_CHANNEL_TYPE == channel_type) {
         p = (virtual_read_write_point_t *)&g_mux_hid_r_w_point[0];
        port_index = 0;
    } else {
         p = (virtual_read_write_point_t *)&g_mux_hid_r_w_point[1];
        port_index = 1;
    }
    uint32_t per_cpu_irq_mask;
    uint32_t next_available_block_len, next_free_block_len;
     bt_hid_get_report_ind_t *get_req;
     bt_hid_set_report_ind_t *set_req;
    switch(event) {
        case BT_HID_GET_REPORT_IND:
            /* 07 case*/
#if 0
            if (p->tx_send_is_running != MUX_DEVICE_HW_RUNNING) {
                LOG_MSGID_I(mux_hid, "MUX Device HW is running", 0);
                return;
            }
#endif
            port_mux_cross_local_enter_critical(&per_cpu_irq_mask);
            next_available_block_len = mux_common_device_get_buf_next_available_block_len(p->tx_buff_start, p->tx_buff_read_point, p->tx_buff_write_point, p->tx_buff_end, p->tx_buff_available_len);                        
            port_mux_cross_local_exit_critical(per_cpu_irq_mask);
            get_req = (bt_hid_get_report_ind_t *)parameter;
            if (next_available_block_len > BT_HID_REPORT_DATA_LENGTH) {
                next_available_block_len = BT_HID_REPORT_DATA_LENGTH;
            }
            bt_hid_get_report_response_t response;
            memset(&response, 0, sizeof(bt_hid_get_report_response_t));
            response.report_len = BT_HID_REPORT_DATA_LENGTH + 1;         //race cmd 59 byte + device flag + length
            response.flag = 1;
            response.report_id = get_req->report_id;
            response.type = get_req->type;
            /* Get target device byte */
            response.report_data[0] = next_available_block_len;
            response.report_data[1] = hid_mux_get_data_flag();
            memcpy(&response.report_data[2], (uint32_t *) p->tx_buff_read_point, next_available_block_len);
            bt_hid_send_get_report_response(channel, &response);
            /* transfer done , then update tx_buff_read_point */
            port_mux_cross_local_enter_critical(&per_cpu_irq_mask);
            mux_common_device_set_tx_hw_rptr_internal_use((virtual_read_write_point_t *)&g_mux_hid_r_w_point[port_index], next_available_block_len);
            port_mux_cross_local_exit_critical(per_cpu_irq_mask);
            g_mux_hid_callback((port_index + MUX_HID_BEGIN), MUX_EVENT_READY_TO_WRITE, NULL);
            break;
        case BT_HID_SET_REPORT_IND:
            /* 06 case*/
            set_req = (bt_hid_set_report_ind_t *)parameter;
            hid_mux_set_data_flag(set_req->report_data[0]);
            next_free_block_len = mux_common_device_get_buf_next_free_block_len(p->rx_buff_start, p->rx_buff_read_point, p->rx_buff_write_point, p->rx_buff_end, p->rx_buff_available_len);            
            LOG_MSGID_I(mux_hid, "BT_HID_SET_REPORT_IND - available len = 0x%X next free block len = 0x%X report data len = 0x%X", 3,
                        p->rx_buff_available_len, next_free_block_len, set_req->report_len);
            /* The MUX-RX ring buffer is enough to copy data from HID-RX  */
            if (next_free_block_len >= set_req->report_len) {
                next_free_block_len = set_req->report_len;
                memcpy((void *)(p->rx_buff_write_point), (set_req->report_data + 1), next_free_block_len);
                mux_common_device_set_rx_hw_wptr_internal_use((virtual_read_write_point_t *)&g_mux_hid_r_w_point[port_index], next_free_block_len);
            }
            /* The MUX-RX ring buffer is NOT enough to copy data from HID-RX  */
             else {
                    memcpy((void *)(p->rx_buff_write_point), set_req->report_data + 1, next_free_block_len);
                    memcpy((void *)(p->rx_buff_start), (set_req->report_data + 1) + next_free_block_len, set_req->report_len - next_free_block_len);
                    mux_common_device_set_rx_hw_wptr_internal_use((virtual_read_write_point_t *)&g_mux_hid_r_w_point[port_index], set_req->report_len);
                }
            g_mux_hid_callback((port_index + MUX_HID_BEGIN), MUX_EVENT_READY_TO_READ, NULL);
            vPortFree(set_req->report_data);
            bt_hid_send_handshake(channel,BT_HID_SUCCESS);
            break;
        case BT_HID_DISCONNECT_IND:
            /*BT HID DISCONNECT*/            
            g_mux_hid_callback((port_index + MUX_HID_BEGIN), MUX_EVENT_DISCONNECTION, NULL);
            break;
    }
}

mux_status_t port_mux_hid_normal_init(uint8_t port_index, mux_port_config_t *p_setting, mux_irq_handler_t irq_handler)
{
    LOG_MSGID_I(mux_hid, "port_mux_hid_init", 0);
    mux_common_device_r_w_point_init((virtual_read_write_point_t *)&g_mux_hid_r_w_point[port_index], p_setting);
    g_mux_hid_callback = irq_handler;
    g_mux_bt_hid_callback = mux_hid_bt_callback;
    if (x_mux_hid_Semaphore[port_index] == NULL) {
        x_mux_hid_Semaphore[port_index] = xSemaphoreCreateMutex();
        configASSERT(x_mux_hid_Semaphore[port_index] != NULL);
    }
    mux_driver_debug_for_check(&g_mux_hid_r_w_point[port_index]);
    return MUX_STATUS_OK;
}

mux_status_t port_mux_hid_deinit(uint8_t port_index)
{
    LOG_MSGID_I(mux_hid, "port_mux_hid_deinit", 0);
    mux_driver_debug_for_check(&g_mux_hid_r_w_point[port_index]);
    //status = serial_port_close(serial_port_if_handle[port_index]);
    return MUX_STATUS_OK;
}

void port_mux_hid_exception_init(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
}
void port_mux_hid_exception_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    //TODO: need bt replace
    //maybe exception bt disconnect ???
    //bt_mux_dump_data(port_index, buffer, size);
}


bool port_mux_hid_buf_is_full(uint8_t port_index, bool is_rx)
{
    return mux_common_device_buf_is_full(&g_mux_hid_r_w_point[port_index], is_rx);
}

uint32_t port_mux_hid_get_hw_rptr(uint8_t port_index, bool is_rx)
{
    return mux_common_device_get_hw_rptr(&g_mux_hid_r_w_point[port_index], is_rx);
}


void port_mux_hid_set_rx_hw_rptr(uint8_t port_index, uint32_t move_bytes)
{
    mux_common_device_set_rx_hw_rptr(&g_mux_hid_r_w_point[port_index], move_bytes);
}

uint32_t port_mux_hid_get_hw_wptr(uint8_t port_index, bool is_rx)
{
    return mux_common_device_get_hw_wptr(&g_mux_hid_r_w_point[port_index], is_rx);
}

void port_mux_hid_set_tx_hw_wptr(uint8_t port_index, uint32_t move_bytes)
{
    virtual_read_write_point_t *p = &g_mux_hid_r_w_point[port_index];

    mux_common_device_set_tx_hw_wptr(p, move_bytes);
}

mux_status_t port_mux_hid_phase1_send(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
    return MUX_STATUS_OK;
}
#if 0
void bt_hid_mux_send_data(uint8_t port_index, uint8_t *packet, uint32_t length, volatile uint32_t *sending_point)
{
    bt_hid_data_t value;
    value.packet = packet;
    value.type = BT_HID_INPUT;
    value.packet_len = length;
    bt_hid_send_data(hid_cntx[0].handle, value);
    bt_hid_send_get_report_response()
    return;
}
#endif
mux_status_t port_mux_hid_phase2_send(uint8_t port_index)
{
#if 0
    uint32_t per_cpu_irq_mask;
    uint32_t send_addr;
    virtual_read_write_point_t *p = &g_mux_hid_r_w_point[port_index];
    uint16_t tx_complete_size = 0;
    uint16_t tx_prepare_size = 0;
    port_mux_local_cpu_enter_critical(&per_cpu_irq_mask);
    mux_driver_debug_for_check(p);
    port_mux_local_cpu_exit_critical(per_cpu_irq_mask);
    g_mux_hid_ponit_init[port_index] = false;
    if (HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) { //xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED
        //LOG_MSGID_I(MUX_BT,"Task thread ", 0);
        /* Task context */
        xSemaphoreTake(x_mux_hid_Semaphore, portMAX_DELAY);
        g_mux_hid_phase2_send_status = MUX_DEVICE_HW_RUNNING;
        p->tx_send_is_running = MUX_DEVICE_HW_RUNNING;

        LOG_MSGID_I(mux_hid, "[MUX HID] point:port_index = %d, r_point = %02x, w_point = %02x, s_point = %02x, e_point = %02x, tx_available_len", 6,
                    port_index, p->tx_buff_read_point, p->tx_buff_write_point, p->tx_buff_start, p->tx_buff_end, p->tx_buff_available_len);
        if (p->tx_buff_read_point <= p->tx_buff_write_point) {
            tx_prepare_size = p->tx_buff_write_point - p->tx_buff_read_point;
            uint8_t *data_point = (uint8_t *)(p->tx_buff_read_point);
            tx_complete_size = bt_hid_mux_send_data(port_index, data_point, p->tx_buff_write_point - p->tx_buff_read_point, &p->tx_sending_read_point);// user must update Read_point equle Write_point in this function
            LOG_MSGID_I(mux_hid, "xSemaphoreGive OK tx complete size = %d,tx_prepare_size = %d\r\n", 2, tx_complete_size, tx_prepare_size);
        } else {
            uint8_t *first_data = (uint8_t *)(p->tx_buff_read_point);
            uint16_t first_data_length = p->tx_buff_end - p->tx_buff_read_point;
            uint8_t *second_data = (uint8_t *)(p->tx_buff_start);
            uint16_t second_data_length = p->tx_buff_write_point - p->tx_buff_start;
            uint8_t *full_buffer = (uint8_t *)pvPortMalloc(first_data_length + second_data_length);
            tx_prepare_size = first_data_length + second_data_length;
            if (full_buffer == NULL) {
                LOG_MSGID_I(mux_hid, "[MUX HID] alloc full buffer fail ", 0);
                p->tx_send_is_running = MUX_DEVICE_HW_IDLE;
                g_mux_hid_phase2_send_status = MUX_DEVICE_HW_IDLE;
                xSemaphoreGive(x_mux_bt_Semaphore);
                return;
            }
            /* copy data from ring buffer. */
            memcpy(full_buffer, first_data, first_data_length);
            memcpy(full_buffer + first_data_length, second_data, second_data_length);
            tx_complete_size = bt_hid_mux_send_data(port_index, full_buffer, first_data_length + second_data_length, &p->tx_sending_read_point);
            vPortFree(full_buffer);
            LOG_MSGID_I(mux_hid, "Joining together xSemaphoreGive OK tx complete size = %d,tx_prepare_size = %d\r\n", 2, tx_complete_size, tx_prepare_size);
        }
        p->tx_send_is_running = MUX_DEVICE_HW_IDLE;
        g_mux_hid_phase2_send_status = MUX_DEVICE_HW_IDLE;
        xSemaphoreGive(x_mux_bt_Semaphore);
    } else {
        /* IRQ context */
        port_mux_local_cpu_enter_critical(&per_cpu_irq_mask);
        if (g_mux_hid_phase2_send_status == MUX_DEVICE_HW_IDLE) {
            g_mux_hid_phase2_send_status = MUX_DEVICE_HW_RUNNING;
            p->tx_send_is_running = MUX_DEVICE_HW_RUNNING;
            port_mux_local_cpu_exit_critical(per_cpu_irq_mask);

            mux_common_device_get_buf_next_available_block_len(p->tx_buff_start, p->tx_buff_read_point, p->tx_buff_write_point, p->tx_buff_end, p->tx_buff_available_len);
            send_addr = p->tx_buff_read_point;
            bt_hid_mux_send_data(port_index, (uint8_t *)send_addr, send_addr, &p->tx_sending_read_point);// user must update Read_point equle Write_point in this function

            p->tx_send_is_running = MUX_DEVICE_HW_IDLE;
            g_mux_hid_phase2_send_status = MUX_DEVICE_HW_IDLE;
        } else {
            port_mux_local_cpu_exit_critical(per_cpu_irq_mask);
            if (p->tx_buff_available_len != 0) {
                hal_gpt_status_t gpt_status = hal_gpt_sw_start_timer_ms ( g_bt_gpt_handler,
                                              BT_GPT_TIMEOUT,
                                              bt_gpt_callback,
                                              (uint8_t *)&port_index); // delay 1ms retry...
                if (gpt_status != HAL_GPT_STATUS_OK) {
                    LOG_MSGID_I(mux_hid, "[MUX HID] gpt start timer fail,status = %d", 1, gpt_status);
                }
            }
            return ;
        }
    }
#endif
   return MUX_STATUS_OK;
}

void hid_mux_clear_rx_buf(uint8_t port_index)
{
    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_hid_r_w_point[port_index];

    p->rx_buff_read_point =  p->rx_buff_write_point;
    p->rx_buff_available_len = 0;
    return;
}

mux_status_t port_mux_hid_control(uint8_t port_index, mux_ctrl_cmd_t command, mux_ctrl_para_t *para)
{
    switch (command) {
        case MUX_CMD_CLEAN: {
            hid_mux_clear_rx_buf(port_index);
            return MUX_STATUS_OK;
        }
        default:  {
        } break;
    }
    return MUX_STATUS_ERROR;

}

port_mux_device_ops_t g_port_mux_hid_ops = {
#ifdef MTK_CPU_NUMBER_0
    port_mux_hid_normal_init,
    port_mux_hid_deinit,
    port_mux_hid_exception_init,
    port_mux_hid_exception_send,
    port_mux_hid_buf_is_full,
#endif
    port_mux_hid_get_hw_rptr,
    port_mux_hid_set_rx_hw_rptr,
    port_mux_hid_get_hw_wptr,
    port_mux_hid_set_tx_hw_wptr,
    port_mux_hid_phase1_send,
    port_mux_hid_phase2_send,
    port_mux_hid_control,
    NULL,
    NULL,
};


#endif //AIR_MUX_BT_HID_ENABLE

/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#ifndef __MUX_LL_UART_REAL_TIME_H__
#define __MUX_LL_UART_REAL_TIME_H__

//#define MUX_LL_UART_REAL_TIME_MEASUREMENT
//#define MUX_LL_UART_IRQ_MIPS_MEASUREMENT

typedef enum{
    LLUART_STA_NOT_START,
    LLUART_STA_RUNNING,
    LLUART_STA_EMPTY,
}lluart_busy_rate_status_t;

typedef enum{
    LLUART_ACTION_TX_LIMTER,
    LLUART_ACTION_DMA_EMPTY_IRQ,
    LLUART_ACTION_STOP,
}lluart_busy_rate_action_t;

typedef void (*cb_t)(void);

typedef enum{
    LLUART_BUSY_RATE_IDLE_TIME_IDX,
    LLUART_BUSY_RATE_BUSY_TIME_IDX,
}lluart_busy_rate_time_idx_t;

typedef struct{
    lluart_busy_rate_status_t sta;
    lluart_busy_rate_time_idx_t time_idx;
} lluart_busy_rate_table_note_t;


// typedef struct{
//     lluart_busy_rate_status_t sta;
//     uint32_t* p_time;
// }lluart_busy_rate_table_note_t;


void lluart_busy_time_measurement_action(lluart_busy_rate_action_t action);
void uart_busy_rate_timer_cb(void *user_data);
void uart_busy_rate_timer_start(void);

void cpu_mips_measurement_start(void);
void cpu_mips_measurement_end(bool is_tx);
void cpu_mips_measurement_timer_cb(void *user_data);
void cpu_mips_measurement_timer_start(void);

#endif

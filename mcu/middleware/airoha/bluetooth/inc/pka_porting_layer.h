
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

#ifndef __BT_CON_PORTING_H__
#define __BT_CON_PORTING_H__

#include <stdint.h>
#include <stdarg.h>

#define ATTR_LOG_STRING      __attribute__ ((__section__(".log_string"))) static const char




typedef enum {
    LOG_DM2L,
    LOG_LC,
    LOG_COMMON,
    LOG_NUM
} pka_log_type_t;

typedef enum {

    PKA_LOG_LEVEL_DEBUG,
    PKA_LOG_LEVEL_INFO,
    PKA_LOG_LEVEL_WARN,
    PKA_LOG_LEVEL_ERROR
} pka_log_level_t;


typedef enum {
    PKA_DCXO_LP_MODE     = 0,//DCXO_LP_MODE = 0,
    PKA_DCXO_NORMAL_MODE = 1,//DCXO_NORMAL_MODE = 1,

} pka_dcxo_mode_t;

void bt_pka_log_msgid_i(pka_log_type_t type, const char *msg, uint32_t arg_cnt, ...);
void bt_pka_log_msgid_d(pka_log_type_t type, const char *msg, uint32_t arg_cnt, ...);
void bt_pka_log_msgid_w(pka_log_type_t type, const char *msg, uint32_t arg_cnt, ...);
void bt_pka_log_msgid_e(pka_log_type_t type, const char *msg, uint32_t arg_cnt, ...);
void bt_pka_dump_msgid_i(pka_log_type_t type, const void *data, uint32_t length, const char *message, ...);
void bt_pka_log_msgid_va(pka_log_type_t type, pka_log_level_t level, const char *message, uint32_t arg_cnt, va_list ap);


#if (FEA_COSIM_SUPPORTED)
extern void MDM_TM_PRINTF(const char *fmt,...);
#define LOG_MSG_I(log_type, msg, argc, ...)    \
do{ \
    MDM_TM_PRINTF(msg, ## __VA_ARGS__); \
}while(0)
#else
#define LOG_MSG_I(log_type, msg, argc, ...)    \
do{ \
    ATTR_LOG_STRING msg_id_string[] = "[M:PKA_"#log_type" C:info F: L: ]: "msg;\
    bt_pka_log_msgid_i(log_type, msg_id_string, argc, ## __VA_ARGS__); \
}while(0)
#endif

#define LOG_MSG_D(log_type, msg, argc, ...)    \
do{ \
    ATTR_LOG_STRING msg_id_string[] = "[M:PKA_"#log_type" C:debug F: L: ]: "msg;\
    bt_pka_log_msgid_d(log_type, msg_id_string, argc, ## __VA_ARGS__); \
}while(0)

#define LOG_MSG_W(log_type, msg, argc, ...)    \
do{ \
    ATTR_LOG_STRING msg_id_string[] = "[M:PKA_"#log_type" C:warning F: L: ]: "msg;\
    bt_pka_log_msgid_w(log_type, msg_id_string, argc, ## __VA_ARGS__); \
}while(0)

#define LOG_MSG_E(log_type, msg, argc, ...)    \
do{ \
    ATTR_LOG_STRING msg_id_string[] = "[M:PKA_"#log_type" C:error F: L: ]: "msg;\
    bt_pka_log_msgid_e(log_type, msg_id_string, argc, ## __VA_ARGS__); \
}while(0)
#define DUMP_MSG_I(log_type, data, length, msg, ...)    \
do{ \
    bt_pka_dump_msgid_i(log_type, data, length, msg, ## __VA_ARGS__); \
}while(0)
typedef enum {
    xNoAction = 0,              /* Notify the task without updating its notify value. */
    xSetBits,                   /* Set bits in the task's notification value. */
    xIncrement,                 /* Increment the task's notification value. */
    xSetValueWithOverwrite,     /* Set the task's notification value to a specific value even if the previous value has not yet been read by the task. */
    xSetValueWithoutOverwrite   /* Set the task's notification value if the previous value has been read by the task. */
} xNotifyAction;

typedef void (*pTimerCallBack)(void *ExpiredTimer);
typedef void (*pTaskFun)(void);
typedef void (*bt_isr_t)(uint32_t irq_number);

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]
#define COMPILE_TIME_ASSERT4(X,MSG,L) STATIC_ASSERT(X,MSG##L)
#define COMPILE_TIME_ASSERT3(X,MSG,L) COMPILE_TIME_ASSERT4(X,MSG,_at_line_##L)
#define COMPILE_TIME_ASSERT2(X,MSG,L) COMPILE_TIME_ASSERT3(X,MSG,L)
#define COMPILE_TIME_ASSERT(X,MSG)    COMPILE_TIME_ASSERT2(X,MSG,__LINE__)

ATTR_TEXT_IN_TCM void pka_dcxo_mode(uint8_t mode);

char *pka_os_get_task_name(void);

void pka_os_task_resume(void *taskHandle);

uint32_t  pka_os_task_resume_from_isr(void *taskHandle);

void *pka_os_get_timer_id(void *xTimer);

void pka_os_cancel_timer(void *xTimer);

void* pka_os_create_timer( const char * const pcTimerName, const uint32_t  xTimerPeriodInTicks, const uint32_t  uxAutoReload, void * const pvTimerID, void* pxCallbackFunction );

void pka_os_start_timer(void *xTimer);

void pka_os_task_suspend(void *taskHandle);

void pka_os_task_notify_wait(uint32_t  ulBitsToClearOnEntry, uint32_t  ulBitsToClearOnExit, uint32_t  *pulNotificationValue);

void pka_os_task_notify_from_isr( void* xTaskToNotify, uint32_t  ulValue, xNotifyAction eAction, unsigned int *pxHigherPriorityTaskWoken );

void pka_os_task_notify(void *xTaskToNotify, uint32_t  ulValue, xNotifyAction eAction);

void *pka_os_semaphore_init();

void pka_os_semaphore_take(void *pSemaphore);

void pka_os_semaphore_give(void *pSemaphore);

//void pka_os_task_create(TaskFunction_t fTaskEntry, const char * const taskName, uint32_t  stackSize, void * const pParameters, uint8_t  taskPriority, void* taskHandle);

uint32_t  pka_os_get_interrupt_mask(void);

void pka_os_restore_interrupt_mask(uint32_t  nvic_mask);

uint32_t pka_gpt_get_current_time(void);

void pka_hal_gpt_get_duration_count(uint32_t start_count, uint32_t end_count, uint32_t *duration_count);

uint32_t pka_hal_gpt_get_free_run_count_32K(void);

uint32_t pka_hal_gpt_get_free_run_count_1M(void);

uint32_t pka_hal_gpt_get_free_run_count(uint8_t gpt_type);

void pka_os_register_bt_interrupt(bt_isr_t intr_handler);

void pka_os_register_bt_timer_interrupt(void* intr_handler);

void pka_os_enable_bt_and_timer_interrupt();

void pka_os_disable_bt_and_timer_interrupt();

void pka_os_yield_from_isr(void);

uint32_t pka_os_is_run_in_isr(void);


uint32_t pka_os_get_lm_task_priority(void);

uint32_t pka_os_get_lc_task_priority(void);

uint32_t pka_os_get_lc_process_task_priority(void);


void pka_disable_all_irq_except_bt(void);


void pka_restore_all_irq_except_bt(void);

/* big endian input, output */
void pka_uECC_p192_compute_public_key(uint8_t *privatekey, uint8_t *publicKey);

void pka_uECC_p192_shared_secret(uint8_t *remotePublicKey, uint8_t *privatekey, uint8_t *DHKey);

void pka_uECC_p256_compute_public_key(uint8_t *privatekey, uint8_t *publicKey);

void pka_uECC_p256_shared_secret(uint8_t *remotePublicKey, uint8_t *privatekey, uint8_t *DHKey);

int8_t pka_hal_gpt_delay_us(uint32_t us);

void pka_external_hmac_sha256(uint8_t *input, uint8_t ilen, uint8_t *key, uint8_t keylen, uint8_t *output);

void pka_external_sha256(uint8_t *input, uint8_t ilen, uint8_t *output);

void pka_dvfs_lock_control_SpeedUpTo208M(uint8_t lock);

void pka_dvfs_lock_control_SpeedUpTo104M(uint8_t lock);

uint32_t pka_dvfs_get_cpu_frequency(void);

uint8_t pka_Get_EDR_Security_Connection_Enable(uint8_t Address[6], uint8_t Cod[3], uint8_t LMP_Version);

ATTR_TEXT_IN_TCM void pka_hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);

ATTR_TEXT_IN_TCM void pka_hal_nvic_restore_interrupt_mask_special(uint32_t mask);

#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
void pka_hal_flash_otp_read(uint32_t start_address, uint8_t *buffer, uint32_t length);
void pka_set_vref_dcxo_sel(uint32_t value);
void pka_set_vref_v2i_sel(uint32_t value);
uint8_t pka_get_ab1577_version(void);
uint8_t pka_free_sche_enable_spec_scan(uint8_t Enable);
void pka_free_sche_set_pec_para(uint32_t HighBound,uint32_t LowBound,uint32_t Timer);
void pka_hid_data_forward_callback_register(void* fHidMsCB);
uint32_t pka_return_rcvd_pkt_time(void);
uint32_t pka_return_call_cb_time(void);
uint32_t pka_return_hdl_intr_time(void);
void pka_hal_clock_mux_sel_157x(void);
#endif

uint32_t pka_Send_LEAudio_PlayInfo_ToDSP(uint32_t Address);

void pka_rtc_switch_to_dcxo(uint8_t enable);

void pka_hal_bt_clock_enable(void);

void pka_hal_clock_mux_sel(void);

uint8_t pka_is_dcxo_normal_mode(void);

uint8_t pka_get_dcxo_26m_fpm(void);

uint8_t pka_get_dcxo_26m_en(void);

#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
uint8_t pka_ice_debug_is_enabled(void);
#endif

uint8_t pka_get_sleep_handle_idx(uint8_t isA2dp);

#endif


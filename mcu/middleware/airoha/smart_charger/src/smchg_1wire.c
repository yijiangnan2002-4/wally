/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
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

#include "FreeRTOS.h"
#include "timers.h"
#include "mux.h"
#include "smartcharger.h"
#include "hal_gpio.h"
#include "hal_uart_internal.h"
#include "hal_eint.h"
#include "hal_gpio.h"
#include "hal_log.h"
#include "chargersmartcase.h"
#include "battery_management.h"
#include "hal.h"
#include "hal_dvfs_internal.h"
#include "hal_sleep_manager_platform.h"
#include "battery_management_core.h"
#include "battery_management.h"
#include "memory_attribute.h"
#include "bt_sink_srv_ami.h"
#include "serial_port_assignment.h"
#include "serial_port.h"
#include "race_xport.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "hal_resource_assignment.h"
#include "hal_pmu.h"

#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
#if defined (AIR_BTA_PMIC_HP)
#include "hal_pmu_internal_hp.h"
#include "hal_pmu_charger_hp.h"
#elif defined(AIR_BTA_PMIC_LP)
#include "hal_pmu_internal_lp.h"
#include "hal_pmu_charger_lp.h"
#endif
#endif

#if defined(AIR_BTA_IC_PREMIUM_G2)
#if defined (AIR_BTA_PMIC_G2_HP)
#elif defined(AIR_BTA_PMIC_G2_LP)
#include "hal_pmu_charger_2565.h"
#include "hal_pmu_internal_2565.h"
#endif
#endif
#include "smchg_1wire_config.h"
#include "smchg_1wire.h"
#include "ept_gpio_var.h"




#if defined(AIR_1WIRE_ENABLE)

/* create mux debug module */
log_create_module(SM_CHG, PRINT_LEVEL_INFO);

#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
#if defined (AIR_BTA_PMIC_HP)
#define HP_1WIRE_G3
#elif defined(AIR_BTA_PMIC_LP)
#define LP_1WIRE_G3
#endif
#elif defined(AIR_BTA_IC_PREMIUM_G2)
#if defined (AIR_BTA_PMIC_G2_HP)
#define HP_1WIRE_G2
#elif defined(AIR_BTA_PMIC_G2_LP)
#define LP_1WIRE_G2
#endif
#else
#endif


#define RACE_CH                 0
#define RACE_TYPE               1
#define LENGTH_L                2
#define LENGTH_H                3
#define CMD_ID                  6
#define EARBUD_ID               7
#define DATA                    8
#define BAT_LV                  8
#define BAUDRATE                8
#define KEY_ID                  8

#define CASE_LID_OPEN           0x2
#define CASE_LID_CLOSE_DONE     0x3
#define CASE_CHARGER_OFF        0x4
#define CASE_CHARGER_KEY        0x5
#define CASE_BATTER_LEVEL       0x6
#define CASE_LID_CLOSE          0x8
#define CASE_LOG_ENABLE         0xE
#define CASE_RACE_ENABLE        0xF
// richard for UI spec
#define CASE_REVERSION		0x9

#define SMCHG_MUX_RX_DATA_SIZE  32
#define SMCHG_MUX_TX_BUFF_SIZE  1024
#define SMCHG_MUX_RX_BUFF_SIZE  2048
#define SMCHG_MUX_RX_HANDLE_TIMER_MS (1)
#define SMCHG_MUX_TX_QUERY_MAX_TIME (1500)


#define SMCHG_MUX_PARSE_ERROR_FETCH_RACE_CH             (0x00000001)
#define SMCHG_MUX_PARSE_ERROR_FETCH_CH_TYPE             (0x00000002)
#define SMCHG_MUX_PARSE_ERROR_CH_TYPE                   (0x00000004)
#define SMCHG_MUX_PARSE_ERROR_FETCH_LENGTH              (0x00000008)
#define SMCHG_MUX_PARSE_ERROR_FETCH_RACE_ID             (0x00000010)
#define SMCHG_MUX_PARSE_ERROR_RACE_CH                   (0x00000020)
#define SMCHG_MUX_PARSE_ERROR_NO_RACE_HANDLE            (0x00000040)
#define SMCHG_MUX_PARSE_ERROR_NO_SMCHG_HANDLE           (0x00000080)
#define SMCHG_MUX_PARSE_ERROR_NO_ATCI_HANDLE            (0x00000100)
#define SMCHG_MUX_PARSE_ERROR_UNKNOWN_1WIRE_STATUS      (0x00000200)
#define SMCHG_MUX_PARSE_ERROR_COM_MODE_STATUS_NOT_MATCH (0x00000400)
#define SMCHG_MUX_PARSE_ERROR_NO_HCI_HANDLE             (0x00000800)

#define SMCHG_RX_PKT_MAX_SIZE (5)
#define SMCHG_OUT_OF_CASE_VOL_THRESHOLD_MV (1300)

#define SMCHG_TMR_START_STATUS_OUT_OF_CASE_TMR_MASK (0x00000001) /* pSmartChargerOutTimer */
#define SMCHG_TMR_START_STATUS_RX_TMR_MASK          (0x00000002) /* pSmartChargerRxTimer */
#define SMCHG_TMR_START_STATUS_CHG_IN_TMR_MASK      (0x00000004) /* pSmchgChgInTmr  */
#define SMCHG_TMR_START_STATUS_CHG_OUT_TMR_MASK     (0x00000008) /* pSmchgChgOutTmr */

const uint8_t smchg_app_table[] = {
    0,
    0,
    SMCHG_LID_OPEN,
    SMCHG_LID_CLOSE_DONE,
    SMCHG_CHG_OFF,
    SMCHG_CHG_KEY,
    0,
    0,
    SMCHG_LID_CLOSE
};

static uint8_t raceEvt[] = {
    0x05,      //race header
    0x5B,      //race type
    0x05,      //lengthL
    0x00,      //lengthH
    0x00,      //cmd type
    0x20,      //cmd tpye
    0x00,      //event
    0x00,      //R or L
    0x00,      //parameter
};


typedef struct {
    mux_buffer_t *mux_buffer;
    uint32_t offset;
    uint32_t idx;
    uint32_t counter;
    uint32_t left_data_len;
    uint32_t total_length;
} smchg_mux_buffer_t;


typedef enum {
  SMCHG_1WIRE_STATE_INIT,
  SMCHG_1WIRE_STATE_COM_IDLE,
  SMCHG_1WIRE_STATE_COM_WAIT_TX_DONE,
  SMCHG_1WIRE_STATE_MAX
} smchg_1wire_state_t;


typedef enum {
  SMCHG_1WIRE_MUX_CFG_NONE                = 0x00000000,
  SMCHG_1WIRE_MUX_CFG_CHANGE_UART_PARAM   = 0x00000001,
  SMCHG_1WIRE_MUX_CFG_CHANGE_UART_TX_INT  = 0x00000002,
  SMCHG_1WIRE_MUX_CFG_UART_TX_ENABLE      = 0x00000004,
  SMCHG_1WIRE_MUX_CFG_UART_RX_ENABLE      = 0x00000008,
  SMCHG_1WIRE_MUX_CFG_UART_TX_RX_DISABLE  = 0x00000010
} smchg_1wire_mux_cfg_cmd_t;

typedef struct {
    mux_port_t                  port;
    smchg_1wire_mux_cfg_cmd_t   cfg_cmd;
    mux_ctrl_para_t             mux_ctrl_para;
} smchg_1wire_mux_ctrl_req_t;


typedef struct {
    hal_uart_baudrate_t race_baudrate;
} smchg_1wire_race_mode_info_t;


typedef struct {
    uint32_t rx_data_len;
    void *p_rx_data;
}smchg_rx_pkt_info_t;


typedef struct {

    smchg_1wire_state_t           state;
    mux_port_t                    mux_port;
    mux_handle_t                  mux_handle;
    uint16_t                      cur_cmd_id;
    void                          *cur_cmd;
    smchg_1wire_race_mode_info_t race_mode_info;
} smchg_handle_t;

smchg_1wire_gpio_t smchg_1wire_gpio_cfg;
TimerHandle_t pSmartChargerOutTimer = NULL;
TimerHandle_t pSmartChargerRxTimer  = NULL;
TimerHandle_t pSmchgChgInTmr        = NULL;
TimerHandle_t pSmchgChgOutTmr       = NULL;


uint8_t pre_cmd = 0;
uint32_t pre_data = 0;
uint16_t pre_data_len = 0;
uint8_t last_cmd = 0;
uint8_t boot_Flag = 0;
uint8_t lock_sleep_flag = 0;
uint8_t race_mode_flag = 0;
uint8_t g_chk_out_case_cnt_thrd = 3;
uint32_t g_rx_data_len = 0;
void    *g_rx_user_data = NULL;
uint32_t    g_rxcb_w_cnt = 0, g_rxcb_r_cnt = 0;
smchg_rx_pkt_info_t g_rx_data[SMCHG_RX_PKT_MAX_SIZE];
uint32_t g_rx_handle_lock = 0;
static uint32_t g_com_mode_chk_cnt = 0;
static uint32_t g_tmr_start_status = 0;
smchg_handle_t g_smchg;
static volatile smchg_1wire_mode_t *p_1wire_mode = (volatile smchg_1wire_mode_t *)HW_SYSRAM_PRIVATE_MEMORY_1WIRE_START;



#define SMCHG_1WIRE_UART_RX_ENABLE_CMD           (SMCHG_1WIRE_MUX_CFG_CHANGE_UART_PARAM | SMCHG_1WIRE_MUX_CFG_CHANGE_UART_TX_INT | SMCHG_1WIRE_MUX_CFG_UART_RX_ENABLE)
#define SMCHG_1WIRE_UART_TX_ENABLE_CMD           (SMCHG_1WIRE_MUX_CFG_CHANGE_UART_PARAM | SMCHG_1WIRE_MUX_CFG_CHANGE_UART_TX_INT | SMCHG_1WIRE_MUX_CFG_UART_TX_ENABLE)
#define SMCHG_1WIRE_UART_TRX_DISABLE_CMD         (SMCHG_1WIRE_MUX_CFG_CHANGE_UART_TX_INT | SMCHG_1WIRE_MUX_CFG_UART_TX_RX_DISABLE)
uint32_t smchg_mux_ctrl_cmd [] = {
    SMCHG_1WIRE_MUX_CFG_NONE,                    /* SMCHG_1WIRE_NORM */
    SMCHG_1WIRE_UART_TRX_DISABLE_CMD,    /* SMCHG_1WIRE_OUT */
    SMCHG_1WIRE_UART_TX_ENABLE_CMD,      /* SMCHG_1WIRE_LOG */
    SMCHG_1WIRE_UART_TRX_DISABLE_CMD,    /* SMCHG_1WIRE_CHG */
    SMCHG_1WIRE_UART_RX_ENABLE_CMD,      /* SMCHG_1WIRE_COM */
    SMCHG_1WIRE_UART_RX_ENABLE_CMD,      /* SMCHG_1WIRE_RACE */
    SMCHG_1WIRE_UART_RX_ENABLE_CMD,      /* SMCHG_1WIRE_ATCI */
    SMCHG_1WIRE_MUX_CFG_NONE
};

static void smchg_1wire_rx_handle(uint32_t user_data_len, void *user_data);
static smchg_status_t smchg_1wire_tx_post_handle(uint32_t user_data_len, void *user_data);
static smchg_status_t smchg_1wire_give_lock(uint32_t *p_lock);
static smchg_status_t smchg_1wire_take_lock(uint32_t *p_lock);
static smchg_status_t smchg_1wire_mux_control(smchg_handle_t *p_sm_handle, void *p_cfg);


/* sleep lock control */
void smchg_1wire_lock_sleep(void)
{
    SMCHG_LOG_MSGID_D("1wire, lock_sleep, flag[%d]", 1, lock_sleep_flag);

    if (!lock_sleep_flag) {
        hal_sleep_manager_lock_sleep(SLEEP_LOCK_CHARGER_CASE);
        lock_sleep_flag = 1;
    }
}

void smchg_1wire_unlock_sleep(void)
{
    SMCHG_LOG_MSGID_D("1wire, unlock_sleep, flag[%d]", 1, lock_sleep_flag);

    if (lock_sleep_flag) {
        hal_sleep_manager_unlock_sleep(SLEEP_LOCK_CHARGER_CASE);
        lock_sleep_flag = 0;
    }
}

smchg_1wire_mode_t smchg_1wire_get_mode_status(void)
{
    return *p_1wire_mode;
}

smchg_status_t smchg_1wire_set_mode_status(smchg_1wire_mode_t mode)
{
    if (mode >= SMCHG_1WIRE_MAX) {
        return SMCHG_STATUS_INVALID_PARAMETER;
    }

    *p_1wire_mode = mode;

    return SMCHG_STATUS_OK;
}


void smchg_1wire_app_cmd(uint8_t cmd)
{
    SMCHG_LOG_MSGID_D("1wire, app_cmd[%d]", 1, cmd);
}


static serial_port_dev_t smchg_1wire_port_convert_mux_to_port_service(mux_port_t port)
{
    serial_port_dev_t ser_port = SERIAL_PORT_DEV_UNDEFINED;

    switch (port) {
        case MUX_UART_1:
            ser_port = SERIAL_PORT_DEV_UART_1;
            break;
        case MUX_UART_0:
            ser_port = SERIAL_PORT_DEV_UART_0;
            break;
        default:
            SMCHG_LOG_MSGID_E("1wire, port not support [%d]", 1, port);
            break;
    }
    return ser_port;
}

static smchg_status_t smchg_1wire_reset_smchg_state(smchg_handle_t *p_smchg)
{
    p_smchg->state       = SMCHG_1WIRE_STATE_COM_IDLE;
    p_smchg->cur_cmd_id  = 0;
    p_smchg->cur_cmd     = NULL;

    return SMCHG_STATUS_OK;
}

static smchg_status_t smchg_1wire_take_lock(uint32_t *p_lock)
{
    uint32_t irq_status;

    if (p_lock == NULL) {
        return SMCHG_STATUS_INVALID_PARAMETER;
    }

    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    if (*p_lock) {
        hal_nvic_restore_interrupt_mask(irq_status);
        return SMCHG_STATUS_ERROR;
    }
    *p_lock = 1;;
    hal_nvic_restore_interrupt_mask(irq_status);
    return SMCHG_STATUS_OK;
}

static smchg_status_t smchg_1wire_give_lock(uint32_t *p_lock)
{
    uint32_t irq_status;

    if (p_lock == NULL) {
        return SMCHG_STATUS_INVALID_PARAMETER;
    }
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    *p_lock = 0;
    hal_nvic_restore_interrupt_mask(irq_status);
    return SMCHG_STATUS_OK;
}



static smchg_status_t smchg_1wire_set_tmr_active_status(TimerHandle_t pxExpiredTimer, uint8_t activate)
{
    uint32_t irq_status;
    if (pxExpiredTimer == NULL) {
        return SMCHG_STATUS_INVALID_PARAMETER;
    }
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    if (pxExpiredTimer == pSmartChargerOutTimer) {
        if (activate) {
            g_tmr_start_status |= SMCHG_TMR_START_STATUS_OUT_OF_CASE_TMR_MASK;
        } else {
            g_tmr_start_status &= (~(SMCHG_TMR_START_STATUS_OUT_OF_CASE_TMR_MASK));
        }
    }
    hal_nvic_restore_interrupt_mask(irq_status);
    return SMCHG_STATUS_OK;
}



static uint32_t smchg_1wire_get_tmr_active_status(TimerHandle_t pxExpiredTimer)
{
    uint32_t irq_status;
    uint32_t tmr_status = 0;

    if (pxExpiredTimer == NULL) {
        return SMCHG_STATUS_INVALID_PARAMETER;
    }
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    if (pxExpiredTimer == pSmartChargerOutTimer) {
        tmr_status = g_tmr_start_status & SMCHG_TMR_START_STATUS_OUT_OF_CASE_TMR_MASK;
    }
    hal_nvic_restore_interrupt_mask(irq_status);
    if (tmr_status) {
        return 1;
    } else {
        return 0;
    }
}


bool smchg_1wire_vbat_over_thrd(uint32_t thrd)
{
#if defined(AIR_BTA_PMIC_LP) || defined(AIR_BTA_PMIC_G2_LP)
    uint32_t vbat = battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE);
    SMCHG_LOG_MSGID_D("1wire, vbat[%d]", 1, vbat);
    if (vbat > thrd) {
        return TRUE;
    } else {
        return FALSE;
    }
#elif defined(AIR_BTA_PMIC_HP)
    return TRUE;
#else
    #error "please check if it needs to check the battery state or not"
#endif
}

bool smchg_1wire_vchg_less_than_thrd(uint32_t thrd)
{
    uint32_t data;

    data = pmu_1wire_get_vbus_uart_volt();

    if (data < thrd) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int32_t smchg_1wire_chg_exist(void)
{
    return battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
}

static uint8_t smchg_1wire_com_mode_exist(void)
{
    uint8_t com_exist = 1;
    mux_status_t  mux_status = MUX_STATUS_ERROR_BUSY;
    bool is_norm_vchg = TRUE;
  
    mux_status = mux_control(g_smchg.mux_port, MUX_CMD_GET_TRX_PORT_IDLE, NULL);
    if (mux_status != MUX_STATUS_OK) {
        goto com_mode;
    }

    if (smchg_1wire_vchg_less_than_thrd(smchg_cfg.out_of_case_vbus_uart_thrd)) {
        is_norm_vchg = FALSE; // the voltage of VBUS_UART is lower than Vcom
    }

    mux_status = mux_control(g_smchg.mux_port, MUX_CMD_GET_TRX_PORT_IDLE, NULL);
    if (mux_status != MUX_STATUS_OK) {
        goto com_mode;
    }

com_mode:
    if ((mux_status != MUX_STATUS_OK) || is_norm_vchg ) {
        g_com_mode_chk_cnt = 0;
        if (mux_status != MUX_STATUS_OK) {
            SMCHG_LOG_MSGID_W("1wire, stay com mode, mux_status[%d], is_norm_vchg [%d]", 2, mux_status, is_norm_vchg);
        }
    } else {
        g_com_mode_chk_cnt++;
        SMCHG_LOG_MSGID_W("1wire, com_mod_chk, cnt[%d]", 1, g_com_mode_chk_cnt);
        if (g_com_mode_chk_cnt >= g_chk_out_case_cnt_thrd) {
            g_com_mode_chk_cnt = 0;
            com_exist = 0;
        }
    }
    return com_exist;
}

static void smchg_1wire_init_to_out_of_case(void)
{
    bool is_vbat_norm = smchg_1wire_vbat_over_thrd(smchg_cfg.com_mode_vbat_thrd);

    SMCHG_LOG_MSGID_D("1wire, init_to_out_of_case, vbat_state[%d]", 1, is_vbat_norm);
    if (is_vbat_norm) {
        pmu_1wire_cfg(PMU_1WIRE_INIT_TO_OUT_OF_CASE_NORM_BAT);
    } else {
        pmu_1wire_cfg(PMU_1WIRE_INIT_TO_OUT_OF_CASE_LOW_BAT);
    }
}

static void smchg_1wire_init_to_chg_mode(void)
{
    pmu_1wire_cfg(PMU_1WIRE_INIT_TO_CHG_IN);
}


void smchg_1wire_init_to_log_mode(void)
{
    bool is_vbat_norm = smchg_1wire_vbat_over_thrd(smchg_cfg.com_mode_vbat_thrd);
    SMCHG_LOG_MSGID_D("1wire, intr_to_log_mode, vbat_state[%d]", 1, is_vbat_norm);
    if (is_vbat_norm) {
        pmu_1wire_cfg(PMU_1WIRE_CHG_IN_TO_COM_NORM_BAT);
    } else {
        pmu_1wire_cfg(PMU_1WIRE_CHG_IN_TO_COM_LOW_BAT);
    }
}

static void smchg_1wire_intr_to_chg_mode(void)
{
    smchg_1wire_mode_t mode = smchg_1wire_get_mode_status();
    bool is_vbat_norm = smchg_1wire_vbat_over_thrd(smchg_cfg.com_mode_vbat_thrd);

    SMCHG_LOG_MSGID_I("1wire, intr_to_chg_mode, mode[%d], vbat_state[%d]", 2, mode, is_vbat_norm);
    switch (mode)
    {
        case SMCHG_1WIRE_LOG:
        case SMCHG_1WIRE_COM:
        case SMCHG_1WIRE_RACE:
        case SMCHG_1WIRE_ATCI: {
            if (is_vbat_norm) {
                pmu_1wire_cfg(PMU_1WIRE_COM_TO_CHG_IN_NORM_BAT);
            } else {
                pmu_1wire_cfg(PMU_1WIRE_COM_TO_CHG_IN_LOW_BAT);
            }
        }
        break;
        case SMCHG_1WIRE_OUT:
        case SMCHG_1WIRE_CHG: {
            if (is_vbat_norm) {
                pmu_1wire_cfg(PMU_1WIRE_OUT_OF_CASE_TO_CHG_IN_NORM_BAT);
            } else {
                pmu_1wire_cfg(PMU_1WIRE_OUT_OF_CASE_TO_CHG_IN_LOW_BAT);
            }
        }
        break;
        default:
            if (is_vbat_norm) {
                pmu_1wire_cfg(PMU_1WIRE_OUT_OF_CASE_TO_CHG_IN_NORM_BAT);
            } else {
                pmu_1wire_cfg(PMU_1WIRE_OUT_OF_CASE_TO_CHG_IN_LOW_BAT);
            }
        break;
    }
}



static void smchg_1wire_intr_to_com_mode(void)
{
    bool is_vbat_norm = smchg_1wire_vbat_over_thrd(smchg_cfg.com_mode_vbat_thrd);

    SMCHG_LOG_MSGID_D("1wire, intr_to_com_mode, vbat_state[%d]", 1, is_vbat_norm);
    if (is_vbat_norm) {
        pmu_1wire_cfg(PMU_1WIRE_CHG_IN_TO_COM_NORM_BAT);
    } else {
        pmu_1wire_cfg(PMU_1WIRE_CHG_IN_TO_COM_LOW_BAT);
    }
}

static void smchg_1wire_com_mode_to_out_of_case(void)
{
    pmu_1wire_cfg(PMU_1WIRE_COM_TO_OUT_OF_CASE);
}



static void smchg_1wire_com_mode_to_pwr_save_mode(void)
{
    pmu_1wire_cfg(PMU_1WIRE_COM_TO_PWR_SAVE);
}



void smchg_1wire_com_mode_tx_done(void)
{
    uint32_t time_s = 0, time_e = 0, time_dur = 0;
    mux_status_t  mux_status = MUX_STATUS_ERROR_BUSY;
    hal_gpt_status_t gpt_status = HAL_GPT_STATUS_ERROR;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &time_s);

    do {

        mux_status = mux_control(g_smchg.mux_port, MUX_CMD_GET_TX_PORT_IDLE, NULL);
        //SMCHG_LOG_MSGID_W("1wire, com_mode_to_race_mode, mux_status[%d]", 1, mux_status);

        gpt_status = hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &time_e);
        if (gpt_status!= HAL_GPT_STATUS_OK) {
            break;
        }

        gpt_status = hal_gpt_get_duration_count(time_s, time_e, &time_dur);
        if (gpt_status != HAL_GPT_STATUS_OK) {
            //SMCHG_LOG_MSGID_E("1wire, com_mode_tx_done, get gpt dur error, status[%d]", 1, gpt_status);
            break;
        }
    } while (mux_status != MUX_STATUS_OK);

    SMCHG_LOG_MSGID_D("1wire, com_mode_tx_done, time[%dus], gpt status[%d]", 2, time_dur, gpt_status);
}



static smchg_status_t smchg_1wire_mux_control(smchg_handle_t *p_sm_handle, void *p_cfg)
{
    smchg_1wire_mux_ctrl_req_t   *p_ctrl_req       = NULL;
    smchg_1wire_mux_cfg_cmd_t     smchg_mux_cmd    = SMCHG_1WIRE_MUX_CFG_NONE;
    mux_ctrl_para_t              *mux_cfg          = NULL;
    mux_ctrl_cmd_t                mux_cmd          = 0;
    uint32_t                      cmd_idx          = 0x1;
    bool                          is_ctrl          = TRUE;
    smchg_status_t                status           = SMCHG_STATUS_OK;
    mux_status_t                  mux_status       = MUX_STATUS_OK; 

    if (p_sm_handle == NULL || p_cfg == NULL) {
        status = SMCHG_STATUS_INVALID_PARAMETER;
        goto end;
    }
    p_ctrl_req       = (smchg_1wire_mux_ctrl_req_t *)p_cfg;
    mux_cfg          = &(p_ctrl_req->mux_ctrl_para);
    smchg_mux_cmd    = p_ctrl_req->cfg_cmd;

    SMCHG_LOG_MSGID_I("1wire, mux ctrl, mux_cmd [0x%x]", 1, smchg_mux_cmd);

    while (smchg_mux_cmd && (mux_status == MUX_STATUS_OK)) {
        SMCHG_LOG_MSGID_D("1wire, mux ctrl, mux_cmd [0x%x],cmd_idx[%d]", 2, smchg_mux_cmd, cmd_idx);
        switch (smchg_mux_cmd & cmd_idx)
        {
            case SMCHG_1WIRE_MUX_CFG_CHANGE_UART_PARAM:
                mux_cmd = MUX_CMD_CHANGE_UART_PARAM;
            break;
            case SMCHG_1WIRE_MUX_CFG_CHANGE_UART_TX_INT:
                mux_cmd = MUX_CMD_CHANGE_UART_TX_INT;
            break;
            case SMCHG_1WIRE_MUX_CFG_UART_TX_ENABLE:
                mux_cmd = MUX_CMD_UART_TX_ENABLE;
            break;
            case SMCHG_1WIRE_MUX_CFG_UART_RX_ENABLE:
                mux_cmd = MUX_CMD_UART_RX_ENABLE;
            break;
            case SMCHG_1WIRE_MUX_CFG_UART_TX_RX_DISABLE:
                mux_cmd = MUX_CMD_UART_TX_RX_DISABLE;
            break;
            default:
                is_ctrl = FALSE;
            break;
        }

        if (is_ctrl) {
            mux_status = mux_control(g_smchg.mux_port, mux_cmd, mux_cfg);
            if (mux_status != MUX_STATUS_OK) {
                status = SMCHG_STATUS_ERROR_MUX_CTRL_FAIL;
            }
            smchg_mux_cmd = smchg_mux_cmd & (~cmd_idx);
        }
        cmd_idx = (cmd_idx << 1);
        is_ctrl = TRUE;
    }
end :
    SMCHG_LOG_MSGID_I("1wire, mux ctrl, mux_status[%d], status[%d]", 2, mux_status, status);
    return status;
}


/*     1wire mode     */
static void smchg_1wire_race_mode(hal_uart_baudrate_t baudrate)
{
    smchg_1wire_mux_ctrl_req_t smchg_mux_ctrl;
    smchg_status_t status = SMCHG_STATUS_OK;
    serial_port_dev_t ser_port = SERIAL_PORT_DEV_UNDEFINED;
    SMCHG_LOG_MSGID_D("1wire, race_mode, baudrate[%d]", 1, smchg_1wire_baudrate_report(baudrate));

    if (smchg_cfg.out_of_case_detect_mode == SMCHG_OUT_OF_CASE_DETECT_NORMAL_SW_POLLING_MODE) {
        if (pSmartChargerOutTimer && xTimerChangePeriodFromISR(pSmartChargerOutTimer, pdMS_TO_TICKS(smchg_cfg.race_mode_chk_timer), 0) == pdFAIL) {
            SMCHG_LOG_MSGID_D("1wire, race_mode, change race_mode_timer fail", 0);
            status = SMCHG_STATUS_ERROR_TMR_OP_FAIL;
        }
    }
    if (status == SMCHG_STATUS_OK) {
        smchg_1wire_set_mode_status(SMCHG_1WIRE_RACE);
        smchg_mux_ctrl.mux_ctrl_para.mux_set_config_uart_param.baudrate   = baudrate;
        smchg_mux_ctrl.mux_ctrl_para.mux_set_config_uart_param.int_enable = TRUE;
        smchg_mux_ctrl.cfg_cmd  = smchg_mux_ctrl_cmd[SMCHG_1WIRE_RACE];

        status = smchg_1wire_mux_control(&g_smchg, (void *)&smchg_mux_ctrl);
        if (status == SMCHG_STATUS_OK) {
            g_com_mode_chk_cnt = 0;
            ser_port = smchg_1wire_port_convert_mux_to_port_service(g_smchg.mux_port);
            if (ser_port != SERIAL_PORT_DEV_UNDEFINED) {
#if defined(MTK_RACE_CMD_ENABLE)
                race_1wire_init(ser_port, baudrate);
#endif
            }
        }
    }
    SMCHG_LOG_MSGID_I("1wire, race_mode, baudrate[%d], status[%d]", 2, baudrate, status);

}

void smchg_1wire_leave_race_mode(void)
{
    SMCHG_LOG_MSGID_I("1wire, leave_race_mode", 0);
#if defined(MTK_RACE_CMD_ENABLE)
    race_1wire_deinit();
#endif
}

void smchg_1wire_log_mode(void)
{
    smchg_1wire_mux_ctrl_req_t smchg_mux_ctrl;
    smchg_status_t status = SMCHG_STATUS_OK;

    SMCHG_LOG_MSGID_D("1wire, log_mode, baudrate[%d]", 1, smchg_1wire_baudrate_report(smchg_cfg.log_mode_baud_rate));

    if (smchg_1wire_get_mode_status() == SMCHG_1WIRE_LOG) {
        SMCHG_LOG_MSGID_D("1wire, already in log_mode", 0);
        return;
    }
    memset(&smchg_mux_ctrl, 0x0, sizeof(smchg_mux_ctrl));
    if (smchg_cfg.out_of_case_detect_mode == SMCHG_OUT_OF_CASE_DETECT_NORMAL_SW_POLLING_MODE) {
        if (pSmartChargerOutTimer && xTimerStopFromISR(pSmartChargerOutTimer, 0) == pdFAIL) {
            SMCHG_LOG_MSGID_D("1wire, log_mode, stop timer fail", 0);
            status = SMCHG_STATUS_ERROR_TMR_OP_FAIL;
            assert(0);
        }
    }
    if (status == SMCHG_STATUS_OK) {
        smchg_mux_ctrl.mux_ctrl_para.mux_set_config_uart_param.baudrate   = smchg_cfg.log_mode_baud_rate;
        smchg_mux_ctrl.mux_ctrl_para.mux_set_config_uart_param.int_enable = FALSE;
        smchg_mux_ctrl.cfg_cmd  = smchg_mux_ctrl_cmd[SMCHG_1WIRE_LOG];
        status = smchg_1wire_mux_control(&g_smchg, (void *)&smchg_mux_ctrl);
        /* BTA-24062 must set behand pin mux setting. */
        smchg_1wire_set_mode_status(SMCHG_1WIRE_LOG);
        smchg_1wire_unlock_sleep();
    }
    SMCHG_LOG_MSGID_I("1wire, log_mode, status [%d]", 1, status);
}

static void smchg_1wire_chg_mode(void)
{
    smchg_1wire_mux_ctrl_req_t smchg_mux_ctrl;
    smchg_status_t status      = SMCHG_STATUS_OK;

    SMCHG_LOG_MSGID_D("1wire, chg_mode", 0);
    memset(&smchg_mux_ctrl, 0x0, sizeof(smchg_mux_ctrl));
    if (smchg_1wire_get_mode_status() == SMCHG_1WIRE_CHG) {
        status = SMCHG_STATUS_REENTER_CHG_MODE;
        goto end;
    }
    smchg_1wire_set_mode_status(SMCHG_1WIRE_CHG);

    if (pSmartChargerOutTimer && xTimerStop(pSmartChargerOutTimer, 0) == pdPASS) {
        smchg_mux_ctrl.mux_ctrl_para.mux_set_config_uart_param.int_enable = FALSE;
        smchg_mux_ctrl.cfg_cmd  = smchg_mux_ctrl_cmd[SMCHG_1WIRE_CHG];
        status = smchg_1wire_mux_control(&g_smchg, (void *)&smchg_mux_ctrl);
        g_com_mode_chk_cnt = 0;
        smchg_1wire_set_tmr_active_status(pSmartChargerOutTimer, 0);
        smchg_1wire_unlock_sleep();
    } else {
        SMCHG_LOG_MSGID_D("1wire, chg_mode, stop timer fail", 0);
        status = SMCHG_STATUS_ERROR_TMR_OP_FAIL;
    }

end:
    SMCHG_LOG_MSGID_I("1wire, chg_mode, status[%d]", 1, status);
}

static void smchg_1wire_com_mode(void)
{
    smchg_1wire_mux_ctrl_req_t smchg_mux_ctrl;
    smchg_status_t status = SMCHG_STATUS_OK;

    SMCHG_LOG_MSGID_D("1wire, com_mode, baudrate[%d]", 1, smchg_1wire_baudrate_report(smchg_cfg.com_mode_baud_rate));
    smchg_1wire_set_mode_status(SMCHG_1WIRE_COM);

    if (smchg_cfg.out_of_case_detect_mode == SMCHG_OUT_OF_CASE_DETECT_NORMAL_SW_POLLING_MODE) {
        if (pSmartChargerOutTimer &&
            xTimerChangePeriodFromISR(pSmartChargerOutTimer, pdMS_TO_TICKS(smchg_cfg.com_mode_chk_timer), 0) == pdFAIL) {
            SMCHG_LOG_MSGID_D("1wire, com_mode, change com_mode_timer fail", 0);
            status = SMCHG_STATUS_ERROR_TMR_OP_FAIL;
            assert(0);
        }
        else {
            smchg_1wire_set_tmr_active_status(pSmartChargerOutTimer, 1);
        }
    }
    if (status == SMCHG_STATUS_OK) {
        smchg_mux_ctrl.mux_ctrl_para.mux_set_config_uart_param.baudrate   = smchg_cfg.com_mode_baud_rate;
        smchg_mux_ctrl.mux_ctrl_para.mux_set_config_uart_param.int_enable = TRUE;
        smchg_mux_ctrl.cfg_cmd  = smchg_mux_ctrl_cmd[SMCHG_1WIRE_COM];
        status = smchg_1wire_mux_control(&g_smchg, (void *)&smchg_mux_ctrl);
        smchg_1wire_lock_sleep();
    }

    SMCHG_LOG_MSGID_I("1wire, com_mode, status [%d]", 1, status);
}

static void smchg_1wire_out_of_case(bool isInit)
{
    smchg_1wire_mux_ctrl_req_t smchg_mux_ctrl;
    smchg_status_t status = SMCHG_STATUS_OK;


    SMCHG_LOG_MSGID_D("1wire, out_of_case", 0);
    memset(&smchg_mux_ctrl, 0x0, sizeof(smchg_mux_ctrl));

    if (pSmartChargerOutTimer && xTimerStop(pSmartChargerOutTimer, 0) == pdPASS) {
        smchg_1wire_set_mode_status(SMCHG_1WIRE_OUT);
        smchg_mux_ctrl.mux_ctrl_para.mux_set_config_uart_param.int_enable = FALSE;
        smchg_mux_ctrl.cfg_cmd  = smchg_mux_ctrl_cmd[SMCHG_1WIRE_OUT];
        status = smchg_1wire_mux_control(&g_smchg, (void *)&smchg_mux_ctrl);
        smchg_1wire_set_tmr_active_status(pSmartChargerOutTimer, 0);
        smchg_1wire_unlock_sleep();
    } else {
        SMCHG_LOG_MSGID_D("1wire, out_of_case, stop timer fail", 0);
        status = SMCHG_STATUS_ERROR_TMR_OP_FAIL;
    }

    if (!isInit) {
        smchg_1wire_com_mode_to_out_of_case();
    } else {
        smchg_1wire_init_to_out_of_case();
    }
    SMCHG_LOG_MSGID_I("1wire, out_of_case, status[%d]", 1, status);
}


static void smchg_1wire_pwr_save_mode(void)
{
    smchg_1wire_mux_ctrl_req_t smchg_mux_ctrl;
    smchg_status_t status = SMCHG_STATUS_OK;
    SMCHG_LOG_MSGID_D("1wire, pwr_save_mode", 0);

    memset(&smchg_mux_ctrl, 0x0, sizeof(smchg_mux_ctrl));
    if(pSmartChargerOutTimer && xTimerStop(pSmartChargerOutTimer, 0) == pdPASS) {
        smchg_1wire_set_mode_status(SMCHG_1WIRE_OUT);
        smchg_mux_ctrl.mux_ctrl_para.mux_set_config_uart_param.int_enable = FALSE;
        smchg_mux_ctrl.cfg_cmd  = smchg_mux_ctrl_cmd[SMCHG_1WIRE_OUT];
        status = smchg_1wire_mux_control(&g_smchg, (void *)&smchg_mux_ctrl);
        smchg_1wire_set_tmr_active_status(pSmartChargerOutTimer, 0);
        smchg_1wire_unlock_sleep();
    } else {
        SMCHG_LOG_MSGID_D("1wire, pwr_save_mode, stop timer fail", 0);
        status = SMCHG_STATUS_ERROR_TMR_OP_FAIL;
    }
    SMCHG_LOG_MSGID_I("1wire, pwr_save_mode, status[%d]", 1, status);
}

smchg_status_t smchg_1wire_enter_pwr_save_mode(void)
{

    smchg_1wire_com_mode_to_pwr_save_mode();
    smchg_1wire_pwr_save_mode();

    return SMCHG_STATUS_OK;
}


static void smchg_1wire_send_to_app(uint8_t cmd, uint32_t data, uint16_t data_len)
{
    if (last_cmd == cmd) {
        if (cmd == SMCHG_LID_OPEN ||
            cmd == SMCHG_LID_CLOSE_DONE ||
            cmd == SMCHG_LID_CLOSE) {
            SMCHG_LOG_MSGID_D("1wire, repeat app_cmd[%d]", 1, cmd);
            return;
        }
    }

    last_cmd = cmd;

    if (cmd) {
        smartcharger_callback_t callback_handler = ChargerSmartCase_GetSmartCaseHandle();
        if (callback_handler) {
            //smchg_1wire_app_cmd(cmd);
            callback_handler(cmd, 1, data, data_len);
            SMCHG_LOG_MSGID_D("1wire, send_to_app, cmd[%d], data[%d], data_len[%d]", 3, cmd, data, data_len);
        } else {
            pre_cmd = cmd;
            pre_data = data;
            pre_data_len = data_len;
        }
    }
}

void smchg_1wire_pre_handle(void)
{
    if (pre_cmd) {
        smchg_1wire_send_to_app(pre_cmd, pre_data, pre_data_len);
        pre_cmd = 0;
        pre_data = 0;
        pre_data_len = 0;
    }
}

static void smchg_1wire_com_mode_timer_cb(TimerHandle_t pxExpiredTimer)
{
    uint8_t is_com_mode;
    BaseType_t is_tmr_active = xTimerIsTimerActive(pxExpiredTimer);
    if (!is_tmr_active) {
        smchg_1wire_set_tmr_active_status(pxExpiredTimer, 0);
    }
    is_com_mode = smchg_1wire_com_mode_exist();

    if (is_com_mode) {
        return;
    }
    SMCHG_LOG_MSGID_D("1wire, out_of_case ok", 0);
    smchg_1wire_com_mode_to_out_of_case();
    if (race_mode_flag) {
        smchg_1wire_leave_race_mode();
        race_mode_flag = 0;
    }
    if (smchg_cfg.one_wire_log) {
        smchg_1wire_log_mode();
    } else {
        smchg_1wire_out_of_case(FALSE);
    }
    smchg_1wire_send_to_app(SMCHG_CHG_OUT, 0, 0);
}




static void smchg_1wire_rx_handle_timer_cb(TimerHandle_t pxExpiredTimer)
{
    uint32_t irq_status = 0;
    int32_t rx_cb_count_diff = 0;


    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    //dequeue
    g_rx_data_len  = g_rx_data[g_rxcb_r_cnt].rx_data_len;
    g_rx_user_data = g_rx_data[g_rxcb_r_cnt].p_rx_data;
    if (g_rxcb_r_cnt == (SMCHG_RX_PKT_MAX_SIZE-1)) {
        g_rxcb_r_cnt = 0;
    } else {
        g_rxcb_r_cnt++;
    }
    hal_nvic_restore_interrupt_mask(irq_status);
    //rx handle
    smchg_1wire_rx_handle(g_rx_data_len, g_rx_user_data);

    //check if there is any unhandled rx pkt or not
    hal_nvic_save_and_set_interrupt_mask(&irq_status);

    rx_cb_count_diff = g_rxcb_w_cnt - g_rxcb_r_cnt;
    if (rx_cb_count_diff == 0) {
        g_rx_data_len     = 0;
        g_rx_user_data    = NULL;
        smchg_1wire_give_lock(&g_rx_handle_lock);
    }
    hal_nvic_restore_interrupt_mask(irq_status);
    if (rx_cb_count_diff) {
        SMCHG_LOG_MSGID_I("1wire, rx_handle, g_rxcb_w_cnt = %d, g_rxcb_r_cnt = %d", 2, g_rxcb_w_cnt, g_rxcb_r_cnt);
        if (xTimerStart(pSmartChargerRxTimer,0) != pdPASS) {
            SMCHG_LOG_MSGID_I("1wire, rx timer start from isr fail", 0);
            assert(0);
        } else {
            SMCHG_LOG_MSGID_I("1wire, rx timer start again", 0);
        }
    }
}

#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
static void smchg_1wire_pmu_hdl_cb(TimerHandle_t pxExpiredTimer)
{
    if (pxExpiredTimer == pSmchgChgInTmr) {
    //SMCHG_LOG_MSGID_I("1wire, Tmr chg-in", 0);
        smchg_1wire_intr_to_chg_mode();
        if (race_mode_flag) {
            smchg_1wire_leave_race_mode();
            race_mode_flag = 0;
        }
        smchg_1wire_chg_mode();
        smchg_1wire_send_to_app(SMCHG_CHG_IN, 0, 0);
    }
    else if (pxExpiredTimer == pSmchgChgOutTmr) {
        //SMCHG_LOG_MSGID_I("1wire, Tmr chg-out", 0);
        if (smchg_1wire_get_mode_status() == SMCHG_1WIRE_CHG) { //any mode to com mode, without enter out of case directly
            smchg_1wire_intr_to_com_mode();
            smchg_1wire_com_mode();
        }
    }
    else {
        SMCHG_LOG_MSGID_I("1wire, Tmr unknown", 0);
    }
}


static void smchg_1wire_chg_in_hdlr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SMCHG_LOG_MSGID_I("1wire, chg-in notification", 0);
    if (xTimerStartFromISR(pSmchgChgInTmr, &xHigherPriorityTaskWoken) != pdPASS) {
        SMCHG_LOG_MSGID_E("1wire, pSmchgChgInTmr start from isr fail", 0);
        assert(0);
    }
    if(xHigherPriorityTaskWoken != pdFALSE){
        portYIELD_FROM_ISR(pdTRUE);
    }
}


static void smchg_1wire_chg_out_hdlr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SMCHG_LOG_MSGID_I("1wire, chg-out notification", 0);
    if (xTimerStartFromISR(pSmchgChgOutTmr, &xHigherPriorityTaskWoken) != pdPASS) {
        SMCHG_LOG_MSGID_E("1wire, pSmchgChgOutTmr start from isr fail", 0);
        assert(0);
    }
    if(xHigherPriorityTaskWoken != pdFALSE){
        portYIELD_FROM_ISR(pdTRUE);
    }
}

#else

static void smchg_1wire_chg_isr_cb(battery_management_event_t event, const void *data)
{
    int32_t chg_exist = smchg_1wire_chg_exist();

    if (event != BATTERY_MANAGEMENT_EVENT_CHARGER_EXIST_UPDATE) {
        return;
    }
    SMCHG_LOG_MSGID_I("1wire, chg_isr_cb, chg_exist[%d]", 1, chg_exist);

    if (chg_exist) { //any mode to chg mode
        smchg_1wire_intr_to_chg_mode();
        if (race_mode_flag) {
            smchg_1wire_leave_race_mode();
            race_mode_flag = 0;
        }
        smchg_1wire_chg_mode();
        smchg_1wire_send_to_app(SMCHG_CHG_IN, 0, 0);
    } else if (smchg_1wire_get_mode_status() == SMCHG_1WIRE_CHG) { //any mode to com mode, without enter out of case directly
        smchg_1wire_intr_to_com_mode();
        smchg_1wire_com_mode();
        if (pSmartChargerOutTimer && !xTimerIsTimerActive(pSmartChargerOutTimer) && xTimerStart(pSmartChargerOutTimer, 0) == pdFAIL) {
            SMCHG_LOG_MSGID_E("1wire, rx_handle, start com_mode_timer fail", 0);
            assert(0);
        }
    }
}

#endif

void smchg_1wire_chg_state_change_cb_register(void)
{
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
    pmu_register_callback_lp(CHG_IN_INT_FLAG, (pmu_callback_t) smchg_1wire_chg_in_hdlr, NULL, PMU_CB_1WIRE);
    pmu_register_callback_lp(CHG_OUT_INT_FLAG, (pmu_callback_t) smchg_1wire_chg_out_hdlr, NULL, PMU_CB_1WIRE);
    pSmchgChgInTmr = xTimerCreate("SmchgChgIn", pdMS_TO_TICKS(1), pdFALSE, NULL, smchg_1wire_pmu_hdl_cb);
    if (!pSmchgChgInTmr) {
        SMCHG_LOG_MSGID_D("1wire, pSmchgChgInTmr", 0);
        assert(0);
    }
    pSmchgChgOutTmr = xTimerCreate("SmchgChgOut", pdMS_TO_TICKS(1), pdFALSE, NULL, smchg_1wire_pmu_hdl_cb);
    if (!pSmchgChgOutTmr) {
        SMCHG_LOG_MSGID_D("1wire, pSmchgChgOutTmr", 0);
        assert(0);
    }
#else
    if (battery_management_register_callback(smchg_1wire_chg_isr_cb) != BATTERY_MANAGEMENT_STATUS_OK) {
        SMCHG_LOG_MSGID_D("1wire, init chg_isr_cb fail", 0);
        assert(0);
    }
#endif
}



static smchg_status_t smchg_1wire_rx_timer_start(uint32_t data_len, void *user_data)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    smchg_status_t status = SMCHG_STATUS_OK;
    uint32_t irq_status;

    // enqueue
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    g_rx_data[g_rxcb_w_cnt].rx_data_len = data_len;
    g_rx_data[g_rxcb_w_cnt].p_rx_data = user_data;
    if (g_rxcb_w_cnt == (SMCHG_RX_PKT_MAX_SIZE - 1)) {
        g_rxcb_w_cnt = 0;
    } else {
        g_rxcb_w_cnt++;
    }
    hal_nvic_restore_interrupt_mask(irq_status);

    // get rx handle lock
    status = smchg_1wire_take_lock(&g_rx_handle_lock);
    if (status == SMCHG_STATUS_OK) {

        if (xTimerStartFromISR(pSmartChargerRxTimer, &xHigherPriorityTaskWoken) != pdPASS) {
            SMCHG_LOG_MSGID_D("1wire, rx timer start from isr fail", 0);
            status = SMCHG_STATUS_ERROR_TMR_OP_FAIL;
            assert(0);
        }
    } else if (status == SMCHG_STATUS_ERROR) {
        SMCHG_LOG_MSGID_D("1wire, rx timer restart", 0);
        status = SMCHG_STATUS_ERROR_RX_HDL_TMR_RESTART;
    }
    if (status == SMCHG_STATUS_OK && xHigherPriorityTaskWoken != pdFALSE) {
        // Actual macro used here is port specific.
        portYIELD_FROM_ISR(pdTRUE);
    }

    SMCHG_LOG_MSGID_I("1wire, rx timer start, status[%d]", 1, status);

    return status;
}


static smchg_status_t smchg_1wire_tx_post_handle(uint32_t user_data_len, void *user_data)
{
    smchg_1wire_mux_ctrl_req_t  smchg_mux_ctrl;
    //mux_status_t mux_status     = MUX_STATUS_OK;
    smchg_status_t smchg_status = SMCHG_STATUS_OK;

    memset(&smchg_mux_ctrl, 0x0, sizeof(smchg_mux_ctrl));
    if (g_smchg.state == SMCHG_1WIRE_STATE_COM_WAIT_TX_DONE) {
        SMCHG_LOG_MSGID_I("1wire, tx post handle, handle cmd id [%d]", 1, g_smchg.cur_cmd_id);
        if (g_smchg.cur_cmd_id != CASE_CHARGER_OFF) {
            smchg_mux_ctrl.cfg_cmd = SMCHG_1WIRE_MUX_CFG_UART_RX_ENABLE;
            smchg_status = smchg_1wire_mux_control(&g_smchg, (void *)&smchg_mux_ctrl);
            if (smchg_status != SMCHG_STATUS_OK) {
                SMCHG_LOG_MSGID_W("1wire, tx post handle, smchg status [%d]", 1, smchg_status);
            }
        }
        switch (g_smchg.cur_cmd_id)
        {
            case CASE_RACE_ENABLE :
                if (g_smchg.cur_cmd != NULL) {
                    smchg_1wire_race_mode_info_t *race_mode_cfg = (smchg_1wire_race_mode_info_t *)g_smchg.cur_cmd;
                    if (race_mode_cfg->race_baudrate > HAL_UART_BAUDRATE_4800 && race_mode_cfg->race_baudrate < HAL_UART_BAUDRATE_3200000) {
                        smchg_1wire_race_mode(race_mode_cfg->race_baudrate);
                        race_mode_flag = 1;
                    } else {
                        smchg_status = SMCHG_STATUS_TX_POST_INVALID_BAUDRATE;
                    }
                } else {
                    smchg_status = SMCHG_STATUS_TX_POST_INVALID_CMD_PTR;
                }
                break;
            default:
                break;
        }
        g_smchg.state       = SMCHG_1WIRE_STATE_COM_IDLE;
        g_smchg.cur_cmd_id  = 0;
        g_smchg.cur_cmd     = NULL;
    }

    SMCHG_LOG_MSGID_I("1wire, tx post handle, status [%d], state [%d], cmd_id [%d]", 3, smchg_status, g_smchg.state, g_smchg.cur_cmd_id);
    return smchg_status;
}



static void smchg_1wire_rx_handle(uint32_t user_data_len, void *user_data)
{
    uint8_t raceCmd[SMCHG_MUX_RX_DATA_SIZE] = {0};
    mux_buffer_t buffer;
    buffer.buf_size = SMCHG_MUX_RX_DATA_SIZE;
    buffer.p_buf = raceCmd;
    uint32_t data_size = 0;;
    uint32_t rcv_size = 0;
    mux_status_t mux_status = MUX_STATUS_ERROR;
    smchg_status_t status = SMCHG_STATUS_OK;
    audio_channel_t earbud_ch = ami_get_audio_channel();


    mux_status = mux_rx(g_smchg.mux_handle, &buffer, &data_size);

    rcv_size = ((raceCmd[LENGTH_H]<<8) | raceCmd[LENGTH_L]) + 4;
    if (smchg_cfg.out_of_case_detect_mode == SMCHG_OUT_OF_CASE_DETECT_NORMAL_SW_POLLING_MODE) {
        if (pSmartChargerOutTimer && xTimerReset(pSmartChargerOutTimer, 0) == pdFAIL) {
            SMCHG_LOG_MSGID_D("1wire, rx_handle, reset com_mode_timer fail", 0);
            status = SMCHG_STATUS_ERROR_TMR_OP_FAIL;
        }
    }
    SMCHG_LOG_MSGID_I("1wire, rx_handle, race_ch[0x%X], race_type[0x%X], cmd_id[0x%X], earbud_id[%d], data[%d], earbud_ch[%d], data_size[%d], rcv_size[%d], statue[%d], mux_status[%d]", 10,
                           raceCmd[RACE_CH], raceCmd[RACE_TYPE], raceCmd[CMD_ID], raceCmd[EARBUD_ID], raceCmd[DATA], earbud_ch, data_size, rcv_size, status, mux_status);

    if ((data_size >= 8) && (rcv_size == data_size)) {

        raceEvt[EARBUD_ID] = earbud_ch;

        if ((raceCmd[RACE_CH] == 0x5) && (raceCmd[RACE_TYPE] == 0x5A) && (raceCmd[CMD_ID])) {
            uint32_t data = 0;
            uint16_t data_len = 0;

            if (raceCmd[CMD_ID] == CASE_LID_OPEN || raceCmd[CMD_ID] == CASE_CHARGER_OFF) {
                data = raceCmd[BAT_LV];
                data_len = 1;
            } else if (raceCmd[CMD_ID] == CASE_CHARGER_KEY) {
                data = raceCmd[KEY_ID];
                data_len = 1;
            } else if (raceCmd[CMD_ID] == CASE_REVERSION) {		// richard for UI spec
                data = raceCmd[DATA];
                data_len = 1;
            }

//            if (raceCmd[CMD_ID] <= CASE_LID_CLOSE) {
            if (raceCmd[CMD_ID] <= CASE_REVERSION) {			// richard for UI spec
                smchg_1wire_send_to_app(smchg_app_table[raceCmd[CMD_ID]], data, data_len);
            }

            if (raceCmd[EARBUD_ID] == earbud_ch) {
                uint32_t tx_size = 0;
                smchg_1wire_mux_ctrl_req_t  smchg_mux_ctrl;

                memset(&smchg_mux_ctrl, 0x0, sizeof(smchg_mux_ctrl));
                raceEvt[CMD_ID] = raceCmd[CMD_ID];
                raceEvt[EARBUD_ID] = earbud_ch;
                raceEvt[BAT_LV] = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);

			// richard for UI spec.
			if (raceCmd[CMD_ID] == CASE_REVERSION)
			{
				raceEvt[DATA] = app_smcharger_get_state();
			}
			else if (raceCmd[CMD_ID] == CASE_LID_CLOSE_DONE)
			{
				raceEvt[DATA] = app_get_shipping_mode_state();
			}
			else if (raceCmd[CMD_ID] == CASE_CHARGER_KEY)
			{
				raceEvt[DATA] = app_smcharger_get_earbud_state();
			}

                g_smchg.state = SMCHG_1WIRE_STATE_COM_WAIT_TX_DONE;
                g_smchg.cur_cmd_id = raceCmd[CMD_ID];
                if (raceCmd[CMD_ID] == CASE_RACE_ENABLE && race_mode_flag == 0) {
                    g_smchg.race_mode_info.race_baudrate = raceCmd[BAUDRATE];
                    g_smchg.cur_cmd = (void *)&(g_smchg.race_mode_info);
                }
                buffer.buf_size = sizeof(raceEvt);
                buffer.p_buf = raceEvt;
                // MUX TX start
                smchg_mux_ctrl.cfg_cmd = SMCHG_1WIRE_MUX_CFG_UART_TX_ENABLE;
                status = smchg_1wire_mux_control(&g_smchg, (void *)&smchg_mux_ctrl);
                if (status != SMCHG_STATUS_OK) {
                    SMCHG_LOG_MSGID_E("1wire, rx_handle, mux ctrl tx en fail, status[%d]", 1, status);
                } else {
                    mux_status = mux_tx(g_smchg.mux_handle, &buffer, 1, &tx_size);
                    if (mux_status != MUX_STATUS_OK) {
                        SMCHG_LOG_MSGID_E("1wire, rx_handle, mux tx error, status[%d]", 1, mux_status);
                        smchg_1wire_reset_smchg_state(&g_smchg);
                        smchg_mux_ctrl.cfg_cmd = SMCHG_1WIRE_MUX_CFG_UART_RX_ENABLE;
                        status = smchg_1wire_mux_control(&g_smchg, (void *)&smchg_mux_ctrl);
                    }
                }
                SMCHG_LOG_MSGID_I("1wire, rx_handle, response, data[%d], size[%d]", 3, raceEvt[DATA], tx_size, status);

            }
        }
    }
}

void smchg_1wire_uart_cb(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data)
{
    SMCHG_LOG_MSGID_I("1wire, uart_cb, rx(tx)_done, event[%d], 1:rx, 6:tx, 8:brk_signal", 1, event);
    if (event == MUX_EVENT_READY_TO_READ) {
        g_com_mode_chk_cnt = 0;
        smchg_1wire_rx_timer_start(data_len, user_data);
    } else if (event == MUX_EVENT_TRANSMISSION_DONE) {
        g_com_mode_chk_cnt = 0;
        smchg_1wire_tx_post_handle(data_len, user_data);
    } else if (event == MUX_EVENT_BREAK_SIGNAL) {
        if (smchg_cfg.out_of_case_detect_mode == SMCHG_OUT_OF_CASE_DETECT_NORMAL_SW_POLLING_MODE) {
            return;
        }
        uint32_t is_active = smchg_1wire_get_tmr_active_status(pSmartChargerOutTimer);
        smchg_1wire_mode_t mode = smchg_1wire_get_mode_status();
        SMCHG_LOG_MSGID_I("1wire, uart_cb, start timer[%d], mode [%d]", 2, is_active, mode);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (is_active == 1 || mode == SMCHG_1WIRE_CHG || mode == SMCHG_1WIRE_OUT || mode == SMCHG_1WIRE_LOG) {
            return;
        }
        if (pSmartChargerOutTimer && xTimerStartFromISR(pSmartChargerOutTimer, &xHigherPriorityTaskWoken)!= pdPASS) {
            SMCHG_LOG_MSGID_E("1wire, uart_cb, brk_signal, start Out of case fail", 0);
            assert(0);
        } else {
            smchg_1wire_set_tmr_active_status(pSmartChargerOutTimer, 1);
        }
        if (xHigherPriorityTaskWoken != pdFALSE) {
            portYIELD_FROM_ISR(pdTRUE);
        }
    }
}

smchg_mux_buffer_t smchg_mux_buffer;
void smchg_1wire_mux_buffer_fetch_init(mux_buffer_t buffers[], uint32_t buffers_counter)
{
    uint32_t i;

    smchg_mux_buffer.mux_buffer = buffers;
    smchg_mux_buffer.offset = 0;
    smchg_mux_buffer.idx = 0;
    smchg_mux_buffer.counter = buffers_counter;
    smchg_mux_buffer.left_data_len = 0;
    for (i = 0; i < buffers_counter; i++) {
        smchg_mux_buffer.left_data_len += buffers[i].buf_size;
    }
    smchg_mux_buffer.total_length = smchg_mux_buffer.left_data_len;
}

bool smchg_1wire_mux_header_fetch(uint8_t *out_buf, uint32_t out_len)
{
    uint32_t i;

    if (smchg_mux_buffer.idx >= smchg_mux_buffer.counter) {
        SMCHG_LOG_MSGID_E("1wire, mux_header_featch counter fail, idx[%d], counter[%d]", 2,
                            smchg_mux_buffer.idx, smchg_mux_buffer.counter);
        return false;
    }

    if (smchg_mux_buffer.left_data_len < out_len) {
        SMCHG_LOG_MSGID_E("1wire, mux_header_featch length fail, left_data_len[%d], out_len[%d]", 2,
                            smchg_mux_buffer.left_data_len, out_len);
        return false;
    }

    for (i = 0; i < out_len; i++, smchg_mux_buffer.left_data_len--, smchg_mux_buffer.offset++) {
        if (smchg_mux_buffer.offset >= smchg_mux_buffer.mux_buffer[smchg_mux_buffer.idx].buf_size) {
            smchg_mux_buffer.idx++;
            smchg_mux_buffer.offset = 0;
            if (smchg_mux_buffer.idx >= smchg_mux_buffer.counter) {
                SMCHG_LOG_MSGID_E("1wire, mux_header_featch counter fail, idx[%d], counter[%d]", 2,
                                    smchg_mux_buffer.idx, smchg_mux_buffer.counter);
                assert(0);
            }
        }
        *(out_buf + i) = *(smchg_mux_buffer.mux_buffer[smchg_mux_buffer.idx].p_buf + smchg_mux_buffer.offset);
    }
    return true;
}

void smchg_1wire_mux_tx_protocol_cb(mux_handle_t handle, const mux_buffer_t payload[], uint32_t buffers_counter, mux_buffer_t *head, mux_buffer_t *tail, void *user_data)
{
    const char *user_name = NULL;
    uint32_t i = 0, race_id = 0, total_size = 0;

    mux_status_t status = mux_query_user_name(handle, &user_name);
    smchg_1wire_mode_t smchg_1wire_status = smchg_1wire_get_mode_status();
    if ((status == MUX_STATUS_OK) && (user_name != NULL)) {
        if (smchg_1wire_status == SMCHG_1WIRE_NORM) {
            if (strcmp(user_name, "ATCI") == 0) {
                /* [Special] Need to fill the head buffer */
                race_id = 0x0F92;
            } else {
                /* SYSLOG/RACE/other */
                head->p_buf = NULL;
                tail->p_buf = NULL;
                head->buf_size = 0;
                tail->buf_size = 0;
                return;
            }
        } else if (smchg_1wire_status == SMCHG_1WIRE_LOG) {
            if (strcmp(user_name, "SYSLOG") == 0) {
                head->p_buf = NULL;
                tail->p_buf = NULL;
                head->buf_size = 0;
                tail->buf_size = 0;
                return;
            } else {
                /* Ignore tx except syslog */
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 0] = 0x65;
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 1] = 0x72;
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 2] = 0x69;
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 3] = 0x57;

                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 0] = 0x65;
                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 1] = 0x72;
                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 2] = 0x69;
                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 3] = 0x57;

                head->p_buf = NULL;
                tail->p_buf = NULL;
                head->buf_size = 0;
                tail->buf_size = 0;
                return;
            }
        } else if (smchg_1wire_status == SMCHG_1WIRE_COM) {
            if (strcmp(user_name, "SM_CHG") == 0) {
                head->p_buf = NULL;
                tail->p_buf = NULL;
                head->buf_size = 0;
                tail->buf_size = 0;
                return;
            } else {
                /* Ignore tx except CHG CMD */
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 0] = 0x65;
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 1] = 0x72;
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 2] = 0x69;
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 3] = 0x57;

                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 0] = 0x65;
                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 1] = 0x72;
                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 2] = 0x69;
                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 3] = 0x57;

                head->p_buf = NULL;
                tail->p_buf = NULL;
                head->buf_size = 0;
                tail->buf_size = 0;
                return;
            }
        } else if (smchg_1wire_status == SMCHG_1WIRE_RACE) {
            if (strcmp(user_name, "RACE_CMD") == 0 || strcmp(user_name, "HCI_CMD") == 0 ) {
                head->p_buf = NULL;
                tail->p_buf = NULL;
                head->buf_size = 0;
                tail->buf_size = 0;
                return;
            } else if (strcmp(user_name, "ATCI") == 0) {
                race_id = 0x0F92;
            } else {
                /* Ignore tx except race CMD */
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 0] = 0x65;
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 1] = 0x72;
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 2] = 0x69;
                head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 3] = 0x57;

                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 0] = 0x65;
                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 1] = 0x72;
                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 2] = 0x69;
                tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 3] = 0x57;

                head->p_buf = NULL;
                tail->p_buf = NULL;
                head->buf_size = 0;
                tail->buf_size = 0;
                return;
            }
        } else if ((smchg_1wire_status == SMCHG_1WIRE_CHG) || (smchg_1wire_status == SMCHG_1WIRE_OUT)) {
            /* Ignore all tx */
            head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 0] = 0x65;
            head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 1] = 0x72;
            head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 2] = 0x69;
            head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 3] = 0x57;

            tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 0] = 0x65;
            tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 1] = 0x72;
            tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 2] = 0x69;
            tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 3] = 0x57;

            head->p_buf = NULL;
            tail->p_buf = NULL;
            head->buf_size = 0;
            tail->buf_size = 0;
            return;
        } else {
            /* mode error */
            head->p_buf = NULL;
            tail->p_buf = NULL;
            head->buf_size = 0;
            tail->buf_size = 0;
            return;
        }
    } else {
        head->p_buf = NULL;
        tail->p_buf = NULL;
        head->buf_size = 0;
        tail->buf_size = 0;
        return;
    }

    total_size = 0;
    for (i = 0; i < buffers_counter; i++) {
        total_size += payload[i].buf_size;
    }

    /* Insert the race header here */
    head->p_buf[0] = 0x05;
    head->p_buf[1] = RACE_TYPE_NOTIFICATION;
    head->p_buf[2] = (uint8_t)((total_size + 2) & 0xFF);
    head->p_buf[3] = (uint8_t)(((total_size + 2) >> 8) & 0xFF);
    head->p_buf[4] = (uint8_t)(race_id & 0xFF);
    head->p_buf[5] = (uint8_t)((race_id >> 8) & 0xFF);
    head->buf_size = 6;

    tail->p_buf = NULL;
    tail->buf_size = 0;
}







void smchg_1wire_mux_rx_protocol_cb(mux_handle_t *handle, mux_buffer_t buffers[], uint32_t buffers_counter,
                                    uint32_t *consume_len, uint32_t *package_len, void *user_data)
{
    uint8_t race_ch = 0, race_type = 0;
    uint16_t race_length = 0, race_id = 0;
    uint32_t collected_length = 0, error_code = 0;
    mux_handle_t p_handle;


    race_length = 0;
    collected_length = 0;
    *package_len = 0;
    *consume_len = 0;

    smchg_1wire_mode_t smchg_1wire_status = smchg_1wire_get_mode_status();

    smchg_1wire_mux_buffer_fetch_init(buffers, buffers_counter);
    /* 1wire sm check */
    if ((smchg_1wire_status != SMCHG_1WIRE_NORM) && (smchg_1wire_status != SMCHG_1WIRE_COM) && (smchg_1wire_status != SMCHG_1WIRE_RACE) && (smchg_1wire_status != SMCHG_1WIRE_ATCI)) {
        /* error */
        *handle = 0x0;
        *package_len = 0;
        *consume_len = smchg_mux_buffer.total_length;
        error_code |= SMCHG_MUX_PARSE_ERROR_UNKNOWN_1WIRE_STATUS;
        goto end;
    }


    if (smchg_1wire_mux_header_fetch(&race_ch, 1) == false) {
        *package_len = 0;
        *consume_len = smchg_mux_buffer.total_length - smchg_mux_buffer.left_data_len;
        error_code |= SMCHG_MUX_PARSE_ERROR_FETCH_RACE_CH;
        //SMCHG_LOG_MSGID_E("1wire, mux_rx_protocol_cb, race_ch fetch fail", 0); 
        goto end;
    }

    if (race_ch == 0x05 || race_ch == 0x15) {
        if (smchg_1wire_mux_header_fetch(&race_type, 1) == false) {
            *package_len = 0;
            *consume_len = smchg_mux_buffer.total_length - smchg_mux_buffer.left_data_len;
            error_code |= SMCHG_MUX_PARSE_ERROR_FETCH_CH_TYPE;
            SMCHG_LOG_MSGID_D("1wire, mux_rx_protocol_cb, race_type fetch fail", 0);
            goto end;
        }

        if ((race_type != RACE_TYPE_COMMAND) && (race_type != RACE_TYPE_COMMAND_WITHOUT_RSP)) {
            *package_len = 0;
            *consume_len = smchg_mux_buffer.total_length - smchg_mux_buffer.left_data_len;
            error_code |= SMCHG_MUX_PARSE_ERROR_CH_TYPE;
            SMCHG_LOG_MSGID_D("1wire, mux_rx_protocol_cb, race_type[0x%X]", 1, race_type);
            goto end;
        }

        if (smchg_1wire_mux_header_fetch((uint8_t *)&race_length, 2) == false) {
            *package_len = 0;
            *consume_len = smchg_mux_buffer.total_length - smchg_mux_buffer.left_data_len;
            error_code |= SMCHG_MUX_PARSE_ERROR_FETCH_LENGTH;
            SMCHG_LOG_MSGID_D("1wire, mux_rx_protocol_cb, race_length fetch fail", 0);
            goto end;
        }

        if (smchg_1wire_mux_header_fetch((uint8_t *)&race_id, 2) == false) {
            *package_len = 0;
            *consume_len = smchg_mux_buffer.total_length - smchg_mux_buffer.left_data_len;
            error_code |= SMCHG_MUX_PARSE_ERROR_FETCH_RACE_ID;
            SMCHG_LOG_MSGID_D("1wire, mux_rx_protocol_cb, race_id fetch fail", 0);
            goto end;
        }

        *handle = 0;
        collected_length = smchg_mux_buffer.total_length - smchg_mux_buffer.left_data_len;
        if (race_id == 0x0F92) {
            if (mux_query_user_handle(g_smchg.mux_port, "ATCI", &p_handle) == MUX_STATUS_OK) {
                /* ATCI race command */
                *handle = p_handle;
                /* RACE cmd format: 05 5D length(2 byte) 0f92 at+test=? */
                *package_len = race_length - 2;
                *consume_len = 6;
            } else {
                *package_len = 0;
                *consume_len = collected_length;
                error_code |= SMCHG_MUX_PARSE_ERROR_NO_ATCI_HANDLE;
            }
        } else if (race_id == 0x2000) {
            if (smchg_1wire_status != SMCHG_1WIRE_COM) {
                *package_len = 0;
                *consume_len = collected_length;
                error_code |= SMCHG_MUX_PARSE_ERROR_COM_MODE_STATUS_NOT_MATCH;
                //return;
            } else if (mux_query_user_handle(g_smchg.mux_port, "SM_CHG", &p_handle) == MUX_STATUS_OK) {
                /* 1wire cmd */
                if (smchg_mux_buffer.left_data_len >= (race_length - 2)) {
                    *package_len = collected_length + race_length - 2;
                    *consume_len = 0;
                    *handle = p_handle;
                } else {
                    *package_len = collected_length + smchg_mux_buffer.left_data_len;
                    *consume_len = 0;
                    *handle = p_handle;
                    SMCHG_LOG_MSGID_E("1wire, mux_rx_protocol_cb fail, data len not enough", 0);
                }
                g_com_mode_chk_cnt = 0;
            } else {
                *package_len = 0;
                *consume_len = collected_length;
                error_code |= SMCHG_MUX_PARSE_ERROR_NO_SMCHG_HANDLE;
            }
        } else {
            if ((mux_query_user_handle(g_smchg.mux_port, "RACE_CMD", &p_handle) == MUX_STATUS_OK) &&
                (smchg_1wire_status == SMCHG_1WIRE_RACE)) {
                //race cmd
                if (smchg_mux_buffer.left_data_len >= (race_length - 2)) {
                    *package_len = collected_length + race_length - 2;
                    *consume_len = 0;
                    *handle = p_handle;
                } else {
                    *package_len = 0;
                    *consume_len = collected_length + smchg_mux_buffer.left_data_len;
                    *handle = p_handle;
                    SMCHG_LOG_MSGID_E("1wire, RACE mode mux_rx_protocol_cb fail, data len not enough", 0);
                }
                g_com_mode_chk_cnt = 0;
            } else {
                *package_len = 0;
                if ((race_length + 4) >= smchg_mux_buffer.total_length) {
                    *consume_len = smchg_mux_buffer.total_length;
                } else {
                    *consume_len = race_length + 4;
                }
                error_code |= SMCHG_MUX_PARSE_ERROR_NO_RACE_HANDLE;
                SMCHG_LOG_MSGID_D("1wire, mux_rx_protocol_cb fail, no_race_handle", 0);
                goto end;
            }
        }
    }
    else if ((smchg_1wire_status == SMCHG_1WIRE_RACE) && (race_ch == 0x01 || race_ch == 0x02 || race_ch == 0x04))
    {
        uint16_t reserved_data = 0;
        uint16_t total_length = 0;
        uint32_t rsv_len = 0, cmd_len = 0;
        g_com_mode_chk_cnt = 0;
        if (race_ch == 0x01) {
            rsv_len = 2;
            cmd_len = 1;
        }
        else if (race_ch == 0x02){
            rsv_len = 2;
            cmd_len = 2;
        }
        else if (race_ch == 0x04){
            rsv_len = 1;
            cmd_len = 1;
        }

        *handle = 0;
        if (mux_query_user_handle(g_smchg.mux_port, "HCI_CMD", &p_handle) == MUX_STATUS_OK)
        {
            if (smchg_1wire_mux_header_fetch((uint8_t *)&reserved_data, rsv_len) == false){
                *package_len = 0;
                *consume_len = smchg_mux_buffer.total_length - smchg_mux_buffer.left_data_len;
                return;
            }
            if (smchg_1wire_mux_header_fetch((uint8_t *)&total_length, cmd_len) == false){
                *package_len = 0;
                *consume_len = smchg_mux_buffer.total_length - smchg_mux_buffer.left_data_len;
                return;
            }
            *handle = p_handle;
            if (total_length >= smchg_mux_buffer.left_data_len) {
                *package_len = smchg_mux_buffer.left_data_len + rsv_len + cmd_len + 1;
            } else {
                *package_len = total_length + rsv_len + cmd_len + 1;
            }
            *consume_len = 0;
        }
        else
        {
            SMCHG_LOG_MSGID_E("1wire, mux_rx_protocol_cb , no HCI_CMD handle", 0);
            *package_len = 0;
            *consume_len = smchg_mux_buffer.total_length - smchg_mux_buffer.left_data_len;
            error_code |= SMCHG_MUX_PARSE_ERROR_NO_HCI_HANDLE;
        }
    }
    else
    {
        *package_len = 0;
        *consume_len = smchg_mux_buffer.total_length - smchg_mux_buffer.left_data_len;
        error_code |= SMCHG_MUX_PARSE_ERROR_RACE_CH;
        SMCHG_LOG_MSGID_D("1wire, mux_rx_protocol_cb fail, race_ch[0x%X]", 1, race_ch);
        goto end;
    }
end:
    if (error_code) {
        SMCHG_LOG_MSGID_I("1wire, mux_rx_protocol_cb fail, 1wire mode [%d], err_code[0x%X], race_ch[0x%X]", 3, smchg_1wire_status, error_code, race_ch);
    }
}

void smchg_1wire_tx_gpio_driving(hal_gpio_driving_current_t driving)
{
    switch (smchg_cfg.uart_sel) {
        case SMCHG_UART0_2GPIO:
        case SMCHG_UART1_2GPIO:
            hal_gpio_set_driving_current(smchg_1wire_gpio_cfg.tx_pin, driving);
            break;

        case SMCHG_UART0_1GPIO:
        case SMCHG_UART1_1GPIO:
            hal_gpio_set_driving_current(smchg_1wire_gpio_cfg.trx_pin, driving);
            break;

        default:
            break;
    }
}

void smchg_1wire_gpio_init(void)
{
    switch (smchg_cfg.uart_sel) {
        case SMCHG_UART0_2GPIO: {
            if ((HAL_UART0_TXD_PIN | HAL_UART0_TXD_PIN_M_GPIO | HAL_UART0_TXD_PIN_M_UART0_TXD |
                 HAL_UART0_RXD_PIN | HAL_UART0_RXD_PIN_M_GPIO | HAL_UART0_RXD_PIN_M_UART0_RXD) == 0xFF) {
                assert(0);
            }

            smchg_1wire_gpio_cfg.tx_pin  = HAL_UART0_TXD_PIN;
            smchg_1wire_gpio_cfg.tx_gpio = HAL_UART0_TXD_PIN_M_GPIO;
            smchg_1wire_gpio_cfg.tx_uart = HAL_UART0_TXD_PIN_M_UART0_TXD;
            smchg_1wire_gpio_cfg.rx_pin  = HAL_UART0_RXD_PIN;
            smchg_1wire_gpio_cfg.rx_gpio = HAL_UART0_RXD_PIN_M_GPIO;
            smchg_1wire_gpio_cfg.rx_uart = HAL_UART0_RXD_PIN_M_UART0_RXD;
        }
        break;

        case SMCHG_UART1_2GPIO: {
            if ((HAL_UART1_TXD_PIN | HAL_UART1_TXD_PIN_M_GPIO | HAL_UART1_TXD_PIN_M_UART1_TXD |
                 HAL_UART1_RXD_PIN | HAL_UART1_RXD_PIN_M_GPIO | HAL_UART1_RXD_PIN_M_UART1_RXD) == 0xFF) {
                assert(0);
            }

            smchg_1wire_gpio_cfg.tx_pin  = HAL_UART1_TXD_PIN;
            smchg_1wire_gpio_cfg.tx_gpio = HAL_UART1_TXD_PIN_M_GPIO;
            smchg_1wire_gpio_cfg.tx_uart = HAL_UART1_TXD_PIN_M_UART1_TXD;
            smchg_1wire_gpio_cfg.rx_pin  = HAL_UART1_RXD_PIN;
            smchg_1wire_gpio_cfg.rx_gpio = HAL_UART1_RXD_PIN_M_GPIO;
            smchg_1wire_gpio_cfg.rx_uart = HAL_UART1_RXD_PIN_M_UART1_RXD;
        }
        break;
        case SMCHG_UART0_1GPIO: {
            if ((HAL_UART0_RXD_PIN | HAL_UART0_RXD_PIN_M_GPIO |
                 HAL_UART0_RXD_PIN_M_UART0_TXD | HAL_UART0_RXD_PIN_M_UART0_RXD) == 0xFF) {
                assert(0);
            }

            smchg_1wire_gpio_cfg.trx_pin  = HAL_UART0_RXD_PIN;
            smchg_1wire_gpio_cfg.trx_gpio = HAL_UART0_RXD_PIN_M_GPIO;
            smchg_1wire_gpio_cfg.tx_uart  = HAL_UART0_RXD_PIN_M_UART0_TXD;
            smchg_1wire_gpio_cfg.rx_uart  = HAL_UART0_RXD_PIN_M_UART0_RXD;
        }
        break;

        case SMCHG_UART1_1GPIO: {
            if ((HAL_UART1_RXD_PIN | HAL_UART1_RXD_PIN_M_GPIO |
                 HAL_UART1_RXD_PIN_M_UART1_TXD | HAL_UART1_RXD_PIN_M_UART1_RXD) == 0xFF) {
                assert(0);
            }

            smchg_1wire_gpio_cfg.trx_pin  = HAL_UART1_RXD_PIN;
            smchg_1wire_gpio_cfg.trx_gpio = HAL_UART1_RXD_PIN_M_GPIO;
            smchg_1wire_gpio_cfg.tx_uart  = HAL_UART1_RXD_PIN_M_UART1_TXD;
            smchg_1wire_gpio_cfg.rx_uart  = HAL_UART1_RXD_PIN_M_UART1_RXD;
        }
        break;
        default:
            SMCHG_LOG_MSGID_D("1wire, gpio_init fail, uart_sel[%d]", 1, smchg_cfg.uart_sel);
            break;
    }
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
    smchg_1wire_tx_gpio_driving(HAL_GPIO_DRIVING_CURRENT_1V8_4MA_OR_3V3_8MA);
#else
     smchg_1wire_tx_gpio_driving(HAL_GPIO_DRIVING_CURRENT_4MA);
#endif
}


static smchg_status_t smchg_1wire_uart_init(smchg_handle_t *p_sm_handle)
{
    mux_port_setting_t setting;
    mux_protocol_t protocol_callback_1wire = {smchg_1wire_mux_tx_protocol_cb, smchg_1wire_mux_rx_protocol_cb, NULL};
    mux_status_t mux_init_status;
    smchg_status_t status = SMCHG_STATUS_OK;

    if (p_sm_handle == NULL) {
        return SMCHG_STATUS_INVALID_PARAMETER;
    }
    memset(&setting, 0, sizeof(setting));
    setting.dev_setting.uart.uart_config.baudrate    = smchg_cfg.com_mode_baud_rate;
    setting.dev_setting.uart.uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    setting.dev_setting.uart.uart_config.stop_bit    = HAL_UART_STOP_BIT_1;
    setting.dev_setting.uart.uart_config.parity      = HAL_UART_PARITY_NONE;
    setting.dev_setting.uart.flowcontrol_type        = MUX_UART_NONE_FLOWCONTROL;
    setting.portLinkRegAddr                          = 0;

    setting.tx_buffer_size = SMCHG_MUX_TX_BUFF_SIZE;
    setting.rx_buffer_size = SMCHG_MUX_RX_BUFF_SIZE;
    mux_init_status = mux_init(p_sm_handle->mux_port, &setting, &protocol_callback_1wire);
    if (mux_init_status != MUX_STATUS_OK) {
        SMCHG_LOG_MSGID_W("1wire, mux_init fail[%d]", 1, mux_init_status);
    }
    mux_init_status = mux_open(p_sm_handle->mux_port, "SM_CHG", &(p_sm_handle->mux_handle), smchg_1wire_uart_cb, NULL);

    if (mux_init_status != MUX_STATUS_OK) {
        SMCHG_LOG_MSGID_E("1wire, mux_open fail[%d]", 1, mux_init_status);
        assert(0);
    }

    SMCHG_LOG_MSGID_D("1wire, uart_init, baudrate[%d]", 1,
                       smchg_1wire_baudrate_report(setting.dev_setting.uart.uart_config.baudrate));
    return status;
}



static smchg_status_t smchg_1wire_com_mode_init(smchg_handle_t *p_sm_handle)
{
    smchg_status_t    status   = SMCHG_STATUS_OK;

    status = smchg_1wire_uart_init(&g_smchg);
    if (status == SMCHG_STATUS_OK) {
        pSmartChargerRxTimer = xTimerCreate("Smart Charger", pdMS_TO_TICKS(SMCHG_MUX_RX_HANDLE_TIMER_MS), pdFALSE, NULL, smchg_1wire_rx_handle_timer_cb);
        if (!pSmartChargerRxTimer) {
            SMCHG_LOG_MSGID_D("1wire, init rx_handle_timer_cb fail", 0);
            assert(0);
        }
    }
    return status;
}




static smchg_status_t smchg_1wire_detect_out_of_case_init(smchg_handle_t *p_sm_handle)
{
    uint16_t                         tmr_prd  = smchg_cfg.com_mode_chk_timer;
    smchg_status_t                   status   = SMCHG_STATUS_OK;
    smchg_out_of_case_detect_mode_t  out_mode = smchg_cfg.out_of_case_detect_mode;
    UBaseType_t                      auto_reload = pdTRUE;

    if (tmr_prd == 0) {
        status = SMCHG_STATUS_INVALID_PARAMETER;
    }

    if (status == SMCHG_STATUS_OK) {
        switch (out_mode) {
            case SMCHG_OUT_OF_CASE_DETECT_NORMAL_SW_POLLING_MODE :
                g_chk_out_case_cnt_thrd = 3;
                break;
            case SMCHG_OUT_OF_CASE_DETECT_FAST_DETECT_INTERRUPT_MODE :
                auto_reload = pdFALSE;
                g_chk_out_case_cnt_thrd = 1;
                if (smchg_cfg.out_of_case_vbus_uart_thrd > SMCHG_OUT_OF_CASE_VOL_THRESHOLD_MV) {
                    smchg_cfg.out_of_case_vbus_uart_thrd = SMCHG_OUT_OF_CASE_VOL_THRESHOLD_MV;
                }
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
                pmu_1wire_uart_pull_low_en(PMU_ON);
#endif
                break;
            default :
                assert(0);
                g_chk_out_case_cnt_thrd = 3;
                break;
        }
        if (g_chk_out_case_cnt_thrd) {
            pSmartChargerOutTimer = xTimerCreate("Smart Charger", pdMS_TO_TICKS(tmr_prd), auto_reload, NULL, smchg_1wire_com_mode_timer_cb);
            if (!pSmartChargerOutTimer) {
                SMCHG_LOG_MSGID_D("1wire, init com_mode_timer_cb fail", 0);
                assert(0);
            }
        }
    }
    return status;
}



void smchg_1wire_init(void)
{

    uint8_t pwr_off_reason  = 0;
    int32_t chg_exist       = 0;
    mux_status_t mux_status = MUX_STATUS_ERROR;
    smchg_status_t status   = SMCHG_STATUS_OK;
    smchg_1wire_mux_ctrl_req_t  smchg_mux_ctrl;

    SMCHG_LOG_MSGID_D("1wire, init enter", 0);
    memset(&smchg_mux_ctrl, 0x0, sizeof(smchg_mux_ctrl));
    memset(&g_smchg, 0x0, sizeof(g_smchg));
    g_smchg.state       = SMCHG_1WIRE_STATE_INIT;
    g_smchg.cur_cmd_id  = 0;
    g_smchg.cur_cmd     = NULL;
    g_smchg.mux_port    = MUX_UART_1;

    // nvkey value check (?)
    if (smchg_cfg.uart_sel > SMCHG_UART1_1GPIO) {
        //SMCHG_LOG_MSGID_E("1wire, init fail, uart_sel[%d]", 1, smchg_cfg.uart_sel);
        assert(0);
    } else if ((smchg_cfg.uart_sel == SMCHG_UART0_1GPIO) || (smchg_cfg.uart_sel == SMCHG_UART0_2GPIO)) {
        g_smchg.mux_port = MUX_UART_0;
    }

    last_cmd = SMCHG_CHG_OUT;

    if (smchg_1wire_detect_out_of_case_init(&g_smchg) == SMCHG_STATUS_OK &&
        smchg_1wire_com_mode_init(&g_smchg) == SMCHG_STATUS_OK) {
        /* 1wire DFU */
        pwr_off_reason = pmu_get_power_off_reason();
        if ((boot_Flag == 0) && (pwr_off_reason == 0x8) && (smchg_1wire_chg_exist() == 0)) {
            uint32_t tx_size = 0;
            SMCHG_LOG_MSGID_D("1wire, boot_evt send", 0);
            raceEvt[RACE_TYPE] = 0x5D;
            mux_buffer_t buf = {raceEvt, sizeof(raceEvt)};
            smchg_1wire_intr_to_com_mode();
            if (smchg_cfg.one_wire_log) {
                smchg_1wire_com_mode();
            }
            smchg_mux_ctrl.cfg_cmd = SMCHG_1WIRE_MUX_CFG_UART_TX_ENABLE;
            status = smchg_1wire_mux_control(&g_smchg, (void *)&smchg_mux_ctrl);
            if (status != SMCHG_STATUS_OK) {
                SMCHG_LOG_MSGID_E("1wire, uart_init mux ctrl fail[%d]", 1, status);
            }
            mux_status = mux_tx(g_smchg.mux_handle, &buf, 1, &tx_size);
            if (mux_status != MUX_STATUS_OK) {
                SMCHG_LOG_MSGID_E("1wire, uart_init mux tx fail [%d]", 1, mux_status);
            }
            if (smchg_cfg.one_wire_log) {
                /*the delay time depends on the sent data size and the baudrate of UART. 500us is enough for 9 Bytes with 3Mbps*/
                hal_gpt_delay_us(500);
            } else {
                smchg_1wire_com_mode_tx_done();
            }
            raceEvt[RACE_TYPE] = 0x5B;
            boot_Flag = 1;
        }
        smchg_1wire_chg_state_change_cb_register();
        chg_exist = smchg_1wire_chg_exist();
        if (chg_exist) {
            smchg_1wire_init_to_chg_mode();
            smchg_1wire_chg_mode();
        } else {
            if (smchg_cfg.one_wire_log) {
                smchg_1wire_init_to_log_mode();
                smchg_1wire_log_mode();
            } else {
                smchg_1wire_out_of_case(TRUE);
            }
        }
    }
    SMCHG_LOG_MSGID_I("1wire, init status[%d], chg_exist[%d], boot_flag[%d], pwr_off[%d]", 4, status, chg_exist, boot_Flag, pwr_off_reason);
}
#endif


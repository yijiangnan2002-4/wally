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

/* Includes ------------------------------------------------------------------*/
#include "hal.h"
#include "hal_platform.h"
#ifdef HAL_I3C_MASTER_MODULE_ENABLED
#include "hal_i3c_master.h"
#include "hal_i3c_master_internal.h"

#include "hal_pdma_internal.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_platform.h"
#include "hal_clock.h"

#ifdef HAL_CLOCK_MODULE_ENABLED
#include "hal_clock.h"
extern hal_clock_status_t clock_mux_sel(clock_mux_sel_id mux_id, uint32_t mux_sel);
#else
    #define clock_mux_sel
#endif

/***********************************************************************************************
 * Pre-define Declarations
***********************************************************************************************/
#define     I3C_SCL_HS_HIGH_TM_NS    (45) /* scl high 40ns */
#define     I3C_SCL_PERIOD_DUTY      (40)
#define     I3C_SCL_PERIOD_TIMER     (1000000000.0f / I3C_CLOCK_FREQUENCY)
#define     I3C_HS_FMT(data)         ((data) & 0x7FFF)

/***********************************************************************************************
 * Private Function Declarations
***********************************************************************************************/
int    i3c_pdma_init(hal_i3c_master_port_t i3c_port, i3c_master_transfer_config_t *config);
int    i3c_pdma_deinit(hal_i3c_master_port_t i3c_port);


/***********************************************************************************************
 * Private Variable Declarations
***********************************************************************************************/
static volatile I3C_MASTER_REGISTER_T *s_i3c_base_register[HAL_I3C_MASTER_MAX] = { (I3C_MASTER_REGISTER_T *)(I3C_0_BASE),     (I3C_MASTER_REGISTER_T *)(I3C_1_BASE)};
static volatile I2C_PDMA_REGISTER_T   *s_i3c_pdma_register[HAL_I3C_MASTER_MAX] = { (I2C_PDMA_REGISTER_T*)(I3C0_PDMA_BASE),    (I2C_PDMA_REGISTER_T*)(I3C1_PDMA_BASE)};
#ifdef I3C_NVIC_ENABLED
static const hal_nvic_irq_t            s_i3c_irq_num_table[HAL_I3C_MASTER_MAX] = {I3C0_IRQn, I3C1_IRQn};
#endif
static const hal_clock_cg_id           s_i3c_clk_pdn_table[HAL_I3C_MASTER_MAX] = {HAL_CLOCK_CG_I3C0, HAL_CLOCK_CG_I3C1};
static const pdma_channel_t            s_i3c_pdma_chnl[HAL_I3C_MASTER_MAX][2]  = { {PDMA_I3C0_TX, PDMA_I3C0_RX}, {PDMA_I3C1_TX, PDMA_I3C1_RX}};
static const sleep_management_lock_request_t    s_i3c_sleep_handle[HAL_I3C_MASTER_MAX] = {SLEEP_LOCK_I3C0, SLEEP_LOCK_I3C1};


static bool    _i3c_calc_speed_with_time(
    uint32_t    speed,
    bool        is_hs,
    double      scl_tm_ns,
    uint32_t    *clk_div,
    uint32_t    *sample_cnt_div,
    uint32_t    *step_cnt_div)
{
    uint32_t  clock_div   = 0, clock_div_max;  /* 5bit:0x1F */
    uint32_t  smp_cnt_div = 0, smp_cnt_div_max;  /* 3bit:0x07 */
    uint32_t  stp_cnt_div = 0, stp_cnt_div_max;  /* 6bit:0x3F */
    uint32_t  temp;
    double    ftemp;

    if (is_hs) {
        clock_div_max   = 0x1F;
        smp_cnt_div_max = 0x07;
        stp_cnt_div_max = 0x07;
    } else {
        clock_div_max   = 0x1F;
        smp_cnt_div_max = 0x07;
        stp_cnt_div_max = 0x3F;
    }

    for (clock_div = 0; clock_div < clock_div_max; clock_div++) {
        ftemp = I3C_SCL_PERIOD_TIMER *(clock_div+1);
        temp  = (uint32_t) ((scl_tm_ns / ftemp) + 0.7);
        for (smp_cnt_div = 0; smp_cnt_div < smp_cnt_div_max; smp_cnt_div++) {
            stp_cnt_div = (temp / (smp_cnt_div + 1)) - 1;
            if (stp_cnt_div < stp_cnt_div_max) {
                break;
            }
        }
        if (stp_cnt_div < stp_cnt_div_max) {
            break;
        }
    }
    log_hal_i3c_debug("[hal][i3c] calc freq: scl width(%dns - %d), clk_div(%u), smp_div_cnt(%u), stp_div_cnt(%u)\r\n", 5, (int)scl_tm_ns, temp,  clock_div, smp_cnt_div, stp_cnt_div);
    if (clock_div > clock_div_max || smp_cnt_div > smp_cnt_div_max || stp_cnt_div > stp_cnt_div_max) {
        log_hal_i3c_error("[hal][i3c] calc freq: hs(%d), clk_div(%u), smp_div_cnt(%u), stp_div_cnt(%u), temp(%u), scl_tm(%lf)\r\n", 6, is_hs, clock_div, smp_cnt_div, stp_cnt_div, temp,scl_tm_ns);
        return false;
    } else {
        *clk_div        = clock_div;
        *sample_cnt_div = smp_cnt_div;
        *step_cnt_div   = stp_cnt_div;
        return true;
    }
}



static bool    _i3c_calc_speed_with_duty(uint32_t speed, bool is_hs,
    uint8_t   duty,
    uint32_t *clk_div,
    uint32_t *sample_cnt_div,
    uint32_t *step_cnt_div)
{
    uint32_t  clock_div   = 0, clock_div_max;  /* 5bit:0x1F */
    uint32_t  smp_cnt_div = 0, smp_cnt_div_max;  /* 3bit:0x07 */
    uint32_t  stp_cnt_div = 0, stp_cnt_div_max;  /* 6bit:0x3F */
    uint32_t temp;

    if (is_hs) {
        clock_div_max   = 0x1F;
        smp_cnt_div_max = 0x07;
        stp_cnt_div_max = 0x07;
    } else {
        clock_div_max   = 0x1F;
        smp_cnt_div_max = 0x07;
        stp_cnt_div_max = 0x3F;
    }

    for (clock_div = 0; clock_div < clock_div_max; clock_div++) {
        temp = I3C_CLOCK_FREQUENCY/((clock_div+1) * speed);
        temp = (temp * duty)/100;
        for (smp_cnt_div = 0; smp_cnt_div < smp_cnt_div_max; smp_cnt_div++) {
            stp_cnt_div = (temp / (smp_cnt_div + 1)) - 1;
            if (stp_cnt_div < stp_cnt_div_max){
                break;
            }
        }
        if (stp_cnt_div < stp_cnt_div_max) {
            break;
        }
    }
    if (clock_div > clock_div_max || smp_cnt_div > smp_cnt_div_max || stp_cnt_div > stp_cnt_div_max) {
        log_hal_i3c_error("[hal][i3c] calc freq: hs(%d), clk_div(%d), smp_div_cnt(%d), stp_div_cnt(%d)\r\n", 4, is_hs, clock_div, smp_cnt_div, stp_cnt_div);
        return false;
    } else {
        *clk_div        = clock_div;
        *sample_cnt_div = smp_cnt_div;
        *step_cnt_div   = stp_cnt_div;
        return true;
    }
}



hal_i3c_master_status_t i3c_config_i3c_speed(hal_i3c_master_port_t  i3c_port, uint32_t i3c_speed, uint32_t i3c_hs_speed)
{
    volatile I3C_MASTER_REGISTER_T *i3c_reg= NULL;
    uint32_t  clock_div   = 0;
    uint32_t  smp_cnt_div = 0;  /* 3bit:0x07 */
    uint32_t  stp_cnt_div = 0;  /* 6bit:0x3F */
    bool      res;
    float     clk_cyc_ns = 0;
    uint32_t  v_hs,v_clkdiv,v_ltime,v_htime;

    i3c_reg = s_i3c_base_register[i3c_port];
    if (i3c_speed >= I3C_CLOCK_FREQUENCY || i3c_hs_speed >= I3C_CLOCK_FREQUENCY) {
        log_hal_i3c_error("[hal][i3c%d] i3c freq invalid(freq:%dHz, hs_freq:%dHz)\r\n", 2, i3c_port, i3c_speed, i3c_hs_speed);
        return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
    }
    v_clkdiv = i3c_reg->CLOCK_DIV;
    v_hs     = i3c_reg->HS;
    v_ltime  = i3c_reg->L_TIMING;
    v_htime  = i3c_reg->H_TIMING;

    if (i3c_speed != 0) {
        clk_cyc_ns = 1000000.0f / i3c_speed;
        /* clear bits*/
        v_clkdiv &= ~I3C_CLOCK_DIV_SMP_DIV_MASK;
        v_ltime  &= ~(I3C_LTIMING_LSMAPLE_CNT_DIV_MASK | I3C_LTIMING_LSTEP_CNT_DIV_MASK);
        v_htime  &= ~(I3C_HTIMING_HSAMPLE_CNT_DIV_MASK | I3C_HTIMING_HSTEP_CNT_DIV_MASK);

        /* config scl low */
        res = _i3c_calc_speed_with_time(i3c_speed, false, (clk_cyc_ns - I3C_SCL_HS_HIGH_TM_NS), &clock_div, &smp_cnt_div, &stp_cnt_div);
        if (res == false) {
            log_hal_i3c_error("[hal][i3c%d] i3c freq invalid(l):%dHz\r\n", 2, i3c_port, i3c_speed);
            return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
        }
        v_clkdiv |= ((clock_div<<I3C_CLOCK_DIV_SMP_DIV_OFFFSET) & I3C_CLOCK_DIV_SMP_DIV_MASK);
        v_ltime  |= ((smp_cnt_div<<I3C_LTIMING_LSMAPLE_CNT_DIV_OFFSET) | (stp_cnt_div<<I3C_LTIMING_LSTEP_CNT_DIV_OFFSET));
        log_hal_i3c_debug("[hal][i3c%d] i3c scl low:  clk_idv(%d), sample cnt(%d), step_cnt(%d)\r\n", 4, i3c_port, clock_div, smp_cnt_div, stp_cnt_div);

        /* config scl high */
        res = _i3c_calc_speed_with_time(i3c_speed, false, I3C_SCL_HS_HIGH_TM_NS, &clock_div, &smp_cnt_div, &stp_cnt_div);
        if (res == false) {
            log_hal_i3c_error("[hal][i3c%d] i3c freq invalid(h):%dHz\r\n", 2, i3c_port, i3c_speed);
            return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
        }
        v_htime |= ((smp_cnt_div<<I3C_HTIMING_HSAMPLE_CNT_DIV_OFFSET) | (stp_cnt_div<<I3C_HTIMING_HSTEP_CNT_DIV_OFFSET));
        log_hal_i3c_debug("[hal][i3c%d] i3c scl high: clk_idv(%d), sample cnt(%d), step_cnt(%d)\r\n", 4, i3c_port, clock_div, smp_cnt_div, stp_cnt_div);
    }

    if (i3c_hs_speed != 0) {
        clk_cyc_ns = 1000000.0f / i3c_hs_speed;

        /* clear bits*/
        v_clkdiv &= ~I3C_CLOCK_DIV_HS_DIV_MASK;
        v_ltime  &= ~(I3C_LTIMING_LHS_SAMPLE_CNT_DIV_MASK | I3C_LTIMING_LHS_STEP_CNT_DIV_MASK);
        v_hs     &= ~(I3C_HS_SAMPLE_CNT_DIV_MASK | I3C_HS_STEP_CNT_DIV_MASK);
        /* config scl low */
        res = _i3c_calc_speed_with_time(i3c_hs_speed, true, (clk_cyc_ns - I3C_SCL_HS_HIGH_TM_NS), &clock_div, &smp_cnt_div, &stp_cnt_div);
        if (res == false || smp_cnt_div >= 0x7 || stp_cnt_div >= 0x7) {
            log_hal_i3c_error("[hal][i3c%d] i3c hs freq invalid(l):%dHz\r\n", 2, i3c_port, i3c_hs_speed);
            return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
        }
        v_clkdiv |= (clock_div<<I3C_CLOCK_DIV_HS_DIV_OFFFSET) & I3C_CLOCK_DIV_HS_DIV_MASK;
        v_ltime  |= ((smp_cnt_div<<I3C_LTIMING_LHS_SAMPLE_CNT_DIV_OFFSET) | (stp_cnt_div<<I3C_LTIMING_LHS_STEP_CNT_DIV_OFFSET));
        log_hal_i3c_debug("[hal][i3c%d] i3c hs scl low:  clk_idv(%d), sample cnt(%d), step_cnt(%d)\r\n", 4, i3c_port, clock_div, smp_cnt_div, stp_cnt_div);
        /* config scl high */
        res = _i3c_calc_speed_with_time(i3c_hs_speed, true, I3C_SCL_HS_HIGH_TM_NS, &clock_div, &smp_cnt_div, &stp_cnt_div);
        if (res == false || smp_cnt_div >= 0x7 || stp_cnt_div >= 0x7) {
            log_hal_i3c_error("[hal][i3c%d] i3c hs freq invalid(h):%dHz\r\n", 2, i3c_port, i3c_hs_speed);
            return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
        }
        v_hs |= ((smp_cnt_div<<I3C_HS_SAMPLE_CNT_DIV_OFFSET) | (stp_cnt_div<<I3C_HS_STEP_CNT_DIV_OFFSET));
        log_hal_i3c_debug("[hal][i3c%d] i3c hs scl high: clk_idv(%d), sample cnt(%d), step_cnt(%d)\r\n", 4, i3c_port, clock_div, smp_cnt_div, stp_cnt_div);
    }

    if (i3c_speed != 0 || i3c_hs_speed != 0) {
        i3c_reg->CLOCK_DIV = v_clkdiv;
        i3c_reg->L_TIMING  = v_ltime;
        i3c_reg->H_TIMING  = v_htime;
        i3c_reg->HS        = I3C_HS_FMT(v_hs);
    }
    return HAL_I3C_MASTER_STATUS_OK;

}


hal_i3c_master_status_t i3c_config_i2c_speed(hal_i3c_master_port_t  i3c_port, uint32_t i3c_speed, uint32_t i3c_hs_speed)
{
    volatile I3C_MASTER_REGISTER_T *i3c_reg= NULL;
    uint32_t  clock_div   = 0;
    uint32_t  smp_cnt_div = 0;  /* 3bit:0x07 */
    uint32_t  stp_cnt_div = 0;  /* 6bit:0x3F */
    bool     res;
    uint32_t  v_hs,v_clkdiv,v_ltime,v_htime;

    i3c_speed    = i3c_speed *1000;
    i3c_hs_speed = i3c_hs_speed * 1000;
    i3c_reg = s_i3c_base_register[i3c_port];
    if (i3c_speed >= I3C_CLOCK_FREQUENCY || i3c_hs_speed >= I3C_CLOCK_FREQUENCY) {
        log_hal_i3c_error("[hal][i3c%d] freq invalid(freq:%dHz, hs_freq:%dHz)\r\n", 2, i3c_port, i3c_speed, i3c_hs_speed);
        return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
    }
    v_clkdiv = i3c_reg->CLOCK_DIV;
    v_hs     = i3c_reg->HS;
    v_ltime  = i3c_reg->L_TIMING;
    v_htime  = i3c_reg->H_TIMING;

    if (i3c_speed != 0) {
        /* clear bits*/
        v_clkdiv &= ~I3C_CLOCK_DIV_SMP_DIV_MASK;
        v_ltime  &= ~(I3C_LTIMING_LSMAPLE_CNT_DIV_MASK | I3C_LTIMING_LSTEP_CNT_DIV_MASK);
        v_htime  &= ~(I3C_HTIMING_HSAMPLE_CNT_DIV_MASK | I3C_HTIMING_HSTEP_CNT_DIV_MASK);

        /* config scl low */
        res = _i3c_calc_speed_with_duty(i3c_speed, 0, (100 - I3C_SCL_PERIOD_DUTY), &clock_div, &smp_cnt_div, &stp_cnt_div);
        if (res == false) {
            log_hal_i3c_error("[hal][i3c%d] freq invalid(l):%dHz\r\n", 2, i3c_port, i3c_speed);
            return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
        }
        v_clkdiv |= ((clock_div<<I3C_CLOCK_DIV_SMP_DIV_OFFFSET) & I3C_CLOCK_DIV_SMP_DIV_MASK);
        v_ltime  |= ((smp_cnt_div<<I3C_LTIMING_LSMAPLE_CNT_DIV_OFFSET) | (stp_cnt_div<<I3C_LTIMING_LSTEP_CNT_DIV_OFFSET));
        log_hal_i3c_debug("[hal][i3c%d] scl low:  clk_idv(%d), sample cnt(%d), step_cnt(%d)\r\n", 4, i3c_port, clock_div, smp_cnt_div, stp_cnt_div);

        /* config scl high */
        res = _i3c_calc_speed_with_duty(i3c_speed, 0, I3C_SCL_PERIOD_DUTY, &clock_div, &smp_cnt_div, &stp_cnt_div);
        if (res == false) {
            log_hal_i3c_error("[hal][i3c%d] freq invalid(h):%dHz\r\n", 2, i3c_port, i3c_speed);
            return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
        }
        v_htime |= ((smp_cnt_div<<I3C_HTIMING_HSAMPLE_CNT_DIV_OFFSET) | (stp_cnt_div<<I3C_HTIMING_HSTEP_CNT_DIV_OFFSET));
        log_hal_i3c_debug("[hal][i3c%d] scl high: clk_idv(%d), sample cnt(%d), step_cnt(%d)\r\n", 4, i3c_port, clock_div, smp_cnt_div, stp_cnt_div);
    }

    if (i3c_hs_speed != 0) {
        /* clear bits*/
        v_ltime  &= ~(I3C_LTIMING_LHS_SAMPLE_CNT_DIV_MASK | I3C_LTIMING_LHS_STEP_CNT_DIV_MASK);
        v_hs     &= ~(I3C_HS_SAMPLE_CNT_DIV_MASK | I3C_HS_STEP_CNT_DIV_MASK);
        v_clkdiv &= ~I3C_CLOCK_DIV_HS_DIV_MASK;

        /* config scl low */
        res = _i3c_calc_speed_with_duty(i3c_hs_speed, 1, (100 - I3C_SCL_PERIOD_DUTY), &clock_div, &smp_cnt_div, &stp_cnt_div);
        if (res == false || smp_cnt_div >= 0x7 || stp_cnt_div >= 0x7) {
            log_hal_i3c_error("[hal][i3c%d] hs freq invalid(l):%dHz\r\n", 2, i3c_port, i3c_hs_speed);
            return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
        }
        v_clkdiv |= (clock_div<<I3C_CLOCK_DIV_HS_DIV_OFFFSET) & I3C_CLOCK_DIV_HS_DIV_MASK;
        v_ltime  |= ((smp_cnt_div<<I3C_LTIMING_LHS_SAMPLE_CNT_DIV_OFFSET) | (stp_cnt_div<<I3C_LTIMING_LHS_STEP_CNT_DIV_OFFSET));
        log_hal_i3c_debug("[hal][i3c%d] hs scl low:  clk_idv(%d), sample cnt(%d), step_cnt(%d)\r\n", 4, i3c_port, clock_div, smp_cnt_div, stp_cnt_div);
        /* config scl high */
        res = _i3c_calc_speed_with_duty(i3c_hs_speed, 1, I3C_SCL_PERIOD_DUTY, &clock_div, &smp_cnt_div, &stp_cnt_div);
        if (res == false || smp_cnt_div >= 0x7 || stp_cnt_div >= 0x7) {
            log_hal_i3c_error("[hal][i3c%d] hs freq invalid(h):%dHz\r\n", 2, i3c_port, i3c_hs_speed);
            return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
        }
        v_hs |= ((smp_cnt_div<<I3C_HS_SAMPLE_CNT_DIV_OFFSET) | (stp_cnt_div<<I3C_HS_STEP_CNT_DIV_OFFSET));
        log_hal_i3c_debug("[hal][i3c%d] hs scl high: clk_idv(%d), sample cnt(%d), step_cnt(%d)\r\n", 4, i3c_port, clock_div, smp_cnt_div, stp_cnt_div);
    }
    if (i3c_speed != 0 || i3c_hs_speed != 0) {
        i3c_reg->CLOCK_DIV = v_clkdiv;
        i3c_reg->L_TIMING  = v_ltime;
        i3c_reg->H_TIMING  = v_htime;
        i3c_reg->HS        = I3C_HS_FMT(v_hs);
    }

    return HAL_I3C_MASTER_STATUS_OK;
}



void    i3c_config_timing(hal_i3c_master_port_t  i3c_port, uint32_t t_min_timing_ns)
{
    volatile I3C_MASTER_REGISTER_T *i3c_reg= NULL;
    uint32_t tm_max = 0;
    uint32_t ext_tm,hs_ext_tm;
    uint32_t clk_unit;

    i3c_reg = s_i3c_base_register[i3c_port];
    if (t_min_timing_ns == 0) {
        i3c_reg->EXT_CONF = (4<<I3C_EXT_CONFIG_EXT_TIME_OFFSET) | I3C_EXT_CONFIG_EXT_EN_MASK;
        return;
    }

    clk_unit = 1000000000/(I3C_CLOCK_FREQUENCY/(i3c_reg->CLOCK_DIV + 1));
    tm_max   = t_min_timing_ns;
    ext_tm   = (uint32_t)(tm_max/clk_unit);

    if (ext_tm > 0xFF) {
        ext_tm = 0xFF;
    }
    log_hal_i3c_debug("[hal][i3c%d] config timing %d = %d/%d\r\n", 4, i3c_port,ext_tm, tm_max, clk_unit);
    hs_ext_tm = (uint32_t)(tm_max/clk_unit);
    if (hs_ext_tm > 0x7F) {
        hs_ext_tm = 0x7F;
    }
    log_hal_i3c_debug("[hal][i3c%d] config hs timing %d = %d/%d\r\n", 4, i3c_port, ext_tm, tm_max, clk_unit);
    i3c_reg->EXT_CONF =  ((ext_tm<<I3C_EXT_CONFIG_EXT_TIME_OFFSET)& I3C_EXT_CONFIG_EXT_TIME_MASK) | \
                         (I3C_EXT_CONFIG_EXT_EN_MASK) | \
                         ((hs_ext_tm<<I3C_EXT_CONFIG_HS_EXT_TIME_OFFSET) & I3C_EXT_CONFIG_HS_EXT_TIME_MASK);

}


void    i3c_config_io(hal_i3c_master_port_t  i3c_port,hal_i3c_io_config_t config)
{
    volatile I3C_MASTER_REGISTER_T *base_reg = s_i3c_base_register[i3c_port];

    if (config == HAL_I3C_IO_CONFIG_OPENDRAIN) {
        base_reg->IO_CONFIG |=  (I3C_IO_CONFIG_SCL_MASK | I3C_IO_CONFIG_SDA_MASK);
    } else {
        base_reg->IO_CONFIG &= ~(I3C_IO_CONFIG_SCL_MASK | I3C_IO_CONFIG_SDA_MASK);
    }
}



int    i3c_config_fifo(hal_i3c_master_port_t  i3c_port, i3c_fifo_op_t  op, uint8_t *buff, uint32_t size)
{
    volatile I3C_MASTER_REGISTER_T *base_reg = s_i3c_base_register[i3c_port];
    uint32_t i = 0;

    /* if op != I3C_FIFO_CLEAR, need check the buffer and size*/
    if (op != I3C_FIFO_CLEAR && (buff == NULL || size == 0)) {
        return -1;
    }
    switch(op){
        case I3C_FIFO_WRITE: {
            for ( i = 0; i < size; i++) {
                base_reg->DATA_PORT = buff[i];
            }
        }break;

        case I3C_FIFO_READ: {
            for ( i = 0; i < size; i++) {
                buff[i] = base_reg->DATA_PORT;
            }
        }break;

        case I3C_FIFO_CLEAR:{
            base_reg->FIFO_ADDR_CLR = 0x3;
        }break;
    }
    return i;
}



int     i3c_config_hardware(hal_i3c_master_port_t  i3c_port, i3c_master_transfer_config_t *config)
{
    volatile I3C_MASTER_REGISTER_T *i3c_reg = s_i3c_base_register[i3c_port];
    uint32_t    v_ctrl = 0,v_slv = 0,v_traffic = 0,v_shape = 0,v_hs = 0;

    /*disable i3c dma function*/
    i3c_reg->CONTROL = 0;
    i3c_reg->FIFO_ADDR_CLR = 0x3;
    /* if is dma mode, first enable pdma */
    if ((config->o_flag & HAL_I3C_FLAG_FIFO_MODE_MASK) == 0) {
        i3c_pdma_deinit(i3c_port);
    }
    v_ctrl = I3C_CONTROL_RS_STOP_MASK | I3C_CONTROL_ACKERR_DET_EN_MASK;
    v_hs   = i3c_reg->HS;

    switch (config->trans_mode) {
        case HAL_I3C_MASTER_TRANSFER_TYPE_SEND_TO_RECEIVE: {
                i3c_reg->TRANSAC_LEN      = 2;
                i3c_reg->TRANSFER_LEN     = config->send_size;
                i3c_reg->TRANSFER_LEN_AUX = config->recv_size;
                v_ctrl |= I3C_CONTROL_DIR_CHANGE_MASK | I3C_CONTROL_TRANSFER_LEN_CHANGE_MASK;
                v_slv   = (config->slave_addr << 1);
            }break;
        case HAL_I3C_MASTER_TRANSFER_TYPE_SEND: {
                i3c_reg->TRANSAC_LEN      = 1;
                i3c_reg->TRANSFER_LEN     = config->send_size;
                i3c_reg->TRANSFER_LEN_AUX = 0;
                v_slv  = (config->slave_addr << 1);
            }break;
        case HAL_I3C_MASTER_TRANSFER_TYPE_RECEIVE: {
                i3c_reg->TRANSAC_LEN      = 1;
                i3c_reg->TRANSFER_LEN     = config->recv_size;
                i3c_reg->TRANSFER_LEN_AUX = 0;
                v_slv  = (config->slave_addr << 1) | 0x1;
            }break;
        case HAL_I3C_TRANSFER_TYPE_DAA: {
                i3c_reg->TRANSAC_LEN      = HAL_I3C_MASTER_MAX_SLAVE_DEVICE_NUM;
                i3c_reg->TRANSFER_LEN     = 9;
                i3c_reg->TRANSFER_LEN_AUX = 0;
                v_traffic                |= I3C_TRAFFIC_DAA_EN_MASK;
                v_slv                     = (0x7E << 1) | 0x1;
            }break;
    }

    if (config->o_flag & HAL_I3C_FLAG_I2C_MODE_MASK) {
        /* i2c mode config */
        i3c_reg->HS = I3C_HS_FMT(~(I3C_HS_EN_MASK | I3C_HS_SPEED_MASK) & i3c_reg->HS);;
    } else {
        /* i3c mode config */
        if (config->trans_mode == HAL_I3C_TRANSFER_TYPE_DAA) {
            v_traffic |= I3C_TRAFFIC_HANDOFF_MASK;
            v_hs      |= I3C_HS_NACKERR_DET_EN_MASK;
            v_hs      &= ~(I3C_HS_SPEED_MASK);
        } else {
            v_traffic |= (I3C_TRAFFIC_HANDOFF_MASK | I3C_TRAFFIC_TBIT_EN_MASK);
            v_hs      |= I3C_HS_SPEED_MASK;
        }
        v_hs       |= (I3C_HS_EN_MASK | I3C_HS_NACKERR_DET_EN_MASK);
        v_shape    |= (I3C_SHAPE_TBIT_STALL_MASK | I3C_SHAPE_TBIT_PARITY_MASK);
        i3c_reg->HS = I3C_HS_FMT(I3C_HS_EN_MASK  | v_hs);

        /* config i3c pricate rw*/
        if (config->o_flag & HAL_I3C_FLAG_PRIVATE_MASK) {
            v_hs &= ~(I3C_HS_NACKERR_DET_EN_MASK);
            i3c_reg->HFIFO_DATA = (I3C_HFIFO_WR_EN_MASK | I3C_HFIFO_9BIT_NACK | (0x7E << 1));/* 0x7E + W */
            log_hal_i3c_debug("[hal][i3c%d] i3c_config_hardware: private", 1, i3c_port);
        } else {
            i3c_reg->HFIFO_DATA = (I3C_HFIFO_WR_EN_MASK | I3C_HFIFO_9BIT_ACK |(0x7E << 1));/* 0x7E + W */
            log_hal_i3c_debug("[hal][i3c%d] i3c_config_hardware: boardcast", 1, i3c_port);
        }
        if (config->o_flag & HAL_I3C_FLAG_CCC_EN_MASK) {
            int i = 0;
            v_hs &= ~(I3C_HS_SPEED_MASK);
            i3c_reg->HFIFO_DATA = (I3C_HFIFO_WR_EN_MASK | I3C_HFIFO_9BIT_PARITY | I3C_HFIFO_HS_SPEED_MASK| (config->ccc_cmd));/* CCC + TBit */
            for (i = 0; i < config->ccc_append_data_sz; i++) {
              i3c_reg->HFIFO_DATA = (I3C_HFIFO_WR_EN_MASK | I3C_HFIFO_9BIT_PARITY | I3C_HFIFO_HS_SPEED_MASK | (config->ccc_append_data[i]));/* CCC + TBit */
            }
        }
    }
    if (config->o_flag & HAL_I3C_FLAG_FIFO_MODE_MASK) {
        i3c_config_fifo(i3c_port, I3C_FIFO_WRITE, config->send_buff, config->send_size);
    } else {
        if (i3c_pdma_init(i3c_port, config) < 0 ) {
            return HAL_I3C_MASTER_STATUS_ERROR;
        }
        if (config->trans_mode != HAL_I3C_TRANSFER_TYPE_DAA) {
            v_ctrl |= I3C_CONTROL_DMA_EN_MASK;
        }
    }
    i3c_reg->HS         = I3C_HS_FMT(v_hs);
    i3c_reg->SLAVE_ADDR = v_slv;
    i3c_reg->CONTROL    = v_ctrl;
    i3c_reg->TRAFFIC    = v_traffic;
    i3c_reg->SHAPE      = v_shape;
    i3c_reg->DELAY_LEN  = 0xa;
    i3c_reg->EXT_CONF   = (0x40 << I3C_EXT_CONFIG_EXT_TIME_OFFSET) | (0x20 << I3C_EXT_CONFIG_HS_EXT_TIME_OFFSET) | I3C_EXT_CONFIG_EXT_EN_MASK;

    log_hal_i3c_debug("[hal][i3c%d] i3c_config_hardware: fifo stat %x, hfifo %x", 3, i3c_port, i3c_reg->FIFO_STAT, i3c_reg->HFIFO_STAT);
    return HAL_I3C_MASTER_STATUS_OK;
}



void    i3c_hardware_reset(hal_i3c_master_port_t  i3c_port)
{
    volatile I3C_MASTER_REGISTER_T *i3c_reg = s_i3c_base_register[i3c_port];
#if 1
    i3c_reg->SOFT_RESET = I3C_SOFTRESET_SOFT_RESET_MASK  |
                          I3C_SOFTRESET_FSM_RESET_MASK   |
                          I3C_SOFTRESET_GRAIN_RESET_MASK |
                          I3C_SOFTRESET_ERROR_RESET_MASK;
    i3c_reg->SOFT_RESET = 0;
#endif
    i3c_reg->CONTROL    = 0;
    i3c_reg->DELAY_LEN  = 0;
    i3c_reg->EXT_CONF   = 0x0405;
    i3c_reg->DELAY_LEN  = 0xa;
    i3c_reg->START      = 0;
    i3c_reg->DEF_DA     = 0;
}

int     i3c_get_port_by_nvic_id(hal_nvic_irq_t irq_number)
{
#ifdef I3C_NVIC_ENABLED
    hal_i3c_master_port_t   i3c_port;

    for (i3c_port = 0; i3c_port < HAL_I3C_MASTER_MAX; i3c_port++) {
        if (irq_number == s_i3c_irq_num_table[i3c_port]) {
            break;
        }
    }
    return i3c_port;
#else
    (void)irq_number;
    return 0;
#endif
}

void    i3c_config_irq(hal_i3c_master_port_t  i3c_port, hal_nvic_isr_t callback)
{
#ifdef HAL_NVIC_MODULE_ENABLED
    hal_nvic_register_isr_handler(s_i3c_irq_num_table[i3c_port], callback);
    hal_nvic_enable_irq(s_i3c_irq_num_table[i3c_port]);
#else
    (void) i3c_port;
    (void) callback;
#endif
}


int     i3c_opt_ioctl(hal_i3c_master_port_t  i3c_port, uint32_t cmd, uint32_t option)
{
    volatile I3C_MASTER_REGISTER_T  *base_reg = s_i3c_base_register[i3c_port];
    uint32_t                result  = 0;
    switch(cmd){
        case I3C_IOCTL_START:             base_reg->START = (option | I3C_START_EN_START_MASK); break;
        case I3C_IOCTL_GET_BUSY_STAT:     result = base_reg->START & 0x1;   break;
        case I3C_IOCTL_GET_IRQ_STAT:      result = base_reg->INTR_STA;      break;
        case I3C_IOCTL_CLR_IRQ_STAT:      base_reg->INTR_STA   = 0xFFFF;    break;
        case I3C_IOCTL_SET_IRQ_MASK:      base_reg->INTR_MASK  = option;    break;
        case I3C_IOCTL_SET_SLV_RADDR:
                base_reg->SLAVE_ADDR = ((option<<1) | 0x1) & 0xFF;
            break;
        case I3C_IOCTL_SET_SLV_WADDR:
                base_reg->SLAVE_ADDR = (option & 0x7F)<<1;
            break;
        case I3C_IOCTL_GET_SLV_ADDR:
                result = (base_reg->SLAVE_ADDR)>>1;
            break;
        case I3C_IOCTL_GET_FIFO_STAT:
                result = base_reg->FIFO_STAT;
            break;
        case I3C_IOCTL_GET_ERROR_STAT:
                result = base_reg->ERROR;
            break;
        case I3C_IOCTL_SET_DEF_ADDR:
                base_reg->DEF_DA = (1<<8) |(1<<7) | (option & 0x7F);
            break;
#ifdef HAL_CLOCK_MODULE_ENABLED
        case I3C_IOCTL_ENABLE_CLOCK:{
                if (option) {
                    hal_clock_enable(HAL_CLOCK_CG_I3C);
                    hal_clock_enable(s_i3c_clk_pdn_table[i3c_port]);
                    clock_mux_sel(CLK_I3C_SEL, 1);
                } else {
                    hal_clock_disable(s_i3c_clk_pdn_table[i3c_port]);
                }
            }break;
#endif
#ifdef HAL_SLEEP_MANAGER_ENABLED
        case I3C_IOCTL_LOCK_SLEEP:{
                static bool lock_st = false;
                if (option && (false == lock_st)) {
                    hal_sleep_manager_lock_sleep(s_i3c_sleep_handle[i3c_port]);
                    lock_st = true;
                } else if (option == false && (true == lock_st)){
                    hal_sleep_manager_unlock_sleep(s_i3c_sleep_handle[i3c_port]);
                    lock_st = false;
                }
            }break;
#endif
        case I3C_IOCTL_SOFTRESET:{
                base_reg->SOFT_RESET = (0x3 << 4);
                base_reg->START      = 0;
                base_reg->SOFT_RESET = 0x00;
            }break;
        case I3C_IOCTL_GET_DEF_DA: {
                result = base_reg->DEF_DA & 0x7F;
            }break;
        case I3C_IOCTL_SET_DEF_DA: {
                base_reg->DEF_DA = (option & 0x7F) | (1 << 7);
            }break;
        default:    break;
    }
    return result;
}



inline bool    i3c_hw_is_busy(hal_i3c_master_port_t  i3c_port)
{
    /*HW is idle is safe for setting */
    if (i3c_opt_ioctl(I3C_IOCTL_GET_BUSY_STAT, 0 ,0)) {
        return true;
    } else {
        return false;
    }
}

hal_i3c_master_callback_event_t    i3c_get_event_type(uint32_t irq_status, uint32_t err_status)
{
    if(irq_status == 0x10){
        return HAL_I3C_EVENT_DAA;
    } else if(irq_status == 0x11) {
        return HAL_I3C_EVENT_SUCCESS;
    }

    if (irq_status & I3C_INTR_MASK_ACKERR_MASK) {
        return HAL_I3C_EVENT_ACK_ERROR;
    } else if (irq_status & I3C_INTR_MASK_HS_NACKERR_MASK) {
        return HAL_I3C_EVENT_NACK_ERROR;
    } else if (irq_status & I3C_INTR_MASK_ARB_LOST_MASK) {
        return HAL_I3C_EVENT_ARB_LOST;
    } else if (irq_status & I3C_INTR_MASK_TIMEOUT_MASK) {
        return HAL_I3C_EVENT_TIMEOUT;
    } else if (irq_status & I3C_INTR_MASK_MDA_ERR_MASK) {
        return HAL_I3C_EVENT_DMA_ERROR;
    } else if (irq_status & I3C_INTR_MASK_IBI_MASK) {
        return HAL_I3C_EVENT_IBI;
    }
    return HAL_I3C_EVENT_SUCCESS;
}





int    i3c_wait_status_with_timeout(hal_i3c_master_port_t i3c_port, uint32_t ms)
{
    volatile I3C_MASTER_REGISTER_T  *base_reg = s_i3c_base_register[i3c_port];
    uint32_t  tick0,tick1,tick_dur, timeout_us;
    int ret = 0;

    timeout_us = ms * 1000;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &tick0);
    while (1) {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &tick1);
        hal_gpt_get_duration_count(tick0, tick1, &tick_dur);
        if (base_reg->INTR_STA != 0) {
            ret = 0;
            break;
        } else if (tick_dur > timeout_us) {
            ret = -1;
            break;
        }
    }
    return ret;
}




void    hal_i3c_master_dump(hal_i3c_master_port_t  i3c_port)
{
    volatile I3C_MASTER_REGISTER_T *base_reg = s_i3c_base_register[i3c_port];
    volatile I2C_PDMA_REGISTER_T   *pdma_reg = s_i3c_pdma_register[i3c_port];

    log_hal_i3c_info("===================== I3C%d REGS Dump(0x%x) =====================\r\n", 2, i3c_port, base_reg);
    log_hal_i3c_info("SLV_ADDR:%x, INTR_MSK:%x, INTR_STA:%x,   CONTROL:%x\r\n", 4,
                base_reg->SLAVE_ADDR,   base_reg->INTR_MASK,
                base_reg->INTR_STA,     base_reg->CONTROL
    );
    log_hal_i3c_info("TRSF_LEN:%x, TRSF_AUX:%x, TRSACT_LEN:%x, DLY_LEN:%x\r\n", 4,
                base_reg->TRANSFER_LEN, base_reg->TRANSFER_LEN_AUX,
                base_reg->TRANSAC_LEN,  base_reg->DELAY_LEN
    );
    log_hal_i3c_info("HTIME:%x,    LTIME:%x,    HS:%x,         START:%x\r\n", 4,
                base_reg->H_TIMING,     base_reg->L_TIMING,
                base_reg->HS,           base_reg->START
    );
    log_hal_i3c_info("EXT_CNF:%x,  IO_CNF:%x,   CLK_DIV:%x,    TRAFFIC:%x\r\n", 4,
                base_reg->EXT_CONF,     base_reg->IO_CONFIG,
                base_reg->CLOCK_DIV,    base_reg->TRAFFIC
    );
    log_hal_i3c_info("SHAPE:%x,     DAT_TM:%x,     TMOUT_DIV:%x,   DEF_DA:%x\r\n", 4,
                base_reg->SHAPE,        base_reg->DATA_TIMING,
                base_reg->TIMEOUT_DIV,  base_reg->DEF_DA
    );
    log_hal_i3c_info("FIFO_STA:%x,  SOFT_RESET:%x, DMA_INFO:%x,    ERROR:%x\r\n", 4,
                base_reg->FIFO_STAT,    base_reg->SOFT_RESET,
                base_reg->DMA_INFO,     base_reg->ERROR
    );
    log_hal_i3c_info("IBI_TM:%x,    MCU_INTR:%x,   TIMEOUT_DIV:%x,  CMD:%x\r\n", 4,
                base_reg->IBI_TIMING,       base_reg->MCU_INTR,
                base_reg->TIMEOUT_DIV,      base_reg->COMMAND
    );
    log_hal_i3c_info("CRC_CODE:%x,  TERNARY:%x,    IBI_TM:%x,       SYS_LATCH:%x\r\n", 4,
                base_reg->CRC_CODE,      base_reg->TERNARY,
                base_reg->IBI_TIMING,    base_reg->SYSTIME_LATCH
    );
    log_hal_i3c_info("AED_PATCH:%x, DLY_STEP:%x,   DLY_SAMPLE:%x,   DMA_INFO:%x\r\n", 4,
                base_reg->AED_PATCH,      base_reg->DELAY_STEP,
                base_reg->DELAY_SAMPLE,   base_reg->DMA_INFO
    );

    log_hal_i3c_info("IRQ_INFO:%x, DBG_STAT:%x, DBG_CTRL:%x, MULTIMAS:%x\r\n", 4,
                base_reg->IRQ_INFO,      base_reg->DEBUG_STAT,
                base_reg->DEBUG_CTRL,    base_reg->MULTIMAS
    );
    log_hal_i3c_info("FIFO_THRESH:%x, HFIFO_STAT:%x, DBG_CTRL:%x, MULTIMAS:%x\r\n", 4,
                base_reg->FIFO_THRESH,      base_reg->HFIFO_STAT,
                base_reg->DEBUG_CTRL,    base_reg->MULTIMAS
    );

    log_hal_i3c_info("===================== I3C%d PDMA Dump(0x%x) =====================\r\n", 2, i3c_port, pdma_reg);
    log_hal_i3c_info("GLB->STA:%x, CPU0_CFG:%x, BUSY:%x, INTR:%x, CLK_CFG:%x\r\n", 5,
                pdma_reg->GLB_STA,   pdma_reg->GLB_CPU0_CFG,
                pdma_reg->GLB_BUSY,  pdma_reg->GLB_INTR,
                pdma_reg->GLB_CLK_CFG
    );

    log_hal_i3c_info("PDMA1-> CON:%x, COUNT:%x, START:%x, INTR_STA:%x, PGMADR:%x, RLCT:%x\r\n", 6,
                pdma_reg->PDMA1_CON,    pdma_reg->PDMA1_COUNT,
                pdma_reg->PDMA1_START,  pdma_reg->PDMA1_INTSTA,
                pdma_reg->PDMA1_PGMADDR,pdma_reg->PDMA1_RLCT
    );
    log_hal_i3c_info("PDMA2-> CON:%x, COUNT:%x, START:%x, INTR_STA:%x, PGMADR:%x, RLCT:%x\r\n", 6,
                pdma_reg->PDMA2_CON,    pdma_reg->PDMA2_COUNT,
                pdma_reg->PDMA2_START,  pdma_reg->PDMA2_INTSTA,
                pdma_reg->PDMA2_PGMADDR,pdma_reg->PDMA1_RLCT
    );
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int    i3c_pdma_init(hal_i3c_master_port_t i3c_port, i3c_master_transfer_config_t *config)
{
    pdma_status_t   dma_status;
    pdma_config_t   dma_config;
    bool            tx_enable = false, rx_enable = false;

    dma_config.burst_mode = false;
    dma_config.size       = PDMA_BYTE;

    if (config->trans_mode == HAL_I3C_MASTER_TRANSFER_TYPE_SEND_TO_RECEIVE) {
        tx_enable = true;
        rx_enable = true;
    } else if(config->trans_mode == HAL_I3C_MASTER_TRANSFER_TYPE_SEND) {
        tx_enable = true;
    } else if(config->trans_mode == HAL_I3C_MASTER_TRANSFER_TYPE_RECEIVE) {
        rx_enable = true;
    }

    if (tx_enable) {
        log_hal_i3c_debug("pdma tx--->mem 0x%x, size %d", 2, config->send_buff, config->send_size);
        log_hal_i3c_debug("[hal][i3c%d] tx pdma mem:%x %x %x %x\r\n", 5, i3c_port,
                            config->send_buff[0], config->send_buff[1],
                            config->send_buff[2], config->send_buff[3]);

        /* config tx pdma */
        dma_status = pdma_init(s_i3c_pdma_chnl[i3c_port][0]);
        if (dma_status != PDMA_OK) {
            log_hal_i3c_debug("[hal][i3c%d] pdma_init() fail: %d\r\n", 2, i3c_port, dma_status);
            return -1;
        }
        dma_config.count        = config->send_size;
        dma_config.master_type  = PDMA_TX;
        dma_status = pdma_configure(s_i3c_pdma_chnl[i3c_port][0], &dma_config);
        if (dma_status != PDMA_OK) {
            log_hal_i3c_debug("[hal][i3c%d] pdma_configure() fail: %d\r\n", 2, i3c_port, dma_status);
            return -2;
        }
        dma_status = pdma_start_interrupt(s_i3c_pdma_chnl[i3c_port][0], (uint32_t)config->send_buff);
        if (dma_status != PDMA_OK) {
            log_hal_i3c_debug("[hal][i3c%d] pdma_start_interrupt() fail: %d\r\n", 2, i3c_port, dma_status);
            return -3;
        }
    }
    if (rx_enable) {
        log_hal_i3c_debug("pdma rx--->mem 0x%x, size %d", 2, config->recv_buff, config->recv_size);
        log_hal_i3c_debug("[hal][i3c%d] rx pdma mem:%x %x %x %x\r\n", 5, i3c_port,
                    config->recv_buff[0], config->recv_buff[1],
                    config->recv_buff[2], config->recv_buff[3]);
        /* config rx pdma */
        dma_status = pdma_init(s_i3c_pdma_chnl[i3c_port][1]);
        if (dma_status != PDMA_OK) {
            log_hal_i3c_debug("[hal][i3c%d] pdma_init() fail: %d\r\n", 2, i3c_port, dma_status);
            return -1;
        }
        dma_config.master_type  = PDMA_RX;
        dma_config.count        = config->recv_size;
        dma_status = pdma_configure(s_i3c_pdma_chnl[i3c_port][1], &dma_config);
        if (dma_status != PDMA_OK) {
            log_hal_i3c_debug("[hal][i3c%d] pdma_configure() fail: %d\r\n", 2, i3c_port, dma_status);
            return -2;
        }

        dma_status = pdma_start_interrupt(s_i3c_pdma_chnl[i3c_port][1], (uint32_t)config->recv_buff);
        if (dma_status != PDMA_OK) {
            log_hal_i3c_debug("[hal][i3c%d] pdma_start_interrupt() fail: %d\r\n", 2, i3c_port, dma_status);
            return -3;
        }
    }
    return 0;
}

int    i3c_pdma_deinit(hal_i3c_master_port_t i3c_port)
{
    pdma_channel_t          rx_dma_ch,tx_dma_ch;

    tx_dma_ch = s_i3c_pdma_chnl[i3c_port][0];
    rx_dma_ch = s_i3c_pdma_chnl[i3c_port][1];
    pdma_stop(tx_dma_ch);
    pdma_deinit(tx_dma_ch);
    pdma_stop(rx_dma_ch);
    pdma_deinit(rx_dma_ch);
    return 0;
}

#endif


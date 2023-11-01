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

#include "hal_irrx.h"
#include "hal_irrx_internal.h"
#include "hal_sleep_manager_platform.h"
#include "hal_eint.h"
#include "hal_nvic.h"
#include "hal_gpt.h"
#include "hal_spm.h"


#ifdef HAL_IRRX_MODULE_ENABLED

static hal_irrx_running_status_t s_hal_irrx_status = HAL_IRRX_IDLE;
static bool                      s_hal_irrx_initialized = false;



hal_irrx_status_t hal_irrx_init(void)
{
    uint32_t mask;

    /*enable hal clk*/
    irrx_power_set(true);

    /* lock spm */
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (s_hal_irrx_initialized == false) {
        s_hal_irrx_initialized = true;
    } else {
        hal_nvic_restore_interrupt_mask(mask);
        return HAL_IRRX_STATUS_OK;
    }
    hal_nvic_restore_interrupt_mask(mask);

    spm_scenario_flag_setting(SPM_IRRX_SCENARIO, SPM_ENABLE);

    return HAL_IRRX_STATUS_OK;
}

hal_irrx_status_t hal_irrx_deinit(void)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;
    uint32_t mask;

    /*clear irrx decode register*/
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;
    /*disable IR hardware receiver function*/
    irrx_register_base->PDREG_IRCFGH &= ~(PDREG_IRCFGH_IREN_MASK);
    /*disable IRRX interrupt*/
    irrx_nvic_set(false);
    /*disable power*/
    irrx_power_set(false);

    /* unlock spm */
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (s_hal_irrx_initialized) {
        s_hal_irrx_initialized = false;
    } else {
        hal_nvic_restore_interrupt_mask(mask);
        return HAL_IRRX_STATUS_OK;
    }
    hal_nvic_restore_interrupt_mask(mask);

    spm_scenario_flag_setting(SPM_IRRX_SCENARIO, SPM_DISABLE);

    return HAL_IRRX_STATUS_OK;
}

hal_irrx_status_t hal_irrx_receive_sirc_start(hal_irrx_callback_t callback, void *parameter)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;

    /*hardware will abandon the first start pulse(header)*/
    if (!callback) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }
    irrx_register_base->PDREG_IRCFGH = 0;
    irrx_register_base->PDREG_IRCFGL = 0;
    irrx_register_base->PDREG_IRTHD  = 0;
    /*clear irrx decode register*/
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;

    //irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IGB0_MASK;

    /*order reverse the decoded IR pulse is bit-reversed*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_ORDINV_MASK;

    /*config ok period to inform ir decoder complete*/
    irrx_register_base->PDREG_IRCFGH &= ~(PDREG_IRCFGH_OK_PERIOD_MASK);
    irrx_register_base->PDREG_IRCFGH |= 0x0f << PDREG_IRCFGH_OK_PERIOD_OFFSET;

    /*config sample period,this field decides the working freq of sampleing counter*/
    irrx_register_base->PDREG_IRCFGL &= ~(PDREG_IRCFGL_SAPERIOD_MASK);
    irrx_register_base->PDREG_IRCFGL |= 0x06 << PDREG_IRCFGL_SAPERIOD_OFFSET;

    /*conifig ir threshold*/
    irrx_register_base->PDREG_IRTHD &= ~(PDREG_IRTHD_IRTHD_MASK);
    irrx_register_base->PDREG_IRTHD |= 0x01 << PDREG_IRTHD_IRTHD_OFFSET;

    /*config diglich*/
    irrx_register_base->PDREG_IRTHD &= ~(PDREG_IRTHD_DE_SEL_MASK);
    irrx_register_base->PDREG_IRTHD |= 0x06 << PDREG_IRTHD_DE_SEL_OFFSET;

    /*enable IR hardware receiver function*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IREN_MASK;

    /*register interrupt callback*/
    irrx_register_callback(callback, parameter);

    /*set irrx busy status*/
    s_hal_irrx_status = HAL_IRRX_BUSY;

    /*enable IRRX interrupt*/
    irrx_nvic_set(true);

    return HAL_IRRX_STATUS_OK;
}

hal_irrx_status_t hal_irrx_receive_sirc(uint8_t *data, uint8_t *receive_bits_length)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;

    if ((!data) || (!receive_bits_length)) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }

    if (s_hal_irrx_status != HAL_IRRX_BUSY) {
        return HAL_IRRX_STATUS_NOT_SUPPORTED;
    }
    /*get irrx data length,the last pulse is no effective*/
    *receive_bits_length = ((irrx_register_base->PDREG_IRH) & PDREG_IRH_BIT_CNT_MASK); /*different with NEC ,all bits valid*/

    /*get irrx data*/
    irrx_read_hardware_mode_data(data, *receive_bits_length);

    /*clear interrupt*/
    irrx_register_base->PDREG_IR_INTCLR |= PDREG_IR_INTCLR_IR_INTCLR_MASK;
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;
    hal_gpt_delay_us(70);

    /*set irrx idle status*/
    s_hal_irrx_status = HAL_IRRX_IDLE;

    /*disable IRRX interrupt*/
    irrx_nvic_set(false);
    return HAL_IRRX_STATUS_OK;
}

hal_irrx_status_t hal_irrx_receive_rc5_start(hal_irrx_callback_t callback, void *parameter)
{
    /*hardware will abandon the two start pulse(s1 s2)*/
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;

    if (!callback) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }
    irrx_register_base->PDREG_IRCFGH = 0;
    irrx_register_base->PDREG_IRCFGL = 0;
    irrx_register_base->PDREG_IRTHD  = 0;

    /*clear irrx decode register*/
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;

    /*set irrx control register*/
    /*use rc5 format*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_RC5_MASK;
    /*order reverse the decoded IR pulse is bit-reversed*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_ORDINV_MASK;

    irrx_register_base->PDREG_IRCFGH &= ~PDREG_IRCFGH_RC5_1ST_MASK;
    //irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_RC5_1ST_MASK;
    /*ingore sync pulse*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IGN_1ST_MASK;

    /*config ok period to inform ir decoder complete*/
    irrx_register_base->PDREG_IRCFGH &= ~(PDREG_IRCFGH_OK_PERIOD_MASK);
    irrx_register_base->PDREG_IRCFGH |= 0x0f << PDREG_IRCFGH_OK_PERIOD_OFFSET;

    /*config sample period,this field decides the working freq of sampleing counter*/
    irrx_register_base->PDREG_IRCFGL &= ~(PDREG_IRCFGL_SAPERIOD_MASK);
    irrx_register_base->PDREG_IRCFGL |= (0x1a << PDREG_IRCFGL_SAPERIOD_OFFSET);

    /*conifig ir threshold*/
    irrx_register_base->PDREG_IRTHD &= ~(PDREG_IRTHD_IRTHD_MASK);
    irrx_register_base->PDREG_IRTHD |= 0x02 << PDREG_IRTHD_IRTHD_OFFSET;

    /*disalbe diglich*/
    irrx_register_base->PDREG_IRTHD &= ~(PDREG_IRTHD_DE_SEL_MASK);

    /*enable IR hardware receiver function*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IREN_MASK;

    /*register interrupt callback*/
    irrx_register_callback(callback, parameter);

    /*set irrx busy status*/
    s_hal_irrx_status = HAL_IRRX_BUSY;

    /*enable IRRX interrupt*/
    irrx_nvic_set(true);

    return HAL_IRRX_STATUS_OK;
}

hal_irrx_status_t hal_irrx_receive_rc5(uint8_t *data, uint8_t *receive_bits_length)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;

    if ((!data) || (!receive_bits_length)) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }

    if (HAL_IRRX_BUSY != s_hal_irrx_status) {
        return HAL_IRRX_STATUS_NOT_SUPPORTED;
    }

    /*get irrx data length*/
    *receive_bits_length = (irrx_register_base->PDREG_IRH) & PDREG_IRH_BIT_CNT_MASK;

    /*get irrx data*/
    irrx_read_hardware_mode_data(data, *receive_bits_length);

    /*reverse data bit*/
    irrx_byte_bit_inverse(data, ((*receive_bits_length) / 8) + 1);

    /*clear interrupt*/
    irrx_register_base->PDREG_IR_INTCLR |= PDREG_IR_INTCLR_IR_INTCLR_MASK;
    irrx_register_base->PDREG_IRCLR     |= PDREG_IRCLR_IRCLR_MASK;
    hal_gpt_delay_us(70);
    /*set irrx idle status*/
    s_hal_irrx_status = HAL_IRRX_IDLE;

    /*disable IRRX interrupt*/
    irrx_nvic_set(false);

    return HAL_IRRX_STATUS_OK;
}

hal_irrx_status_t hal_irrx_receive_rc6_start(hal_irrx_callback_t callback, void *parameter)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;

    /*hardware will abandon the first start pulse(header)*/
    if (!callback) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }
    irrx_register_base->PDREG_IRCFGH = 0;
    irrx_register_base->PDREG_IRCFGL = 0;
    irrx_register_base->PDREG_IRTHD  = 0;
    /*clear irrx decode register*/
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;

    /*set irrx control register*/
    /*use rc5 format*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_RC5_MASK;
    /*order reverse the decoded IR pulse is bit-reversed*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_ORDINV_MASK;
    /*ingore sync pulse*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IGN_1ST_MASK;

    /*config ok period to inform ir decoder complete*/
    irrx_register_base->PDREG_IRCFGH &= ~(PDREG_IRCFGH_OK_PERIOD_MASK);
    irrx_register_base->PDREG_IRCFGH |= 0x0f << PDREG_IRCFGH_OK_PERIOD_OFFSET;

    /*config sample period,this field decides the working freq of sampleing counter*/
    irrx_register_base->PDREG_IRCFGL &= ~(PDREG_IRCFGL_SAPERIOD_MASK);
    irrx_register_base->PDREG_IRCFGL |= 0x0e << PDREG_IRCFGL_SAPERIOD_OFFSET;

    /*conifig ir threshold*/
    irrx_register_base->PDREG_IRTHD &= ~(PDREG_IRTHD_IRTHD_MASK);
    irrx_register_base->PDREG_IRTHD |= 0x01 << PDREG_IRTHD_IRTHD_OFFSET;

    /*disalbe diglich*/
    irrx_register_base->PDREG_IRTHD &= ~(PDREG_IRTHD_DE_SEL_MASK);

    /*enable IR hardware receiver function*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IREN_MASK;

    /*register interrupt callback*/
    irrx_register_callback(callback, parameter);

    /*set irrx busy status*/
    s_hal_irrx_status = HAL_IRRX_BUSY;

    /*enable IRRX interrupt*/
    irrx_nvic_set(true);

    return HAL_IRRX_STATUS_OK;

}

hal_irrx_status_t hal_irrx_receive_rc6(uint8_t *data, uint8_t *receive_bits_length)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;

    if ((!data) || (!receive_bits_length)) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }

    /*get irrx data length*/
    *receive_bits_length = (irrx_register_base->PDREG_IRH) & PDREG_IRH_BIT_CNT_MASK;

    /*get irrx data*/
    irrx_read_hardware_mode_data(data, *receive_bits_length);

    /*reverse data bit*/
    //  irrx_byte_bit_inverse(data,((*receive_bits_length) / 8) + 1); //rc6 mode not need inverse

    /*clear interrupt*/
    irrx_register_base->PDREG_IR_INTCLR |= PDREG_IR_INTCLR_IR_INTCLR_MASK;
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;
    hal_gpt_delay_us(70);

    /*set irrx idle status*/
    s_hal_irrx_status = HAL_IRRX_IDLE;

    /*disable IRRX interrupt*/
    irrx_nvic_set(false);

    return HAL_IRRX_STATUS_OK;
}

hal_irrx_status_t hal_irrx_receive_nec_start(hal_irrx_callback_t callback, void *parameter)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;

    /*hardware will abandon the first start pulse(header)*/
    if (!callback) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }
    irrx_register_base->PDREG_IRCFGH = 0;
    irrx_register_base->PDREG_IRCFGL = 0;
    irrx_register_base->PDREG_IRTHD  = 0;

    /*clear irrx decode register*/
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;
    /*ingore sync pulse*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IGN_1ST_MASK;

    /*config ok period to inform ir decoder complete*/
    irrx_register_base->PDREG_IRCFGH &= ~(PDREG_IRCFGH_OK_PERIOD_MASK);
    irrx_register_base->PDREG_IRCFGH |= 0x0f << PDREG_IRCFGH_OK_PERIOD_OFFSET;

    /*config sample period,this field decides the working freq of sampleing counter*/
    irrx_register_base->PDREG_IRCFGL &= ~(PDREG_IRCFGL_SAPERIOD_MASK);
    irrx_register_base->PDREG_IRCFGL |= 0x0f << PDREG_IRCFGL_SAPERIOD_OFFSET;

    /*conifig ir threshold*/
    irrx_register_base->PDREG_IRTHD &= ~(PDREG_IRTHD_IRTHD_MASK);
    irrx_register_base->PDREG_IRTHD |= 0x01 << PDREG_IRTHD_IRTHD_OFFSET;

    /*disalbe diglich*/
    irrx_register_base->PDREG_IRTHD &= ~(PDREG_IRTHD_DE_SEL_MASK);

    /*enable IR hardware receiver function*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IREN_MASK;

    /*register interrupt callback*/
    irrx_register_callback(callback, parameter);

    /*set irrx busy status*/
    s_hal_irrx_status = HAL_IRRX_BUSY;

    /*enable IRRX interrupt*/
    irrx_nvic_set(true);

    return HAL_IRRX_STATUS_OK;
}

hal_irrx_status_t hal_irrx_receive_nec(uint8_t *data, uint8_t *receive_bits_length)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;

    if ((!data) || (!receive_bits_length)) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }

    if (HAL_IRRX_BUSY != s_hal_irrx_status) {
        return HAL_IRRX_STATUS_NOT_SUPPORTED;
    }

    /*get irrx data length,the last pulse is no effective*/
    *receive_bits_length = ((irrx_register_base->PDREG_IRH) & PDREG_IRH_BIT_CNT_MASK) - 1; /*last pulse will have no use*/

    /*get irrx data*/
    irrx_read_hardware_mode_data(data, *receive_bits_length);

    /*clear interrupt*/
    irrx_register_base->PDREG_IR_INTCLR |= PDREG_IR_INTCLR_IR_INTCLR_MASK;
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;
    hal_gpt_delay_us(70);

    /*set irrx idle status*/
    s_hal_irrx_status = HAL_IRRX_IDLE;

    /*disable IRRX interrupt*/
    irrx_nvic_set(false);

    return HAL_IRRX_STATUS_OK;
}


hal_irrx_status_t hal_irrx_receive_rcmm_start(hal_irrx_callback_t callback, void *parameter)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;

    /*hardware will abandon the first start pulse(header)*/
    if (!callback) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }
    irrx_register_base->PDREG_IRCFGH = 0;
    irrx_register_base->PDREG_IRCFGL = 0;
    irrx_register_base->PDREG_IRTHD  = 0;
    irrx_register_base->PDREG_IRRCM_THD   = 0;
    irrx_register_base->PDREG_IRRCM_THD_0 = 0;
    /*clear irrx decode register*/
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;
    /*ingore sync pulse*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IGN_1ST_MASK;

    /*config ok period to inform ir decoder complete*/
    irrx_register_base->PDREG_IRCFGH &= ~(PDREG_IRCFGH_OK_PERIOD_MASK);
    irrx_register_base->PDREG_IRCFGH |= 0x1f << PDREG_IRCFGH_OK_PERIOD_OFFSET;

    /*config sample period*/
    irrx_register_base->PDREG_IRCFGL &= ~(PDREG_IRCFGL_SAPERIOD_MASK);
    irrx_register_base->PDREG_IRCFGL |= 0x0;//0x02 << PDREG_IRCFGL_SAPERIOD_OFFSET;

    /*config irrx de glitch*/
    irrx_register_base->PDREG_IRTHD &= ~(PDREG_IRTHD_DE_SEL_MASK);
    irrx_register_base->PDREG_IRTHD |= (0x02 << PDREG_IRTHD_DE_SEL_OFFSET);

    /*config irrx rcmm threshold*/
    irrx_register_base->PDREG_IRRCM_THD &= ~(PDREG_IRRCM_THD_IRRCM_THD_01_MASK);
    irrx_register_base->PDREG_IRRCM_THD |= (12 << PDREG_IRRCM_THD_IRRCM_THD_01_OFFSET); //04 << PDREG_IRRCM_THD_IRRCM_THD_01_OFFSET;
    irrx_register_base->PDREG_IRRCM_THD &= ~(PDREG_IRRCM_THD_IRRCM_THD_10_MASK);
    irrx_register_base->PDREG_IRRCM_THD |= (12 << PDREG_IRRCM_THD_IRRCM_THD_10_OFFSET); //04 << PDREG_IRRCM_THD_IRRCM_THD_10_OFFSET;
    irrx_register_base->PDREG_IRRCM_THD &= ~(PDREG_IRRCM_THD_IRRCM_THD_11_MASK);
    irrx_register_base->PDREG_IRRCM_THD |= (17 << PDREG_IRRCM_THD_IRRCM_THD_11_OFFSET); //6 << PDREG_IRRCM_THD_IRRCM_THD_11_OFFSET;
    /*enable rcmm */
    irrx_register_base->PDREG_IRRCM_THD |= PDREG_IRRCM_THD_IRRCM_MASK;
    /*config irrx rcmm threshold*/
    irrx_register_base->PDREG_IRRCM_THD_0 &= ~(PDREG_IRRCM_THD_0_IRRCM_THD_20_MASK);
    irrx_register_base->PDREG_IRRCM_THD_0 |= (17 << PDREG_IRRCM_THD_0_IRRCM_THD_20_OFFSET); //0x06 << PDREG_IRRCM_THD_0_IRRCM_THD_20_OFFSET;
    irrx_register_base->PDREG_IRRCM_THD_0 &= ~(PDREG_IRRCM_THD_0_IRRCM_THD_21_MASK);
    irrx_register_base->PDREG_IRRCM_THD_0 |= (23 << PDREG_IRRCM_THD_0_IRRCM_THD_21_OFFSET);; //0x08 << PDREG_IRRCM_THD_0_IRRCM_THD_21_OFFSET;

    /*enable IR hardware receiver function*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IREN_MASK;

    /*register interrupt callback*/
    irrx_register_callback(callback, parameter);

    /*set irrx busy status*/
    s_hal_irrx_status = HAL_IRRX_BUSY;

    /*enable IRRX interrupt*/
    irrx_nvic_set(true);

    return HAL_IRRX_STATUS_OK;
}

hal_irrx_status_t hal_irrx_receive_rcmm(uint8_t *data, uint8_t *receive_bits_length)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;

    if ((!data) || (!receive_bits_length)) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }

    if (HAL_IRRX_BUSY != s_hal_irrx_status) {
        return HAL_IRRX_STATUS_NOT_SUPPORTED;
    }

    /*get irrx data length,the last pulse is no effective*/
    *receive_bits_length = ((irrx_register_base->PDREG_IRH) & PDREG_IRH_BIT_CNT_MASK) - 1;

    /*get irrx data*/
    irrx_read_hardware_mode_data(data, *receive_bits_length);

    /*clear interrupt*/
    irrx_register_base->PDREG_IR_INTCLR |= PDREG_IR_INTCLR_IR_INTCLR_MASK;
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;
    hal_gpt_delay_us(70);

    /*set irrx idle status*/
    s_hal_irrx_status = HAL_IRRX_IDLE;

    /*disable IRRX interrupt*/
    irrx_nvic_set(false);

    return HAL_IRRX_STATUS_OK;
}



hal_irrx_status_t hal_irrx_receive_pwd_start(
    const hal_irrx_pwd_config_t     *config,
    // hal_irrx_pwd_config_t           *config,
    hal_irrx_callback_t             callback,
    uint32_t                        *precision_us)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;
    uint32_t         check_period = 0;
    uint32_t         ok_period    = 0; //

    /*hardware will abandon the first start pulse(header)*/
    if (!callback) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }

    /*clear irrx decode register*/
    irrx_register_base->PDREG_IRCLR |= PDREG_IRCLR_IRCLR_MASK;
    irrx_register_base->PDREG_IRCFGH = 0;
    irrx_register_base->PDREG_IRCFGL = 0;
    irrx_register_base->PDREG_IRTHD  = 0;

    /* config pwd irrx*/
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IREN_MASK;

    if (config->inverse) {
        //irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_IRINV_MASK;
    }
    irrx_register_base->PDREG_IRCFGH |= PDREG_IRCFGH_CHK_EN_MASK;
    ok_period = 50;
    irrx_register_base->PDREG_IRCFGH |= (ok_period << PDREG_IRCFGH_OK_PERIOD_OFFSET);

    /* config ir sample rate */
    check_period = 0x2D2; // for 36Khz
    irrx_register_base->PDREG_IRCFGL |= (check_period << PDREG_IRCFGL_CHECKPERIOD_OFFSET);
    irrx_register_base->PDREG_IRCFGL |= (4 << PDREG_IRCFGL_SAPERIOD_OFFSET);

    /*  */
    irrx_register_base->PDREG_IRTHD |= (3 << PDREG_IRTHD_DE_SEL_OFFSET);
    irrx_register_base->PDREG_IRTHD |= PDREG_IRTHD_ICLR_MASK;

    /*register interrupt callback*/
    irrx_register_callback(callback, NULL);

    /*set irrx busy status*/
    s_hal_irrx_status = HAL_IRRX_BUSY;

    /*enable IRRX interrupt*/
    irrx_nvic_set(true);

    return HAL_IRRX_STATUS_OK;
}



void    copy_uint32_to_uint8buff(uint32_t data, uint8_t *buffer, uint8_t len)
{
    uint32_t temp = 0;
    uint32_t u8data;
    uint32_t i = 0;

    if (len <= 0 || buffer == NULL) {
        return;
    }

    for (i = 0; i < 4;) {
        temp = i * 8;
        u8data = (data >> temp) & 0xFF;
        buffer[i++] = u8data;
        if (i >= len) {
            break;
        }
    }
}


hal_irrx_status_t hal_irrx_receive_pwd(uint8_t  *received_length,
                                       uint8_t  *buffer,
                                       uint8_t   buffer_length)
{
    volatile IRRX_REGISTER_T   *irrx_register_base = IRRX_REG;
    volatile uint32_t *reg_addr = NULL;
    uint32_t           i = 0;
    uint32_t           rsize = 0;
    uint32_t           offset = 0;
    uint32_t           u32temp;

    if ((buffer == NULL) || (buffer_length == 0)) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }
    if (HAL_IRRX_BUSY != s_hal_irrx_status) {
        return HAL_IRRX_STATUS_NOT_SUPPORTED;
    }
    rsize    = buffer_length / 4;
    offset   = 0;
    reg_addr = &(irrx_register_base->PDREG_CHK_DATA0);
    //printf("[hal][irrx][pwd] data addr %x \r\n", reg_addr);
    //printf("[hal][irrx][pwd] recv data %x-%x-%x-%x\r\n", irrx_register_base->PDREG_CHK_DATA0,irrx_register_base->PDREG_CHK_DATA1,
    //        irrx_register_base->PDREG_CHK_DATA2,irrx_register_base->PDREG_CHK_DATA3);
    for (i = 0; i < rsize; i++) {
        u32temp = *reg_addr;
        copy_uint32_to_uint8buff(u32temp, &buffer[offset], 4);
        reg_addr = (reg_addr + 4);
        offset  += 4;
    }
    *received_length  = offset;
    s_hal_irrx_status = HAL_IRRX_IDLE;
    irrx_nvic_set(false);   /*disable IRRX interrupt*/

    return HAL_IRRX_STATUS_OK;
}

hal_irrx_status_t hal_irrx_get_running_status(hal_irrx_running_status_t *running_status)
{
    /*check param*/
    if (NULL == running_status) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }

    *running_status = s_hal_irrx_status;

    return HAL_IRRX_STATUS_OK;
}


void        hal_irrx_printf_irrx_reg()
{


}



#if 0
hal_irrx_status_t hal_irrx_wakeup_config(hal_irrx_wakeup_type_t wakeup_type, hal_irrx_wakeup_key *wakeup_key, uint8_t key_group_number)
{
    IRRX_REGISTER_T *irrx_register_base = IRRX_REG;
    hal_eint_config_t config = {HAL_EINT_LEVEL_LOW, 1};

    /*check param*/
    if ((NULL == wakeup_key) || (key_group_number > 8) || (key_group_number < 1)) {
        return HAL_IRRX_STATUS_INVALID_PARAM;
    }

    switch (wakeup_type) {
        case HAL_IRRX_WAKEUP_ANY_KEY:

            break;
        case HAL_IRRX_WAKEUP_SPECIAL_KEY:
            /*enable how many data group for wakeup*/
            irrx_register_base->PDREG_IREXP_EN = 0x101;
            /*config the wakeup data bits number*/
            irrx_register_base->PDREG_EXP_BCNT |= wakeup_key->wakeup_bit_cnt;
            /*enable the unmask bit for wakeup*/
            irrx_register_base->PDREG_ENEXP_IRL = 0xffffffff;
            irrx_register_base->PDREG_ENEXP_IRM = 0xffffffff;
            /*config the data group*/
            *(volatile unsigned int *)(0xA2200000 + 0x34 + key_group_number - 1) = wakeup_key->l;
            *(volatile unsigned int *)(0xA2200000 + 0x5c + key_group_number - 1) = wakeup_key->m;
            /*enable the wakeup */
            irrx_register_base->PDREG_WAKEEN |= 0x01;
            /*config the power done out*/
            irrx_register_base->PDREG_PDWN_OUT |= 0x01;
            break;
        default:
            return HAL_IRRX_STATUS_INVALID_PARAM;
    }

    /*config eint wake up interrupt*/
    hal_eint_init(HAL_EINT_WAKE_AP, &config);
    hal_eint_register_callback(HAL_EINT_WAKE_AP, irrx_wakeup_callback, NULL);
    hal_eint_unmask(HAL_EINT_WAKE_AP);

    return HAL_IRRX_STATUS_OK;
}
#endif

#endif

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


#include "hal_audio_volume.h"
#include "hal_audio_register.h"
#include "hal_audio_driver.h"
#include "hal_audio_afe_control.h"

#ifdef HAL_AUDIO_MODULE_ENABLED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables Declaration //////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

afe_volume_digital_control_t afe_digital_gain[AFE_HW_DIGITAL_GAIN_NUM];
afe_volume_analog_control_t afe_analog_gain[AFE_HW_ANALOG_GAIN_NUM];
extern hal_audio_performance_mode_t afe_adc_performance_mode[AFE_ANALOG_NUMBER];


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functiion Declaration //////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************************
*                                       dB convert                                         *
********************************************************************************************/
#define DSP_AUDIO_FIXED_POINT_CALCULATION
#define LN10 2.3025851  // ln(10)

const uint32_t afe_db_match_table[AFE_DB_CONVERSION_TABLE_NUMBER] = {
    0x7FFFFF,   //0dB 1 8388607
    0x721481,  //-1db 0.8912509381337455299531086810783
    0x65AC8B,  //-2db 0.79432823472428150206591828283639
    0x5A9DF6,  //-3db 0.70794578438413791080221494218931
    0x50C335,  //-4db 0.63095734448019324943436013662234
    0x47FACC,  //-5db 0.56234132519034908039495103977648

    //First decimal place
    0x7FFFFF,   // -0.0db 1
    0x7E88E7,   // -0.1db 0.98855309465693884028524792978203
    0x7D161B,   // -0.2db 0.97723722095581068269707600696156
    0x7BA78D,   // -0.3db 0.96605087898981337427673560676923
    0x7A3D31,   // -0.4db 0.95499258602143594972395937950148
    0x78D6FC,   // -0.5db 0.94406087628592338036438049660227
    0x7774E0,   // -0.6db 0.93325430079699104353209661168365
    0x7616D1,   // -0.7db 0.92257142715476316003073802267767
    0x74BCC5,   // -0.8db 0.91201083935590974212095940791872
    0x7366AE,   // -0.9db 0.90157113760595688589246344194515

    //Second decimal place
    0x7FFFFF,   // -0.00db 1
    0x7FDA4B,   // -0.01db 0.99884936993650514951538205746463
    0x7FB4A2,   // -0.02db 0.99770006382255331719442194285376
    0x7F8F04,   // -0.03db 0.9965520801347683562901517234443
    0x7F6971,   // -0.04db 0.99540541735152696244806147089511
    0x7F43E9,   // -0.05db 0.9942600739529566568329459308278
    0x7F1E6C,   // -0.06db 0.99311604842093377157642607688515
    0x7EF8FA,   // -0.07db 0.99197333923908143754247531097752
    0x7ED394,   // -0.08db 0.99083194489276757440828314388392
    0x7EAE38,   // -0.09db 0.9896918638691028830577922592959
};
uint32_t afe_calculate_db_result(int32_t digital_gain_in_01unit_db, uint32_t digital_0db_register_value)
{
    int32_t integer_db, first_decimal_db, second_decimal_db;
    int32_t times_6db, integer_multiply, multiplier;
    uint64_t calculate_result;
    calculate_result = (uint64_t)digital_0db_register_value;

    if (digital_gain_in_01unit_db == 0) {
        return digital_0db_register_value;
    } else if (digital_gain_in_01unit_db > 0) {
        integer_db = (digital_gain_in_01unit_db + 99) / 100;
        first_decimal_db = (integer_db * 100 - digital_gain_in_01unit_db) / 10;
        second_decimal_db = (integer_db * 100 - digital_gain_in_01unit_db) % 10;
        times_6db = integer_db / 6;
        if ((integer_db % 6) != 0) {
            times_6db += 1;
        }
        integer_multiply = times_6db * 6 - integer_db;
    } else {
        integer_db = (-digital_gain_in_01unit_db) / 100;
        first_decimal_db = ((-digital_gain_in_01unit_db) % 100) / 10;
        second_decimal_db = ((-digital_gain_in_01unit_db) % 100) % 10;
        times_6db = integer_db / 6;
        integer_multiply = integer_db - times_6db * 6;
    }
    if (second_decimal_db != 0) {
       multiplier = afe_db_match_table[second_decimal_db+AFE_DB_CONVERSION_TABLE_SECOND_DECIMAL_PLACE];
       calculate_result = calculate_result*multiplier;
       calculate_result >>= 23;
    }

    if (first_decimal_db != 0) {
       multiplier = afe_db_match_table[first_decimal_db+AFE_DB_CONVERSION_TABLE_FIRST_DECIMAL_PLACE];
       calculate_result = calculate_result*multiplier;
       calculate_result >>= 23;
    }

    if (integer_multiply != 0) {
       multiplier = afe_db_match_table[integer_multiply+AFE_DB_CONVERSION_TABLE_0_DB];
       calculate_result = calculate_result*multiplier;
       calculate_result >>= 23;
    }

    if(times_6db != 0) {
        if (digital_gain_in_01unit_db > 0) {
            calculate_result = calculate_result << times_6db;
        } else {
            calculate_result = calculate_result >> times_6db;
        }
    }
    return (uint32_t)calculate_result;
}

float afe_fast_pow_function(float x, int n)
{
    float result = 1.0;
    float temp = x;
    unsigned m = (n >= 0) ? n : -n;
    while (m) {
        if (m & 1) {
            result *= temp;
        }
        temp *= temp;
        m >>= 1;
    }
    return (n >= 0) ? result : (float)1.0 / result;
}

// e^x
float afe_exp_power_function(const float x, const float minimum_acceptable_error)
{
    float ans1 ;
    float ans2 = 1.0;
    float fact = 1, xn = x, cnt = 2.0;
    do {
        ans1 = ans2;
        ans2 = ans1 + xn / fact;
        fact *= cnt;
        xn = xn * x;
        cnt = cnt + (float)1.0;
    } while ((ans1 > ans2 + minimum_acceptable_error) || (ans2 > ans1 + minimum_acceptable_error));
    return ans2;
}

void afe_seperate_int_and_decimal(float input, float *int_part, float *decimal_part)
{
    int b = 0;
    float c = 0;
    b = (int)input;
    c = input - (float)b;

    *int_part = (float)b;
    *decimal_part = c;
}

// e^x fast function
float afe_exp_fast_function(const float exponment, const float minimum_acceptable_error)
{
    const float Euler = 2.718281828459045;
    //float rst=1.0;
    float p1 = 0, p2 = 0;
    afe_seperate_int_and_decimal(exponment, &p1, &p2);

    if (exponment > (float)709.0) {
        p1 = 1.0;
        p2 = 0.0;
        return 0xFFFFFFFF + 1; // too big not to calculate
    } else if (exponment < (float)(-709.0)) {
        return 0.0;
    } else {
        return afe_exp_power_function(p2, minimum_acceptable_error) * afe_fast_pow_function(Euler, (int)p1);
    }
}

uint32_t afe_calculate_digital_gain_index(int32_t digital_gain_in_01unit_db, uint32_t digital_0db_register_value)
{
#ifdef DSP_AUDIO_FIXED_POINT_CALCULATION
    return afe_calculate_db_result(digital_gain_in_01unit_db, digital_0db_register_value);
#else
    uint32_t digital_gain_index = 0;
    int32_t temp_int32_digital_gain = (int32_t)digital_gain_in_01unit_db;
    int32_t temp_int32_digital_gain_register_value = 0;
    float temp_float_digital_gain_register_value = 0;
    float temp_float_digital_gain_in_unit_db = (float)temp_int32_digital_gain / 100;
    float temp_digital_0db_register_value = (float)digital_0db_register_value;
    float exp_exponment = temp_float_digital_gain_in_unit_db / (float)20 * (float)LN10;
    temp_float_digital_gain_register_value = temp_digital_0db_register_value * afe_exp_fast_function(exp_exponment, 1 / temp_digital_0db_register_value);
    temp_int32_digital_gain_register_value = (int32_t)temp_float_digital_gain_register_value;
    digital_gain_index = (uint32_t)temp_int32_digital_gain_register_value;
    return digital_gain_index;
#endif

}

int afe_calc_Lnx(double n)
{
    int num, mul, cal, sum = 0;
    num = (n - 1) / (n + 1);

    for (int i = 1; i <= 1000; i++) {
        mul = (2 * i) - 1;
        cal = afe_fast_pow_function(num, mul);
        cal = cal / mul;
        sum = sum + cal;
    }
    sum = 2 * sum;

    return sum;
}

/*******************************************************************************************
*                                 Register Value Convert                                   *
********************************************************************************************/
static void afe_truncate_out_of_range_value(int16_t *truncate_value, int32_t minimum, int32_t maximum)
{
    *truncate_value = *truncate_value < (int16_t)minimum ? (int16_t)minimum : *truncate_value;
    *truncate_value = *truncate_value > (int16_t)maximum ? (int16_t)maximum : *truncate_value;
}

static uint32_t afe_to_register_value(int16_t input_db, int32_t max_db_value, int32_t min_db_value, int32_t db_step_value, uint32_t max_reg_value, uint32_t min_reg_value, uint32_t min_db_to_min_reg_value)
{
    int32_t input_db_to_db_step = 0;
    uint32_t register_value = 0;

    afe_truncate_out_of_range_value(&input_db, min_db_value, max_db_value);

    input_db_to_db_step = (input_db - min_db_value) / db_step_value;

    if (min_db_to_min_reg_value) {
        register_value = min_reg_value + (uint32_t)input_db_to_db_step;
    } else {
        register_value = max_reg_value - (uint32_t)input_db_to_db_step;
    }

    if (register_value > max_reg_value) {
        register_value = max_reg_value;
    }

    if ((int)register_value < 0) {
        register_value = 0;
    }

    return register_value;
}

uint32_t afe_volume_analog_convert_output_register(int32_t index)
{
    uint32_t register_value = 0;
    int32_t db_unit = 0;
    int32_t db_max = 0;
    int32_t db_min = 0;
    int32_t db_step = 1;
    uint32_t reg_max = 0;
    uint32_t reg_min = 0;
    uint32_t minimum_db_to_minimum_reg_value = 0;

    db_unit = index / 100;
    db_max = 12;
    db_min = -32;
    db_step = 1;
    reg_max = (AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSAB_NEG_32_DB & ZCD_CON2_L_GAIN_MASK);
    reg_min = (AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSAB_POS_12_DB & ZCD_CON2_L_GAIN_MASK);
    register_value = afe_to_register_value(db_unit, db_max, db_min, db_step, reg_max, reg_min, minimum_db_to_minimum_reg_value);

    return register_value;
}
uint32_t afe_volume_analog_convert_output_register_class_d(int32_t index)
{
    uint32_t register_value = 0;
    int32_t db_unit = 0;
    int32_t db_max = 4;
    int32_t db_min = -8;
    int32_t db_step = 3;
    db_unit = index / 100;
    int32_t offset = 0;
    int16_t db_unit_16 = (int16_t)db_unit;

    afe_truncate_out_of_range_value((int16_t *)&db_unit_16, db_min, db_max);
    offset = (db_unit_16 - db_min + 1) / db_step;
    register_value = 1 << (offset);
    return register_value;
}

uint32_t afe_volume_convert_input_acc10k_mode(int32_t value)
{
#if (HAL_AUDIO_VOLUME_VALUE_CONVERT)
    uint32_t convert_value;
    switch (value) {
        case 0://0dB
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_0_DB;
            break;
        case 3://3dB
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_3_DB;
            break;
        case 6://6dB
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_6_DB;
            break;
        case 9://9dB
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_9_DB;
            break;
        case 12://12dB
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_12_DB;
            break;
        case 15://15dB
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_15_DB;
            break;
        case 18://18dB
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_18_DB;
            break;
        case 24://24dB
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_24_DB;
            break;
        case 30://30dB
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_30_DB;
            break;
        case 36://36dB
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_36_DB;
            break;
        default:
            convert_value = 0xFF;
            break;
    }
    return convert_value;
#else
    return (uint32_t)value;
#endif

}

uint32_t afe_volume_analog_convert_input_register(int32_t index)
{
    uint32_t register_value = 0;
    int32_t db_unit = 0;
    int32_t db_max = 0;
    int32_t db_min = 0;
    int32_t db_step = 1;
    uint32_t reg_max = 0;
    uint32_t reg_min = 0;
    uint32_t minimum_db_to_minimum_reg_value = 1;

    db_unit = index / 100;
    register_value = afe_volume_convert_input_acc10k_mode(db_unit);
    if (register_value == 0xFF) { //Cannot find specific dB value
        db_max = 36;
        db_min = 0;
        db_step = 3;
        reg_max = AFE_HW_ANALOG_INPUT_MAX_REGISTER_VALUE;
        reg_min = AFE_HW_ANALOG_INPUT_MIN_REGISTER_VALUE;
        register_value = afe_to_register_value(db_unit, db_max, db_min, db_step, reg_max, reg_min, minimum_db_to_minimum_reg_value);
    }

    return register_value;

}

uint32_t afe_volume_convert_input_acc20k_mode(int32_t value)
{
#if (HAL_AUDIO_VOLUME_VALUE_CONVERT)
    uint32_t convert_value;
    switch (value) {
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_0_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_0_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_3_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_3_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_6_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_6_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_9_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_9_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_12_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_12_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_15_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_12_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_18_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_18_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_24_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_24_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_30_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_30_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_36_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_30_DB;
            break;
        default:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC20K_POS_18_DB;
            break;
    }
    return convert_value;
#else
    return (uint32_t)value;
#endif

}

uint32_t afe_volume_convert_input_dcc_mode(int32_t value)
{
#if (HAL_AUDIO_VOLUME_VALUE_CONVERT)
    uint32_t convert_value;
    switch (value) {
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_0_DB:
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_3_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_DCC_POS_0_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_6_DB:
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_9_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_DCC_POS_6_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_12_DB:
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_15_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_DCC_POS_12_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_18_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_DCC_POS_18_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_24_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_DCC_POS_24_DB;
            break;
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_30_DB:
        case AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_36_DB:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_DCC_POS_30_DB;
            break;
        default:
            convert_value = AFE_VOLUME_ANALOG_INPUT_GAIN_DCC_POS_18_DB;
            break;
    }
    return convert_value;
#else
    return (uint32_t)value;
#endif
}

uint32_t afe_volume_convert_output_classab_mode(int32_t value)
{
#if (HAL_AUDIO_VOLUME_VALUE_CONVERT)
    uint32_t convert_value;
    switch (value) {
        case AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSD_NEG_8_DB:
            convert_value = AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSAB_NEG_8_DB;
            break;
        case AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSD_NEG_5_DB:
            convert_value = AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSAB_NEG_5_DB;
            break;
        case AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSD_NEG_2_DB:
            convert_value = AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSAB_NEG_2_DB;
            break;
        case AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSD_POS_1_DB:
            convert_value = AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSAB_POS_1_DB;
            break;
        case AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSD_POS_4_DB:
            convert_value = AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSAB_POS_4_DB;
            break;
        default:
            convert_value = AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSAB_NEG_5_DB;
            break;

    }
    return convert_value;
#else
    return (uint32_t)value;
#endif
}

/*******************************************************************************************
*                                         gain setting                                     *
********************************************************************************************/
void hal_audio_volume_init(void)
{
    uint32_t i;
    for (i = 0 ; i < AFE_HW_DIGITAL_GAIN_NUM ; i++) {
        afe_digital_gain[i].mute  = 0;
        afe_digital_gain[i].index = 0;
        afe_digital_gain[i].index_compensation = 0;
        afe_digital_gain[i].sample_per_step = AFE_HW_DIGITAL_GAIN_DEFAULT_SAMPLE_PER_STEP;
        afe_digital_gain[i].register_value = AFE_HW_DIGITAL_GAIN_0DB_REGISTER_VALUE;
    }

    for (i = 0 ; i < AFE_HW_ANALOG_GAIN_NUM ; i++) {
        afe_analog_gain[i].mute    = 0;
        afe_analog_gain[i].index_l = 0;
        afe_analog_gain[i].index_r = 0;
#ifdef AIR_AUDIO_LR_OUT_ANALOG_GAIN_OFFSET_ENABLE
        afe_analog_gain[i].index_offset_l = 0;
        afe_analog_gain[i].index_offset_r = 0;
#endif
        if (i == AFE_HW_ANALOG_GAIN_OUTPUT) {
#ifdef ANALOG_OUTPUT_CLASSD_ENABLE
            afe_analog_gain[i].analog_mode = HAL_AUDIO_ANALOG_OUTPUT_CLASSD;
            afe_analog_gain[i].register_value_l = AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSD_POS_0_DB; //AFE_HW_ANALOG_OUTPUT_0DB_REGISTER_VALUE;
            afe_analog_gain[i].register_value_r = AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSD_POS_0_DB; //AFE_HW_ANALOG_OUTPUT_0DB_REGISTER_VALUE;
#else
            afe_analog_gain[i].analog_mode = HAL_AUDIO_ANALOG_OUTPUT_CLASSAB;
            afe_analog_gain[i].register_value_l = AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSAB_NEG_2_DB; //AFE_HW_ANALOG_OUTPUT_0DB_REGISTER_VALUE;
            afe_analog_gain[i].register_value_r = AFE_VOLUME_ANALOG_OUTPUT_GAIN_CLASSAB_NEG_2_DB; //AFE_HW_ANALOG_OUTPUT_0DB_REGISTER_VALUE;
#endif
            HAL_AUDIO_LOG_INFO("analog_mode 0x%x, register_value_l 0x%x,register_value_r 0x%x\r\n", 3, afe_analog_gain[i].analog_mode, afe_analog_gain[i].register_value_l, afe_analog_gain[i].register_value_r);
        } else {
            afe_analog_gain[i].analog_mode = HAL_AUDIO_ANALOG_INPUT_ACC10K;
            afe_analog_gain[i].register_value_l = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_18_DB; //AFE_HW_ANALOG_INPUT_0DB_REGISTER_VALUE;
            afe_analog_gain[i].register_value_r = AFE_VOLUME_ANALOG_INPUT_GAIN_ACC10K_POS_18_DB; //AFE_HW_ANALOG_INPUT_0DB_REGISTER_VALUE;
        }
    }
}
void hal_volume_set_analog_mode(afe_hardware_analog_gain_t gain_select, hal_audio_analog_mdoe_t mdoe)
{
    afe_analog_gain[gain_select].analog_mode = mdoe;
    HAL_AUDIO_LOG_INFO("DSP - Hal Audio Volume Select:%d, mode:%d", 2, gain_select, mdoe);
}

hal_audio_analog_mdoe_t hal_volume_get_analog_mode(afe_hardware_analog_gain_t gain_select)
{
    return afe_analog_gain[gain_select].analog_mode;
}

void hal_volume_convert_register_value(afe_hardware_analog_gain_t gain_select, uint32_t *value)
{
#if (HAL_AUDIO_VOLUME_VALUE_CONVERT)
    if (gain_select == AFE_HW_ANALOG_GAIN_OUTPUT) {
        //Output analog
        if (afe_analog_gain[gain_select].analog_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSAB) {
            *value = afe_volume_convert_output_classab_mode(*value);
        }
    } else {
        //Input analog
        if (afe_analog_gain[gain_select].analog_mode == HAL_AUDIO_ANALOG_INPUT_ACC20K) {
            *value = afe_volume_convert_input_acc20k_mode(*value);
        } //else if (afe_analog_gain[gain_select].analog_mode == HAL_AUDIO_ANALOG_INPUT_DCC) {
        //*value = afe_volume_convert_input_dcc_mode(*value);
        //}
    }
#else
    UNUSED(gain_select);
    UNUSED(value);
#endif
}

/*******************************************************************************************
*                                      HW digital gain                                     *
********************************************************************************************/
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ bool afe_volume_digital_set_mute(afe_hardware_digital_gain_t gain_select, afe_volume_mute_control_t mask, bool enable)
{
    bool status_change;
    uint32_t irq_mask;
    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    status_change = afe_digital_gain[gain_select].mute > 0 ? 1 : 0;
    status_change = status_change != enable ? 1 : 0;
    if (enable) {
        afe_digital_gain[gain_select].mute |= (1 << mask);
    } else {
        afe_digital_gain[gain_select].mute &= ~(1 << mask);
    }
    hal_nvic_restore_interrupt_mask(irq_mask);
    if (status_change) {
        afe_volume_digital_update(gain_select);
        HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain %d mute enable: %d", 2, gain_select, afe_digital_gain[gain_select].mute);
    }
    return afe_digital_gain[gain_select].mute;
}

int32_t afe_volume_digital_get_gain_index(afe_hardware_digital_gain_t gain_select)
{
    if (gain_select < AFE_HW_DIGITAL_GAIN_NUM) {
        return afe_digital_gain[gain_select].index;
    }
    return 0;
}
bool afe_volume_digital_set_gain_by_index(afe_hardware_digital_gain_t gain_select, int32_t index)
{
    afe_digital_gain[gain_select].index = index;

#ifdef AIR_VOLUME_CONTROL_ON_DRC_ENABLE
    if (gain_select == AFE_HW_DIGITAL_GAIN1) {
        index = 0;
    }
#endif
    afe_digital_gain[gain_select].register_value = afe_calculate_digital_gain_index(index, AFE_HW_DIGITAL_GAIN_0DB_REGISTER_VALUE);
    afe_digital_gain[gain_select].register_value_with_compensation = afe_calculate_digital_gain_index(index + afe_digital_gain[gain_select].index_compensation, AFE_HW_DIGITAL_GAIN_0DB_REGISTER_VALUE);
    afe_volume_digital_update(gain_select);
    HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain %d set by index 0x%x register_value 0x%x", 3, gain_select, index, afe_digital_gain[gain_select].register_value);
    return false;
}

bool afe_volume_digital_set_gain_by_value(afe_hardware_digital_gain_t gain_select, uint32_t value)
{
    afe_digital_gain[gain_select].register_value = value;
    afe_volume_digital_update(gain_select);
    HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain %d set by value 0x%x", 2, gain_select, value);
    return false;
}

bool afe_volume_digital_set_gain(afe_hardware_digital_gain_t gain_select, uint32_t value)
{
    hal_hw_gain_set_target(gain_select, value);
    return false;
}

extern bool hal_audio_status_get_agent_status(hal_audio_agent_t agent);
bool afe_volume_digital_update(afe_hardware_digital_gain_t gain_select)
{
    uint32_t gain;

    if (afe_digital_gain[gain_select].mute) {
        gain = AFE_HW_DIGITAL_GAIN_MIN_REGISTER_VALUE;
    } else {
        switch (gain_select) {
            case AFE_HW_DIGITAL_GAIN1:
#ifndef AIR_AUDIO_MIXER_GAIN_ENABLE
                if (hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_BLOCK_HWGAIN2) || hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_BLOCK_HWGAIN3)) {
#else
                if (hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_BLOCK_HWGAIN2)) {
#endif                    //is concurrently
                    gain = afe_digital_gain[gain_select].register_value_with_compensation;
                    HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain gain select:%d,gain:0x%x,index:%d,cp:%d", 4, gain_select, gain, afe_digital_gain[gain_select].index, afe_digital_gain[gain_select].index_compensation);
                } else {
                    gain = afe_digital_gain[gain_select].register_value;
                    HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain gain select:%d,gain:0x%x", 2, gain_select, gain);
                }
#ifdef AIR_A2DP_DRC_TO_USE_DGAIN_ENABLE
                hal_dgian_to_drc(gain);
#endif
                break;
            case AFE_HW_DIGITAL_GAIN2:
#ifndef AIR_AUDIO_MIXER_GAIN_ENABLE
                if (hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_BLOCK_HWGAIN1) || hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_BLOCK_HWGAIN3)) {
#else
                if (hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_BLOCK_HWGAIN1)) {
#endif
                    //is concurrently
                    gain = afe_digital_gain[gain_select].register_value_with_compensation;
                    HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain gain select:%d,gain:0x%x,index:%d,cp:%d", 4, gain_select, gain, afe_digital_gain[gain_select].index, afe_digital_gain[gain_select].index_compensation);
                } else {
                    gain = afe_digital_gain[gain_select].register_value;
                    HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain gain select:%d,gain:0x%x", 2, gain_select, gain);
                }
                break;
            case AFE_HW_DIGITAL_GAIN3:
#ifndef AIR_AUDIO_MIXER_GAIN_ENABLE
                if (hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_BLOCK_HWGAIN1) || hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_BLOCK_HWGAIN2)) {
#else
                if (0) {
#endif
                    //is concurrently
                    gain = afe_digital_gain[gain_select].register_value_with_compensation;
                    HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain gain select:%d,gain:0x%x,index:%d,cp:%d", 4, gain_select, gain, afe_digital_gain[gain_select].index, afe_digital_gain[gain_select].index_compensation);
                } else {
                    gain = afe_digital_gain[gain_select].register_value;
                    HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain gain select:%d,gain:0x%x", 2, gain_select, gain);
                }
                break;
            case AFE_HW_DIGITAL_GAIN4:
#ifndef AIR_AUDIO_MIXER_GAIN_ENABLE
                if (hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_BLOCK_HWGAIN1) || hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_BLOCK_HWGAIN2)) {
#else
                if (0) {
#endif
                    //is concurrently
                    gain = afe_digital_gain[gain_select].register_value_with_compensation;
                    HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain gain select:%d,gain:0x%x,index:%d,cp:%d", 4, gain_select, gain, afe_digital_gain[gain_select].index, afe_digital_gain[gain_select].index_compensation);
                } else {
                    gain = afe_digital_gain[gain_select].register_value;
                    HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain gain select:%d,gain:0x%x", 2, gain_select, gain);
                }
                break;
            default:
                gain = 0xFFFFFFFF;
                HAL_AUDIO_LOG_INFO("DSP - Hal Audio HW Gain gain select wrong", 0);
                break;
        }

    }

    //Update gain setting
    hal_hw_gain_set_target(gain_select, gain);
    return false;
}

bool afe_volume_digital_set_compensation(afe_hardware_digital_gain_t gain_select, uint32_t index_compensation)
{
    //Set compensation when output concurrently
    afe_digital_gain[gain_select].index_compensation = index_compensation;
    return false;
}

bool afe_volume_digital_set_ramp_step(afe_hardware_digital_gain_t gain_select, uint32_t sample_per_step)
{
// Do *not* update sample_per_step when AIR_HWGAIN_SET_FADE_TIME_ENABLE=y
#ifndef AIR_HWGAIN_SET_FADE_TIME_ENABLE
    afe_digital_gain[gain_select].sample_per_step = sample_per_step;
#else
    UNUSED(gain_select);
    UNUSED(sample_per_step);
#endif
    return false;
}

uint32_t afe_volume_digital_get_ramp_step(afe_hardware_digital_gain_t gain_select)
{
    return afe_digital_gain[gain_select].sample_per_step;
}

bool afe_volume_digital_set_fade_time(afe_hardware_digital_gain_t gain_select, uint32_t sample_per_step, uint32_t down_step, uint32_t up_step)
{
    UNUSED(sample_per_step);
    hal_hw_gain_set_down_step(gain_select, down_step);
    hal_hw_gain_set_up_step(gain_select, up_step);
    //hal_hw_gain_set_sample_per_step(gain_select,sample_per_step);//cannot dynamically set.

    return false;
}

/*******************************************************************************************
*                                         analog gain                                      *
********************************************************************************************/
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ bool afe_volume_analog_set_mute(afe_hardware_analog_gain_t gain_select, afe_volume_mute_control_t mask, bool enable)
{
    uint32_t irq_mask;
    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    if (enable) {
        afe_analog_gain[gain_select].mute |= (1 << mask);
    } else {
        afe_analog_gain[gain_select].mute &= ~(1 << mask);
    }
    hal_nvic_restore_interrupt_mask(irq_mask);
    //Update gain setting
    afe_volume_analog_update(gain_select);
    return afe_analog_gain[gain_select].mute;
}

afe_volume_analog_output_gain_value_t afe_volume_analog_get_output_mute_value(hal_audio_analog_mdoe_t analog_mode)
{
    uint32_t register_value;
    if (analog_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSD) {
        register_value = AFE_HW_ANALOG_OUTPUT_CLASSD_MIN_VALUE;
    } else {
        register_value = AFE_HW_ANALOG_OUTPUT_CLASSAB_MIN_VALUE;
    }
    return register_value;
}

bool afe_volume_analog_set_gain_by_index(afe_hardware_analog_gain_t gain_select, int32_t index_l, int32_t index_r)
{
    uint32_t gain_l, gain_r;
    if (index_l != HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX) {
        afe_analog_gain[gain_select].index_l = index_l;
    }
    if (index_r != HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX) {
        afe_analog_gain[gain_select].index_r = index_r;
    }

    if (gain_select == AFE_HW_ANALOG_GAIN_OUTPUT) {
#if 0
#ifndef CONFIG_TOOL_TEST
#ifdef ANALOG_OUTPUT_CLASSD_ENABLE
        gain_l = afe_volume_analog_convert_output_register_class_d(afe_analog_gain[gain_select].index_l);
        gain_r = afe_volume_analog_convert_output_register_class_d(afe_analog_gain[gain_select].index_r);
        afe_analog_gain[gain_select].register_value_l = gain_l;
        afe_analog_gain[gain_select].register_value_r = gain_r;
#else
        gain_l = afe_volume_analog_convert_output_register(afe_analog_gain[gain_select].index_l);
        gain_r = afe_volume_analog_convert_output_register(afe_analog_gain[gain_select].index_r);
        afe_analog_gain[gain_select].register_value_l = ((gain_r & ZCD_CON2_L_GAIN_MASK) << ZCD_CON2_R_GAIN_POS) | ((gain_l & ZCD_CON2_L_GAIN_MASK) << ZCD_CON2_L_GAIN_POS);
        afe_analog_gain[gain_select].register_value_r = afe_analog_gain[gain_select].register_value_l;
#endif
#endif
#endif
        //#ifdef CONFIG_TOOL_TEST
        if (afe_analog_gain[gain_select].analog_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSD) {
            gain_l = afe_volume_analog_convert_output_register_class_d(afe_analog_gain[gain_select].index_l + afe_audio_get_component_gain_offset(AUDIO_CALIBRATION_COMPONENT_DAC_L));
            gain_r = afe_volume_analog_convert_output_register_class_d(afe_analog_gain[gain_select].index_r + afe_audio_get_component_gain_offset(AUDIO_CALIBRATION_COMPONENT_DAC_R));
            afe_analog_gain[gain_select].register_value_l = gain_l;
            afe_analog_gain[gain_select].register_value_r = gain_r;
        } else {
#ifdef AIR_AUDIO_LR_OUT_ANALOG_GAIN_OFFSET_ENABLE
            gain_l = afe_volume_analog_convert_output_register(afe_analog_gain[gain_select].index_l + afe_analog_gain[gain_select].index_offset_l + afe_audio_get_component_gain_offset(AUDIO_CALIBRATION_COMPONENT_DAC_L));
            gain_r = afe_volume_analog_convert_output_register(afe_analog_gain[gain_select].index_r + afe_analog_gain[gain_select].index_offset_r + afe_audio_get_component_gain_offset(AUDIO_CALIBRATION_COMPONENT_DAC_R));
            DSP_MW_LOG_I("[GAIN] SET OUTPUT CLASSAB AGAIN, L=%d, R=%d, L_offset=%d, R_offset=%d, gain_l=%d, gain_R=%d", 6, index_l, index_r, afe_analog_gain[gain_select].index_offset_l, afe_analog_gain[gain_select].index_offset_r, gain_l, gain_r);
#else
            gain_l = afe_volume_analog_convert_output_register(afe_analog_gain[gain_select].index_l + afe_audio_get_component_gain_offset(AUDIO_CALIBRATION_COMPONENT_DAC_L));
            gain_r = afe_volume_analog_convert_output_register(afe_analog_gain[gain_select].index_r + afe_audio_get_component_gain_offset(AUDIO_CALIBRATION_COMPONENT_DAC_R));
#endif

            afe_analog_gain[gain_select].register_value_l = ((gain_r & ZCD_CON2_L_GAIN_MASK) << ZCD_CON2_R_GAIN_POS) | ((gain_l & ZCD_CON2_L_GAIN_MASK) << ZCD_CON2_L_GAIN_POS);
            afe_analog_gain[gain_select].register_value_r = afe_analog_gain[gain_select].register_value_l;
        }
        //#endif
    } else {
        gain_l = afe_volume_analog_convert_input_register(afe_analog_gain[gain_select].index_l);
        gain_r = afe_volume_analog_convert_input_register(afe_analog_gain[gain_select].index_r);
        afe_analog_gain[gain_select].register_value_l = gain_l;
        afe_analog_gain[gain_select].register_value_r = gain_r;
    }
    afe_volume_analog_update(gain_select);
    return false;
}

bool afe_volume_analog_set_gain_by_value(afe_hardware_analog_gain_t gain_select, uint32_t value_l, uint32_t value_r)
{
    if (value_l != HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX) {
        hal_volume_convert_register_value(gain_select, &value_l);
        afe_analog_gain[gain_select].register_value_l = value_l;
    }
    if (value_r != HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX) {
        hal_volume_convert_register_value(gain_select, &value_r);
        afe_analog_gain[gain_select].register_value_r = value_r;
    }
    afe_volume_analog_update(gain_select);
    return false;
}

#ifdef AIR_AUDIO_LR_OUT_ANALOG_GAIN_OFFSET_ENABLE
bool afe_volume_analog_set_offset_gain_by_index(afe_hardware_analog_gain_t gain_select, int32_t index_l, int32_t index_r)
{
    if (index_l != HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX) {
        afe_analog_gain[gain_select].index_offset_l = index_l;
    }
    if (index_r != HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX) {
        afe_analog_gain[gain_select].index_offset_r = index_r;
    }
    DSP_MW_LOG_I("[GAIN] SAVE OUTPUT OFFSET AGAIN, L_offset=%d, R_offset=%d", 2, afe_analog_gain[gain_select].index_offset_l, afe_analog_gain[gain_select].index_offset_r);
    return false;
}
#endif

bool afe_volume_analog_update(afe_hardware_analog_gain_t gain_select)
{
    uint32_t gain_l, gain_r;
    if (gain_select == AFE_HW_ANALOG_GAIN_OUTPUT) {
        //Output analog gain
        if (afe_analog_gain[gain_select].mute) {
            gain_l = afe_volume_analog_get_output_mute_value(afe_analog_gain[gain_select].analog_mode);
            gain_r = gain_l;
        } else {
            gain_l = afe_analog_gain[gain_select].register_value_l;
            gain_r = afe_analog_gain[gain_select].register_value_r;
        }
        if (afe_analog_gain[gain_select].analog_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSD ||
            afe_analog_gain[gain_select].analog_mode == HAL_AUDIO_ANALOG_OUTPUT_OLCLASSD) {
            hal_gain_set_analog_output_class_d(gain_l, gain_r);
        } else {
            hal_gain_set_analog_output_class_ab(hal_audio_gain_mapping_enable(gain_l, afe_adc_performance_mode[AFE_ANALOG_DAC]));
            //hal_gain_set_analog_output_class_ab(gain_l);
        }
        HAL_AUDIO_LOG_INFO("DSP - Hal Audio Gain Output Gain:0x%x, Gain_R:0x%x", 2, gain_l, gain_r);
    } else {
        //Input analog gain
        if (afe_analog_gain[gain_select].mute) {
            gain_l = AFE_HW_ANALOG_INPUT_MIN_REGISTER_VALUE;
            gain_r = AFE_HW_ANALOG_INPUT_MIN_REGISTER_VALUE;

        } else {
            gain_l = afe_analog_gain[gain_select].register_value_l;
            gain_r = afe_analog_gain[gain_select].register_value_r;
            if (afe_analog_gain[gain_select].analog_mode == HAL_AUDIO_ANALOG_INPUT_ACC20K) {
                gain_l = afe_volume_convert_input_acc20k_mode(gain_l);
                gain_r = afe_volume_convert_input_acc20k_mode(gain_r);
            } //else if (afe_analog_gain[gain_select].analog_mode == HAL_AUDIO_ANALOG_INPUT_DCC) {
            //gain_l = afe_volume_convert_input_dcc_mode(gain_l);
            //gain_r = afe_volume_convert_input_dcc_mode(gain_r);
            //}
        }
        hal_gain_set_analog_input(gain_select, gain_l, gain_r);
    }
    return false;
}

uint32_t afe_volume_analog_get_target_register_value(afe_hardware_analog_gain_t gain_select)
{
    uint32_t register_value;
    if (afe_analog_gain[gain_select].mute) {
        if (gain_select == AFE_HW_ANALOG_GAIN_OUTPUT) {
            register_value = afe_volume_analog_get_output_mute_value(afe_analog_gain[gain_select].analog_mode);
        } else {


            register_value = AFE_HW_ANALOG_INPUT_MIN_REGISTER_VALUE;
        }
    } else {
        register_value = afe_analog_gain[gain_select].register_value_l;
    }
    return register_value;
}

bool afe_volume_analog_ramp_output(uint32_t target_gain)
{
#if 1
    uint32_t target_l, target_r, current_l, current_r;
    uint32_t rg_value;
    int32_t gain_step_l, gain_step_r;

    if (afe_analog_gain[AFE_HW_ANALOG_GAIN_OUTPUT].analog_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSD) {
        //For ClassD
        if (target_gain == AFE_HW_ANALOG_OUTPUT_MIN_REGISTER_VALUE) {
            target_gain = afe_volume_analog_get_output_mute_value(HAL_AUDIO_ANALOG_OUTPUT_CLASSD);
        }
        HAL_AUDIO_DELAY_US(100);
    } else {
        //For ClassAB
        if (target_gain == AFE_HW_ANALOG_OUTPUT_MIN_REGISTER_VALUE) {
            target_gain = afe_volume_analog_get_output_mute_value(afe_analog_gain[AFE_HW_ANALOG_GAIN_OUTPUT].analog_mode);
        }

        rg_value = hal_gain_get_analog_output();
        current_l = (rg_value & ZCD_CON2_L_GAIN_MASK);
        current_r = ((rg_value >> ZCD_CON2_R_GAIN_POS)&ZCD_CON2_L_GAIN_MASK);

        target_l = (target_gain & ZCD_CON2_L_GAIN_MASK);
        target_r = ((target_gain >> ZCD_CON2_R_GAIN_POS)&ZCD_CON2_L_GAIN_MASK);

        gain_step_l = (target_l >= current_l) ? 1 : -1;
        gain_step_r = (target_r >= current_r) ? 1 : -1;

        HAL_AUDIO_LOG_INFO("DSP - Hal Audio Gain Output Analog Ramp, current:0x%x, target:0x%x", 2, rg_value, target_gain);
        while (rg_value != target_gain) {

            if (target_l != current_l) {
                current_l = current_l + gain_step_l;
            }

            if (target_r != current_r) {
                current_r = current_r + gain_step_r;
            }

            rg_value = ((current_r & ZCD_CON2_L_GAIN_MASK) << ZCD_CON2_R_GAIN_POS) | (current_l & ZCD_CON2_L_GAIN_MASK);
            hal_gain_set_analog_output_class_ab(rg_value);
            //delay timer
            HAL_AUDIO_DELAY_US(100);
        }
    }
#else
    uint32_t current;
    int32_t gain_step;

    current = AFE_READ(ZCD_CON2) & 0xFFF;

    gain_step = (target_gain >= current) ? 0x41 : -0x41;

    while (current != target_gain) {
        current = (int32_t)current + gain_step;
        AFE_SET_REG(ZCD_CON2, current, 0xfffff);

        //delay timer
    }
#endif
    return false;
}

#endif /*HAL_AUDIO_MODULE_ENABLED*/

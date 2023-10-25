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

#include "hal.h"
#ifdef HAL_PMU_MODULE_ENABLED
#include "assert.h"
#include "syslog.h"
#include "hal_pmu_internal_lp.h"
#include "hal_pmu_charger_lp.h"
#include "hal_pmu_cal_lp.h"
#include "hal_pmu_lp_platform.h"
#include "hal_flash_disk_internal.h"
#include "hal_flash.h"

#define UNUSED(x)  ((void)(x))


bat_cfg_t bat_cfg;
volt_dac_t otp_dac;
curr_val_t otp_curr;
cc_curr_t cc_curr;
vichg_adc_t vichg_adc;
uint8_t ripple_vaud18_en;
uint8_t pmu_kflag;
uint8_t pmu_otp_dump;
uint32_t pmu_fast_buffer_tick = 0;
uint8_t hpbg_trim_sel = 0;

const uint16_t bat_table[] = {
    4050,
    4100,
    4150,
    4200,
    4250,
    4300,
    4350,
    4400,
    4450,
    4500,
};

pmu_status_t pmu_get_otp(uint16_t addr, uint8_t *ptr, uint32_t size)
{
    hal_flash_status_t status = hal_flash_otp_read(addr, ptr, size);
    if (status != HAL_FLASH_STATUS_OK) {
        log_hal_msgid_error("[PMU_CAL]get_otp fail, otp_addr[%d], status[%d], size[%d]", 3, addr, status, size);
        assert(0);
        return PMU_STATUS_ERROR;
    }
    if ((addr != OTP_ADIE_VER_ADDR) && (ptr[0] != OTP_OK)) {
        log_hal_msgid_error("[PMU_CAL]get_otp fail, otp_addr[%d], kflag[%d]", 2, addr, ptr[0]);
        assert(0);
        return PMU_STATUS_ERROR;
    }
    return PMU_STATUS_SUCCESS;
}

#ifdef AIR_NVDM_ENABLE
#include "nvkey_id_list.h"
#include "nvkey.h"

/************* transfer function *************/
uint16_t pmu_range(uint16_t val, uint16_t min, uint16_t max, uint8_t byte)
{
    uint16_t result = 0;

    if (byte > 2) {
        log_hal_msgid_error("[PMU_CAL]range fail, byte[%d]", 1, byte);
        assert(0);
    }
    uint16_t thrd = 1 << (byte * 8 - 1);

    if ((min & thrd) || (min > max)) {
        log_hal_msgid_error("[PMU_CAL]range fail, min[%d], max[%d], thrd[%d]", 3, min, max, thrd);
        assert(0);
    }

    if (val & thrd) {
        result = min;
    } else {
        if (val > max) {
            result = max;
        } else {
            result = val;
        }
    }

    return result;
}

int32_t pmu_round(int32_t val1, int32_t val2)
{
    int32_t result = 0, tmp = 0;

    if (val2) {
        tmp = (val1 * 10) / val2;
        if (val1 > 0) {
            tmp += 5;
        } else {
            tmp -= 5;
        }

        result = tmp / 10;
        //log_hal_msgid_info("[PMU_CAL]round, val1[%d], val2[%d], tmp[%d], result[%d]", 4, val1, val2, tmp, result);
    } else {
        log_hal_msgid_error("[PMU_CAL]round fail, val1[%d], val2[%d]", 2, val1, val2);
        assert(0);
    }

    return result;
}

uint16_t pmu_lerp(uint16_t volt1, uint16_t adc1, uint16_t volt2, uint16_t adc2, uint16_t volt_val)
{
    int16_t adc_val = 0;
    int16_t volt_diff = 0;
    int16_t adc_diff = 0;

    if (volt1 > volt2) {
        volt_diff = volt1 - volt2;
        adc_diff = adc1 - adc2;
    } else {
        volt_diff = volt2 - volt1;
        adc_diff = adc2 - adc1;
    }

    if (volt_val > volt2) {
        adc_val = adc2 + pmu_round((adc_diff * (volt_val - volt2)), volt_diff);
    } else if (volt_val < volt2) {
        adc_val = adc2 - pmu_round((adc_diff * (volt2 - volt_val)), volt_diff);
    } else {
        adc_val = adc2;
    }
    //log_hal_msgid_info("[PMU_CAL]lerp V1[%d] A1[%d] V2[%d] A2[%d] VC[%d] AC[%d]", 6, volt1, adc1, volt2,adc2, volt_val, adc_val);

    return adc_val;
}

uint8_t pmu_vdig_bg_lerp(uint16_t volt1, uint8_t sel1, uint16_t volt2, uint8_t sel2, uint16_t volt_val)
{
    int8_t sel_val1 = 0;
    uint8_t sel_val2 = 0;

    if (sel1 > sel2) {
        sel_val1 = sel1 + pmu_round(((sel1 - sel2 - 32) * (volt_val - volt1)), (volt1 - volt2));
    } else {
        sel_val1 = sel1 + pmu_round(((sel1 + 32 - sel2) * (volt_val - volt1)), (volt1 - volt2));
    }

    if (sel_val1 < 0) {
        sel_val2 = sel_val1 + 32;
    } else if (sel_val1 > 31) {
        sel_val2 = sel_val1 - 32;
    } else {
        sel_val2 = sel_val1;
    }

    //log_hal_msgid_info("[PMU_CAL]vdig_bg_lerp, V1[%d] S1[%d] V2[%d] S2[%d] VC[%d] sel_val1[%d], sel_val2[%d]", 7, volt1, sel1, volt2, sel2, volt_val, sel_val1, sel_val2);

    return sel_val2;
}

pmu_status_t pmu_get_nvkey(uint16_t id, uint8_t *ptr, uint32_t size)
{
    uint32_t len = 0;
    nvkey_status_t status = nvkey_data_item_length(id, &len);
    if ((status != NVKEY_STATUS_OK) || (len != size)) {
        log_hal_msgid_error("[PMU_CAL]get_nvkey fail, id[0x%X], status[%d], len[%d], size[%d]", 4,
                            id, status, len, size);
        assert(0);
        return PMU_STATUS_ERROR;
    }
    status = nvkey_read_data(id, ptr, &len);
    if (status != NVKEY_STATUS_OK) {
        log_hal_msgid_error("[PMU_CAL]get_nvkey fail, id[0x%X], status[%d]", 2, id, status);
        assert(0);
        return PMU_STATUS_ERROR;
    }
    return PMU_STATUS_SUCCESS;
}

pmu_status_t pmu_set_nvkey(uint16_t id, uint8_t *ptr, uint32_t size)
{
    uint32_t len = 0;
    nvkey_status_t status = nvkey_data_item_length(id, &len);
    if ((status != NVKEY_STATUS_OK) || (len != size)) {
        log_hal_msgid_error("[PMU_CAL]set_nvkey fail, id[0x%X], status[%d], len[%d], size[%d]", 4,
                            id, status, len, size);
        assert(0);
        return PMU_STATUS_ERROR;
    }

    status = nvkey_write_data(id, ptr, len);
    if (status != NVKEY_STATUS_OK) {
        log_hal_msgid_error("[PMU_CAL]set_nvkey fail, id[0x%X], status[%d]", 2, id, status);
        assert(0);
        return PMU_STATUS_ERROR;
    }
    return PMU_STATUS_SUCCESS;
}


/************* dump function *************/
void pmu_dump_otp_lp(void)
{
    otp_vbat_t vbat;
    otp_buck_volt_t buck_volt;
    otp_buck_ipeak_t buck_ipeak;
    otp_buck_freq_t buck_freq;
    otp_buck_dl_t buck_dummy;
    otp_ldo_vsram_t ldo_vsram;
    otp_ldo_vdig18_t ldo_vdig18;
    otp_ldo_vdd33_t ldo_vdd33_reg;
    otp_ldo_vdd33_t ldo_vdd33_ret;
    otp_hpbg_t hpbg;
    otp_lpbg_t lpbg;
    otp_chg_dac_t chg_dac;
    otp_vsys_ldo_t vsys_ldo;
    otp_ocp_t ocp;
    otp_chg_4v2_curr_t chg_4v2_curr;
    otp_chg_4v35_curr_t chg_4v35_curr;
    otp_vichg_adc_val_t vichg_adc_val;
    otp_lpo32_t lpo32;
    otp_sido_1_t sido_1;
    otp_sido_2_t sido_2;
    otp_sido_3_t sido_3;
    otp_sido_4_t sido_4;
    uint8_t adie_ver;

    uint16_t addr;
    pmu_status_t status;

    addr = OTP_VBAT_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&vbat, sizeof(vbat));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]otp_dump, addr[%03d], kflag[%d], 4v35_volt[%d], 4v35_adc[%d], 4v2_volt[%d], 4v2_adc[%d], 3v_volt[%d], 3v_adc[%d]", 8,
                           addr, vbat.kflag, vbat.bat_4v35.volt, vbat.bat_4v35.adc, vbat.bat_4v2.volt, vbat.bat_4v2.adc, vbat.bat_3v.volt, vbat.bat_3v.adc);
    }

    addr = OTP_BUCK_VOLT_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&buck_volt, sizeof(buck_volt));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "vio_nm_volt[%d], vio_nm_sel[%d], vio_lpm_volt[%d], vio_lpm_sel[%d], "
            "vcore_nm_volt[%d], vcore_nm_sel[%d], vcore_lp_volt[%d], vcore_lp_sel[%d]",
            10, addr, buck_volt.kflag,
            buck_volt.vio_nm.volt, buck_volt.vio_nm.sel, buck_volt.vio_lpm.volt, buck_volt.vio_lpm.sel,
            buck_volt.vcore_nm.volt, buck_volt.vcore_nm.sel, buck_volt.vcore_lpm.volt, buck_volt.vcore_lpm.sel
        );
    }

    addr = OTP_BUCK_IPEAK_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&buck_ipeak, sizeof(buck_ipeak));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], vio_sel[%d], vcore_sel[%d]",
            4, addr, buck_volt.kflag, buck_ipeak.vio_sel, buck_ipeak.vcore_sel
        );
    }

    addr = OTP_BUCK_FREQ_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&buck_freq, sizeof(buck_freq));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], vio_freq[%d], vio_sel[%d], vcore_freq[%d], vcore_sel[%d]",
            6, addr, buck_volt.kflag, buck_freq.vio.freq, buck_freq.vio.sel, buck_freq.vcore.freq, buck_freq.vcore.sel
        );
    }

    addr = OTP_BUCK_DL_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&buck_dummy, sizeof(buck_dummy));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "vio18_dl_k_1_curr[%d], vio18_dl_k_1[%d], vio18_dl_k_2_curr[%d], vio18_dl_k_2[%d], "
            "vcore_dl_k_1_curr[%d], vcore_dl_k_1[%d], vcore_dl_k_2_curr[%d], vcore_dl_k_2[%d]",
            10, addr, buck_dummy.kflag,
            buck_dummy.vio18_dl_k[0].curr, buck_dummy.vio18_dl_k[0].val, buck_dummy.vio18_dl_k[1].curr, buck_dummy.vio18_dl_k[1].val,
            buck_dummy.vcore_dl_k[0].curr, buck_dummy.vcore_dl_k[0].val, buck_dummy.vcore_dl_k[1].curr, buck_dummy.vcore_dl_k[1].val
        );
    }

    addr = OTP_LDO_VSRAM_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&ldo_vsram, sizeof(ldo_vsram));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], vsram_votrim[%d]",
            3, addr, ldo_vsram.kflag, ldo_vsram.vsram_votrim
        );
    }

    addr = OTP_LDO_VDIG18_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&ldo_vdig18, sizeof(ldo_vdig18));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "volt0[%d], sel0[%d], volt1[%d], sel1[%d]",
            6, addr, ldo_vdig18.kflag,
            ldo_vdig18.data[0].volt, ldo_vdig18.data[0].sel, ldo_vdig18.data[1].volt, ldo_vdig18.data[1].sel
        );
    }

    addr = OTP_LDO_VDD33_REG_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&ldo_vdd33_reg, sizeof(ldo_vdd33_reg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "volt0[%d], sel0[%d], volt1[%d], sel1[%d]",
            6, addr, ldo_vdd33_reg.kflag,
            ldo_vdd33_reg.data[0].volt, ldo_vdd33_reg.data[0].sel, ldo_vdd33_reg.data[1].volt, ldo_vdd33_reg.data[1].sel
        );
    }

    addr = OTP_LDO_VDD33_RET_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&ldo_vdd33_ret, sizeof(ldo_vdd33_ret));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "volt0[%d], sel0[%d], volt1[%d], sel1[%d]",
            6, addr, ldo_vdd33_ret.kflag,
            ldo_vdd33_ret.data[0].volt, ldo_vdd33_ret.data[0].sel, ldo_vdd33_ret.data[1].volt, ldo_vdd33_ret.data[1].sel
        );
    }

    addr = OTP_HPBG_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&hpbg, sizeof(hpbg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "volt0[%d], sel0[%d], volt1[%d], sel1[%d]",
            6, addr, hpbg.kflag,
            hpbg.data[0].volt, hpbg.data[0].sel, hpbg.data[1].volt, hpbg.data[1].sel
        );
    }

    addr = OTP_LPBG_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&lpbg, sizeof(lpbg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "volt0[%d], sel0[%d], volt1[%d], sel1[%d]",
            6, addr, lpbg.kflag,
            lpbg.data[0].volt, lpbg.data[0].sel, lpbg.data[1].volt, lpbg.data[1].sel
        );
    }

    addr = OTP_CHG_DAC_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&chg_dac, sizeof(chg_dac));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], dac_4v35[%d] ,dac_4v2[%d] ,dac_4v05[%d]",
            5, addr, chg_dac.kflag, chg_dac.dac_4v35, chg_dac.dac_4v2, chg_dac.dac_4v05
        );
    }

    addr = OTP_VSYS_LDO_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&vsys_ldo, sizeof(vsys_ldo));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "volt0[%d], sel0[%d], volt1[%d], sel1[%d]",
            6, addr, vsys_ldo.kflag,
            vsys_ldo.data[0].volt, vsys_ldo.data[0].sel, vsys_ldo.data[1].volt, vsys_ldo.data[1].sel
        );
    }

    addr = OTP_OCP_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&ocp, sizeof(ocp));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], load_switch_ocp_trim[%d], vsys_ldo_ocp_trim[%d]",
            4, addr, ocp.kflag, ocp.load_switch_ocp_trim, ocp.vsys_ldo_ocp_trim
        );
    }

    addr = OTP_CHG_4V2_CURR_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&chg_4v2_curr, sizeof(chg_4v2_curr));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "curr1[%d], val1[%d], curr2[%d], val2[%d], "
            "curr3[%d], val3[%d], curr4[%d], val4[%d]",
            10, addr, chg_4v2_curr.kflag,
            chg_4v2_curr.data[0].curr, chg_4v2_curr.data[0].val, chg_4v2_curr.data[1].curr, chg_4v2_curr.data[1].val,
            chg_4v2_curr.data[2].curr, chg_4v2_curr.data[2].val, chg_4v2_curr.data[3].curr, chg_4v2_curr.data[3].val
        );
    }

    addr = OTP_CHG_4V35_CURR_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&chg_4v35_curr, sizeof(chg_4v35_curr));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "curr1[%d], val1[%d], curr2[%d], val2[%d], "
            "curr3[%d], val3[%d], curr4[%d], val4[%d]",
            10, addr, chg_4v2_curr.kflag,
            chg_4v35_curr.data[0].curr, chg_4v35_curr.data[0].val, chg_4v35_curr.data[1].curr, chg_4v35_curr.data[1].val,
            chg_4v35_curr.data[2].curr, chg_4v35_curr.data[2].val, chg_4v35_curr.data[3].curr, chg_4v35_curr.data[3].val
        );
    }

    addr = OTP_VICHG_ADC_VAL_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&vichg_adc_val, sizeof(vichg_adc_val));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], ""adc_4v35[%d] ,adc_4v2[%d] ,adc_4v05[%d]",
            5, addr, vichg_adc_val.kflag, vichg_adc_val.adc_4v35, vichg_adc_val.adc_4v2, vichg_adc_val.adc_4v05
        );
    }

    addr = OTP_LPO32_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&lpo32, sizeof(lpo32));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], ftune[%d], ctune[%d]",
            4, addr, lpo32.kflag, lpo32.ftune, lpo32.ctune
        );
    }

    addr = OTP_SIDO_1_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&sido_1, sizeof(sido_1));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], vaud18_nm_k[%d], vrf_nm_k[%d], vrf_lpm_k[%d]",
            5, addr, sido_1.kflag, sido_1.vaud18_nm_k, sido_1.vrf_nm_k, sido_1.vrf_lpm_k
        );
    }

    addr = OTP_SIDO_2_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&sido_2, sizeof(sido_2));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], vrf_ipeak_k[%d], vaud18_ipeak_k[%d]",
            4, addr, sido_2.kflag, sido_2.vrf_ipeak_k, sido_2.vaud18_ipeak_k
        );
    }

    addr = OTP_SIDO_3_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&sido_3, sizeof(sido_3));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], "
            "vrf_dl_k_1_curr[%d], vrf_dl_k_1_val[%d], vrf_dl_k_2_curr[%d], vrf_dl_k_2_val[%d], "
            "vaud18_dl_k_1_curr[%d], vaud18_dl_k_1_val[%d], vaud18_dl_k_2_curr[%d], vaud18_dl_k_2_val[%d]",
            10, addr, sido_3.kflag,
            sido_3.vrf_dl_k[0].curr, sido_3.vrf_dl_k[0].val, sido_3.vrf_dl_k[1].curr, sido_3.vrf_dl_k[1].val,
            sido_3.vaud18_dl_k[0].curr, sido_3.vaud18_dl_k[0].val, sido_3.vaud18_dl_k[1].curr, sido_3.vaud18_dl_k[1].val
        );
    }

    addr = OTP_SIDO_4_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&sido_4, sizeof(sido_4));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info(
            "[PMU_CAL]otp_dump, addr[%03d], kflag[%d], vrf_freq[%d], vaud18_freq[%d]",
            4, addr, sido_4.kflag, sido_4.vrf_freq, sido_4.vaud18_freq
        );
    }

    addr = OTP_ADIE_VER_ADDR;
    status = pmu_get_otp(addr, (uint8_t *)&adie_ver, sizeof(adie_ver));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]otp_dump, addr[%d], adie_ver[%d]", 2, addr, adie_ver);
    }
}

void pmu_dump_nvkey_lp(void)
{
    chg_cfg_t chg_cfg;
    chg_adc_cfg_t chg_adc_cfg;
    chg_dac_cfg_t chg_dac_cfg;
    chg_tri_curr_cfg_t chg_tri_curr_cfg;
    chg_cc1_curr_cfg_t chg_cc1_curr_cfg;
    chg_cc2_curr_cfg_t chg_cc2_curr_cfg;
    cv_stop_curr_cfg_t cv_stop_curr_cfg;
    sys_ldo_t sys_ldo;
    ocp_t ocp;
    jeita_t jeita;
    jeita_warm_t jeita_warm;
    jeita_cool_t jeita_cool;
    vcore_dl_t vcore_dl;
    vio18_dl_t vio18_dl;
    vaud18_dl_t vaud18_dl;
    vrf_dl_t vrf_dl;
    buck_ldo_cfg_t vdd33_reg;
    buck_ldo_cfg_t vdd33_reg_ret;
    buck_ldo_cfg_t vdd33_ret;
    buck_ldo_cfg_t ldo_vdig18_cfg;
    bg_cfg_t hpbg_cfg;
    bg_cfg_t lpbg_cfg;
    vbat_adc_cal_t vbat_adc_cal;
    vbat_volt_cfg_t vbat_volt_cfg;
    buck_ripple_ctrl_t buck_ripple_ctrl;
    buck_volt_trim_t buck_volt_trim;
    buck_ipeak_t buck_ipeak;
    buck_freq_t buck_freq;
    sido_volt_cfg_t sido_volt_cfg;
    sido_ipeak_t sido_ipeak;
    sido_freq_t sido_freq;
    buck_vcore_volt_cfg_t vcore_volt;
    buck_vio18_volt_cfg_t vio18_volt;
    buck_vrf_volt_cfg_t vrf_volt;
    buck_vaud18_volt_cfg_t vaud18_volt;
    lpo32_cfg_t lpo32;
    ldo_vsram_cfg_t vsram;

    uint16_t addr;
    pmu_status_t status;

    addr = NVID_CAL_CHG_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&chg_cfg, sizeof(chg_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_CHG_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   otp_dump[%d], bat_volt_sel[%d], two_step_en[%d], kflag[%d]",
                           4, chg_cfg.otp_dump, chg_cfg.bat_volt_sel, chg_cfg.two_step_en, chg_cfg.kflag
                          );
    }

    addr = NVID_CAL_CHG_ADC_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&chg_adc_cfg, sizeof(chg_adc_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_CHG_ADC_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "cc1_thrd_volt[%d], cc1_thrd_adc[%d], cc2_thrd_volt[%d], cc2_thrd_adc[%d], "
                           "cv_thrd_volt[%d], cv_thrd_adc[%d], rechg_volt[%d], rechg_adc[%d]",
                           8,
                           chg_adc_cfg.cc1_thrd_volt, chg_adc_cfg.cc1_thrd_adc, chg_adc_cfg.cc2_thrd_volt, chg_adc_cfg.cc2_thrd_adc,
                           chg_adc_cfg.cv_thrd_volt, chg_adc_cfg.cv_thrd_adc, chg_adc_cfg.rechg_volt, chg_adc_cfg.rechg_adc
                          );
    }

    addr = NVID_CAL_INT_CHG_DAC_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&chg_dac_cfg, sizeof(chg_dac_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_INT_CHG_DAC_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   tri_curr_dac[%d], cc1_curr_dac[%d], cc2_curr_dac[%d], cv_dac[%d]",
                           4, chg_dac_cfg.tri_curr_dac, chg_dac_cfg.cc1_curr_dac, chg_dac_cfg.cc2_curr_dac, chg_dac_cfg.cv_dac
                          );
    }

    addr = NVID_CAL_INT_CHG_TRI_CURR_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&chg_tri_curr_cfg, sizeof(chg_tri_curr_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_INT_CHG_TRI_CURR_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   cal_cnt[%d], sel[%d]",
                           2, chg_tri_curr_cfg.cal_cnt, chg_tri_curr_cfg.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr1[%d], sel1[%d], curr2[%d], sel2[%d], "
                           "curr3[%d], sel3[%d], curr4[%d], sel4[%d], curr5[%d], sel5[%d]",
                           10, chg_tri_curr_cfg.data[0].curr, chg_tri_curr_cfg.data[0].sel,
                           chg_tri_curr_cfg.data[1].curr, chg_tri_curr_cfg.data[1].sel,
                           chg_tri_curr_cfg.data[2].curr, chg_tri_curr_cfg.data[2].sel,
                           chg_tri_curr_cfg.data[3].curr, chg_tri_curr_cfg.data[3].sel,
                           chg_tri_curr_cfg.data[4].curr, chg_tri_curr_cfg.data[4].sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr6[%d], sel6[%d], curr7[%d], sel7[%d], "
                           "curr8[%d], sel8[%d], curr9[%d], sel9[%d], curr10[%d], sel10[%d],",
                           10, chg_tri_curr_cfg.data[5].curr, chg_tri_curr_cfg.data[5].sel,
                           chg_tri_curr_cfg.data[6].curr, chg_tri_curr_cfg.data[6].sel,
                           chg_tri_curr_cfg.data[7].curr, chg_tri_curr_cfg.data[7].sel,
                           chg_tri_curr_cfg.data[8].curr, chg_tri_curr_cfg.data[8].sel,
                           chg_tri_curr_cfg.data[9].curr, chg_tri_curr_cfg.data[9].sel
                          );
    }

    addr = NVID_CAL_INT_CHG_CC1_CURR_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&chg_cc1_curr_cfg, sizeof(chg_cc1_curr_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_INT_CHG_CC1_CURR_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   cal_cnt[%d], sel[%d]",
                           2, chg_cc1_curr_cfg.cal_cnt, chg_cc1_curr_cfg.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr1[%d], sel1[%d], curr2[%d], sel2[%d], "
                           "curr3[%d], sel3[%d], curr4[%d], sel4[%d], curr5[%d], sel5[%d]",
                           10, chg_cc1_curr_cfg.data[0].curr, chg_cc1_curr_cfg.data[0].sel,
                           chg_cc1_curr_cfg.data[1].curr, chg_cc1_curr_cfg.data[1].sel,
                           chg_cc1_curr_cfg.data[2].curr, chg_cc1_curr_cfg.data[2].sel,
                           chg_cc1_curr_cfg.data[3].curr, chg_cc1_curr_cfg.data[3].sel,
                           chg_cc1_curr_cfg.data[4].curr, chg_cc1_curr_cfg.data[4].sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr6[%d], sel6[%d], curr7[%d], sel7[%d], "
                           "curr8[%d], sel8[%d], curr9[%d], sel9[%d], curr10[%d], sel10[%d]",
                           10, chg_cc1_curr_cfg.data[5].curr, chg_cc1_curr_cfg.data[5].sel,
                           chg_cc1_curr_cfg.data[6].curr, chg_cc1_curr_cfg.data[6].sel,
                           chg_cc1_curr_cfg.data[7].curr, chg_cc1_curr_cfg.data[7].sel,
                           chg_cc1_curr_cfg.data[8].curr, chg_cc1_curr_cfg.data[8].sel,
                           chg_cc1_curr_cfg.data[9].curr, chg_cc1_curr_cfg.data[9].sel
                          );
    }

    addr = NVID_CAL_INT_CHG_CC2_CURR_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&chg_cc2_curr_cfg, sizeof(chg_cc2_curr_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_INT_CHG_CC2_CURR_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   cal_cnt[%d], sel[%d]",
                           2, chg_cc2_curr_cfg.cal_cnt, chg_cc2_curr_cfg.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr1[%d], sel1[%d], curr2[%d], sel2[%d], "
                           "curr3[%d], sel3[%d], curr4[%d], sel4[%d], curr5[%d], sel5[%d]",
                           10, chg_cc2_curr_cfg.data[0].curr, chg_cc2_curr_cfg.data[0].sel,
                           chg_cc2_curr_cfg.data[1].curr, chg_cc2_curr_cfg.data[1].sel,
                           chg_cc2_curr_cfg.data[2].curr, chg_cc2_curr_cfg.data[2].sel,
                           chg_cc2_curr_cfg.data[3].curr, chg_cc2_curr_cfg.data[3].sel,
                           chg_cc2_curr_cfg.data[4].curr, chg_cc2_curr_cfg.data[4].sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr6[%d], sel6[%d], curr7[%d], sel7[%d], "
                           "curr8[%d], sel8[%d], curr9[%d], sel9[%d], curr10[%d], sel10[%d]",
                           10, chg_cc2_curr_cfg.data[5].curr, chg_cc2_curr_cfg.data[5].sel,
                           chg_cc2_curr_cfg.data[6].curr, chg_cc2_curr_cfg.data[6].sel,
                           chg_cc2_curr_cfg.data[7].curr, chg_cc2_curr_cfg.data[7].sel,
                           chg_cc2_curr_cfg.data[8].curr, chg_cc2_curr_cfg.data[8].sel,
                           chg_cc2_curr_cfg.data[9].curr, chg_cc2_curr_cfg.data[9].sel
                          );
    }

    addr = NVID_CAL_CV_STOP_CURR_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&cv_stop_curr_cfg, sizeof(cv_stop_curr_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_CV_STOP_CURR_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   stop curr: "
                           "cal_cnt[%d], sel[%d], perc1[%d], adc1[%d], perc2[%d], adc2[%d]",
                           6, cv_stop_curr_cfg.cal_cnt, cv_stop_curr_cfg.sel,
                           cv_stop_curr_cfg.cv_stop_curr[0].perc, cv_stop_curr_cfg.cv_stop_curr[0].adc,
                           cv_stop_curr_cfg.cv_stop_curr[1].perc, cv_stop_curr_cfg.cv_stop_curr[1].adc
                          );

        log_hal_msgid_info("[PMU_CAL]nv_dump,   pre-complete: "
                           "int_en[%d], cal_cnt[%d], sel[%d], perc1[%d], adc1[%d], perc2[%d], adc2[%d]",
                           7, cv_stop_curr_cfg.chg_pre_compl_int_en, cv_stop_curr_cfg.pre_compl_cal_cnt, cv_stop_curr_cfg.pre_compl_sel,
                           cv_stop_curr_cfg.pre_compl_curr[0].perc, cv_stop_curr_cfg.pre_compl_curr[0].adc,
                           cv_stop_curr_cfg.pre_compl_curr[1].perc, cv_stop_curr_cfg.pre_compl_curr[1].adc
                          );
    }

    addr = NVID_CAL_SYS_LDO;
    status = pmu_get_nvkey(addr, (uint8_t *)&sys_ldo, sizeof(sys_ldo));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_SYS_LDO", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   sysldo_output_volt[%d], chg_ldo_sel[%d]",
                           2, sys_ldo.sysldo_output_volt, sys_ldo.chg_ldo_sel
                          );
    }

    addr = NVID_CAL_OCP;
    status = pmu_get_nvkey(addr, (uint8_t *)&ocp, sizeof(ocp));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_OCP", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   sw_oc_lmt[%d], i_lim_trim[%d]",
                           2, ocp.sw_oc_lmt, ocp.i_lim_trim
                          );
    }

    addr = NVID_CAL_JEITA;
    status = pmu_get_nvkey(addr, (uint8_t *)&jeita, sizeof(jeita));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_JEITA", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "cool_state_curr_perc[%d], cool_state_dac_dec[%d], warm_state_curr_perc[%d], warm_state_dac_dec[%d]",
                           4, jeita.cool_state_curr_perc, jeita.cool_state_dac_dec,
                           jeita.warm_state_curr_perc, jeita.warm_state_dac_dec
                          );
    }

    addr = NVID_CAL_JEITA_WARM;
    status = pmu_get_nvkey(addr, (uint8_t *)&jeita_warm, sizeof(jeita_warm));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_JEITA_WARM", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "cc2_thrd_volt[%d], cc2_thrd_adc[%d], cv_thrd_volt[%d], cv_thrd_adc[%d], "
                           "rechg_volt[%d], rechg_adc[%d], cc1_curr_dac[%d], cc2_curr_dac[%d]",
                           8,
                           jeita_warm.cc2_thrd_volt, jeita_warm.cc2_thrd_adc,
                           jeita_warm.cv_thrd_volt, jeita_warm.cv_thrd_adc,
                           jeita_warm.rechg_volt, jeita_warm.rechg_adc,
                           jeita_warm.cc1_curr_dac, jeita_warm.cc2_curr_dac
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   cv_dac[%d], "
                           "cc1_curr[%d], cc1_rsel[%d], cc2_curr[%d], cc2_rsel[%d], "
                           "cv_stop_curr_adc[%d], pre_compl_curr_adc[%d]",
                           7, jeita_warm.cv_dac,
                           jeita_warm.cc1_curr, jeita_warm.cc1_rsel,
                           jeita_warm.cc2_curr, jeita_warm.cc2_rsel,
                           jeita_warm.cv_stop_curr_adc, jeita_warm.pre_compl_curr_adc
                          );
    }

    addr = NVID_CAL_JEITA_COOL;
    status = pmu_get_nvkey(addr, (uint8_t *)&jeita_cool, sizeof(jeita_cool));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_JEITA_COOL", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "cc2_thrd_volt[%d], cc2_thrd_adc[%d], cv_thrd_volt[%d], rechg_adc[%d], "
                           "rechg_volt[%d], rechg_adc[%d], cc1_curr_dac[%d], cc2_curr_dac[%d]",
                           8,
                           jeita_cool.cc2_thrd_volt, jeita_cool.cc2_thrd_adc,
                           jeita_cool.cv_thrd_volt, jeita_cool.cv_thrd_adc,
                           jeita_cool.rechg_volt, jeita_cool.rechg_adc,
                           jeita_cool.cc1_curr_dac, jeita_cool.cc2_curr_dac
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   cv_dac[%d], "
                           "cc1_curr[%d], cc1_rsel[%d], cc2_curr[%d], cc2_rsel[%d], "
                           "cv_stop_curr_adc[%d], pre_compl_curr_adc[%d]",
                           7, jeita_cool.cv_dac,
                           jeita_cool.cc1_curr, jeita_cool.cc1_rsel,
                           jeita_cool.cc2_curr, jeita_cool.cc2_rsel,
                           jeita_cool.cv_stop_curr_adc, jeita_cool.pre_compl_curr_adc
                          );
    }

    addr = NVID_CAL_VCORE_DL;
    status = pmu_get_nvkey(addr, (uint8_t *)&vcore_dl, sizeof(vcore_dl));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VCORE_DL", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "vcore_dummy_en[%d], curr1[%d], val1[%d], curr2[%d], val2[%d], cal_cnt[%d], sel[%d]",
                           7, vcore_dl.vcore_dummy_en,
                           vcore_dl.otp[0].curr, vcore_dl.otp[0].val,
                           vcore_dl.otp[1].curr, vcore_dl.otp[1].val,
                           vcore_dl.cal_cnt, vcore_dl.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr1[%d], val1[%d], curr2[%d], val2[%d], "
                           "curr3[%d], val3[%d], curr4[%d], val4[%d]",
                           8,
                           vcore_dl.data[0].curr, vcore_dl.data[0].val, vcore_dl.data[1].curr, vcore_dl.data[1].val,
                           vcore_dl.data[2].curr, vcore_dl.data[2].val, vcore_dl.data[3].curr, vcore_dl.data[3].val
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr5[%d], val5[%d], curr6[%d], val6[%d], "
                           "curr7[%d], val7[%d], curr8[%d], val8[%d]",
                           8,
                           vcore_dl.data[4].curr, vcore_dl.data[4].val, vcore_dl.data[5].curr, vcore_dl.data[5].val,
                           vcore_dl.data[6].curr, vcore_dl.data[6].val, vcore_dl.data[7].curr, vcore_dl.data[7].val
                          );
    }

    addr = NVID_CAL_VIO18_DL;
    status = pmu_get_nvkey(addr, (uint8_t *)&vio18_dl, sizeof(vio18_dl));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VIO18_DL", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "en[%d], curr1[%d], val1[%d], curr2[%d], val2[%d], cal_cnt[%d], sel[%d]",
                           7, vio18_dl.en,
                           vio18_dl.otp[0].curr, vio18_dl.otp[0].val,
                           vio18_dl.otp[1].curr, vio18_dl.otp[1].val,
                           vio18_dl.cal_cnt, vio18_dl.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr1[%d], val1[%d], curr2[%d], val2[%d], "
                           "curr3[%d], val3[%d], curr4[%d], val4[%d]",
                           8,
                           vio18_dl.data[0].curr, vio18_dl.data[0].val, vio18_dl.data[1].curr, vio18_dl.data[1].val,
                           vio18_dl.data[2].curr, vio18_dl.data[2].val, vio18_dl.data[3].curr, vio18_dl.data[3].val
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr5[%d], val5[%d], curr6[%d], val6[%d], "
                           "curr7[%d], val7[%d], curr8[%d], val8[%d]",
                           8,
                           vio18_dl.data[4].curr, vio18_dl.data[4].val, vio18_dl.data[5].curr, vio18_dl.data[5].val,
                           vio18_dl.data[6].curr, vio18_dl.data[6].val, vio18_dl.data[7].curr, vio18_dl.data[7].val
                          );
    }

    addr = NVID_CAL_VAUD18_DL;
    status = pmu_get_nvkey(addr, (uint8_t *)&vaud18_dl, sizeof(vaud18_dl));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VAUD18_DL", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "en[%d], curr1[%d], val1[%d], curr2[%d], val2[%d], cal_cnt[%d], sel[%d]",
                           7, vaud18_dl.en,
                           vaud18_dl.otp[0].curr, vaud18_dl.otp[0].val,
                           vaud18_dl.otp[1].curr, vaud18_dl.otp[1].val,
                           vaud18_dl.cal_cnt, vaud18_dl.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr1[%d], val1[%d], curr2[%d], val2[%d], "
                           "curr3[%d], val3[%d], curr4[%d], val4[%d]",
                           8,
                           vaud18_dl.data[0].curr, vaud18_dl.data[0].val, vaud18_dl.data[1].curr, vaud18_dl.data[1].val,
                           vaud18_dl.data[2].curr, vaud18_dl.data[2].val, vaud18_dl.data[3].curr, vaud18_dl.data[3].val
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr5[%d], val5[%d], curr6[%d], val6[%d], "
                           "curr7[%d], val7[%d], curr8[%d], val8[%d]",
                           8,
                           vaud18_dl.data[4].curr, vaud18_dl.data[4].val, vaud18_dl.data[5].curr, vaud18_dl.data[5].val,
                           vaud18_dl.data[6].curr, vaud18_dl.data[6].val, vaud18_dl.data[7].curr, vaud18_dl.data[7].val
                          );
    }

    addr = NVID_CAL_VRF_DL;
    status = pmu_get_nvkey(addr, (uint8_t *)&vrf_dl, sizeof(vrf_dl));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VRF_DL", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "en[%d], curr1[%d], val1[%d], curr2[%d], val2[%d], cal_cnt[%d], sel[%d]",
                           7, vrf_dl.en,
                           vrf_dl.otp[0].curr, vrf_dl.otp[0].val,
                           vrf_dl.otp[1].curr, vrf_dl.otp[1].val,
                           vrf_dl.cal_cnt, vrf_dl.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr1[%d], val1[%d], curr2[%d], val2[%d], "
                           "curr3[%d], val3[%d], curr4[%d], val4[%d]",
                           8,
                           vrf_dl.data[0].curr, vrf_dl.data[0].val, vrf_dl.data[1].curr, vrf_dl.data[1].val,
                           vrf_dl.data[2].curr, vrf_dl.data[2].val, vrf_dl.data[3].curr, vrf_dl.data[3].val
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   curr5[%d], val5[%d], curr6[%d], val6[%d], "
                           "curr7[%d], val7[%d], curr8[%d], val8[%d]",
                           8,
                           vrf_dl.data[4].curr, vrf_dl.data[4].val, vrf_dl.data[5].curr, vrf_dl.data[5].val,
                           vrf_dl.data[6].curr, vrf_dl.data[6].val, vrf_dl.data[7].curr, vrf_dl.data[7].val
                          );
    }

    addr = NVID_CAL_LDO_VDD33_REG_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&vdd33_reg, sizeof(vdd33_reg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_LDO_VDD33_REG_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "kflag[%d], sloep1_volt[%d], sloep1_sel[%d], sloep2_volt[%d], sloep2_sel[%d], "
                           "cal_cnt[%d], sel[%d]",
                           7,
                           vdd33_reg.kflag,
                           vdd33_reg.otp[0].volt, vdd33_reg.otp[0].sel,
                           vdd33_reg.otp[1].volt, vdd33_reg.otp[1].sel,
                           vdd33_reg.cal_cnt, vdd33_reg.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt1[%d], sel1[%d], volt2[%d], sel2[%d], "
                           "volt3[%d], sel3[%d], volt4[%d], sel4[%d]",
                           8,
                           vdd33_reg.data[0].volt, vdd33_reg.data[0].sel,
                           vdd33_reg.data[1].volt, vdd33_reg.data[1].sel,
                           vdd33_reg.data[2].volt, vdd33_reg.data[2].sel,
                           vdd33_reg.data[3].volt, vdd33_reg.data[3].sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt5[%d], sel5[%d], volt6[%d], sel6[%d], "
                           "volt7[%d], sel7[%d], volt8[%d], sel8[%d]",
                           8,
                           vdd33_reg.data[4].volt, vdd33_reg.data[4].sel,
                           vdd33_reg.data[5].volt, vdd33_reg.data[5].sel,
                           vdd33_reg.data[6].volt, vdd33_reg.data[6].sel,
                           vdd33_reg.data[7].volt, vdd33_reg.data[7].sel
                          );
    }

    addr = NVID_CAL_LDO_VDD33_REG_RET_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&vdd33_reg_ret, sizeof(vdd33_reg_ret));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_LDO_VDD33_REG_RET_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "kflag[%d], sloep1_volt[%d], sloep1_sel[%d], sloep2_volt[%d], sloep2_sel[%d], "
                           "cal_cnt[%d], sel[%d]",
                           7,
                           vdd33_reg_ret.kflag,
                           vdd33_reg_ret.otp[0].volt, vdd33_reg_ret.otp[0].sel,
                           vdd33_reg_ret.otp[1].volt, vdd33_reg_ret.otp[1].sel,
                           vdd33_reg_ret.cal_cnt, vdd33_reg_ret.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt1[%d], sel1[%d], volt2[%d], sel2[%d], "
                           "volt3[%d], sel3[%d], volt4[%d], sel4[%d]",
                           8,
                           vdd33_reg_ret.data[0].volt, vdd33_reg_ret.data[0].sel,
                           vdd33_reg_ret.data[1].volt, vdd33_reg_ret.data[1].sel,
                           vdd33_reg_ret.data[2].volt, vdd33_reg_ret.data[2].sel,
                           vdd33_reg_ret.data[3].volt, vdd33_reg_ret.data[3].sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt5[%d], sel5[%d], volt6[%d], sel6[%d], "
                           "volt7[%d], sel7[%d], volt8[%d], sel8[%d]",
                           8,
                           vdd33_reg_ret.data[4].volt, vdd33_reg_ret.data[4].sel,
                           vdd33_reg_ret.data[5].volt, vdd33_reg_ret.data[5].sel,
                           vdd33_reg_ret.data[6].volt, vdd33_reg_ret.data[6].sel,
                           vdd33_reg_ret.data[7].volt, vdd33_reg_ret.data[7].sel
                          );
    }

    addr = NVID_CAL_LDO_VDD33_RET_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&vdd33_ret, sizeof(vdd33_ret));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_LDO_VDD33_RET_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "kflag[%d], sloep1_volt[%d], sloep1_sel[%d], sloep2_volt[%d], sloep2_sel[%d], "
                           "cal_cnt[%d], sel[%d]",
                           7,
                           vdd33_ret.kflag,
                           vdd33_ret.otp[0].volt, vdd33_ret.otp[0].sel,
                           vdd33_ret.otp[1].volt, vdd33_ret.otp[1].sel,
                           vdd33_ret.cal_cnt, vdd33_ret.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt1[%d], sel1[%d], volt2[%d], sel2[%d], "
                           "volt3[%d], sel3[%d], volt4[%d], sel4[%d]",
                           8,
                           vdd33_ret.data[0].volt, vdd33_ret.data[0].sel,
                           vdd33_ret.data[1].volt, vdd33_ret.data[1].sel,
                           vdd33_ret.data[2].volt, vdd33_ret.data[2].sel,
                           vdd33_ret.data[3].volt, vdd33_ret.data[3].sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt5[%d], sel5[%d], volt6[%d], sel6[%d], "
                           "volt7[%d], sel7[%d], volt8[%d], sel8[%d]",
                           8,
                           vdd33_ret.data[4].volt, vdd33_ret.data[4].sel,
                           vdd33_ret.data[5].volt, vdd33_ret.data[5].sel,
                           vdd33_ret.data[6].volt, vdd33_ret.data[6].sel,
                           vdd33_ret.data[7].volt, vdd33_ret.data[7].sel
                          );
    }

    addr = NVID_CAL_VDIG18_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&ldo_vdig18_cfg, sizeof(ldo_vdig18_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VDIG18_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "kflag[%d], sloep1_volt[%d], sloep1_sel[%d], sloep2_volt[%d], sloep2_sel[%d], "
                           "cal_cnt[%d], sel[%d]",
                           7,
                           ldo_vdig18_cfg.kflag,
                           ldo_vdig18_cfg.otp[0].volt, ldo_vdig18_cfg.otp[0].sel,
                           ldo_vdig18_cfg.otp[1].volt, ldo_vdig18_cfg.otp[1].sel,
                           ldo_vdig18_cfg.cal_cnt, ldo_vdig18_cfg.sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt1[%d], sel1[%d], volt2[%d], sel2[%d], "
                           "volt3[%d], sel3[%d], volt4[%d], sel4[%d]",
                           8,
                           ldo_vdig18_cfg.data[0].volt, ldo_vdig18_cfg.data[0].sel,
                           ldo_vdig18_cfg.data[1].volt, ldo_vdig18_cfg.data[1].sel,
                           ldo_vdig18_cfg.data[2].volt, ldo_vdig18_cfg.data[2].sel,
                           ldo_vdig18_cfg.data[3].volt, ldo_vdig18_cfg.data[3].sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt5[%d], sel5[%d], volt6[%d], sel6[%d], "
                           "volt7[%d], sel7[%d], volt8[%d], sel8[%d]",
                           8,
                           ldo_vdig18_cfg.data[4].volt, ldo_vdig18_cfg.data[4].sel,
                           ldo_vdig18_cfg.data[5].volt, ldo_vdig18_cfg.data[5].sel,
                           ldo_vdig18_cfg.data[6].volt, ldo_vdig18_cfg.data[6].sel,
                           ldo_vdig18_cfg.data[7].volt, ldo_vdig18_cfg.data[7].sel
                          );
    }

    addr = NVID_CAL_HPBG_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&hpbg_cfg, sizeof(hpbg_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_HPBG_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "kflag[%d], sloep1_volt[%d], sloep1_sel[%d], sloep2_volt[%d], sloep2_sel[%d]",
                           5,
                           hpbg_cfg.kflag,
                           hpbg_cfg.otp[0].volt, hpbg_cfg.otp[0].sel,
                           hpbg_cfg.otp[1].volt, hpbg_cfg.otp[1].sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "cal_cnt[%d], sel[%d], "
                           "volt1[%d], val1[%d], volt2[%d], val2[%d]",
                           6,
                           hpbg_cfg.cal_cnt, hpbg_cfg.sel,
                           hpbg_cfg.data[0].volt, hpbg_cfg.data[0].sel,
                           hpbg_cfg.data[1].volt, hpbg_cfg.data[1].sel
                          );
    }

    addr = NVID_CAL_LPBG_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&lpbg_cfg, sizeof(lpbg_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_LPBG_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "kflag[%d], sloep1_volt[%d], sloep1_sel[%d], sloep2_volt[%d], sloep2_sel[%d]",
                           5,
                           lpbg_cfg.kflag,
                           lpbg_cfg.otp[0].volt, lpbg_cfg.otp[0].sel,
                           lpbg_cfg.otp[1].volt, lpbg_cfg.otp[1].sel
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "cal_cnt[%d], sel[%d], "
                           "volt1[%d], val1[%d], volt2[%d], val2[%d]",
                           6,
                           lpbg_cfg.cal_cnt, lpbg_cfg.sel,
                           lpbg_cfg.data[0].volt, lpbg_cfg.data[0].sel,
                           lpbg_cfg.data[1].volt, lpbg_cfg.data[1].sel
                          );
    }

    addr = NVID_CAL_VBAT_ADC_CAL_TABLE;
    status = pmu_get_nvkey(addr, (uint8_t *)&vbat_adc_cal, sizeof(vbat_adc_cal));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VBAT_ADC_CAL_TABLE", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   kflag[%d], cal_cnt[%d]",
                           2, vbat_adc_cal.kflag, vbat_adc_cal.cal_cnt
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt1[%d], adc1[%d], volt2[%d], adc2[%d], "
                           "volt3[%d], adc3[%d], volt4[%d], adc4[%d], "
                           "volt5[%d], adc5[%d]",
                           10,
                           vbat_adc_cal.data[0].volt, vbat_adc_cal.data[0].adc,
                           vbat_adc_cal.data[1].volt, vbat_adc_cal.data[1].adc,
                           vbat_adc_cal.data[2].volt, vbat_adc_cal.data[2].adc,
                           vbat_adc_cal.data[3].volt, vbat_adc_cal.data[3].adc,
                           vbat_adc_cal.data[4].volt, vbat_adc_cal.data[4].adc
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt6[%d], adc6[%d], volt7[%d], adc7[%d], "
                           "volt8[%d], adc8[%d], volt9[%d], adc9[%d], "
                           "volt10[%d], adc10[%d]",
                           10,
                           vbat_adc_cal.data[5].volt, vbat_adc_cal.data[5].adc,
                           vbat_adc_cal.data[6].volt, vbat_adc_cal.data[6].adc,
                           vbat_adc_cal.data[7].volt, vbat_adc_cal.data[7].adc,
                           vbat_adc_cal.data[8].volt, vbat_adc_cal.data[8].adc,
                           vbat_adc_cal.data[9].volt, vbat_adc_cal.data[9].adc
                          );
    }

    addr = NVID_CAL_VBAT_VOLT_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&vbat_volt_cfg, sizeof(vbat_volt_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VBAT_VOLT_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   kflag[%d], init_bat_volt[%d], init_bat_adc[%d], "
                           "sd_bat_volt[%d], sd_bat_adc[%d]",
                           5, vbat_volt_cfg.kflag,
                           vbat_volt_cfg.init_bat.volt, vbat_volt_cfg.init_bat.adc,
                           vbat_volt_cfg.sd_bat.volt, vbat_volt_cfg.sd_bat.adc
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt1[%d], adc1[%d], volt2[%d], adc2[%d], "
                           "volt3[%d], adc3[%d], volt4[%d], adc4[%d], "
                           "volt5[%d], adc5[%d]",
                           10,
                           vbat_volt_cfg.data[0].volt, vbat_volt_cfg.data[0].adc,
                           vbat_volt_cfg.data[1].volt, vbat_volt_cfg.data[1].adc,
                           vbat_volt_cfg.data[2].volt, vbat_volt_cfg.data[2].adc,
                           vbat_volt_cfg.data[3].volt, vbat_volt_cfg.data[3].adc,
                           vbat_volt_cfg.data[4].volt, vbat_volt_cfg.data[4].adc
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "volt6[%d], adc6[%d], volt7[%d], adc7[%d], "
                           "volt8[%d], adc8[%d], volt9[%d], adc9[%d], ",
                           10,
                           vbat_volt_cfg.data[5].volt, vbat_volt_cfg.data[5].adc,
                           vbat_volt_cfg.data[6].volt, vbat_volt_cfg.data[6].adc,
                           vbat_volt_cfg.data[7].volt, vbat_volt_cfg.data[7].adc,
                           vbat_volt_cfg.data[8].volt, vbat_volt_cfg.data[8].adc
                          );

        log_hal_msgid_info("[PMU_CAL]nv_dump,   rsv: "
                           "volt1[%d], adc1[%d], volt2[%d], adc2[%d], "
                           "volt3[%d], adc3[%d], volt4[%d], adc4[%d], "
                           "volt5[%d], adc5[%d]",
                           10,
                           vbat_volt_cfg.data[9].volt, vbat_volt_cfg.data[9].adc,
                           vbat_volt_cfg.data[10].volt, vbat_volt_cfg.data[10].adc,
                           vbat_volt_cfg.data[11].volt, vbat_volt_cfg.data[11].adc,
                           vbat_volt_cfg.data[12].volt, vbat_volt_cfg.data[12].adc,
                           vbat_volt_cfg.data[13].volt, vbat_volt_cfg.data[13].adc
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   rsv: "
                           "volt6[%d], adc6[%d], volt7[%d], adc7[%d], "
                           "volt8[%d], adc8[%d], volt9[%d], adc9[%d], ",
                           10,
                           vbat_volt_cfg.data[14].volt, vbat_volt_cfg.data[14].adc,
                           vbat_volt_cfg.data[15].volt, vbat_volt_cfg.data[15].adc,
                           vbat_volt_cfg.data[16].volt, vbat_volt_cfg.data[16].adc,
                           vbat_volt_cfg.data[17].volt, vbat_volt_cfg.data[17].adc
                          );
    }

    addr = NVID_CAL_BUCK_RIPPLE_CTL;
    status = pmu_get_nvkey(addr, (uint8_t *)&buck_ripple_ctrl, sizeof(buck_ripple_ctrl));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_BUCK_RIPPLE_CTL", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "vcore_en[%d], vio18_en[%d], vaud18_en[%d], vpa_en[%d], vrf_en[%d]",
                           5,
                           buck_ripple_ctrl.ripple_vcore_en, buck_ripple_ctrl.ripple_vio18_en,
                           buck_ripple_ctrl.ripple_vaud18_en, buck_ripple_ctrl.ripple_vpa_en,
                           buck_ripple_ctrl.ripple_vrf_en
                          );
    }

    addr = NVID_CAL_BUCK_VOLT_TRIM;
    status = pmu_get_nvkey(addr, (uint8_t *)&buck_volt_trim, sizeof(buck_volt_trim));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_BUCK_VOLT_TRIM", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "vcore_nm_trim[%d], vcore_lpm_trim[%d], vio18_nm_trim[%d], vio18_lpm_trim[%d]",
                           4,
                           buck_volt_trim.vcore_nm_trim, buck_volt_trim.vcore_lpm_trim,
                           buck_volt_trim.vio18_nm_trim, buck_volt_trim.vio18_lpm_trim
                          );
    }

    addr = NVID_CAL_BUCK_IPEAK;
    status = pmu_get_nvkey(addr, (uint8_t *)&buck_ipeak, sizeof(buck_ipeak));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_BUCK_IPEAK", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   vcore_ipeak_trim[%d], vio18_ipeak_trim[%d]",
                           2,
                           buck_ipeak.vcore_ipeak_trim, buck_ipeak.vio18_ipeak_trim
                          );
    }

    addr = NVID_CAL_BUCK_FREQ;
    status = pmu_get_nvkey(addr, (uint8_t *)&buck_freq, sizeof(buck_freq));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_BUCK_FREQ", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   vcore_freq[%d], vio18_freq[%d]",
                           2,
                           buck_freq.vcore_freq, buck_freq.vio18_freq
                          );
    }

    addr = NVID_CAL_SIDO_VOLT;
    status = pmu_get_nvkey(addr, (uint8_t *)&sido_volt_cfg, sizeof(sido_volt_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_SIDO_VOLT", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "vaud18_nm_trim[%d], vaud18_lpm_trim[%d], vrf_nm_trim[%d], vrf_lpm_trim[%d]",
                           4,
                           sido_volt_cfg.vaud18_nm_trim, sido_volt_cfg.vaud18_lpm_trim,
                           sido_volt_cfg.vrf_nm_trim, sido_volt_cfg.vrf_lpm_trim
                          );
    }

    addr = NVID_CAL_SIDO_IPEAK;
    status = pmu_get_nvkey(addr, (uint8_t *)&sido_ipeak, sizeof(sido_ipeak));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_SIDO_IPEAK", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   vaud18_ipeak_trim[%d], vrf_ipeak_trim[%d]",
                           2,
                           sido_ipeak.vaud18_ipeak_trim, sido_ipeak.vrf_ipeak_trim
                          );
    }

    addr = NVID_CAL_SIDO_FREQ;
    status = pmu_get_nvkey(addr, (uint8_t *)&sido_freq, sizeof(sido_freq));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_SIDO_FREQ", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   vaud18_freq[%d], vrf_freq[%d]",
                           2,
                           sido_freq.vaud18_freq, sido_freq.vrf_freq
                          );
    }

    addr = NVID_CAL_VCORE_VOLT;
    status = pmu_get_nvkey(addr, (uint8_t *)&vcore_volt, sizeof(vcore_volt));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VCORE_VOLT", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   kflag[%d]", 1, vcore_volt.kflag);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "hv_volt[%d], hv_val[%d], hv_ipeak[%d], "
                           "mv_volt[%d], mv_val[%d], mv_ipeak[%d]",
                           6,
                           vcore_volt.hv.volt, vcore_volt.hv.val, vcore_volt.hv.ipeak,
                           vcore_volt.mv.volt, vcore_volt.mv.val, vcore_volt.mv.ipeak
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "lv_volt[%d], lv_val[%d], lv_ipeak[%d], "
                           "slp_volt[%d], slp_val[%d], slp_ipeak[%d]",
                           6,
                           vcore_volt.lv.volt, vcore_volt.lv.val, vcore_volt.lv.ipeak,
                           vcore_volt.slp.volt, vcore_volt.slp.val, vcore_volt.slp.ipeak
                          );
    }

    addr = NVID_CAL_VIO18_VOLT;
    status = pmu_get_nvkey(addr, (uint8_t *)&vio18_volt, sizeof(vio18_volt));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VIO18_VOLT", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   kflag[%d]", 1, vio18_volt.kflag);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "nm_volt[%d], nm_val[%d], nm_ipeak[%d], "
                           "slp_volt[%d], slp_val[%d], slp_ipeak[%d]",
                           6,
                           vio18_volt.nm.volt, vio18_volt.nm.val, vio18_volt.nm.ipeak,
                           vio18_volt.slp.volt, vio18_volt.slp.val, vio18_volt.slp.ipeak
                          );
    }

    addr = NVID_CAL_VRF_VOLT;
    status = pmu_get_nvkey(addr, (uint8_t *)&vrf_volt, sizeof(vrf_volt));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VRF_VOLT", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   kflag[%d]", 1, vrf_volt.kflag);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "nm_volt[%d], nm_val[%d], nm_ipeak[%d], "
                           "slp_volt[%d], slp_val[%d], slp_ipeak[%d]",
                           6,
                           vrf_volt.nm.volt, vrf_volt.nm.val, vrf_volt.nm.ipeak,
                           vrf_volt.slp.volt, vrf_volt.slp.val, vrf_volt.slp.ipeak
                          );
    }

    addr = NVID_CAL_VAUD18_VOLT;
    status = pmu_get_nvkey(addr, (uint8_t *)&vaud18_volt, sizeof(vaud18_volt));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_VAUD18_VOLT", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   kflag[%d]", 1, vaud18_volt.kflag);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "hv_volt[%d], hv_val[%d], hv_ipeak[%d], "
                           "mv_volt[%d], mv_val[%d], mv_ipeak[%d]",
                           6,
                           vaud18_volt.hv.volt, vaud18_volt.hv.val, vaud18_volt.hv.ipeak,
                           vaud18_volt.mv.volt, vaud18_volt.mv.val, vaud18_volt.mv.ipeak
                          );
        log_hal_msgid_info("[PMU_CAL]nv_dump,   "
                           "lv_volt[%d], lv_val[%d], lv_ipeak[%d], ",
                           3,
                           vaud18_volt.lv.volt, vaud18_volt.lv.val, vaud18_volt.lv.ipeak
                          );
    }

    addr = NVID_CAL_LPO32_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&lpo32, sizeof(lpo32));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_LPO32_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   ftune[%d], ctune[%d]", 2, lpo32.ftune, lpo32.ctune);
    }

    addr = NVID_CAL_LDO_VSRAM_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&vsram, sizeof(vsram));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]nv_dump, addr[0x%04X] NVID_CAL_LDO_VSRAM_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]nv_dump,   volt[%d], vcore_nm_trim[%d]",
                           2, vsram.volt, vsram.vcore_nm_trim
                          );
    }
}

/*******************pmu_set_init********************/
#ifndef AIR_PMU_DISABLE_CHARGER
void pmu_set_chg_adc_cfg(void)
{
    chg_adc_cfg_t nvkey;
    if (pmu_get_nvkey(NVID_CAL_CHG_ADC_CFG, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(CC1_THRESHOLD_ADDR, CC1_THRESHOLD_MASK, CC1_THRESHOLD_SHIFT, nvkey.cc1_thrd_adc);
    pmu_set_register_value_lp(CC2_THRESHOLD_ADDR, CC2_THRESHOLD_MASK, CC2_THRESHOLD_SHIFT, nvkey.cc2_thrd_adc);
    pmu_set_register_value_lp(CV_THRESHOLD_ADDR, CV_THRESHOLD_MASK, CV_THRESHOLD_SHIFT, nvkey.cv_thrd_adc);
    pmu_set_register_value_lp(RECHARGE_THRESHOLD_ADDR, RECHARGE_THRESHOLD_MASK, RECHARGE_THRESHOLD_SHIFT, nvkey.rechg_adc);
}

void pmu_set_chg_dac_cfg(void)
{
    chg_dac_cfg_t nvkey;
    if (pmu_get_nvkey(NVID_CAL_INT_CHG_DAC_CFG, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(TRICKLE_DAC_VALUE_ADDR, TRICKLE_DAC_VALUE_MASK, TRICKLE_DAC_VALUE_SHIFT, nvkey.tri_curr_dac);
    pmu_set_register_value_lp(TRICKLE_DAC_OUT_UPDATE_ADDR, TRICKLE_DAC_OUT_UPDATE_MASK, TRICKLE_DAC_OUT_UPDATE_SHIFT, 1);

    pmu_set_register_value_lp(CC1_DAC_VALUE_ADDR, CC1_DAC_VALUE_MASK, CC1_DAC_VALUE_SHIFT, nvkey.cc1_curr_dac);
    pmu_set_register_value_lp(CC1_DAC_OUT_UPDATE_ADDR, CC1_DAC_OUT_UPDATE_MASK, CC1_DAC_OUT_UPDATE_SHIFT, 1);

    pmu_set_register_value_lp(CC2_DAC_VALUE_ADDR, CC2_DAC_VALUE_MASK, CC2_DAC_VALUE_SHIFT, nvkey.cc2_curr_dac);
    pmu_set_register_value_lp(CC2_DAC_OUT_UPDATE_ADDR, CC2_DAC_OUT_UPDATE_MASK, CC2_DAC_OUT_UPDATE_SHIFT, 1);

    pmu_set_register_value_lp(CV_DAC_VALUE_ADDR, CV_DAC_VALUE_MASK, CV_DAC_VALUE_SHIFT, nvkey.cv_dac);
    pmu_set_register_value_lp(CV_DAC_OUT_UPDATE_ADDR, CV_DAC_OUT_UPDATE_MASK, CV_DAC_OUT_UPDATE_SHIFT, 1);
}

void pmu_set_chg_tri_curr_cfg(void)
{
    chg_tri_curr_cfg_t nvkey;
    if (pmu_get_nvkey(NVID_CAL_INT_CHG_TRI_CURR_CFG, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(TRICKLE_RCHG_SEL_ADDR, TRICKLE_RCHG_SEL_MASK, TRICKLE_RCHG_SEL_SHIFT, nvkey.data[nvkey.sel - 1].sel);
    pmu_set_register_value_lp(TRICKLE_RCHG_SEL_UPDATE_ADDR, TRICKLE_RCHG_SEL_UPDATE_MASK, TRICKLE_RCHG_SEL_UPDATE_SHIFT, 1);
}

void pmu_set_chg_cc1_curr_cfg(void)
{
    chg_cc1_curr_cfg_t nvkey;
    if (pmu_get_nvkey(NVID_CAL_INT_CHG_CC1_CURR_CFG, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    uint16_t tmp_val = nvkey.data[nvkey.sel - 1].sel;
    chg_rsel.cc1[PMU_RSEL_NORM] = tmp_val;
    pmu_chg_rsel_ctl(PMU_RSEL_NORM);

    if (!bat_cfg.two_step_en) {
        pmu_set_register_value_lp(CV_RCHG_SEL_ADDR, CV_RCHG_SEL_MASK, CV_RCHG_SEL_SHIFT, tmp_val);
        pmu_set_register_value_lp(CV_RCHG_SEL_UPDATE_ADDR, CV_RCHG_SEL_UPDATE_MASK, CV_RCHG_SEL_UPDATE_SHIFT, 1);
    }
}

void pmu_set_chg_cc2_curr_cfg(void)
{
    chg_cc2_curr_cfg_t nvkey;
    if (pmu_get_nvkey(NVID_CAL_INT_CHG_CC2_CURR_CFG, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    uint16_t tmp_val = nvkey.data[nvkey.sel - 1].sel;
    chg_rsel.cc2[PMU_RSEL_NORM] = tmp_val;

    if (bat_cfg.two_step_en) {
        pmu_set_register_value_lp(CC2_EN_ADDR, CC2_EN_MASK, CC2_EN_SHIFT, 1);
        pmu_set_register_value_lp(CV_RCHG_SEL_ADDR, CV_RCHG_SEL_MASK, CV_RCHG_SEL_SHIFT, tmp_val);
        pmu_set_register_value_lp(CV_RCHG_SEL_UPDATE_ADDR, CV_RCHG_SEL_UPDATE_MASK, CV_RCHG_SEL_UPDATE_SHIFT, 1);
    }

    pmu_chg_rsel_ctl(PMU_RSEL_NORM);
}

void pmu_set_chg_cv_stop_curr_adc(void)
{
    cv_stop_curr_cfg_t nvkey;
    if (pmu_get_nvkey(NVID_CAL_CV_STOP_CURR_CFG, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(CV_STOP_CURRENT_ADDR, CV_STOP_CURRENT_MASK, CV_STOP_CURRENT_SHIFT, nvkey.cv_stop_curr[nvkey.sel - 1].adc);

    //pre_compl_cfg, todo, add new nvkey?
    pmu_set_register_value_lp(CHG_PRE_COMPLETE_INT_EN_ADDR, CHG_PRE_COMPLETE_INT_EN_MASK, CHG_PRE_COMPLETE_INT_EN_SHIFT, nvkey.chg_pre_compl_int_en);
    pmu_set_register_value_lp(PRE_COMPLETE_THRESHOLD_ADDR, PRE_COMPLETE_THRESHOLD_MASK, PRE_COMPLETE_THRESHOLD_SHIFT, nvkey.pre_compl_curr[nvkey.pre_compl_sel - 1].adc);
}
#endif  /* AIR_PMU_DISABLE_CHARGER */

void pmu_set_sys_ldo(void)
{
    sys_ldo_t nvkey;
    if (pmu_get_nvkey(NVID_CAL_SYS_LDO, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    uint16_t tmp_val = nvkey.chg_ldo_sel;
    if (bat_cfg.volt1 >= 4300) { //if CV >=4.3V, VSYSLDO output +1, todo, need remove?
        tmp_val++;
        tmp_val = pmu_range(tmp_val, 0, 7, 1);
    }

    pmu_set_register_value_lp(RG_CHG_LDO_SEL_ADDR, RG_CHG_LDO_SEL_MASK, RG_CHG_LDO_SEL_SHIFT, tmp_val);
}

void pmu_set_ocp(void)
{
    ocp_t nvkey;
    if (pmu_get_nvkey(NVID_CAL_OCP, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(RG_SW_OC_LMT_ADDR, RG_SW_OC_LMT_MASK, RG_SW_OC_LMT_SHIFT, nvkey.sw_oc_lmt);
    pmu_set_register_value_lp(RG_I_LIM_TRIM_ADDR, RG_I_LIM_TRIM_MASK, RG_I_LIM_TRIM_SHIFT, nvkey.i_lim_trim);
}

void pmu_set_lpo32(void)
{
    lpo32_cfg_t nvkey;
    uint16_t id = NVID_CAL_LPO32_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(RG_LPO32_FTUNE_ADDR, RG_LPO32_FTUNE_MASK, RG_LPO32_FTUNE_SHIFT, nvkey.ftune);
    pmu_set_register_value_lp(RG_LPO32_CTUNE_ADDR, RG_LPO32_CTUNE_MASK, RG_LPO32_CTUNE_SHIFT, nvkey.ctune);

    //set trim done rg
    pmu_set_register_value_lp(CLK32K_TRIM_DONE_ADDR, CLK32K_TRIM_DONE_MASK, CLK32K_TRIM_DONE_SHIFT, 1);
}

/*******************BUCK********************/
void pmu_set_buck_ripple(void)
{
    buck_ripple_ctrl_t nvkey;
    pmu_status_t status;

    status = pmu_get_nvkey(NVID_CAL_BUCK_RIPPLE_CTL, (uint8_t *)&nvkey, sizeof(nvkey));
    if (status != PMU_STATUS_SUCCESS) {
        return;
    }

    // TODO: Align RG map excel and macro
    pmu_set_register_value_lp(RG_VCORE_DYNAMIC_IPEAK_ADDR, 0x01, 0, nvkey.ripple_vcore_en);
    pmu_set_register_value_lp(RG_VIO18_DYNAMIC_IPEAK_ADDR, 0x01, 0, nvkey.ripple_vio18_en);
    /**
     * NOTE: vaud18 would changed by audio module
     * Keep the nvkey setting in sram variable.
     * When audio module call pmu api to set vaud18,
     * use this variable to set RG.
     */
    ripple_vaud18_en = nvkey.ripple_vaud18_en;
}

void pmu_set_buck_volt(void)
{
    buck_volt_trim_t nvkey;
    pmu_status_t status;

    status = pmu_get_nvkey(NVID_CAL_BUCK_VOLT_TRIM, (uint8_t *)&nvkey, sizeof(nvkey));
    if (status != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(RG_VCORE_NM_TRIM_ADDR, RG_VCORE_NM_TRIM_MASK, RG_VCORE_NM_TRIM_SHIFT, nvkey.vcore_nm_trim);
    pmu_set_register_value_lp(RG_VCORE_LPM_TRIM_ADDR, RG_VCORE_LPM_TRIM_MASK, RG_VCORE_LPM_TRIM_SHIFT, nvkey.vcore_lpm_trim);
    pmu_set_register_value_lp(RG_VIO18_NM_TRIM_ADDR, RG_VIO18_NM_TRIM_MASK, RG_VIO18_NM_TRIM_SHIFT, nvkey.vio18_nm_trim);
    pmu_set_register_value_lp(RG_VIO18_LPM_TRIM_ADDR, RG_VIO18_LPM_TRIM_MASK, RG_VIO18_LPM_TRIM_SHIFT, nvkey.vio18_lpm_trim);

    pmu_set_register_value_lp(VCORE_IS_TRIMMED_ADDR, VCORE_IS_TRIMMED_MASK, VCORE_IS_TRIMMED_SHIFT, 1);
    pmu_set_register_value_lp(VIO18_IS_TRIMMED_ADDR, VIO18_IS_TRIMMED_MASK, VIO18_IS_TRIMMED_SHIFT, 1);

}

void pmu_set_buck_ipeak(void)
{
    buck_ipeak_t nvkey;
    pmu_status_t status;

    status = pmu_get_nvkey(NVID_CAL_BUCK_IPEAK, (uint8_t *)&nvkey, sizeof(nvkey));
    if (status != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(RG_VCORE_IPEAK_TRIM_ADDR, RG_VCORE_IPEAK_TRIM_MASK, RG_VCORE_IPEAK_TRIM_SHIFT, nvkey.vcore_ipeak_trim);
    pmu_set_register_value_lp(RG_VIO18_IPEAK_TRIM_ADDR, RG_VIO18_IPEAK_TRIM_MASK, RG_VIO18_IPEAK_TRIM_SHIFT, nvkey.vio18_ipeak_trim);
}

void pmu_set_buck_freq(void)
{
    buck_freq_t nvkey;
    pmu_status_t status;

    status = pmu_get_nvkey(NVID_CAL_BUCK_FREQ, (uint8_t *)&nvkey, sizeof(nvkey));
    if (status != PMU_STATUS_SUCCESS) {
        return;
    }

    // TODO: Align RG map excel and macro
    pmu_set_register_value_lp(RG_VCORE_DYNAMIC_IPEAK_ADDR, 0x03, 1, nvkey.vcore_freq);
    pmu_set_register_value_lp(RG_VIO18_DYNAMIC_IPEAK_ADDR, 0x03, 1, nvkey.vio18_freq);
}

void pmu_set_vcore_dl(void)
{
    vcore_dl_t nvkey;
    uint16_t id = NVID_CAL_VCORE_DL;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    if (nvkey.vcore_dummy_en) {
        pmu_set_register_value_lp(RG_VCORE_RVD_ADDR, 0x1, 10, nvkey.vcore_dummy_en);
        pmu_set_register_value_lp(RG_VCORE_RVD_ADDR, 0x1F, 11, nvkey.data[nvkey.sel - 1].val);
    } else {
        log_hal_msgid_warning("[PMU_CAL]set_vcore_dl disable, id[0x%X], en[%d]", 2, id, nvkey.vcore_dummy_en);
    }
}

void pmu_set_vio18_dl(void)
{
    vio18_dl_t nvkey;
    uint16_t id = NVID_CAL_VIO18_DL;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    if (nvkey.en) {
        pmu_set_register_value_lp(RG_VIO18_RVD_ADDR, 0x1, 10, nvkey.en);
        pmu_set_register_value_lp(RG_VIO18_RVD_ADDR, 0x1F, 11, nvkey.data[nvkey.sel - 1].val);
    } else {
        log_hal_msgid_warning("[PMU_CAL]set_vio18_dl disable, id[0x%X], en[%d]", 2, id, nvkey.en);
    }
}

/*******************SIDO********************/
void pmu_set_sido_volt(void)
{
    sido_volt_cfg_t nvkey;
    pmu_status_t status;

    status = pmu_get_nvkey(NVID_CAL_SIDO_VOLT, (uint8_t *)&nvkey, sizeof(nvkey));
    if (status != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(RG_TRIM_VAUD18_NM_ADDR, RG_TRIM_VAUD18_NM_MASK, RG_TRIM_VAUD18_NM_SHIFT, nvkey.vaud18_nm_trim);
    /* Vaud18 LPM trim is reversed */
    pmu_set_register_value_lp(RG_TRIM_VRF_NM_ADDR, RG_TRIM_VRF_NM_MASK, RG_TRIM_VRF_NM_SHIFT, nvkey.vrf_nm_trim);
    pmu_set_register_value_lp(RG_TRIM_VRF_LPM_ADDR, RG_TRIM_VRF_LPM_MASK, RG_TRIM_VRF_LPM_SHIFT, nvkey.vrf_lpm_trim);

    pmu_set_register_value_lp(VAUD18_IS_TRIMMED_ADDR, VAUD18_IS_TRIMMED_MASK, VAUD18_IS_TRIMMED_SHIFT, 1);
    pmu_set_register_value_lp(VRF_IS_TRIMMED_ADDR, VRF_IS_TRIMMED_MASK, VRF_IS_TRIMMED_SHIFT, 1);
}

void pmu_set_sido_ipeak(void)
{
    sido_ipeak_t nvkey;
    pmu_status_t status;

    status = pmu_get_nvkey(NVID_CAL_SIDO_IPEAK, (uint8_t *)&nvkey, sizeof(nvkey));
    if (status != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(RG_VAUD18_IPEAK_TRIM_ADDR, RG_VAUD18_IPEAK_TRIM_MASK, RG_VAUD18_IPEAK_TRIM_SHIFT, nvkey.vaud18_ipeak_trim);
    pmu_set_register_value_lp(RG_VRF_IPEAK_TRIM_ADDR, RG_VRF_IPEAK_TRIM_MASK, RG_VRF_IPEAK_TRIM_SHIFT, nvkey.vrf_ipeak_trim);
}

void pmu_set_sido_freq(void)
{
    sido_freq_t nvkey;
    pmu_status_t status;

    status = pmu_get_nvkey(NVID_CAL_SIDO_FREQ, (uint8_t *)&nvkey, sizeof(nvkey));
    if (status != PMU_STATUS_SUCCESS) {
        return;
    }

    pmu_set_register_value_lp(RG_VAUD18_DYNAMIC_IPEAK_ADDR, 0x0F, 0, nvkey.vaud18_freq);
    /* vrf freq setting is reserved  */
}

void pmu_set_vaud18_dl(void)
{
    vaud18_dl_t nvkey;
    if (pmu_get_nvkey(NVID_CAL_VAUD18_DL, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }
    pmu_set_register_value_lp(RG_VRF_RVD_ADDR, 0x1, 5, nvkey.en);
    pmu_set_register_value_lp(RG_VRF_RVD2_ADDR, 0xF, 8, nvkey.data[nvkey.sel - 1].val);
}

void pmu_set_vrf_dl(void)
{
    vrf_dl_t nvkey;
    if (pmu_get_nvkey(NVID_CAL_VRF_DL, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }
    pmu_set_register_value_lp(RG_VRF_RVD_ADDR, 0x1, 4, nvkey.en);
    pmu_set_register_value_lp(RG_VRF_RVD2_ADDR, 0xF, 12, nvkey.data[nvkey.sel - 1].val);
}

/*******************LDO********************/
void pmu_set_ldo_sram(void)
{
    ldo_vsram_cfg_t nvkey;
    uint16_t id = NVID_CAL_LDO_VSRAM_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }
    pmu_set_register_value_lp(RG_VSRAM_VOTRIM_ADDR, RG_VSRAM_VOTRIM_MASK, RG_VSRAM_VOTRIM_SHIFT, nvkey.vcore_nm_trim);
    pmu_set_register_value_lp(VSRAM_IS_TRIMMED_ADDR, VSRAM_IS_TRIMMED_MASK, VSRAM_IS_TRIMMED_SHIFT, 1);
}

void pmu_set_ldo_vdd33_reg(void)
{
    buck_ldo_cfg_t nvkey;
    uint16_t id = NVID_CAL_LDO_VDD33_REG_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    if (nvkey.kflag == CAL_DONE) {
        pmu_set_register_value_lp(REGHV_SEL_NM_ADDR, REGHV_SEL_NM_MASK, REGHV_SEL_NM_SHIFT, nvkey.data[nvkey.sel - 1].sel);
    } else {
        log_hal_msgid_error("[PMU_CAL]set_vdd33_reg fail, id[0x%X], kflag[%d]", 2, id, nvkey.kflag);
    }
}

void pmu_set_ldo_vdd33_reg_ret(void)
{
    buck_ldo_cfg_t nvkey;
    uint16_t id = NVID_CAL_LDO_VDD33_REG_RET_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    if (nvkey.kflag == CAL_DONE) {
        pmu_set_register_value_lp(REGHV_SEL_RET_ADDR, REGHV_SEL_RET_MASK, REGHV_SEL_RET_SHIFT, nvkey.data[nvkey.sel - 1].sel);
    } else {
        log_hal_msgid_error("[PMU_CAL]set_vdd33_reg_ret fail, id[0x%X], kflag[%d]", 2, id, nvkey.kflag);
    }
}

void pmu_set_ldo_vdd33_ret(void)
{
    buck_ldo_cfg_t nvkey;
    uint16_t id = NVID_CAL_LDO_VDD33_RET_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    if (nvkey.kflag == CAL_DONE) {
        pmu_set_register_value_lp(RG_HVSTBSEL_ADDR, RG_HVSTBSEL_MASK, RG_HVSTBSEL_SHIFT, nvkey.data[nvkey.sel - 1].sel);
    } else {
        log_hal_msgid_error("[PMU_CAL]set_vdd33_ret fail, id[0x%X], kflag[%d]", 2, id, nvkey.kflag);
    }
}

void pmu_set_ldo_vdig18(void)
{
    buck_ldo_cfg_t nvkey;
    uint16_t id = NVID_CAL_VDIG18_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    if (nvkey.kflag == CAL_DONE) {
        pmu_set_register_value_lp(RG_VDIG18_SEL_ADDR, RG_VDIG18_SEL_MASK, RG_VDIG18_SEL_SHIFT, nvkey.data[nvkey.sel - 1].sel);
    } else {
        log_hal_msgid_error("[PMU_CAL]set_vdig18 fail, id[0x%X], kflag[%d]", 2, id, nvkey.kflag);
    }
}

/*******************VRF********************/
void pmu_set_hpbg(void)
{
    bg_cfg_t nvkey;
    uint16_t id = NVID_CAL_HPBG_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }
    pmu_set_register_value_lp(0x00C, RG_BGHP_SEL_MASK, 1, 9); //moved from init setting
    pmu_set_register_value_lp(RG_FAST_BUFFER_ENB_ADDR, RG_FAST_BUFFER_ENB_MASK, RG_FAST_BUFFER_ENB_SHIFT, 0);
    if (nvkey.kflag == CAL_DONE) {
        pmu_set_register_value_lp(RG_BGR_TRIM_ADDR, RG_BGR_TRIM_MASK, RG_BGR_TRIM_SHIFT, nvkey.data[nvkey.sel - 1].sel);
        hpbg_trim_sel = nvkey.data[nvkey.sel - 1].sel;
    } else {
        log_hal_msgid_error("[PMU_CAL]set_hpbg fail, id[0x%X], kflag[%d]", 2, id, nvkey.kflag);
    }
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &pmu_fast_buffer_tick);
}

void pmu_set_lpbg(void)
{
    bg_cfg_t nvkey;
    uint16_t id = NVID_CAL_LPBG_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    if (nvkey.kflag == CAL_DONE) {
        pmu_set_register_value_lp(RG_BGR_TRIM_ADDR, RG_BGR_TRIM_MASK, RG_BGR_TRIM_SHIFT, nvkey.data[nvkey.sel - 1].sel);
    } else {
        log_hal_msgid_error("[PMU_CAL]set_lpbg fail, id[0x%X], kflag[%d]", 2, id, nvkey.kflag);
    }
}

/*******************pmu_cal_init********************/
void pmu_cal_chg_cfg(void)
{
    chg_cfg_t nvkey;
    uint16_t id = NVID_CAL_CHG_CFG;

    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        assert(0);
    }

    if(nvkey.bat_volt_sel == 0xFF){
       bat_cfg.volt1 = nvkey.bat_volt_sel2;
    }else{
       bat_cfg.volt1 = bat_table[nvkey.bat_volt_sel];
    }
    bat_cfg.two_step_en = nvkey.two_step_en;
    pmu_kflag = nvkey.kflag;
    pmu_otp_dump = nvkey.otp_dump;
}

#ifndef AIR_PMU_DISABLE_CHARGER
void pmu_cal_vbat_adc(void)
{
    chg_cfg_t nvkey_temp;

    uint16_t id_temp = NVID_CAL_CHG_CFG;
    if (pmu_get_nvkey(id_temp, (uint8_t *)&nvkey_temp, sizeof(nvkey_temp)) != PMU_STATUS_SUCCESS) {
        return;
    }
    vbat_adc_cal_t nvkey;

    uint16_t id = NVID_CAL_VBAT_ADC_CAL_TABLE;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_vbat_t otp;
    if (pmu_get_otp(OTP_VBAT_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    uint16_t volt = 0, adc = 0, volt1 = 0, adc1 = 0, volt2 = 0, adc2 = 0, volt3 = 0, adc3 = 0;

    //todo (3 choose 2)
    volt1 = otp.bat_4v35.volt;
    adc1  = otp.bat_4v35.adc;
    volt2 = otp.bat_4v2.volt;
    adc2  = otp.bat_4v2.adc;
    volt3 = otp.bat_3v.volt;
    adc3  = otp.bat_3v.adc;


    if(nvkey_temp.bat_volt_sel == 0xFF){
       volt = nvkey_temp.bat_volt_sel2;
    }else{
       volt = bat_table[nvkey_temp.bat_volt_sel];
    }
    adc  = pmu_lerp(volt1, adc1, volt2, adc2, volt);

    //todo (4 choose 2)
    bat_cfg.volt1 = volt;
    bat_cfg.adc1  = adc;
    bat_cfg.volt2 = volt3; //3.0v
    bat_cfg.adc2  = adc3;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        nvkey.data[i].adc = pmu_lerp(bat_cfg.volt1, bat_cfg.adc1, bat_cfg.volt2, bat_cfg.adc2, nvkey.data[i].volt);
        nvkey.data[i].adc = pmu_range(nvkey.data[i].adc, 0, 1023, 2);
    }
    nvkey.kflag = CAL_DONE;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_vbat_volt(void)
{
    vbat_volt_cfg_t bat_volt;
    uint16_t id = NVID_CAL_VBAT_VOLT_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&bat_volt, sizeof(bat_volt)) != PMU_STATUS_SUCCESS) {
        return;
    }

    vbat_adc_cal_t bat_adc;
    if (pmu_get_nvkey(NVID_CAL_VBAT_ADC_CAL_TABLE, (uint8_t *)&bat_adc, sizeof(bat_adc)) != PMU_STATUS_SUCCESS) {
        return;
    }

    //todo (4 choose 2)
    uint16_t volt1 = bat_adc.data[0].volt;
    uint16_t adc1 = bat_adc.data[0].adc;
    uint16_t volt2 = bat_adc.data[1].volt;
    uint16_t adc2 = bat_adc.data[1].adc;

    bat_volt.init_bat.adc = pmu_lerp(volt1, adc1, volt2, adc2, bat_volt.init_bat.volt);
    bat_volt.sd_bat.adc   = pmu_lerp(volt1, adc1, volt2, adc2, bat_volt.sd_bat.volt);

    //todo (3 choose 2)
    for (uint8_t i = 0; i < 18; i++) {
        if (bat_volt.data[i].volt == 0) {
            if (i < 9) {
                log_hal_msgid_error("[PMU_CAL]cal_vbat_volt, vbat[%d]=0", 1, i);
                assert(0);
            }
            continue;
        }
        bat_volt.data[i].adc = pmu_lerp(volt1, adc1, volt2, adc2, bat_volt.data[i].volt);
        bat_volt.data[i].adc = pmu_range(bat_volt.data[i].adc, 0, 1023, 2);
    }
    bat_volt.kflag = CAL_DONE;

    pmu_set_nvkey(id, (uint8_t *)&bat_volt, sizeof(bat_volt));
}

void pmu_cal_chg_adc(void)
{
    chg_adc_cfg_t nvkey;
    uint16_t id = NVID_CAL_CHG_ADC_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }
    uint16_t volt1 = bat_cfg.volt1;
    uint16_t adc1 = bat_cfg.adc1;
    uint16_t volt2 = bat_cfg.volt2;//form otp?
    uint16_t adc2 = bat_cfg.adc2;

    nvkey.cc1_thrd_adc = pmu_lerp(volt1, adc1, volt2, adc2, nvkey.cc1_thrd_volt);
    nvkey.cc2_thrd_adc = pmu_lerp(volt1, adc1, volt2, adc2, nvkey.cc2_thrd_volt);
    nvkey.cv_thrd_adc  = pmu_lerp(volt1, adc1, volt2, adc2, nvkey.cv_thrd_volt);
    nvkey.rechg_adc    = pmu_lerp(volt1, adc1, volt2, adc2, nvkey.rechg_volt);

    nvkey.cc1_thrd_adc = pmu_range(nvkey.cc1_thrd_adc, 0, 1023, 2);
    nvkey.cc2_thrd_adc = pmu_range(nvkey.cc2_thrd_adc, 0, 1023, 2);
    nvkey.cv_thrd_adc  = pmu_range(nvkey.cv_thrd_adc,  0, 1023, 2);
    nvkey.rechg_adc    = pmu_range(nvkey.rechg_adc,    0, 1023, 2);

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));

    if((nvkey.cc1_thrd_volt) > (nvkey.cc2_thrd_volt)){
        log_hal_msgid_error("[PMU_CAL] cc1_thd_volt > cc2_thd_volt, cc1_thd_volt[%d], cc2_thd_volt[%d]", 2, nvkey.cc1_thrd_volt, nvkey.cc2_thrd_volt);
    }
}
#endif  /* AIR_PMU_DISABLE_CHARGER */

void pmu_cal_buck_volt(void)
{
    buck_volt_trim_t nvkey;
    pmu_status_t nv_status;
    uint16_t id = NVID_CAL_BUCK_VOLT_TRIM;

    otp_buck_volt_t otp;
    pmu_status_t otp_status;

    nv_status = pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
    otp_status = pmu_get_otp(OTP_BUCK_VOLT_ADDR, (uint8_t *)&otp, sizeof(otp));

    if (nv_status != PMU_STATUS_SUCCESS) {
        return;
    }
    if (otp_status != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.vcore_nm_trim = otp.vcore_nm.sel;
    nvkey.vcore_lpm_trim = otp.vcore_lpm.sel;
    nvkey.vio18_nm_trim = otp.vio_nm.sel;
    nvkey.vio18_lpm_trim = otp.vio_lpm.sel;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_buck_ipeak(void)
{
    buck_ipeak_t nvkey;
    pmu_status_t nv_status;
    uint16_t id = NVID_CAL_BUCK_IPEAK;

    otp_buck_ipeak_t otp;
    pmu_status_t otp_status;

    nv_status = pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
    otp_status = pmu_get_otp(OTP_BUCK_IPEAK_ADDR, (uint8_t *)&otp, sizeof(otp));

    if (nv_status != PMU_STATUS_SUCCESS) {
        return;
    }
    if (otp_status != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.vcore_ipeak_trim = otp.vcore_sel;
    nvkey.vio18_ipeak_trim = otp.vio_sel;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_buck_freq(void)
{
    buck_freq_t nvkey;
    pmu_status_t nv_status;
    uint16_t id = NVID_CAL_BUCK_FREQ;

    otp_buck_freq_t otp;
    pmu_status_t otp_status;

    nv_status = pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
    otp_status = pmu_get_otp(OTP_BUCK_FREQ_ADDR, (uint8_t *)&otp, sizeof(otp));

    if (nv_status != PMU_STATUS_SUCCESS) {
        return;
    }
    if (otp_status != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.vcore_freq = otp.vcore.sel;
    nvkey.vio18_freq = otp.vio.sel;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_vcore_dl(void)
{
    vcore_dl_t nvkey;
    uint16_t id = NVID_CAL_VCORE_DL;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_buck_dl_t otp;
    if (pmu_get_otp(OTP_BUCK_DL_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.otp[0].curr = otp.vcore_dl_k[0].curr;
    nvkey.otp[0].val  = otp.vcore_dl_k[0].val;
    nvkey.otp[1].curr = otp.vcore_dl_k[1].curr;
    nvkey.otp[1].val  = otp.vcore_dl_k[1].val;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        nvkey.data[i].val = pmu_lerp(nvkey.otp[0].curr, nvkey.otp[0].val, nvkey.otp[1].curr, nvkey.otp[1].val, nvkey.data[i].curr);
        nvkey.data[i].val = pmu_range(nvkey.data[i].val, 0, 255, 1);
    }

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_vio18_dl(void)
{
    vio18_dl_t nvkey;
    uint16_t id = NVID_CAL_VIO18_DL;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_buck_dl_t otp;
    if (pmu_get_otp(OTP_BUCK_DL_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.otp[0].curr = otp.vio18_dl_k[0].curr;
    nvkey.otp[0].val  = otp.vio18_dl_k[0].val;
    nvkey.otp[1].curr = otp.vio18_dl_k[1].curr;
    nvkey.otp[1].val  = otp.vio18_dl_k[1].val;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        nvkey.data[i].val = pmu_lerp(nvkey.otp[0].curr, nvkey.otp[0].val, nvkey.otp[1].curr, nvkey.otp[1].val, nvkey.data[i].curr);
        nvkey.data[i].val = pmu_range(nvkey.data[i].val, 0, 31, 1);
    }

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_sido_volt(void)
{
    sido_volt_cfg_t nvkey;
    pmu_status_t nv_status;
    uint16_t id = NVID_CAL_SIDO_VOLT;

    otp_sido_1_t otp;
    pmu_status_t otp_status;

    nv_status = pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
    otp_status = pmu_get_otp(OTP_SIDO_1_ADDR, (uint8_t *)&otp, sizeof(otp));

    if (nv_status != PMU_STATUS_SUCCESS) {
        return;
    }
    if (otp_status != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.vaud18_nm_trim = otp.vaud18_nm_k;
    /* vaud18 lpm trim is reversed */
    nvkey.vrf_nm_trim = otp.vrf_nm_k;
    nvkey.vrf_lpm_trim = otp.vrf_lpm_k;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_sido_ipeak(void)
{
    sido_ipeak_t nvkey;
    pmu_status_t nv_status;
    uint16_t id = NVID_CAL_SIDO_IPEAK;

    otp_sido_2_t otp;
    pmu_status_t otp_status;

    nv_status = pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
    otp_status = pmu_get_otp(OTP_SIDO_2_ADDR, (uint8_t *)&otp, sizeof(otp));

    if (nv_status != PMU_STATUS_SUCCESS) {
        return;
    }
    if (otp_status != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.vaud18_ipeak_trim = otp.vaud18_ipeak_k;
    nvkey.vrf_ipeak_trim = otp.vrf_ipeak_k;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_sido_freq(void)
{
    sido_freq_t nvkey;
    pmu_status_t nv_status;
    uint16_t id = NVID_CAL_SIDO_FREQ;

    otp_sido_4_t otp;
    pmu_status_t otp_status;

    nv_status = pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
    otp_status = pmu_get_otp(OTP_SIDO_4_ADDR, (uint8_t *)&otp, sizeof(otp));

    if (nv_status != PMU_STATUS_SUCCESS) {
        return;
    }
    if (otp_status != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.vaud18_freq = otp.vaud18_freq;
    /* vrf freq is reversed */

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_vaud18_dl(void)
{
    vio18_dl_t nvkey;
    uint16_t id = NVID_CAL_VAUD18_DL;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_sido_3_t otp;
    if (pmu_get_otp(OTP_SIDO_3_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.otp[0].curr = otp.vaud18_dl_k[0].curr;
    nvkey.otp[0].val  = otp.vaud18_dl_k[0].val;
    nvkey.otp[1].curr = otp.vaud18_dl_k[1].curr;
    nvkey.otp[1].val  = otp.vaud18_dl_k[1].val;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        nvkey.data[i].val = pmu_lerp(nvkey.otp[0].curr, nvkey.otp[0].val, nvkey.otp[1].curr, nvkey.otp[1].val, nvkey.data[i].curr);
        nvkey.data[i].val = pmu_range(nvkey.data[i].val, 0, 15, 1);
    }

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_vrf_dl(void)
{
    vio18_dl_t nvkey;
    uint16_t id = NVID_CAL_VRF_DL;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_sido_3_t otp;
    if (pmu_get_otp(OTP_SIDO_3_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.otp[0].curr = otp.vrf_dl_k[0].curr;
    nvkey.otp[0].val  = otp.vrf_dl_k[0].val;
    nvkey.otp[1].curr = otp.vrf_dl_k[1].curr;
    nvkey.otp[1].val  = otp.vrf_dl_k[1].val;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        nvkey.data[i].val = pmu_lerp(nvkey.otp[0].curr, nvkey.otp[0].val, nvkey.otp[1].curr, nvkey.otp[1].val, nvkey.data[i].curr);
        nvkey.data[i].val = pmu_range(nvkey.data[i].val, 0, 15, 1);
    }

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_ldo_sram(void)
{
    ldo_vsram_cfg_t nvkey;
    uint16_t id = NVID_CAL_LDO_VSRAM_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_ldo_vsram_t otp;
    if (pmu_get_otp(OTP_LDO_VSRAM_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.vcore_nm_trim = otp.vsram_votrim;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_ldo_vdd33_reg(void)
{
    buck_ldo_cfg_t nvkey;
    uint16_t id = NVID_CAL_LDO_VDD33_REG_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_ldo_vdd33_t otp;
    if (pmu_get_otp(OTP_LDO_VDD33_REG_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.otp[0].volt = otp.data[0].volt;
    nvkey.otp[0].sel  = otp.data[0].sel;
    nvkey.otp[1].volt = otp.data[1].volt;
    nvkey.otp[1].sel  = otp.data[1].sel;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        nvkey.data[i].sel = pmu_lerp(nvkey.otp[0].volt, nvkey.otp[0].sel, nvkey.otp[1].volt, nvkey.otp[1].sel, nvkey.data[i].volt);
        nvkey.data[i].sel = pmu_range(nvkey.data[i].sel, 0, 127, 1);
    }
    nvkey.kflag = CAL_DONE;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_ldo_vdd33_reg_ret(void)
{
    buck_ldo_cfg_t nvkey;
    uint16_t id = NVID_CAL_LDO_VDD33_REG_RET_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_ldo_vdd33_t otp;
    if (pmu_get_otp(OTP_LDO_VDD33_REG_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.otp[0].volt = otp.data[0].volt;
    nvkey.otp[0].sel  = otp.data[0].sel;
    nvkey.otp[1].volt = otp.data[1].volt;
    nvkey.otp[1].sel  = otp.data[1].sel;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        nvkey.data[i].sel = pmu_lerp(nvkey.otp[0].volt, nvkey.otp[0].sel, nvkey.otp[1].volt, nvkey.otp[1].sel, nvkey.data[i].volt);
        nvkey.data[i].sel = pmu_range(nvkey.data[i].sel, 0, 127, 1);
    }
    nvkey.kflag = CAL_DONE;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_ldo_vdd33_ret(void)
{
    buck_ldo_cfg_t nvkey;
    uint16_t id = NVID_CAL_LDO_VDD33_RET_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_ldo_vdd33_t otp;
    if (pmu_get_otp(OTP_LDO_VDD33_RET_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.otp[0].volt = otp.data[0].volt;
    nvkey.otp[0].sel  = otp.data[0].sel;
    nvkey.otp[1].volt = otp.data[1].volt;
    nvkey.otp[1].sel  = otp.data[1].sel;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        nvkey.data[i].sel = pmu_lerp(nvkey.otp[0].volt, nvkey.otp[0].sel, nvkey.otp[1].volt, nvkey.otp[1].sel, nvkey.data[i].volt);
        nvkey.data[i].sel = pmu_range(nvkey.data[i].sel, 0, 127, 1);
    }
    nvkey.kflag = CAL_DONE;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_ldo_vdig18(void)
{
    buck_ldo_cfg_t nvkey;
    uint16_t id = NVID_CAL_VDIG18_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_ldo_vdig18_t otp;
    if (pmu_get_otp(OTP_LDO_VDIG18_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.otp[0].volt = otp.data[0].volt;
    nvkey.otp[0].sel  = otp.data[0].sel;
    nvkey.otp[1].volt = otp.data[1].volt;
    nvkey.otp[1].sel  = otp.data[1].sel;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        if (nvkey.data[i].volt == 0) {
            nvkey.data[i].sel = 0;
            continue;
        }
        if ((otp.data[0].sel >= 16 && otp.data[1].sel >= 16) || (otp.data[0].sel < 16 && otp.data[1].sel < 16)) {
            log_hal_msgid_error("[PMU_CAL]cal_vdig18 fail, id[0x%X], sel1[%d], sel2[%d]", 3, id, otp.data[0].sel, otp.data[1].sel);
            assert(0);
        } else {
            nvkey.data[i].sel = pmu_vdig_bg_lerp(otp.data[0].volt, otp.data[0].sel, otp.data[1].volt, otp.data[1].sel, nvkey.data[i].volt);
            nvkey.data[i].sel = (uint8_t)pmu_range(nvkey.data[i].sel, 0, 31, 1);
        }
    }
    nvkey.kflag = CAL_DONE;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_hpbg(void)
{
    bg_cfg_t nvkey;
    uint16_t id = NVID_CAL_HPBG_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_hpbg_t otp;
    if (pmu_get_otp(OTP_HPBG_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.otp[0].volt = otp.data[0].volt;
    nvkey.otp[0].sel = otp.data[0].sel;
    nvkey.otp[1].volt = otp.data[1].volt;
    nvkey.otp[1].sel = otp.data[1].sel;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        if ((otp.data[0].sel >= 16 && otp.data[1].sel >= 16) || (otp.data[0].sel < 16 && otp.data[1].sel < 16)) {
            log_hal_msgid_error("[PMU_CAL]cal_hpbg fail, id[0x%X], sel1[%d], sel2[%d]", 3, id, otp.data[0].sel, otp.data[1].sel);
            assert(0);
        } else {
            nvkey.data[i].sel = pmu_vdig_bg_lerp(otp.data[0].volt, otp.data[0].sel, otp.data[1].volt, otp.data[1].sel, nvkey.data[i].volt);
        }
        nvkey.data[i].sel = (uint8_t)pmu_range(nvkey.data[i].sel, 0, 31, 1);
    }
    nvkey.kflag = CAL_DONE;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_lpbg(void)
{
    bg_cfg_t nvkey;
    uint16_t id = NVID_CAL_LPBG_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_lpbg_t otp;
    if (pmu_get_otp(OTP_LPBG_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.otp[0].volt = otp.data[0].volt;
    nvkey.otp[0].sel = otp.data[0].sel;
    nvkey.otp[1].volt = otp.data[1].volt;
    nvkey.otp[1].sel = otp.data[1].sel;

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        if ((otp.data[0].sel >= 16 && otp.data[1].sel >= 16) || (otp.data[0].sel < 16 && otp.data[1].sel < 16)) {
            log_hal_msgid_error("[PMU_CAL]cal_lpbg fail, id[0x%X], sel1[%d], sel2[%d]", 3, id, otp.data[0].sel, otp.data[1].sel);
            assert(0);
        } else {
            nvkey.data[i].sel = pmu_vdig_bg_lerp(otp.data[0].volt, otp.data[0].sel, otp.data[1].volt, otp.data[1].sel, nvkey.data[i].volt);
        }
        nvkey.data[i].sel = (uint8_t)pmu_range(nvkey.data[i].sel, 0, 31, 1);
    }
    nvkey.kflag = CAL_DONE;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

#ifndef AIR_PMU_DISABLE_CHARGER
void pmu_cal_chg_dac(void)
{
    chg_dac_cfg_t nvkey;
    uint16_t id = NVID_CAL_INT_CHG_DAC_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }
    /*fail here*/

    otp_chg_dac_t otp;
    if (pmu_get_otp(OTP_CHG_DAC_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    //todo
    otp_dac.dac_4v35.volt = 4350;
    otp_dac.dac_4v35.dac = otp.dac_4v35;
    otp_dac.dac_4v2.volt = 4200;
    otp_dac.dac_4v2.dac = otp.dac_4v2;

    uint16_t chg_dac = pmu_lerp(otp_dac.dac_4v35.volt, otp_dac.dac_4v35.dac, otp_dac.dac_4v2.volt, otp_dac.dac_4v2.dac, bat_cfg.volt1);
    chg_dac = pmu_range(chg_dac, 0, 1023, 2);

    nvkey.tri_curr_dac = chg_dac;
    nvkey.cc1_curr_dac = chg_dac;
    nvkey.cc2_curr_dac = chg_dac;
    nvkey.cv_dac = chg_dac;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}
#endif  /* AIR_PMU_DISABLE_CHARGER */

void pmu_cal_sys_ldo(void)
{
    sys_ldo_t nvkey;
    uint16_t id = NVID_CAL_SYS_LDO;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_vsys_ldo_t otp;
    if (pmu_get_otp(OTP_VSYS_LDO_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    //todo
    uint16_t volt1 = otp.data[0].volt;
    uint16_t sel1  = otp.data[0].sel;
    uint16_t volt2 = otp.data[1].volt;
    uint16_t sel2  = otp.data[1].sel;

    nvkey.chg_ldo_sel = pmu_lerp(volt1, sel1, volt2, sel2, nvkey.sysldo_output_volt);
    nvkey.chg_ldo_sel = pmu_range(nvkey.chg_ldo_sel, 0, 7, 1);

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_ocp(void)
{
    ocp_t nvkey;
    uint16_t id = NVID_CAL_OCP;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_ocp_t otp;
    if (pmu_get_otp(OTP_OCP_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    //todo
    nvkey.sw_oc_lmt = pmu_range(otp.load_switch_ocp_trim, 0, 15, 1);
    nvkey.i_lim_trim = pmu_range(otp.vsys_ldo_ocp_trim, 0, 7, 1);

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_lpo32(void)
{
    lpo32_cfg_t nvkey;
    uint16_t id = NVID_CAL_LPO32_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_lpo32_t otp;
    if (pmu_get_otp(OTP_LPO32_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    nvkey.ftune = otp.ftune;
    nvkey.ctune = otp.ctune;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

#ifndef AIR_PMU_DISABLE_CHARGER
void pmu_cal_chg_tri_curr(void)
{
    uint16_t rsel_4v35 = 0, rsel_4v2 = 0;
    extern uint16_t precc_curr;
    chg_tri_curr_cfg_t nvkey;
    uint16_t id = NVID_CAL_INT_CHG_TRI_CURR_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        rsel_4v35 = pmu_lerp(otp_curr.bat_4v35[0].curr, otp_curr.bat_4v35[0].val, otp_curr.bat_4v35[1].curr, otp_curr.bat_4v35[1].val, nvkey.data[i].curr);
        rsel_4v2 = pmu_lerp(otp_curr.bat_4v2[0].curr, otp_curr.bat_4v2[0].val, otp_curr.bat_4v2[1].curr, otp_curr.bat_4v2[1].val, nvkey.data[i].curr);

        nvkey.data[i].sel = pmu_lerp(otp_dac.dac_4v35.volt, rsel_4v35, otp_dac.dac_4v2.volt, rsel_4v2, bat_cfg.volt1);
        nvkey.data[i].sel = pmu_range(nvkey.data[i].sel, 0, 1023, 2);
    }
    precc_curr = nvkey.data[nvkey.sel - 1].curr / 10;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_chg_cc1_curr(void)
{
    uint16_t rsel_4v35 = 0, rsel_4v2 = 0;

    chg_cc1_curr_cfg_t nvkey;
    extern uint16_t cc1_curr;
    uint16_t id = NVID_CAL_INT_CHG_CC1_CURR_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        rsel_4v35 = pmu_lerp(otp_curr.bat_4v35[0].curr, otp_curr.bat_4v35[0].val, otp_curr.bat_4v35[1].curr, otp_curr.bat_4v35[1].val, nvkey.data[i].curr);
        rsel_4v2 = pmu_lerp(otp_curr.bat_4v2[0].curr, otp_curr.bat_4v2[0].val, otp_curr.bat_4v2[1].curr, otp_curr.bat_4v2[1].val, nvkey.data[i].curr);

        nvkey.data[i].sel = pmu_lerp(otp_dac.dac_4v35.volt, rsel_4v35, otp_dac.dac_4v2.volt, rsel_4v2, bat_cfg.volt1);
        nvkey.data[i].sel = pmu_range(nvkey.data[i].sel, 0, 1023, 2);
    }
    cc_curr.cc1_curr = nvkey.data[nvkey.sel - 1].curr;
    cc1_curr = nvkey.data[nvkey.sel - 1].curr / 10;

    log_hal_msgid_info("[PMU_CHG] cc_curr.cc1_curr[%d], cc1_curr[%d]", 2, cc_curr.cc1_curr, cc1_curr);

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_chg_cc2_curr(void)
{
    uint16_t rsel_4v35 = 0, rsel_4v2 = 0;

    chg_cc2_curr_cfg_t nvkey;
    extern uint16_t cc2_curr;
    uint16_t id = NVID_CAL_INT_CHG_CC2_CURR_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        rsel_4v35 = pmu_lerp(otp_curr.bat_4v35[0].curr, otp_curr.bat_4v35[0].val, otp_curr.bat_4v35[1].curr, otp_curr.bat_4v35[1].val, nvkey.data[i].curr);
        rsel_4v2 = pmu_lerp(otp_curr.bat_4v2[0].curr, otp_curr.bat_4v2[0].val, otp_curr.bat_4v2[1].curr, otp_curr.bat_4v2[1].val, nvkey.data[i].curr);

        nvkey.data[i].sel = pmu_lerp(otp_dac.dac_4v35.volt, rsel_4v35, otp_dac.dac_4v2.volt, rsel_4v2, bat_cfg.volt1);
        nvkey.data[i].sel = pmu_range(nvkey.data[i].sel, 0, 1023, 2);
    }
    cc_curr.cc2_curr = nvkey.data[nvkey.sel - 1].curr;
    cc2_curr = nvkey.data[nvkey.sel - 1].curr / 10;

    log_hal_msgid_info("[PMU_CHG] cc_curr.cc2_curr[%d], cc2_curr[%d]", 2, cc_curr.cc2_curr, cc2_curr);

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

uint16_t g_iterm_current_default = 0;
uint16_t g_iterm_ratio_default = 0;
void pmu_cal_cv_stop_curr(void)
{
    cv_stop_curr_cfg_t nvkey;
    extern uint16_t iterm_setting;
    uint16_t id = NVID_CAL_CV_STOP_CURR_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_vichg_adc_val_t otp;
    if (pmu_get_otp(OTP_VICHG_ADC_VAL_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
        return;
    }

    vichg_adc.adc_4v35 = otp.adc_4v35;
    vichg_adc.adc_4v2 = otp.adc_4v2;
    vichg_adc.adc_4v05 = otp.adc_4v05;
    vichg_adc.cv_stop_curr_perc = nvkey.cv_stop_curr[nvkey.sel-1].perc;

    uint16_t adc = pmu_lerp(4350, vichg_adc.adc_4v35, 4200, vichg_adc.adc_4v2, bat_cfg.volt1);
    for (uint8_t i = 0; i < nvkey.cal_cnt; i++) {
        //todo
        nvkey.cv_stop_curr[i].adc = pmu_round((nvkey.cv_stop_curr[i].perc * adc), 100);
        nvkey.cv_stop_curr[i].adc = pmu_range(nvkey.cv_stop_curr[i].adc, 0, 1023, 2);
    }
    iterm_setting = vichg_adc.cv_stop_curr_perc;
    g_iterm_ratio_default = vichg_adc.cv_stop_curr_perc;
    g_iterm_current_default = pmu_round((cc_curr.cc1_curr * iterm_setting), 100);

    log_hal_msgid_info("[PMU_CHG] g_iterm_ratio_default[%d], g_iterm_current_default[%d]", 2, g_iterm_ratio_default, g_iterm_current_default);

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_cal_chg_curr_otp(void)
{
    otp_chg_4v35_curr_t otp_4v35;
    if (pmu_get_otp(OTP_CHG_4V35_CURR_ADDR, (uint8_t *)&otp_4v35, sizeof(otp_4v35)) != PMU_STATUS_SUCCESS) {
        return;
    }

    otp_chg_4v2_curr_t otp_4v2;
    if (pmu_get_otp(OTP_CHG_4V2_CURR_ADDR, (uint8_t *)&otp_4v2, sizeof(otp_4v2)) != PMU_STATUS_SUCCESS) {
        return;
    }

    //todo
    otp_curr.bat_4v35[0].curr = otp_4v35.data[0].curr;
    otp_curr.bat_4v35[0].val  = otp_4v35.data[0].val;
    otp_curr.bat_4v35[1].curr = otp_4v35.data[1].curr;
    otp_curr.bat_4v35[1].val  = otp_4v35.data[1].val;
    otp_curr.bat_4v2[0].curr  = otp_4v2.data[0].curr;
    otp_curr.bat_4v2[0].val   = otp_4v2.data[0].val;
    otp_curr.bat_4v2[1].curr  = otp_4v2.data[1].curr;
    otp_curr.bat_4v2[1].val   = otp_4v2.data[1].val;
}

void pmu_cal_chg_jeita_warm(void)
{
    uint16_t sel_volt, curr_dac, cc1_rsel_4v35, cc1_rsel_4v2, cc2_rsel_4v35, cc2_rsel_4v2;
    uint16_t jeita_cv, adc, prec;

    chg_adc_cfg_t chg_adc;
    if (pmu_get_nvkey(NVID_CAL_CHG_ADC_CFG, (uint8_t *)&chg_adc, sizeof(chg_adc)) != PMU_STATUS_SUCCESS) {
        return;
    }

    jeita_t jeita;
    if (pmu_get_nvkey(NVID_CAL_JEITA, (uint8_t *)&jeita, sizeof(jeita)) != PMU_STATUS_SUCCESS) {
        return;
    }

    jeita_warm_t jeita_warm;
    if (pmu_get_nvkey(NVID_CAL_JEITA_WARM, (uint8_t *)&jeita_warm, sizeof(jeita_warm)) != PMU_STATUS_SUCCESS) {
        return;
    }

    jeita_warm.cc2_thrd_volt = chg_adc.cc2_thrd_volt;
    jeita_warm.cv_thrd_volt  = chg_adc.cv_thrd_volt - jeita.warm_state_dac_dec;
    jeita_warm.rechg_volt    = chg_adc.rechg_volt - jeita.warm_state_dac_dec;

    jeita_warm.cc2_thrd_adc  = pmu_lerp(bat_cfg.volt1, bat_cfg.adc1, bat_cfg.volt2, bat_cfg.adc2, jeita_warm.cc2_thrd_volt);
    jeita_warm.cv_thrd_adc   = pmu_lerp(bat_cfg.volt1, bat_cfg.adc1, bat_cfg.volt2, bat_cfg.adc2, jeita_warm.cv_thrd_volt);
    jeita_warm.rechg_adc     = pmu_lerp(bat_cfg.volt1, bat_cfg.adc1, bat_cfg.volt2, bat_cfg.adc2, jeita_warm.rechg_volt);

    jeita_warm.cc2_thrd_adc  = pmu_range(jeita_warm.cc2_thrd_adc, 0, 1023, 2);
    jeita_warm.cv_thrd_adc   = pmu_range(jeita_warm.cv_thrd_adc,  0, 1023, 2);
    jeita_warm.rechg_adc     = pmu_range(jeita_warm.rechg_adc,    0, 1023, 2);

    sel_volt = bat_cfg.volt1 - jeita.warm_state_dac_dec;
    curr_dac = pmu_lerp(otp_dac.dac_4v35.volt, otp_dac.dac_4v35.dac, otp_dac.dac_4v2.volt, otp_dac.dac_4v2.dac, sel_volt);
    curr_dac = pmu_range(curr_dac, 0, 1023, 2);

    jeita_warm.cc1_curr_dac  = curr_dac;
    jeita_warm.cc2_curr_dac  = curr_dac;
    jeita_warm.cv_dac        = curr_dac;

    jeita_warm.cc1_curr = pmu_round((cc_curr.cc1_curr * jeita.warm_state_curr_perc), 100);
    jeita_warm.cc2_curr = pmu_round((cc_curr.cc2_curr * jeita.warm_state_curr_perc), 100);

    cc1_rsel_4v2  = pmu_lerp(otp_curr.bat_4v2[0].curr, otp_curr.bat_4v2[0].val, otp_curr.bat_4v2[1].curr, otp_curr.bat_4v2[1].val, jeita_warm.cc1_curr);
    cc1_rsel_4v2  = pmu_range(cc1_rsel_4v2, 0, 1023, 2);
    cc1_rsel_4v35 = pmu_lerp(otp_curr.bat_4v35[0].curr, otp_curr.bat_4v35[0].val, otp_curr.bat_4v35[1].curr, otp_curr.bat_4v35[1].val, jeita_warm.cc1_curr);
    cc1_rsel_4v35 = pmu_range(cc1_rsel_4v35, 0, 1023, 2);
    jeita_warm.cc1_rsel = pmu_lerp(otp_dac.dac_4v35.volt, cc1_rsel_4v35, otp_dac.dac_4v2.volt, cc1_rsel_4v2, sel_volt);
    jeita_warm.cc1_rsel = pmu_range(jeita_warm.cc1_rsel, 0, 1023, 2);

    cc2_rsel_4v2  = pmu_lerp(otp_curr.bat_4v2[0].curr, otp_curr.bat_4v2[0].val, otp_curr.bat_4v2[1].curr, otp_curr.bat_4v2[1].val, jeita_warm.cc2_curr);
    cc2_rsel_4v2  = pmu_range(cc2_rsel_4v2, 0, 1023, 2);
    cc2_rsel_4v35 = pmu_lerp(otp_curr.bat_4v35[0].curr, otp_curr.bat_4v35[0].val, otp_curr.bat_4v35[1].curr, otp_curr.bat_4v35[1].val, jeita_warm.cc2_curr);
    cc2_rsel_4v35 = pmu_range(cc2_rsel_4v35, 0, 1023, 2);
    jeita_warm.cc2_rsel = pmu_lerp(otp_dac.dac_4v35.volt, cc2_rsel_4v35, otp_dac.dac_4v2.volt, cc2_rsel_4v2, sel_volt);
    jeita_warm.cc2_rsel = pmu_range(jeita_warm.cc2_rsel, 0, 1023, 2);

    cv_stop_curr_cfg_t cv_stop_curr_adc;
    if (pmu_get_nvkey(NVID_CAL_CV_STOP_CURR_CFG, (uint8_t *)&cv_stop_curr_adc, sizeof(cv_stop_curr_adc)) != PMU_STATUS_SUCCESS) {
        return;
    }
    jeita_cv = bat_cfg.volt1 - jeita.warm_state_dac_dec;
    adc = pmu_lerp(otp_dac.dac_4v35.volt, vichg_adc.adc_4v35, otp_dac.dac_4v2.volt, vichg_adc.adc_4v2, jeita_cv);
    prec = cv_stop_curr_adc.cv_stop_curr[cv_stop_curr_adc.sel - 1].perc;
    jeita_warm.cv_stop_curr_adc = pmu_round(adc * prec, 100);

    pmu_set_nvkey(NVID_CAL_JEITA_WARM, (uint8_t *)&jeita_warm, sizeof(jeita_warm));
}

void pmu_cal_chg_jeita_cool(void)
{
    uint16_t sel_volt, curr_dac, cc1_rsel_4v35, cc1_rsel_4v2, cc2_rsel_4v35, cc2_rsel_4v2;
    uint16_t jeita_cv, adc, prec;

    chg_adc_cfg_t chg_adc;
    if (pmu_get_nvkey(NVID_CAL_CHG_ADC_CFG, (uint8_t *)&chg_adc, sizeof(chg_adc)) != PMU_STATUS_SUCCESS) {
        return;
    }

    jeita_t jeita;
    if (pmu_get_nvkey(NVID_CAL_JEITA, (uint8_t *)&jeita, sizeof(jeita)) != PMU_STATUS_SUCCESS) {
        return;
    }

    jeita_cool_t jeita_cool;
    if (pmu_get_nvkey(NVID_CAL_JEITA_COOL, (uint8_t *)&jeita_cool, sizeof(jeita_cool)) != PMU_STATUS_SUCCESS) {
        return;
    }

    jeita_cool.cc2_thrd_volt = chg_adc.cc2_thrd_volt;
    jeita_cool.cv_thrd_volt  = chg_adc.cv_thrd_volt - jeita.cool_state_dac_dec;
    jeita_cool.rechg_volt    = chg_adc.rechg_volt - jeita.cool_state_dac_dec;

    jeita_cool.cc2_thrd_adc  = pmu_lerp(bat_cfg.volt1, bat_cfg.adc1, bat_cfg.volt2, bat_cfg.adc2, jeita_cool.cc2_thrd_volt);
    jeita_cool.cv_thrd_adc   = pmu_lerp(bat_cfg.volt1, bat_cfg.adc1, bat_cfg.volt2, bat_cfg.adc2, jeita_cool.cv_thrd_volt);
    jeita_cool.rechg_adc     = pmu_lerp(bat_cfg.volt1, bat_cfg.adc1, bat_cfg.volt2, bat_cfg.adc2, jeita_cool.rechg_volt);

    jeita_cool.cc2_thrd_adc  = pmu_range(jeita_cool.cc2_thrd_adc, 0, 1023, 2);
    jeita_cool.cv_thrd_adc   = pmu_range(jeita_cool.cv_thrd_adc,  0, 1023, 2);
    jeita_cool.rechg_adc     = pmu_range(jeita_cool.rechg_adc,    0, 1023, 2);

    sel_volt = bat_cfg.volt1 - jeita.cool_state_dac_dec;
    curr_dac = pmu_lerp(otp_dac.dac_4v35.volt, otp_dac.dac_4v35.dac, otp_dac.dac_4v2.volt, otp_dac.dac_4v2.dac, sel_volt);
    curr_dac = pmu_range(curr_dac, 0, 1023, 2);

    jeita_cool.cc1_curr_dac  = curr_dac;
    jeita_cool.cc2_curr_dac  = curr_dac;
    jeita_cool.cv_dac        = curr_dac;

    jeita_cool.cc1_curr = pmu_round((cc_curr.cc1_curr * jeita.cool_state_curr_perc), 100);
    jeita_cool.cc2_curr = pmu_round((cc_curr.cc2_curr * jeita.cool_state_curr_perc), 100);

    cc1_rsel_4v2  = pmu_lerp(otp_curr.bat_4v2[0].curr, otp_curr.bat_4v2[0].val, otp_curr.bat_4v2[1].curr, otp_curr.bat_4v2[1].val, jeita_cool.cc1_curr);
    cc1_rsel_4v2  = pmu_range(cc1_rsel_4v2, 0, 1023, 2);
    cc1_rsel_4v35 = pmu_lerp(otp_curr.bat_4v35[0].curr, otp_curr.bat_4v35[0].val, otp_curr.bat_4v35[1].curr, otp_curr.bat_4v35[1].val, jeita_cool.cc1_curr);
    cc1_rsel_4v35 = pmu_range(cc1_rsel_4v35, 0, 1023, 2);
    jeita_cool.cc1_rsel = pmu_lerp(otp_dac.dac_4v35.volt, cc1_rsel_4v35, otp_dac.dac_4v2.volt, cc1_rsel_4v2, sel_volt);
    jeita_cool.cc1_rsel = pmu_range(jeita_cool.cc1_rsel, 0, 1023, 2);

    cc2_rsel_4v2  = pmu_lerp(otp_curr.bat_4v2[0].curr, otp_curr.bat_4v2[0].val, otp_curr.bat_4v2[1].curr, otp_curr.bat_4v2[1].val, jeita_cool.cc2_curr);
    cc2_rsel_4v2  = pmu_range(cc2_rsel_4v2, 0, 1023, 2);
    cc2_rsel_4v35 = pmu_lerp(otp_curr.bat_4v35[0].curr, otp_curr.bat_4v35[0].val, otp_curr.bat_4v35[1].curr, otp_curr.bat_4v35[1].val, jeita_cool.cc2_curr);
    cc2_rsel_4v35 = pmu_range(cc2_rsel_4v35, 0, 1023, 2);
    jeita_cool.cc2_rsel = pmu_lerp(otp_dac.dac_4v35.volt, cc2_rsel_4v35, otp_dac.dac_4v2.volt, cc2_rsel_4v2, sel_volt);
    jeita_cool.cc2_rsel = pmu_range(jeita_cool.cc2_rsel, 0, 1023, 2);

    cv_stop_curr_cfg_t cv_stop_curr_adc;
    if (pmu_get_nvkey(NVID_CAL_CV_STOP_CURR_CFG, (uint8_t *)&cv_stop_curr_adc, sizeof(cv_stop_curr_adc)) != PMU_STATUS_SUCCESS) {
        return;
    }
    jeita_cv = bat_cfg.volt1 - jeita.cool_state_dac_dec;
    adc = pmu_lerp(otp_dac.dac_4v35.volt, vichg_adc.adc_4v35, otp_dac.dac_4v2.volt, vichg_adc.adc_4v2, jeita_cv);
    prec = cv_stop_curr_adc.cv_stop_curr[cv_stop_curr_adc.sel - 1].perc;
    jeita_cool.cv_stop_curr_adc = pmu_round(adc * prec, 100);

    pmu_set_nvkey(NVID_CAL_JEITA_COOL, (uint8_t *)&jeita_cool, sizeof(jeita_cool));
}
#endif  /* AIR_PMU_DISABLE_CHARGER */

void pmu_cal_done(void)
{
    chg_cfg_t nvkey;
    uint16_t id = NVID_CAL_CHG_CFG;
    if (pmu_get_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey)) != PMU_STATUS_SUCCESS) {
        return;
    }

    if (nvkey.kflag != NO_CAL) {
        log_hal_msgid_error("[PMU_CAL]cal_done fail, id[0x%X], kflag[%d]", 2, id, nvkey.kflag);
        return;
    }
    pmu_kflag = CAL_DONE;
    nvkey.kflag = pmu_kflag;

    pmu_set_nvkey(id, (uint8_t *)&nvkey, sizeof(nvkey));
}

void pmu_set_init(void)
{
    if (pmu_kflag != CAL_DONE) {
        log_hal_msgid_error("[PMU_CAL]set_init fail, kflag[%d]", 1, pmu_kflag);
        assert(0);
    }

#ifndef AIR_PMU_DISABLE_CHARGER
    pmu_set_chg_adc_cfg();
    pmu_set_chg_dac_cfg();
    pmu_set_chg_tri_curr_cfg();
    pmu_set_chg_cc1_curr_cfg();
    pmu_set_chg_cc2_curr_cfg();
    pmu_set_chg_cv_stop_curr_adc();
#else
    log_hal_msgid_info("[PMU_CAL]set_init, bypass for dongle", 0);
#endif
    pmu_set_sys_ldo();
    pmu_set_ocp();
    pmu_set_lpo32();

    pmu_set_buck_ripple();
    pmu_set_buck_volt();
    pmu_set_buck_ipeak();
    pmu_set_buck_freq();
    pmu_set_vcore_dl();
    pmu_set_vio18_dl();

    pmu_set_sido_volt();
    pmu_set_sido_ipeak();
    pmu_set_sido_freq();
    pmu_set_vaud18_dl();
    pmu_set_vrf_dl();

    pmu_set_ldo_sram();
    pmu_set_ldo_vdd33_reg();
    pmu_set_ldo_vdd33_reg_ret();
    pmu_set_ldo_vdd33_ret();
    pmu_set_ldo_vdig18();

    pmu_set_hpbg();
    /* lpbg is not used current */
    /* pmu_set_lpbg(); */

    log_hal_msgid_info("[PMU_CAL]set_init, done", 0);
}

void pmu_cal_init(void)
{
    pmu_cal_chg_cfg();
#ifndef AIR_PMU_DISABLE_CHARGER
    pmu_cal_chg_dac();
    pmu_cal_chg_curr_otp();
    pmu_cal_chg_tri_curr();
    pmu_cal_chg_cc1_curr();
    pmu_cal_chg_cc2_curr();
    pmu_cal_cv_stop_curr();
#endif
    log_hal_msgid_info("[PMU_CAL]cal_init, otp_dump[%d], kflag[%d]", 2, pmu_otp_dump, pmu_kflag);

    if (pmu_kflag == NO_CAL) {
    } else if (pmu_kflag == CAL_DONE) {
        log_hal_msgid_info("[PMU_CAL]cal_init, exist", 0);
        return;
    } else {
        //log_hal_msgid_error("[PMU_CAL]cal_init fail, kflag[%d]", 1, pmu_kflag);
        assert(0);
    }

    if (pmu_otp_dump == 0x01) {
        pmu_dump_otp();
    }

#ifndef AIR_PMU_DISABLE_CHARGER
    pmu_cal_vbat_adc();
    pmu_cal_vbat_volt();
    pmu_cal_chg_adc();
#endif

    pmu_cal_buck_volt();
    pmu_cal_buck_ipeak();
    pmu_cal_buck_freq();
    pmu_cal_vcore_dl();
    pmu_cal_vio18_dl();

    pmu_cal_sido_volt();
    pmu_cal_sido_ipeak();
    pmu_cal_sido_freq();
    pmu_cal_vaud18_dl();
    pmu_cal_vrf_dl();

    pmu_cal_ldo_sram();
    pmu_cal_ldo_vdd33_reg();
    pmu_cal_ldo_vdd33_reg_ret();
    pmu_cal_ldo_vdd33_ret();
    pmu_cal_ldo_vdig18();

    pmu_cal_hpbg();
    pmu_cal_lpbg();

    pmu_cal_sys_ldo();
    pmu_cal_ocp();
    pmu_cal_lpo32();

#ifndef AIR_PMU_DISABLE_CHARGER
    pmu_cal_chg_jeita_warm();
    pmu_cal_chg_jeita_cool();
#endif

    pmu_cal_done();

    log_hal_msgid_info("[PMU_CAL]cal_init, done", 0);

    if (pmu_otp_dump == 0x01) {
        pmu_dump_nvkey_lp();
    }
}
#endif  /* AIR_NVDM_ENABLE */
#endif  /* HAL_PMU_MODULE_ENABLED */

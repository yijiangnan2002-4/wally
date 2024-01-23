/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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
#ifdef AIR_LE_AUDIO_ENABLE

#include "app_le_audio_bcst_utillity.h"
#include "app_le_audio_utillity.h"

#include "ble_bass.h"

#include "bt_le_audio_def.h"

#include "bt_le_audio_msglog.h"

#ifdef AIR_REPLACE_NVDM_WITH_NVKEY
#include "nvkey_id_list.h"
#include "nvdm.h"
#include "nvkey.h"
#endif
/**************************************************************************************************
* Define
**************************************************************************************************/
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
#define APP_LE_AUDIO_BCST_GMAP_AC_LEVEL_LOWEST_LATENCY      0
#define APP_LE_AUDIO_BCST_GMAP_AC_LEVEL_LOW_LATENCY         1
#define APP_LE_AUDIO_BCST_GMAP_AC_LEVEL_BALANCED            2
#define APP_LE_AUDIO_BCST_GMAP_AC_LEVEL_HIGH_RELIABILITY    3
#define APP_LE_AUDIO_BCST_GMAP_AC_LEVEL_MAX                 4

#define APP_LE_BCST_GMAP_QOS_CONFIG(sampling_rate, sdu_size, sdu_interval, bitrate, lowest_rtn, lowest_latency, low_rtn, low_latency, balanced_rtn, balanced_latency, high_rtn, high_latency) \
    {sampling_rate, sdu_size, sdu_interval, bitrate, {{lowest_rtn, lowest_latency}, {low_rtn, low_latency}, {balanced_rtn, balanced_latency}, {high_rtn, high_latency}}}
#endif
/**************************************************************************************************
* Structure
**************************************************************************************************/
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
typedef struct {
    uint8_t rtn;                                /* retransmission number */
    uint8_t latency;                            /* Max transport latency (ms) */
} app_le_audio_bcst_gmap_ac_level_t;

typedef struct {
    uint8_t sampling_rate;                      /* sampling rate */
    uint16_t sdu_size;                          /* Maximum SDU size (octets) */
    app_le_audio_sdu_interval_t sdu_interval;   /* SDU interval */
    float bitrate;                              /* Bitrate (kbps) */
    app_le_audio_bcst_gmap_ac_level_t ac_level[APP_LE_AUDIO_BCST_GMAP_AC_LEVEL_MAX];
} app_le_audio_bcst_gmap_qos_params_t;

typedef struct {
    uint8_t ft;
    uint8_t nse;
    uint8_t bn;
    uint8_t rtn;
} app_le_audio_bcst_gmap_ll_params_t;
#endif

/**************************************************************************************************
* Variable
**************************************************************************************************/
app_le_audio_bcst_ctrl_t g_lea_bcst_ctrl;

app_le_audio_qos_params_t g_lea_bcst_qos_params = {
    CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,    /* sampling_freq */
    120,                                        /* sdu_size */
    SDU_INTERVAL_10_MS,                         /* sdu_interval */
    96,                                         /* bitrate */
    4,                                          /* rtn */
    65,                                         /* latency */
};

static const app_le_audio_qos_params_tbl_t g_lea_bcst_qos_params_tbl[] = {
/*  sdu_size    sdu_interval        bitrate low_rtn low_latency high_rtn   high_latency*/
    {30,        SDU_INTERVAL_7P5_MS,   32,      2,      8,        4,        45},       /* 0 16_1 */
    {45,        SDU_INTERVAL_7P5_MS,   48,      2,      8,        4,        45},       /* 1 24_1 */
    {60,        SDU_INTERVAL_7P5_MS,   64,      2,      8,        4,        45},       /* 2 32_1 */
    {75,        SDU_INTERVAL_7P5_MS,   80,      4,      15,       4,        50},       /* 3 48_1 */
    {90,        SDU_INTERVAL_7P5_MS,   96,      4,      15,       4,        50},       /* 4 48_3 */
    {117,       SDU_INTERVAL_7P5_MS,   124.8,   4,      15,       4,        50},       /* 5 48_5 */
    {30,        SDU_INTERVAL_10_MS,    24,      2,      10,       4,        60},       /* 6 8_2 */
    {40,        SDU_INTERVAL_10_MS,    32,      2,      10,       4,        60},       /* 7 16_2 */
    {60,        SDU_INTERVAL_10_MS,    48,      2,      10,       4,        60},       /* 8 24_2 */
    {80,        SDU_INTERVAL_10_MS,    64,      2,      10,       4,        60},       /* 9 32_2 */
    {100,       SDU_INTERVAL_10_MS,    80,      4,      20,       4,        65},       /* 10 48_2 */
    {120,       SDU_INTERVAL_10_MS,    96,      4,      20,       4,        65},       /* 11 48_4 */
    {155,       SDU_INTERVAL_10_MS,    124,     4,      20,       4,        65},       /* 12 48_6 */
    {26,        SDU_INTERVAL_7P5_MS,   27.734,  2,       8,       4,        45},       /* 13 8_1 */
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
    {160,       SDU_INTERVAL_10_MS,    128,     13,    100,      13,       100},       /* 14 48_1_LC3plusHR_CBR */
    {190,       SDU_INTERVAL_10_MS,    152,     13,    100,      13,       100},       /* 15 96_1_LC3plusHR_CBR */
#endif
};

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
static const app_le_audio_bcst_gmap_qos_params_t g_lea_bcst_gmap_qos_params_tbl[] = {
    /* GMAP d09r09 for mic and speaker */
    APP_LE_BCST_GMAP_QOS_CONFIG(48,  75, SDU_INTERVAL_7P5_MS,  80, 2,  8, 2,  8, 2,  8, 2,  8),      /* 0  48_1 */
    APP_LE_BCST_GMAP_QOS_CONFIG(48, 100, SDU_INTERVAL_10_MS,   80, 2, 10, 2, 10, 2, 10, 2, 10),      /* 1  48_2 */
    APP_LE_BCST_GMAP_QOS_CONFIG(48,  90, SDU_INTERVAL_7P5_MS,  96, 2,  8, 2,  8, 2,  8, 2,  8),      /* 2  48_3 */
    APP_LE_BCST_GMAP_QOS_CONFIG(48, 120, SDU_INTERVAL_10_MS,   96, 2, 10, 2, 10, 2, 10, 2, 10),      /* 3  48_4 */
};

#define APP_LE_AUDIO_BCST_GMAP_QOS_PARAMS_TBL_COUNT  (sizeof(g_lea_bcst_gmap_qos_params_tbl)/sizeof(app_le_audio_bcst_gmap_qos_params_t))
#endif

#define APP_LE_AUDIO_BCST_QOS_PARAMS_TBL_COUNT (sizeof(g_lea_bcst_qos_params_tbl)/sizeof(app_le_audio_qos_params_tbl_t))

uint8_t g_lea_bcst_code[BLE_BASS_BROADCAST_CODE_SIZE] = {0};

uint8_t g_lea_bcst_id[BLE_BASS_BROADCAST_ID_SIZE] = {0};

extern const uint32_t g_lea_sdu_interval_tbl[];

/**************************************************************************************************
* Prototype
**************************************************************************************************/

/**************************************************************************************************
* Static Functions
**************************************************************************************************/


/**************************************************************************************************
* Public Functions
**************************************************************************************************/
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
bool app_le_audio_bcst_gmap_set_qos_params(uint8_t sel_setting, uint8_t audio_config_level)
{
    app_le_audio_qos_params_t *qos_parameter = NULL;
    app_le_audio_bcst_gmap_qos_params_t *gmap_qos;

    LE_AUDIO_MSGLOG_I("[APP][B][GMAP] set_qos_params, sel_setting:%d audio_config_level:%d", 2, sel_setting, audio_config_level);

    if (APP_LE_AUDIO_BCST_GMAP_AC_LEVEL_MAX <= audio_config_level || APP_LE_AUDIO_BCST_GMAP_QOS_PARAMS_TBL_COUNT <= sel_setting) {
        LE_AUDIO_MSGLOG_I("[APP][B][GMAP] set_qos_params invalid, max sel_setting:%d level:%d", 2,
            APP_LE_AUDIO_BCST_GMAP_QOS_PARAMS_TBL_COUNT, APP_LE_AUDIO_BCST_GMAP_AC_LEVEL_MAX);
        return FALSE;
    }

    gmap_qos = (app_le_audio_bcst_gmap_qos_params_t *)&g_lea_bcst_gmap_qos_params_tbl[sel_setting];
    qos_parameter = &g_lea_bcst_qos_params;

    qos_parameter->sampling_freq = app_le_audio_get_sample_freq(gmap_qos->sampling_rate);
    qos_parameter->sdu_size      = gmap_qos->sdu_size;
    qos_parameter->sdu_interval  = gmap_qos->sdu_interval;
    qos_parameter->bitrate       = gmap_qos->bitrate;
    qos_parameter->rtn           = gmap_qos->ac_level[audio_config_level].rtn;
    qos_parameter->latency       = gmap_qos->ac_level[audio_config_level].latency;

    LE_AUDIO_MSGLOG_I("[APP][B][GMAP] set_qos_params qos_parameter:%d %d %d %d %d %d", 6,
        qos_parameter->sampling_freq,
        qos_parameter->sdu_size,
        qos_parameter->sdu_interval,
        (int)(qos_parameter->bitrate * 10),
        qos_parameter->rtn,
        qos_parameter->latency);

    app_le_audio_bcst_write_qos_params_nvkey(gmap_qos->sampling_rate, sel_setting, audio_config_level);

    return TRUE;
}
#endif

void app_le_audio_bcst_set_qos_params(uint8_t sampling_rate, uint8_t sel_setting, bool high_reliability)
{
    uint8_t sampling_freq;

    LE_AUDIO_MSGLOG_I("[APP][B] set_qos_params, sampling_rate:%d sel_setting:%d high_reliability:%x", 3, sampling_rate, sel_setting, high_reliability);

    if (APP_LE_AUDIO_BCST_QOS_PARAMS_TBL_COUNT <= sel_setting) {
        LE_AUDIO_MSGLOG_I("[APP][B] set_qos_params, invalid sel_setting:%d max:%d", 2, sel_setting, APP_LE_AUDIO_BCST_QOS_PARAMS_TBL_COUNT);
        return;
    }

    if (APP_LE_AUDIO_SAMPLING_FREQ_INVALID == (sampling_freq = app_le_audio_get_sample_freq(sampling_rate))) {
        LE_AUDIO_MSGLOG_I("[APP][B] set_qos_params, invalid sampling_rate:%x", 1, sampling_rate);
        return;
    }

    g_lea_bcst_qos_params.sampling_freq = sampling_freq;

    g_lea_bcst_qos_params.sdu_size = g_lea_bcst_qos_params_tbl[sel_setting].sdu_size;
    g_lea_bcst_qos_params.sdu_interval = g_lea_bcst_qos_params_tbl[sel_setting].sdu_interval;
    g_lea_bcst_qos_params.bitrate = g_lea_bcst_qos_params_tbl[sel_setting].bitrate;

    if (high_reliability) {
        g_lea_bcst_qos_params.rtn = g_lea_bcst_qos_params_tbl[sel_setting].high_rtn;
        g_lea_bcst_qos_params.latency = g_lea_bcst_qos_params_tbl[sel_setting].high_latency;
    } else {
        g_lea_bcst_qos_params.rtn = g_lea_bcst_qos_params_tbl[sel_setting].low_rtn;
        g_lea_bcst_qos_params.latency = g_lea_bcst_qos_params_tbl[sel_setting].low_latency;
    }

    LE_AUDIO_MSGLOG_I("[APP][B] set_qos_params, %d %d %d %d %d %d %d", 7,
                      sampling_rate,
                      g_lea_bcst_qos_params.sampling_freq,
                      g_lea_bcst_qos_params.sdu_size,
                      g_lea_bcst_qos_params.sdu_interval,
                      (int)(g_lea_bcst_qos_params.bitrate * 10),
                      g_lea_bcst_qos_params.rtn,
                      g_lea_bcst_qos_params.latency);
}

uint32_t app_le_audio_bcst_get_sampling_rate(void)
{
    switch (g_lea_bcst_qos_params.sampling_freq) {
        case CODEC_CONFIGURATION_SAMPLING_FREQ_8KHZ:
            return 8000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_16KHZ:
            return 16000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_24KHZ:
            return 24000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_32KHZ:
            return 32000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_44_1KHZ:
            return 44100;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ:
            return 48000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_96KHZ:
            return 96000;
        default:
            break;
    }

    return 0;
}

uint16_t app_le_audio_bcst_get_sdu_size(void)
{
    return g_lea_bcst_qos_params.sdu_size;
}

uint32_t app_le_audio_bcst_get_sdu_interval(void)
{
    return g_lea_sdu_interval_tbl[g_lea_bcst_qos_params.sdu_interval];
}

float app_le_audio_bcst_get_bitrate(void)
{
    return g_lea_bcst_qos_params.bitrate;
}

void app_le_audio_bcst_reset_info(void)
{
    uint8_t i = APP_LE_AUDIO_BCST_BIS_MAX_NUM;

    g_lea_bcst_ctrl.bis_num = APP_LE_AUDIO_BCST_BIS_MAX_NUM;

    while (i > 0) {
        i--;
        g_lea_bcst_ctrl.bis_handle[i] = BT_HANDLE_INVALID;
    }
}

void app_le_audio_bcst_reset_info_ex(void)
{
    uint8_t i = APP_LE_AUDIO_BCST_BIS_MAX_NUM;

    memset(&g_lea_bcst_ctrl, 0, sizeof(app_le_audio_bcst_ctrl_t));

    g_lea_bcst_ctrl.bis_num = APP_LE_AUDIO_BCST_BIS_MAX_NUM;

    while (i > 0) {
        i--;
        g_lea_bcst_ctrl.bis_handle[i] = BT_HANDLE_INVALID;
    }
}

app_le_audio_bcst_state_t app_le_audio_bcst_gat_curr_state(void)
{
    return g_lea_bcst_ctrl.curr_state;
}

void app_le_audio_bcst_set_code(uint8_t *bcst_code)
{
    if (NULL != bcst_code) {
        memcpy(g_lea_bcst_code, bcst_code, BLE_BASS_BROADCAST_CODE_SIZE);
    }
}

uint8_t *app_le_audio_bcst_get_code(void)
{
    return g_lea_bcst_code;
}

void app_le_audio_bcst_set_id(uint32_t bcst_id)
{
    if (bcst_id) {
        memcpy(g_lea_bcst_id, &bcst_id, BLE_BASS_BROADCAST_ID_SIZE);
    }
}

uint8_t *app_le_audio_bcst_get_id(void)
{
    return g_lea_bcst_id;
}


#ifdef AIR_REPLACE_NVDM_WITH_NVKEY
void app_le_audio_bcst_write_nvkey_callback(nvkey_status_t status, void *user_data)
{
    if (status != NVKEY_STATUS_OK){
        LE_AUDIO_MSGLOG_I("[APP][B] app_le_audio_bcst_write_nvkey_callback, write nvkey fail! ", 0);
    }
}
#endif

bt_status_t app_le_audio_bcst_write_qos_params_nvkey(uint8_t sampling_rate, uint8_t sel_setting, bool high_reliability)
{
#ifdef AIR_REPLACE_NVDM_WITH_NVKEY
    nvkey_status_t write_status;
    app_le_audio_bcst_qos_params_db_t bcst_qos_params_db;
    bcst_qos_params_db.sampling_rate = sampling_rate;
    bcst_qos_params_db.sel_setting = sel_setting;
    bcst_qos_params_db.high_reliability = high_reliability;

    write_status = nvkey_write_data_non_blocking(NVID_BT_LEA_BCST_QOS_PARAMS, (const uint8_t *)&bcst_qos_params_db,
                                                sizeof(app_le_audio_bcst_qos_params_db_t),app_le_audio_bcst_write_nvkey_callback, NULL);
    if (write_status != NVKEY_STATUS_OK) {
        LE_AUDIO_MSGLOG_I("[APP][B] Write bcst qos params nvkey fail!", 0);
        return BT_STATUS_FAIL;
    }
    LE_AUDIO_MSGLOG_I("[APP][B] Write bcst qos params nvkey success", 0);
#endif
    return BT_STATUS_SUCCESS;
}

bt_status_t app_le_audio_bcst_read_qos_params_nvkey(app_le_audio_bcst_qos_params_db_t *bcst_qos_params_db)
{
#ifdef AIR_REPLACE_NVDM_WITH_NVKEY
    nvkey_status_t read_status;
    uint32_t size = sizeof(app_le_audio_bcst_qos_params_db_t);

    read_status = nvkey_read_data(NVID_BT_LEA_BCST_QOS_PARAMS, (uint8_t *)bcst_qos_params_db, &size);
    if (read_status != NVKEY_STATUS_OK) {
        LE_AUDIO_MSGLOG_I("[APP][B] Read bcst qos params nvkey fail!", 0);
        return BT_STATUS_FAIL;
    }
    LE_AUDIO_MSGLOG_I("[APP][B] Read bcst qos params nvkey success", 0);
#endif
    return BT_STATUS_SUCCESS;
}

#endif  /* AIR_LE_AUDIO_ENABLE */


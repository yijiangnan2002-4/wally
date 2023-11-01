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

#include "app_le_audio.h"
#include "app_le_audio_usb.h"
#include "app_le_audio_ucst.h"
#include "app_le_audio_air.h"
#include "app_le_audio_aird.h"
#include "usbaudio_drv.h"
#include "usb_hid_srv.h"
#include "app_le_audio_ucst_utillity.h"
#include "app_le_audio_ccp_call_control_server.h"
#include "app_le_audio_csip_set_coordinator.h"

#include "app_le_audio_nvkey_struct.h"
#include "app_dongle_connection_common.h"

#include "bt_gattc_discovery.h"
#include "bt_device_manager_le.h"

#include "bt_le_audio_source.h"
#include "ble_bap_client.h"
#include "ble_csip.h"
#include "ble_bap.h"

#include "apps_events_event_group.h"
#include "ui_shell_manager.h"
#include "hal_usb.h"
#include "bt_le_audio_msglog.h"
#ifdef AIR_SILENCE_DETECTION_ENABLE
#include "bt_sink_srv_ami.h"
#include "app_le_audio_utillity.h"
#include "app_le_audio_ucst_utillity.h"
#endif
#include "app_le_audio_tmap.h"


#ifdef AIR_LE_AUDIO_BA_ENABLE
#include "app_le_audio_ba.h"
#endif
#include "app_le_audio_ucst_utillity.h"

#ifdef AIR_MS_TEAMS_ENABLE
#include "app_ms_teams_utils.h"
#include "ms_teams.h"
#endif

#ifndef AIR_VOLUME_CONTROL_BY_DONGLE
#include "apps_events_usb_event.h"
#endif

#ifdef AIR_LE_AUDIO_HAPC_ENABLE
#include "app_le_audio_hapc.h"
#endif

/**************************************************************************************************
* Define
**************************************************************************************************/
/* ASE releated */
#define APP_LE_AUDIO_UCST_ASE_ID_INVALID    0

#define APP_LE_AUDIO_UCST_ASE_OPCODE_NONE   0

#define APP_LE_AUDIO_UCST_ASE_STATE_INVALID         0xFF

#define APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN    2       /* ASE control point header(2B) = | Opcode (1B) | ASE num (1B) | */
#define APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN      2       /* ASE metadata:(2B) = | ASE ID (1B) | metadata len (1B) | */

#define APP_LE_AUDIO_UCST_MAX_MODE_NUM      2   /* call mode & media mode */

/* CIS releated */
#define APP_LE_AUDIO_CIG_ID_1           0x01    /* unidirectional (c->s) */
#define APP_LE_AUDIO_CIG_ID_2           0x02    /* bidirectional */

#define APP_LE_AUDIO_CIS_ID_1_MEDIA     0x01    /* Left */
#define APP_LE_AUDIO_CIS_ID_2_MEDIA     0x02    /* Right */
#define APP_LE_AUDIO_CIS_ID_3_CALL      0x03    /* Left */
#define APP_LE_AUDIO_CIS_ID_4_CALL      0x04    /* Right */



/* Metadata: Streaming Audio Contexts */
#define APP_LE_AUDIO_METADATA_STREAMING_AUDIO_CONTEXTS_LEN      0x03
#define APP_LE_AUDIO_METADATA_STREAMING_AUDIO_CONTEXTS_TYPE     0x02
#define APP_LE_AUDIO_CONTEXTS_TYPE_GAME                         0x0004

#define APP_LE_AUDIO_UCST_VCP_DEFAULT_VOLUME 0xFF

#define APP_LE_AUDIO_UCST_IS_CALL_MODE  ((APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) || \
                                        ((APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.curr_target) && \
                                        (APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.next_target) && \
                                        (APP_LE_AUDIO_USB_PORT_MASK_MIC_0 & app_le_audio_usb_get_streaming_port())))

#define APP_LE_AUDIO_PREPARE_VCMD_MODE_DISCONN  0x01
#define APP_LE_AUDIO_PREPARE_VCMD_MODE_CONN     0x02

enum {
    CODEC_SPECIFIC_CAPABILITIIES_TYPE_SAMPLING_FREQUENCY = 0x01, /**< Supported Sampling Frequencies.*/
    CODEC_SPECIFIC_CAPABILITIIES_TYPE_FRAME_DURATIONS,           /**< Supported Frame Durations.*/
    CODEC_SPECIFIC_CAPABILITIIES_TYPE_AUDIO_CHANNEL_COUNTS,      /**< Audio Channel Counts.*/
    CODEC_SPECIFIC_CAPABILITIIES_TYPE_OCTETS_PER_CODEC_FRAME,     /**< Supported Octets Per Codec Frame.*/
    CODEC_SPECIFIC_CAPABILITIIES_TYPE_CODEC_FRAME_BLOCKS_PER_SDU, /**< Max Supported LC3 Frames Per SDU.*/
};

#define MATADATA_TYPE_PREFERRED_AUDIO_CONTEXTS     0x01
#define MATADATA_TYPE_STREAMING_AUDIO_CONTEXTS     0x02

#define FRAME_DURATION_7P5_MS           0x00 /**< 7.5 ms.*/
#define FRAME_DURATION_10_MS            0x01  /**< 10 ms.*/

enum {
    CODEC_SPECIFIC_CAPABILITIY_SETTING_8_1 = 0x00,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_8_2,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_16_1,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_16_2,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_24_1,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_24_2,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_32_1,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_32_2,
    //CODEC_SPECIFIC_CAPABILITIY_SETTING_441_1,
    //CODEC_SPECIFIC_CAPABILITIY_SETTING_441_2,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_48_1,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_48_2,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_48_3,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_48_4,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_48_5,
    CODEC_SPECIFIC_CAPABILITIY_SETTING_48_6,
};
typedef uint8_t app_le_audio_ucst_codec_capability_setting;

#define TMAP_ROLE_CG        BLE_TMAP_ROLE_MASK_CG
#define TMAP_ROLE_CT        BLE_TMAP_ROLE_MASK_CT
#define TMAP_ROLE_UMS       BLE_TMAP_ROLE_MASK_UMS
#define TMAP_ROLE_UMR       BLE_TMAP_ROLE_MASK_UMR
#define TMAP_ROLE_UMR_UMS   (BLE_TMAP_ROLE_MASK_UMR|BLE_TMAP_ROLE_MASK_UMS)
#define TMAP_ROLE_CT_CG     (BLE_TMAP_ROLE_MASK_CT|BLE_TMAP_ROLE_MASK_CG)

/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct {
    uint8_t sampling_freq;                       /* Sampling frequency */
    uint8_t frame_duration;                      /* 0x00:7.5ms, 0x01:10ms */
    uint16_t octets_per_codec_frame;             /* octets */
    float bitrate;                               /* Bitrate (kbps) */
    uint16_t support_tmap_role;                  /* supported TMAP role */
    //uint16_t support_gmap_role;                  /* supported GMAP role */
} app_le_audio_ucst_codec_specific_capabilities_t;


BT_PACKED(
typedef struct {
    bt_handle_t handle;     /* LE Connection Handle */
    uint8_t mode;           /* bit0: Disconnect bit1:Connection */
    uint8_t enable;         /* 0: Disable, 1:Enable */
}) app_le_audio_prepare_vcmd_t;

/**************************************************************************************************
* Variable
**************************************************************************************************/
static bt_handle_t g_lea_ucst_prepare_vcmd_conn_handle[APP_LE_AUDIO_UCST_LINK_MAX_NUM];
static bt_handle_t g_lea_ucst_prepare_vcmd_disconn_handle[APP_LE_AUDIO_UCST_LINK_MAX_NUM];
app_le_audio_ucst_cis_info_t g_lea_ucst_cis_info[APP_LE_AUDIO_UCST_CIS_MAX_NUM];

static ble_bap_config_codec_param_t g_lea_cofig_code_param = {
    .ase_id = APP_LE_AUDIO_UCST_ASE_ID_INVALID,
    .target_latency = 1,
    .target_phy = 2,
    .codec_id.coding_format = 6,
    .codec_id.company_id = 0,
    .codec_id.vendor_specific_codec_id = 0,
    .codec_specific_configuration_length = 19,
    .codec_specific_configuration.sampling_freq_len = CODEC_CONFIGURATION_LEN_SAMPLING_FREQUENCY,
    .codec_specific_configuration.sampling_freq_type = CODEC_CONFIGURATION_TYPE_SAMPLING_FREQUENCY,
    .codec_specific_configuration.sampling_freq_value = CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,
    .codec_specific_configuration.frame_duration_len = CODEC_CONFIGURATION_LEN_FRAME_DURATIONS,
    .codec_specific_configuration.frame_duration_type = CODEC_CONFIGURATION_TYPE_FRAME_DURATIONS,
    .codec_specific_configuration.frame_duration_value = SDU_INTERVAL_10_MS,
    .codec_specific_configuration.octets_per_codec_frame_len = CODEC_CONFIGURATION_LEN_OCTETS_PER_CODEC_FRAME,
    .codec_specific_configuration.octets_per_codec_frame_type = CODEC_CONFIGURATION_TYPE_OCTETS_PER_CODEC_FRAME,
    .codec_specific_configuration.octets_per_codec_frame_value = 155,
    .codec_specific_configuration.audio_channel_alloaction_len = CODEC_CONFIGURATION_LEN_AUDIO_CHANNEL_ALLOCATION,
    .codec_specific_configuration.audio_channel_alloaction_type = CODEC_CONFIGURATION_TYPE_AUDIO_CHANNEL_ALLOCATION,
    .codec_specific_configuration.audio_channel_alloaction_value = AUDIO_LOCATION_FRONT_LEFT,
    .codec_specific_configuration.codec_frame_blocks_per_sdu_len = CODEC_CONFIGURATION_LEN_CODEC_FRAME_BLOCKS_PER_SDU,
    .codec_specific_configuration.codec_frame_blocks_per_sdu_type = CODEC_CONFIGURATION_TYPE_CODEC_FRAME_BLOCKS_PER_SDU,
    .codec_specific_configuration.codec_frame_blocks_per_sdu_value = 1,
};
//BAP 3.6.7 Unicast Client audio capability configuration support is same to 3.5.2 Additional Published Audio Capabilities Service requirements

static const app_le_audio_ucst_codec_specific_capabilities_t g_lea_ucst_codec_specific_capabilities_tbl[] =
{
    /*     sampling_freq                      frame_duration  octets_per_codec_frame  bitrate support_tmap_role */ /* Sink Source */
/*8_1*/ {CODEC_CONFIGURATION_SAMPLING_FREQ_8KHZ,   FRAME_DURATION_7P5_MS,    26,        27.734, 0                },/*  O     O   */
/*8_2*/ {CODEC_CONFIGURATION_SAMPLING_FREQ_8KHZ,   FRAME_DURATION_10_MS,     30,        24,     0                },/*  O     O   */
/*16_1*/{CODEC_CONFIGURATION_SAMPLING_FREQ_16KHZ,  FRAME_DURATION_7P5_MS,    30,        32,     TMAP_ROLE_CT     },/*  O     O   */ /* CT:M */
/*16_2*/{CODEC_CONFIGURATION_SAMPLING_FREQ_16KHZ,  FRAME_DURATION_10_MS,     40,        32,     0                },/*  M     M   */
/*24_1*/{CODEC_CONFIGURATION_SAMPLING_FREQ_24KHZ,  FRAME_DURATION_7P5_MS,    45,        48,     0                },/*  O     O   */
/*24_2*/{CODEC_CONFIGURATION_SAMPLING_FREQ_24KHZ,  FRAME_DURATION_10_MS,     60,        48,     0                },/*  M     O   */
/*32_1*/{CODEC_CONFIGURATION_SAMPLING_FREQ_32KHZ,  FRAME_DURATION_7P5_MS,    60,        64,     TMAP_ROLE_CT     },/*  O     O   */ /* CT:M */
/*32_2*/{CODEC_CONFIGURATION_SAMPLING_FREQ_32KHZ,  FRAME_DURATION_10_MS,     80,        64,     TMAP_ROLE_CT_CG  },/*  O     O   */ /* CT:M CG:M */
/*441_1*///{CODEC_CONFIGURATION_SAMPLING_FREQ_44_1KHZ,FRAME_DURATION_8_163,     97,       95.06,  0                },/*  O     O   */
/*441_2*///{CODEC_CONFIGURATION_SAMPLING_FREQ_44_1KHZ,FRAME_DURATION_10_884,    130,      95.55,  0                },/*  O     O   */
/*48_1*/{CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,  FRAME_DURATION_7P5_MS,    75,        80,     TMAP_ROLE_UMR    },/*  O     O   */ /* UMR:M */
/*48_2*/{CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,  FRAME_DURATION_10_MS,     100,       80,     TMAP_ROLE_UMR_UMS},/*  O     O   */ /* UMR:M UMS:M */
/*48_3*/{CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,  FRAME_DURATION_7P5_MS,    90,        96,     TMAP_ROLE_UMR    },/*  O     O   */ /* UMR:M */
/*48_4*/{CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,  FRAME_DURATION_10_MS,     120,       96,     TMAP_ROLE_UMR_UMS},/*  O     O   */ /* UMR:M UMS:C1 */
/*48_5*/{CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,  FRAME_DURATION_7P5_MS,    117,       124.8,  TMAP_ROLE_UMR    },/*  O     O   */ /* UMR:M */
/*48_6*/{CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,  FRAME_DURATION_10_MS,     155,       124,    TMAP_ROLE_UMR_UMS} /*  O     O   */ /* UMR:M UMS:C1 */
};


extern app_le_audio_ctrl_t g_lea_ctrl;

extern app_le_audio_ucst_ctrl_t g_lea_ucst_ctrl;

extern app_le_audio_ucst_group_info_t g_lea_ucst_group_info[APP_LE_AUDIO_UCST_GROUP_ID_MAX];

extern app_le_audio_ucst_link_info_t g_lea_ucst_link_info[APP_LE_AUDIO_UCST_LINK_MAX_NUM];

extern uint8_t g_lea_ucst_qos_params_selected;

extern app_le_audio_qos_params_t g_lea_ucst_qos_params_spk_0;

extern app_le_audio_qos_params_t g_lea_ucst_qos_params_spk_1;

extern app_le_audio_qos_params_t g_lea_ucst_qos_params_mic_0;

extern app_le_audio_ucst_cig_params_test_t *g_lea_ucst_cig_params_test;

extern uint8_t g_lea_ucst_test_mode_flag;

extern uint8_t g_lea_ucst_ccid_list_size;

extern uint8_t g_lea_ucst_ccid_list[APP_LE_AUDIO_UCST_MAX_CCID_LIST_SIZE];

#ifdef AIR_MS_TEAMS_ENABLE
bool g_lea_ucst_pts_remote_call_service = false;
extern ble_tbs_call_index_t g_curr_call_idx;
#endif

/**************************************************************************************************
* Prototype
**************************************************************************************************/

extern void app_le_audio_usb_hid_handle_busy_call(bool busy);

extern void bt_app_common_at_cmd_print_report(char *string);

extern bt_status_t ble_tbs_switch_device_completed(void);

extern bt_status_t bt_gap_le_remove_cig(const bt_gap_le_remove_cig_t *params);

extern bool ble_vcp_check_update_volume_complete(uint8_t link_idx);

#if defined(AIR_MS_TEAMS_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE)
extern bool app_le_audio_usb_hid_handle_call_active(void);
extern bool app_le_audio_usb_hid_handle_call_end(void);
extern bool app_le_audio_usb_hid_handle_call_hold(void);
extern bool app_le_audio_usb_hid_handle_call_unhold(void);
#endif

/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static void app_le_audio_ucst_reset_release(void)
{
    uint32_t i;

    for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
        g_lea_ucst_link_info[i].release = false;
    }

    g_lea_ucst_ctrl.release = false;
}

static void app_le_audio_ucst_send_prepare_vcmd(bt_handle_t handle, uint8_t mode, uint8_t enable)
{
    app_le_audio_prepare_vcmd_t param = {
        .handle = handle,
        .mode = mode,
        .enable = enable,
    };

    bt_hci_send_vendor_cmd(0xFDCE, (uint8_t *)&param, sizeof(app_le_audio_prepare_vcmd_t));

    uint8_t i = 0;
    if (APP_LE_AUDIO_PREPARE_VCMD_MODE_DISCONN == mode) {
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (enable) {
                if (BT_HANDLE_INVALID == g_lea_ucst_prepare_vcmd_disconn_handle[i]) {
                    g_lea_ucst_prepare_vcmd_disconn_handle[i] = handle;
                    break;
                }
            } else {
                if (handle == g_lea_ucst_prepare_vcmd_disconn_handle[i]) {
                    g_lea_ucst_prepare_vcmd_disconn_handle[i] = BT_HANDLE_INVALID;
                    break;
                }
            }
        }
    } else if (APP_LE_AUDIO_PREPARE_VCMD_MODE_CONN == mode) {
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (enable) {
                if (BT_HANDLE_INVALID == g_lea_ucst_prepare_vcmd_conn_handle[i]) {
                    g_lea_ucst_prepare_vcmd_conn_handle[i] = handle;
                    break;
                }
            } else {
                if (handle == g_lea_ucst_prepare_vcmd_conn_handle[i]) {
                    g_lea_ucst_prepare_vcmd_conn_handle[i] = BT_HANDLE_INVALID;
                    break;
                }
            }
        }
    }
}

static uint8_t app_le_audio_ucst_get_ase_idx(app_le_audio_ucst_link_info_t *p_info, uint8_t ase_id)
{
    uint8_t i;

    if ((NULL == p_info) ||
        (APP_LE_AUDIO_UCST_ASE_ID_INVALID == ase_id)) {
        LE_AUDIO_MSGLOG_I("[APP] get_ase_idx error, ase_id", 1, ase_id);
        return APP_LE_AUDIO_UCST_ASE_MAX_NUM;
    }

    for (i = 0; i < APP_LE_AUDIO_UCST_ASE_MAX_NUM; i++) {
        if (ase_id == p_info->ase[i].id) {
            /*LE_AUDIO_MSGLOG_I("[APP] get_ase_idx, id[%x]:%x direction:%x ase_state:%x", 4, i,
                                p_info->ase[i].id,
                                p_info->ase[i].direction,
                                p_info->ase[i].curr_state);*/
            break;
        }
    }

    if (APP_LE_AUDIO_UCST_ASE_MAX_NUM == i) {
        LE_AUDIO_MSGLOG_I("[APP] get_ase_idx not found, ase_id", 1, ase_id);
    }

    return i;
}

uint8_t app_le_audio_ucst_get_max_link_num(void)
{
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    return APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM;
#else
    return APP_LE_AUDIO_UCST_LINK_MAX_NUM;
#endif
}

uint8_t app_le_audio_ucst_get_link_num_ex(void)
{
    uint8_t i = 0, count = 0;

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        if ((BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) &&
            (APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL != g_lea_ucst_link_info[i].next_state)) {
            count++;
        }
    }

    return count;
}

static uint16_t app_le_audio_ucst_get_metadata_len(void)
{
    /* MUST sync with API: app_le_audio_ucst_set_metadata() */

    if (g_lea_ucst_ccid_list_size) {
        /* metadata_len: (num of metadata + context_type metadata len + CCID_list metadata len) */
        return (2 + METADATA_LTV_LEN_STREAMING_AUDIO_CONTEXTS + (1 + g_lea_ucst_ccid_list_size));
    }

    /* metadata: (num of metadata + context_type metadata len) */
    return (1 + METADATA_LTV_LEN_STREAMING_AUDIO_CONTEXTS);
}

static uint8_t app_le_audio_ucst_get_sink_ase_num(app_le_audio_ucst_link_info_t *p_info)
{
    switch (g_lea_ucst_ctrl.create_cis_mode) {
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_2:
        //case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_9_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_9_2:
            //case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_10:
        {
            return 0;
        }
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_3:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4:
        //case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_5:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_6_2:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_2:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_11_2: {
            return 1;
        }
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_6_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_1:
            //case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_11_1:
        {
            return 2;
        }
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_2: {
            if (AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location) {
                return 1;
            }
            return 0;
        }
        default:
            break;
    }

    if ((0 != p_info->sink_location_num) && (p_info->sink_ase_num > p_info->sink_location_num)) {
        return p_info->sink_location_num;
    }

    return p_info->sink_ase_num;
}

static uint8_t app_le_audio_ucst_get_source_ase_num(app_le_audio_ucst_link_info_t *p_info)
{
    switch (g_lea_ucst_ctrl.create_cis_mode) {
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_6_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_6_2: {
            return 0;
        }
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_2:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_3:
        //case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_5:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_9_2:
        //case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_10:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_11_2: {
            return 1;
        }
#if 0
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_9_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_11_1: {
            return 2;
        }
#endif
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_2:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_2: {
            if (AUDIO_LOCATION_FRONT_LEFT == p_info->source_location) {
                return 1;
            }
            return 0;
        }
        default:
            break;
    }

    if ((0 != p_info->source_location_num) && (p_info->source_ase_num > p_info->source_location_num)) {
        return p_info->source_location_num;
    }

    return p_info->source_ase_num;
}

static bool app_le_audio_ucst_set_ase(app_le_audio_ucst_link_info_t *p_info, uint8_t ase_id, uint8_t ase_state, bt_le_audio_direction_t direction)
{
    uint8_t i;

    if (NULL == p_info) {
        LE_AUDIO_MSGLOG_I("[APP][U] set_ase, p_info null", 0);
        return false;
    }

    for (i = 0; i < APP_LE_AUDIO_UCST_ASE_MAX_NUM; i++) {
        if (0 == p_info->ase[i].id) {
            p_info->ase[i].id = ase_id;
            p_info->ase[i].curr_state = ase_state;
            p_info->ase[i].direction = direction;
            LE_AUDIO_MSGLOG_I("[APP][U] set_ase, id[%x]:%x direction:%x ase_state:%x", 4, i,
                              p_info->ase[i].id,
                              p_info->ase[i].direction,
                              p_info->ase[i].curr_state);
            return true;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][U] set_ase, no space!", 0);
    return false;
}

static uint16_t app_le_audio_ucst_set_metadata(uint8_t *p_buf, uint16_t context_type)
{
    /* MUST sync with API: app_le_audio_ucst_get_metadata_len() */

    ble_bap_ltv_structure_t *p_ltv = NULL;
    uint16_t i = 0;

    if (NULL == p_buf) {
        return 0;
    }

    /* LTV_0: AUDIO_CONTEXTS */
    p_ltv = (ble_bap_ltv_structure_t *)&p_buf[0];
    /* lenght: */
    p_ltv->length = METADATA_LTV_LEN_STREAMING_AUDIO_CONTEXTS;
    /* type: */
    p_ltv->type = METADATA_LTV_TYPE_STREAMING_AUDIO_CONTEXTS;
    /* value: */
    p_ltv->value[0] = (context_type & 0xFF);
    p_ltv->value[1] = ((context_type & 0xFF00) >> 8);
    i += (METADATA_LTV_LEN_STREAMING_AUDIO_CONTEXTS + 1);


    /* LTV_1: CCID_LIST */
    if (g_lea_ucst_ccid_list_size) {
        p_ltv = (ble_bap_ltv_structure_t *)&p_buf[i];
        /* lenght: */
        p_ltv->length = (1 + g_lea_ucst_ccid_list_size);
        /* type: */
        p_ltv->type = METADATA_LTV_TYPE_CCID_LIST;
        /* value: */
        p_ltv->value[0] = g_lea_ucst_ccid_list[0];
        if (APP_LE_AUDIO_UCST_MAX_CCID_LIST_SIZE == g_lea_ucst_ccid_list_size) {
            p_ltv->value[1] = g_lea_ucst_ccid_list[1];
        }
        i += (2 + g_lea_ucst_ccid_list_size);
    }

    return i;
}

void app_le_audio_ucst_set_mic_channel(void)
{
    uint32_t channel = 0;
    uint8_t i, tmp;

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    app_le_audio_ucst_group_info_t *p_group_info = NULL;

    if (NULL == (p_group_info = app_le_audio_ucst_get_group_info(g_lea_ucst_ctrl.curr_group))) {
        return;
    }
#endif

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = p_group_info->link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        if (BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) {
            switch (g_lea_ucst_link_info[i].source_ase_num) {
                case 1: {
                    if (AUDIO_LOCATION_FRONT_LEFT == g_lea_ucst_link_info[i].source_location) {
                        channel |= APP_LE_AUDIO_TRANSMITTER_CHANNEL_L;
                    } else if (AUDIO_LOCATION_FRONT_RIGHT == g_lea_ucst_link_info[i].source_location) {
                        channel |= APP_LE_AUDIO_TRANSMITTER_CHANNEL_R;
                    }
                    break;
                }
                case 2: {
                    channel = APP_LE_AUDIO_TRANSMITTER_CHANNEL_DUAL;
                    if (g_lea_ucst_link_info[i].source_location_num == g_lea_ucst_link_info[i].source_ase_num) {
                        app_le_audio_set_audio_transmitter_mic_channel(channel);
                        return;
                    }
                    break;
                }
                default: {
                    channel |= APP_LE_AUDIO_TRANSMITTER_CHANNEL_L;
                    break;
                }
            }
        }
    }

    if ((0 == channel) || (APP_LE_AUDIO_TRANSMITTER_CHANNEL_DUAL == channel
#ifdef APP_LE_AUDIO_UCST_UPLINK_MIX_ENABLE
        && !app_le_audio_ucst_get_uplink_mix_status()
#endif
        )) {
        LE_AUDIO_MSGLOG_I("[APP][U] set_mic_channel, use L (channel:%x)", 1, channel);
        channel = APP_LE_AUDIO_TRANSMITTER_CHANNEL_L;
    }

    app_le_audio_set_audio_transmitter_mic_channel(channel);
}


static bt_status_t app_le_audio_ucst_exchange_mtu(bt_handle_t handle)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_EXCHANGE_MTU == p_info->next_state) {
        return BT_STATUS_SUCCESS;
    }

    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_EXCHANGE_MTU;

    BT_GATTC_NEW_EXCHANGE_MTU_REQ(req, 512);
    ret = bt_gattc_exchange_mtu(handle, &req);

    if (BT_STATUS_SUCCESS != ret) {
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
        LE_AUDIO_MSGLOG_I("[APP][U] exchange_mtu failed, handle:%x ret:%x", 2, handle, ret);
    }

    return ret;
}

app_le_audio_ucst_codec_specific_capabilities_t* app_le_audio_ucst_get_codec_configuration(bt_handle_t handle, uint16_t context_type, bt_le_audio_direction_t direction)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    app_le_audio_pac_t *p_pac = NULL;
    uint8_t sampling_freq = 0, frame_duration = 0;
    //uint16_t octets_per_codec_frame = 0;
    uint8_t record_index, preferred_record_index, codec_index, preferred_audio_contexts_cnt;
    uint8_t sampling_freq_max = CODEC_CONFIGURATION_SAMPLING_FREQ_8KHZ,record_index_max = 0;
    uint8_t audio_channel_num = 0x01;//channel count per cis
    uint16_t tmap_role = 0;
    uint16_t preferred_audio_contexts;
    uint16_t octets_per_codec_frame;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return NULL;
    }

    if (AUDIO_DIRECTION_SINK == direction) {
        p_pac = &p_info->sink_pac;
    }
    else {
        p_pac = &p_info->source_pac;
    }

    if (g_lea_ucst_qos_params_selected) {
        app_le_audio_qos_params_t *p_qos = NULL;
        if ((AUDIO_DIRECTION_SINK == direction) && ((AUDIO_CONTENT_TYPE_CONVERSATIONAL == context_type) || (AUDIO_CONTENT_TYPE_GAME == context_type))) {
            p_qos = &g_lea_ucst_qos_params_spk_0;
        }
        else if ((AUDIO_DIRECTION_SOURCE == direction) && ((AUDIO_CONTENT_TYPE_CONVERSATIONAL == context_type) || (AUDIO_CONTENT_TYPE_GAME == context_type))){
            p_qos = &g_lea_ucst_qos_params_mic_0;
        }
        else {
            p_qos = &g_lea_ucst_qos_params_spk_1;
        }
        sampling_freq = p_qos->sampling_freq;
        frame_duration = p_qos->sdu_interval;
        octets_per_codec_frame = p_qos->sdu_size;

        for (codec_index = 0; codec_index < sizeof(g_lea_ucst_codec_specific_capabilities_tbl)/sizeof(app_le_audio_ucst_codec_specific_capabilities_t); codec_index++) {
            if ((sampling_freq == g_lea_ucst_codec_specific_capabilities_tbl[codec_index].sampling_freq) &&
                (frame_duration == g_lea_ucst_codec_specific_capabilities_tbl[codec_index].frame_duration) &&
                (octets_per_codec_frame == g_lea_ucst_codec_specific_capabilities_tbl[codec_index].octets_per_codec_frame)) {
                // Find bitrate is highest and bitrate < = 96kbps
                //octets_per_codec_frame = g_lea_ucst_codec_specific_capabilities_tbl[record_index].octets_per_codec_frame;
                record_index_max = codec_index;
            }
        }

        if (record_index_max >= sizeof(g_lea_ucst_codec_specific_capabilities_tbl)/sizeof(app_le_audio_ucst_codec_specific_capabilities_t)) {
            return NULL;
        }
        //Check codec is supported in PAC
        for (record_index = 0; record_index < p_pac->num_of_record; record_index++) {
            if ((p_pac->pac_record[record_index].supported_sampling_frequencies & (0x0001 << (sampling_freq - 1))) &&
                (p_pac->pac_record[record_index].supported_frame_durations & (0x01 << frame_duration)) &&
                (p_pac->pac_record[record_index].supported_octets_per_codec_frame_min <= octets_per_codec_frame) &&
                (p_pac->pac_record[record_index].supported_octets_per_codec_frame_max >= octets_per_codec_frame)) {

                if (p_pac->pac_record[record_index].supported_audio_channel_counts & 0x02) {
                    audio_channel_num = 0x02;
                }
                if (AUDIO_DIRECTION_SINK == direction) {
                    p_info->sink_audio_channel_num = audio_channel_num;
                }
                else {
                    p_info->source_audio_channel_num = audio_channel_num;
                }

                return (app_le_audio_ucst_codec_specific_capabilities_t*)&g_lea_ucst_codec_specific_capabilities_tbl[record_index_max];
            }
        }
        return NULL;
    }
    // find preferred codec by preferred_audio_contexts
    preferred_record_index = p_pac->num_of_record;
    for (record_index = 0; record_index < p_pac->num_of_record; record_index++) {
        preferred_audio_contexts = p_pac->pac_record[record_index].preferred_audio_contexts;
        preferred_audio_contexts_cnt = 0;
        if (context_type & preferred_audio_contexts) {
            while (preferred_audio_contexts) {
                preferred_audio_contexts = preferred_audio_contexts >> 1;
                preferred_audio_contexts_cnt++;
                if (preferred_audio_contexts_cnt > 1) {
                    break;
                }
            }
            if (preferred_audio_contexts_cnt == 1) {
                preferred_record_index = record_index;
                break;
            }
        }
    }

    if (preferred_record_index >= p_pac->num_of_record) {// Not Found preferred
        if((context_type & (AUDIO_CONTENT_TYPE_CONVERSATIONAL | AUDIO_CONTENT_TYPE_GAME)) &&
            (p_info->tmap_role & TMAP_ROLE_CG)) {
            tmap_role = TMAP_ROLE_CT_CG;
        }
        else if((context_type & (AUDIO_CONTENT_TYPE_CONVERSATIONAL | AUDIO_CONTENT_TYPE_GAME)) &&
            (p_info->tmap_role & TMAP_ROLE_CT)) {
            tmap_role = TMAP_ROLE_CT;
        }
        else if((context_type & AUDIO_CONTENT_TYPE_MEDIA) &&
            (p_info->tmap_role & TMAP_ROLE_UMS)) {
            tmap_role = TMAP_ROLE_UMS;
        }
        else if((context_type & AUDIO_CONTENT_TYPE_MEDIA) &&
            (p_info->tmap_role & TMAP_ROLE_UMR)) {
            tmap_role = TMAP_ROLE_UMR;
        }

        if (tmap_role) {
            for (codec_index = sizeof(g_lea_ucst_codec_specific_capabilities_tbl)/sizeof(app_le_audio_ucst_codec_specific_capabilities_t); codec_index > 0; codec_index--) {
                //Find codec_index by tmap_role and bitrate < = 96kbps
                if ((tmap_role & g_lea_ucst_codec_specific_capabilities_tbl[codec_index -1].support_tmap_role) &&
                    (97 > g_lea_ucst_codec_specific_capabilities_tbl[codec_index - 1].bitrate)) {
                    sampling_freq = g_lea_ucst_codec_specific_capabilities_tbl[codec_index - 1].sampling_freq;
                    frame_duration = g_lea_ucst_codec_specific_capabilities_tbl[codec_index - 1].frame_duration;
                    octets_per_codec_frame = g_lea_ucst_codec_specific_capabilities_tbl[codec_index - 1].octets_per_codec_frame;
                    //Check codec is supported in PAC
                    for (record_index = 0; record_index < p_pac->num_of_record; record_index++) {
                        if ((p_pac->pac_record[record_index].supported_sampling_frequencies & (0x0001 << (sampling_freq - 1))) &&
                            (p_pac->pac_record[record_index].supported_frame_durations & (0x01 << frame_duration)) &&
                            (p_pac->pac_record[record_index].supported_octets_per_codec_frame_min <= octets_per_codec_frame) &&
                            (p_pac->pac_record[record_index].supported_octets_per_codec_frame_max >= octets_per_codec_frame)) {

                            if (p_pac->pac_record[record_index].supported_audio_channel_counts & 0x02) {
                                audio_channel_num = 0x02;
                            }
                            if (AUDIO_DIRECTION_SINK == direction) {
                                p_info->sink_audio_channel_num = audio_channel_num;
                            }
                            else {
                                p_info->source_audio_channel_num = audio_channel_num;
                            }

                            return (app_le_audio_ucst_codec_specific_capabilities_t*)&g_lea_ucst_codec_specific_capabilities_tbl[codec_index - 1];
                        }
                    }
                }
            }
        }

        // tmap_role is not supported, find codec by max sampling_freq
        for (record_index = 0; record_index < p_pac->num_of_record; record_index++) {
            for (sampling_freq = CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ; sampling_freq >= CODEC_CONFIGURATION_SAMPLING_FREQ_8KHZ; sampling_freq--) {
                if (p_pac->pac_record[record_index].supported_sampling_frequencies & 0x00FF & (0x0001 << (sampling_freq - 1))) {
                    if (sampling_freq >= sampling_freq_max) {
                        sampling_freq_max = sampling_freq;
                        record_index_max = record_index;
                    }
                    break;
                }
            }
        }
        record_index = record_index_max;
    }
    else {
        record_index = preferred_record_index;
    }

    // Get max sampling_freq in pac_record[record_index]
    for (sampling_freq = CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ; sampling_freq >= CODEC_CONFIGURATION_SAMPLING_FREQ_8KHZ; sampling_freq--) {
        if (p_pac->pac_record[record_index].supported_sampling_frequencies & 0x00FF & (0x0001 << (sampling_freq - 1))) {
            break;
        }
    }
    // Get frame_duration
#if 0
    for (frame_duration = FRAME_DURATION_7P5_MS; frame_duration <= FRAME_DURATION_10_MS; frame_duration++) {
        if ((p_pac->pac_record[record_index].supported_frame_durations >> 4 ) & 0x03) {// prefered:at most 1 bit
            if ((p_pac->pac_record[record_index].supported_frame_durations >> 4 ) & 0x03 & (0x01 << frame_duration)) {
                break;
            }
            else {
                continue;
            }
        }
        if (p_pac->pac_record[record_index].supported_frame_durations & 0x03 & (0x01 << frame_duration)) {// supported:at most 2 bits
            break;
        }
    }
#else
    if ((p_pac->pac_record[record_index].supported_frame_durations >> 4 ) & (0x01 << FRAME_DURATION_10_MS)) {// prefered:at most 1 bit
        frame_duration = FRAME_DURATION_10_MS;//prefered frame_durations
    }
    else if ((p_pac->pac_record[record_index].supported_frame_durations >> 4 ) & (0x01 << FRAME_DURATION_7P5_MS)) {// prefered:at most 1 bit
        frame_duration = FRAME_DURATION_7P5_MS;//prefered frame_durations
    }
    else {
        if (p_pac->pac_record[record_index].supported_frame_durations & (0x01 << FRAME_DURATION_10_MS)) {// supported:at most 2 bits
            frame_duration = FRAME_DURATION_10_MS;
        }
        else {
            frame_duration = FRAME_DURATION_7P5_MS;
        }
    }
#endif
    LE_AUDIO_MSGLOG_I("[APP] get_codec_configuration sampling_freq:%02x, frame_duration:%02x ", 2, sampling_freq, frame_duration);
    // Get octets_per_codec_frame by sampling_freq and frame_duration
    for (codec_index = 0; codec_index < sizeof(g_lea_ucst_codec_specific_capabilities_tbl)/sizeof(app_le_audio_ucst_codec_specific_capabilities_t); codec_index++) {
        if ((sampling_freq == g_lea_ucst_codec_specific_capabilities_tbl[codec_index].sampling_freq) &&
            (frame_duration == g_lea_ucst_codec_specific_capabilities_tbl[codec_index].frame_duration) &&
            (97 > g_lea_ucst_codec_specific_capabilities_tbl[codec_index].bitrate) &&
            (p_pac->pac_record[record_index].supported_octets_per_codec_frame_min <= g_lea_ucst_codec_specific_capabilities_tbl[codec_index].octets_per_codec_frame) &&
            (p_pac->pac_record[record_index].supported_octets_per_codec_frame_max >= g_lea_ucst_codec_specific_capabilities_tbl[codec_index].octets_per_codec_frame)) {
            // Find bitrate is highest and bitrate < = 96kbps
            //octets_per_codec_frame = g_lea_ucst_codec_specific_capabilities_tbl[record_index].octets_per_codec_frame;
            record_index_max = codec_index;
        }
    }

    if (record_index_max >= sizeof(g_lea_ucst_codec_specific_capabilities_tbl)/sizeof(app_le_audio_ucst_codec_specific_capabilities_t)) {
        return NULL;
    }

    if (p_pac->pac_record[record_index].supported_audio_channel_counts & 0x02) {
        audio_channel_num = 0x02;
    }

    if (AUDIO_DIRECTION_SINK == direction) {
        p_info->sink_audio_channel_num = audio_channel_num;
    }
    else {
        p_info->source_audio_channel_num = audio_channel_num;
    }

    return (app_le_audio_ucst_codec_specific_capabilities_t*)&g_lea_ucst_codec_specific_capabilities_tbl[record_index_max];
}


bt_status_t app_le_audio_ucst_config_codec(bt_handle_t handle)
{
    ble_bap_ascs_config_codec_operation_t *buf = NULL;
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret;
    uint16_t context_type;
    uint8_t idx = 0, ase_num = 0, sink_ase_num = 0, source_ase_num = 0;
    app_le_audio_ucst_codec_specific_capabilities_t* p_audio_capability_configuration = NULL;


    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_FAIL;
    }

    /* get context type */
    context_type = app_le_audio_ucst_get_audio_context_type();

    //Check Available Audio Context
    if (!g_lea_ucst_test_mode_flag) {
        if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
            //if(!((p_info->source_available_contexts & context_type)&&(p_info->sink_available_contexts & context_type))) {
            //CALL_MODE is determined by mic port, but recorder is not call mode and should not check sink
            if((!(p_info->source_available_contexts & context_type)) && (p_info->source_supported_contexts & context_type)) {
                return BT_STATUS_FAIL;
            }
        }
        else {
            if((!(p_info->sink_available_contexts & context_type)) && (p_info->sink_supported_contexts & context_type)) {
                return BT_STATUS_FAIL;
            }
        }
    }

    /* get the num of SINK ASE to be configured */
    sink_ase_num = app_le_audio_ucst_get_sink_ase_num(p_info);
    p_info->cis_num = sink_ase_num;

#if 0 // For Airoha device
    if ((APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1 > g_lea_ucst_ctrl.create_cis_mode) &&
        (0 != p_info->sink_location_num) && (p_info->sink_ase_num >= (p_info->sink_location_num * 2))) {
        sink_ase_num = (p_info->sink_location_num * 2); /* call mode and media mode */
    }
#endif
    /* get the num of SOURCE ASE to be configured */
    source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);

    if (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_1 == g_lea_ucst_ctrl.create_cis_mode) {
        p_info->cis_num = 2;

    } else if (p_info->cis_num < source_ase_num) {
        p_info->cis_num = source_ase_num;
    }

    ase_num = (sink_ase_num + source_ase_num);

    LE_AUDIO_MSGLOG_I("[APP][ASE] config_codec, group_size:%x ase_num:(%x,%x) location_num:(%x,%x) cis_num:%x ase_num:%x", 7,
                      p_info->group_size,
                      p_info->sink_ase_num,
                      p_info->source_ase_num,
                      p_info->sink_location_num,
                      p_info->source_location_num,
                      p_info->cis_num,
                      ase_num);

    if (NULL == (buf = pvPortMalloc(APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_config_codec_param_t)))) {
        LE_AUDIO_MSGLOG_I("[APP][ASE] config_codec, malloc failed", 0);
        return BT_STATUS_FAIL;
    }

    memset(buf, 0, APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_config_codec_param_t));
    LE_AUDIO_MSGLOG_I("[APP][ASE] config_codec, ase_num(total:%x sink:%x source:%x)", 3, ase_num, sink_ase_num, source_ase_num);

    buf->opcode = ASE_OPCODE_CONFIG_CODEC;
    //buf->num_of_ase = ase_num;

    uint8_t i, location_idx;
    app_le_audio_qos_params_t *p_qos_params = NULL;

    /* SINK ASE */
    location_idx = 0;
    for (i = 0; i < sink_ase_num; i++) {
        memcpy(&buf->param[idx], &g_lea_cofig_code_param, sizeof(ble_bap_config_codec_param_t));
        if (p_info->sink_ase_num < (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) {
            buf->param[idx].ase_id = p_info->ase[i].id;
            p_info->ase[i].codec_state.audio_contexts = context_type;
            if (APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL < g_lea_ucst_ctrl.create_cis_mode) {
                if ((APP_LE_AUDIO_UCST_IS_CALL_MODE) && ((0 < source_ase_num) ||
                    (((AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location) &&
                       ((APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_2 == g_lea_ucst_ctrl.create_cis_mode) ||
                        (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_2 == g_lea_ucst_ctrl.create_cis_mode)))))) {
                    /* call mode */
                    LE_AUDIO_MSGLOG_I("[APP][ASE] config_codec, sink 2 call", 0);
                    p_qos_params = &g_lea_ucst_qos_params_spk_0;
                } else {
                    /* media mode */
                    LE_AUDIO_MSGLOG_I("[APP][ASE] config_codec, sink 1 media", 0);
                    p_qos_params = &g_lea_ucst_qos_params_spk_1;
                }

                buf->param[idx].codec_specific_configuration.sampling_freq_value = p_qos_params->sampling_freq;
                buf->param[idx].codec_specific_configuration.frame_duration_value = p_qos_params->sdu_interval;
                buf->param[idx].codec_specific_configuration.octets_per_codec_frame_value = p_qos_params->sdu_size;
            }
            else {
                if(NULL != (p_audio_capability_configuration = app_le_audio_ucst_get_codec_configuration(p_info->handle, context_type, AUDIO_DIRECTION_SINK))) {
                    buf->param[idx].codec_specific_configuration.sampling_freq_value = p_audio_capability_configuration->sampling_freq;
                    buf->param[idx].codec_specific_configuration.frame_duration_value = p_audio_capability_configuration->frame_duration;
                    buf->param[idx].codec_specific_configuration.octets_per_codec_frame_value = p_audio_capability_configuration->octets_per_codec_frame;
                }
            }
        } else {
            /* config SINK ASE for media and call */
            /* earbuds: media(ase[0]), call(ase[1]) */
            /* headset: media(ase[0], ase[1]), call(ase[2], ase[3]) */

            //if (idx < p_info->cis_num) {
            if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
                /* call mode */
                buf->param[idx].ase_id = p_info->ase[sink_ase_num + i].id;
                p_info->ase[sink_ase_num + i].codec_state.audio_contexts = context_type;
                LE_AUDIO_MSGLOG_I("[APP][ASE] config_codec, sink 4 call", 0);
                p_qos_params = &g_lea_ucst_qos_params_spk_0;
            } else {
                /* media mode */
                buf->param[idx].ase_id = p_info->ase[i].id;
                p_info->ase[i].codec_state.audio_contexts = context_type;
                LE_AUDIO_MSGLOG_I("[APP][ASE] config_codec, sink 3 media", 0);
                p_qos_params = &g_lea_ucst_qos_params_spk_1;
            }
            if (APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL < g_lea_ucst_ctrl.create_cis_mode) {
                buf->param[idx].codec_specific_configuration.sampling_freq_value = p_qos_params->sampling_freq;
                buf->param[idx].codec_specific_configuration.frame_duration_value = p_qos_params->sdu_interval;
                buf->param[idx].codec_specific_configuration.octets_per_codec_frame_value = p_qos_params->sdu_size;
            }
            else {
                if(NULL != (p_audio_capability_configuration = app_le_audio_ucst_get_codec_configuration(p_info->handle, context_type, AUDIO_DIRECTION_SINK))) {
                    buf->param[idx].codec_specific_configuration.sampling_freq_value = p_audio_capability_configuration->sampling_freq;
                    buf->param[idx].codec_specific_configuration.frame_duration_value = p_audio_capability_configuration->frame_duration;
                    buf->param[idx].codec_specific_configuration.octets_per_codec_frame_value = p_audio_capability_configuration->octets_per_codec_frame;
                }
            }
            if (idx == p_info->cis_num) {
                location_idx = 0;
            }
        }
        if (0 == p_info->sink_location_num) {
            buf->param[idx].codec_specific_configuration.audio_channel_alloaction_value = AUDIO_LOCATION_NONE;
        } else if ((1 == p_info->sink_location_num) || (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4 == g_lea_ucst_ctrl.create_cis_mode)) {
            buf->param[idx].codec_specific_configuration.audio_channel_alloaction_value = p_info->sink_location;
        } else if (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_1 == g_lea_ucst_ctrl.create_cis_mode) {
            buf->param[idx].codec_specific_configuration.audio_channel_alloaction_value = AUDIO_LOCATION_FRONT_RIGHT;
        } else {
            buf->param[idx].codec_specific_configuration.audio_channel_alloaction_value = app_le_audio_ucst_get_location(location_idx, p_info->sink_location);
            location_idx++;
        }

        if (g_lea_ucst_qos_params_selected) {
            buf->param[idx].target_latency = g_lea_ucst_qos_params_selected;
        }

        LE_AUDIO_MSGLOG_I("[APP][ASE] config_codec, parameter sink_ase[%x]:%x sampling_freq:%02x frame_duration:%02x octets_per_codec_frame:%04x location:%08x ", 6,
                          idx,
                          buf->param[idx].ase_id,
                          buf->param[idx].codec_specific_configuration.sampling_freq_value,
                          buf->param[idx].codec_specific_configuration.frame_duration_value,
                          buf->param[idx].codec_specific_configuration.octets_per_codec_frame_value,
                          buf->param[idx].codec_specific_configuration.audio_channel_alloaction_value);
        idx++;
    }

    /* SOURCE ASE */
    if ((APP_LE_AUDIO_UCST_IS_CALL_MODE) || (0 == sink_ase_num)) {
        if ((0 == sink_ase_num) && (!(context_type &(AUDIO_CONTENT_TYPE_CONVERSATIONAL | AUDIO_CONTENT_TYPE_GAME | AUDIO_CONTENT_TYPE_VA)))) {
            context_type = AUDIO_CONTENT_TYPE_CONVERSATIONAL;
        }

        location_idx = 0;
        for (i = 0; i < source_ase_num; i++) {
            memcpy(&buf->param[idx], &g_lea_cofig_code_param, sizeof(ble_bap_config_codec_param_t));
            buf->param[idx].ase_id = p_info->ase[p_info->source_ase_idx + i].id;
            p_info->ase[p_info->source_ase_idx + i].codec_state.audio_contexts = context_type;
            if (APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL < g_lea_ucst_ctrl.create_cis_mode) {
                buf->param[idx].codec_specific_configuration.sampling_freq_value = g_lea_ucst_qos_params_mic_0.sampling_freq;
                buf->param[idx].codec_specific_configuration.frame_duration_value = g_lea_ucst_qos_params_mic_0.sdu_interval;
                buf->param[idx].codec_specific_configuration.octets_per_codec_frame_value = g_lea_ucst_qos_params_mic_0.sdu_size;
            }
            else {
                if(NULL != (p_audio_capability_configuration = app_le_audio_ucst_get_codec_configuration(p_info->handle, context_type, AUDIO_DIRECTION_SOURCE))) {
                    buf->param[idx].codec_specific_configuration.sampling_freq_value = p_audio_capability_configuration->sampling_freq;
                    buf->param[idx].codec_specific_configuration.frame_duration_value = p_audio_capability_configuration->frame_duration;
                    buf->param[idx].codec_specific_configuration.octets_per_codec_frame_value = p_audio_capability_configuration->octets_per_codec_frame;
                }
            }
            if (0 == p_info->source_location_num) {
                buf->param[idx].codec_specific_configuration.audio_channel_alloaction_value = AUDIO_LOCATION_NONE;
            } else if (1 == p_info->source_location_num) {
                buf->param[idx].codec_specific_configuration.audio_channel_alloaction_value = p_info->source_location;
            } else {
                buf->param[idx].codec_specific_configuration.audio_channel_alloaction_value = app_le_audio_ucst_get_location(location_idx, p_info->source_location);
                location_idx++;
            }
            if (g_lea_ucst_qos_params_selected) {
                buf->param[idx].target_latency = g_lea_ucst_qos_params_selected;
            }

            LE_AUDIO_MSGLOG_I("[APP][ASE] config_codec, parameter source_ase[%x]:%x sampling_freq:%02x frame_duration:%02x octets_per_codec_frame:%04x location:%08x ", 6,
                              idx,
                              buf->param[idx].ase_id,
                              buf->param[idx].codec_specific_configuration.sampling_freq_value,
                              buf->param[idx].codec_specific_configuration.frame_duration_value,
                              buf->param[idx].codec_specific_configuration.octets_per_codec_frame_value,
                              buf->param[idx].codec_specific_configuration.audio_channel_alloaction_value);

            idx++;
        }
    }

    buf->num_of_ase = idx;
    p_info->wait_event.wait_ase_event = buf->num_of_ase;
    p_info->wait_event.wait_ase_cp_event = 0;//Some devices will violate Spec: Nofify ASE status first, then Nofify Control Point status; so the Control Point return result is no longer checked.

    ret = ble_bap_ascs_config_codec(handle, buf);

    LE_AUDIO_MSGLOG_I("[APP][ASE] config_codec, handle:%x ret:%x w_ase:%x", 3, handle, ret, p_info->wait_event.wait_ase_event);

    vPortFree(buf);

#if 0
    memcpy(&g_lea_ucst_ctrl.qos_params_spk_0, &g_lea_ucst_qos_params_spk_0, sizeof(app_le_audio_qos_params_t));
    memcpy(&g_lea_ucst_ctrl.qos_params_spk_1, &g_lea_ucst_qos_params_spk_1, sizeof(app_le_audio_qos_params_t));
    memcpy(&g_lea_ucst_ctrl.qos_params_mic_0, &g_lea_ucst_qos_params_mic_0, sizeof(app_le_audio_qos_params_t));
#endif

    return ret;
}


static bt_status_t app_le_audio_ucst_config_qos(bt_handle_t handle)
{
    ble_bap_ascs_config_qos_operation_t *buf = NULL;
    app_le_audio_ucst_link_info_t *p_info = NULL;
    app_le_audio_ase_codec_t *p_codec_state = NULL;
    bt_status_t ret;
    uint16_t audio_location_channels = 0;
    uint16_t context_type = 0;
    uint8_t ase_num, idx = 0, i, ase_idx = 0;
    uint8_t sink_ase_num = 0, source_ase_num = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        LE_AUDIO_MSGLOG_I("[APP][ASE] config_qos, link not exist (hdl:%x)", 1, handle);
        return BT_STATUS_FAIL;
    }

    /* get context type */
    context_type = app_le_audio_ucst_get_audio_context_type();

    //Check Available Audio Context
    if (!g_lea_ucst_test_mode_flag) {
        if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
            //if(!((p_info->source_available_contexts & context_type)&&(p_info->sink_available_contexts & context_type))) {
            //CALL_MODE is determined by mic port, but recorder is not call mode and should not check sink
            if((!(p_info->source_available_contexts & context_type)) && (p_info->source_supported_contexts & context_type)) {
                return BT_STATUS_FAIL;
            }
        }
        else {
            if((!(p_info->sink_available_contexts & context_type)) && (p_info->sink_supported_contexts & context_type)) {
                return BT_STATUS_FAIL;
            }
        }
    }

    /* get the num of SINK ASE to be configured */
    sink_ase_num = app_le_audio_ucst_get_sink_ase_num(p_info);
    ase_num = sink_ase_num;

    if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
        /* Call mode */

        /* get the num of SOURCE ASE to be configured */
        source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);
        ase_num += source_ase_num;

        if (NULL == (buf = pvPortMalloc(APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_config_qos_param_t)))) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] config_qos, malloc failed (call)", 0);
            return BT_STATUS_FAIL;
        }

        memset(buf, 0, APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_config_qos_param_t));

        LE_AUDIO_MSGLOG_I("[APP][ASE] config_qos, ase_num(total:%x sink:%x source:%x) (call)", 3, ase_num, sink_ase_num, source_ase_num);

        buf->opcode = ASE_OPCODE_CONFIG_QOS;
        buf->num_of_ase = ase_num;

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            buf->param[idx].cig_id = APP_LE_AUDIO_CIG_ID_2;
            //if ((sink_ase_num == p_info->sink_ase_num) || (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1 <= g_lea_ucst_ctrl.create_cis_mode)) {
            if ((p_info->sink_ase_num < (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) || (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1 <= g_lea_ucst_ctrl.create_cis_mode)) {
                ase_idx = i;
                buf->param[idx].ase_id = p_info->ase[ase_idx].id;
            } else {
                if (p_info->sink_ase_num >= (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) {
                    ase_idx = sink_ase_num + i;
                    buf->param[idx].ase_id = p_info->ase[ase_idx].id;
                } else {
                    LE_AUDIO_MSGLOG_I("[APP][ASE] config_qos, failed, sink_ase_num:%x %x (call)", 2, sink_ase_num, p_info->sink_ase_num);
                    return BT_STATUS_FAIL;
                }
            }
            if ((1 == g_lea_ucst_ctrl.cis_num) && (AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location)) {
                buf->param[idx].cis_id = APP_LE_AUDIO_CIS_ID_3_CALL;
            } else {
                if (AUDIO_LOCATION_FRONT_LEFT == p_info->sink_location) {
                    buf->param[idx].cis_id = APP_LE_AUDIO_CIS_ID_3_CALL;
                } else if ((AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location) ||
                           (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_1 == g_lea_ucst_ctrl.create_cis_mode)) {
                    buf->param[idx].cis_id = APP_LE_AUDIO_CIS_ID_4_CALL;
                } else {
                    buf->param[idx].cis_id = (APP_LE_AUDIO_CIS_ID_3_CALL + i);
                }
            }
            p_codec_state = &p_info->ase[ase_idx].codec_state;

            buf->param[idx].sdu_interval = (p_codec_state->frame_duration == FRAME_DURATION_7P5_MS) ? 7500 : 10000;
            buf->param[idx].framing = p_codec_state->framing;
            if (p_codec_state->preferred_phy & 0x01) {
                buf->param[idx].phy = 0x01;
            }
            else if (p_codec_state->preferred_phy & 0x02) {
                buf->param[idx].phy = 0x02;
            }
            else if (p_codec_state->preferred_phy & 0x04) {
                buf->param[idx].phy = 0x04;
            }
            else {
                buf->param[idx].phy = 0x02;
            }
            audio_location_channels = app_le_audio_ucst_get_location_count(p_codec_state->audio_channel_allocation);
            if (0 == audio_location_channels) {
                audio_location_channels = 1;
            }
            buf->param[idx].maximum_sdu_size = p_codec_state->codec_frame_blocks_per_sdu * audio_location_channels * p_codec_state->octets_per_codec_frame;
            buf->param[idx].retransmission_number = p_codec_state->preferred_retransmission_number;
            buf->param[idx].transport_latency = p_codec_state->max_transport_latency;
            if (0 == p_codec_state->preferred_presentation_delay_min) {
                buf->param[idx].presentation_delay = p_codec_state->presentation_delay_min;
            }
            else {
                buf->param[idx].presentation_delay = p_codec_state->preferred_presentation_delay_min;
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] config_qos, ase_id[%x]:%x sink[%x] cig_id:%x cis_id:%x", 5,
                              ase_idx,
                              buf->param[idx].ase_id,
                              i,
                              buf->param[idx].cig_id,
                              buf->param[idx].cis_id);
            idx++;
        }

        /* SORUCE ASE */
        for (i = 0; i < source_ase_num; i++) {
            ase_idx = p_info->source_ase_idx + i;
            buf->param[idx].cig_id = APP_LE_AUDIO_CIG_ID_2;
            buf->param[idx].ase_id = p_info->ase[ase_idx].id;
            if ((1 == g_lea_ucst_ctrl.cis_num) && (AUDIO_LOCATION_FRONT_RIGHT == p_info->source_location)) {
                buf->param[idx].cis_id = APP_LE_AUDIO_CIS_ID_3_CALL;
            } else {
                if (AUDIO_LOCATION_FRONT_LEFT == p_info->source_location) {
                    buf->param[idx].cis_id = APP_LE_AUDIO_CIS_ID_3_CALL;
                } else if (AUDIO_LOCATION_FRONT_RIGHT == p_info->source_location) {
                    buf->param[idx].cis_id = APP_LE_AUDIO_CIS_ID_4_CALL;
                } else {
                    buf->param[idx].cis_id = (APP_LE_AUDIO_CIS_ID_3_CALL + i);
                }
            }
            p_codec_state = &p_info->ase[ase_idx].codec_state;
            buf->param[idx].sdu_interval = (p_codec_state->frame_duration == FRAME_DURATION_7P5_MS) ? 7500 : 10000;
            buf->param[idx].framing = p_codec_state->framing;
            if (p_codec_state->preferred_phy & 0x02) {
                buf->param[idx].phy = 0x02;
            }
            else if (p_codec_state->preferred_phy & 0x04) {
                buf->param[idx].phy = 0x04;
            }
            else {
                buf->param[idx].phy = 0x01;
            }
            audio_location_channels = app_le_audio_ucst_get_location_count(p_codec_state->audio_channel_allocation);
            if (0 == audio_location_channels) {
                audio_location_channels = 1;
            }
            buf->param[idx].maximum_sdu_size = p_codec_state->codec_frame_blocks_per_sdu * audio_location_channels * p_codec_state->octets_per_codec_frame;
            buf->param[idx].retransmission_number = p_codec_state->preferred_retransmission_number;
            buf->param[idx].transport_latency = p_codec_state->max_transport_latency;
            if (0 == p_codec_state->preferred_presentation_delay_min) {
                buf->param[idx].presentation_delay = p_codec_state->presentation_delay_min;
            }
            else {
                buf->param[idx].presentation_delay = p_codec_state->preferred_presentation_delay_min;
            }

            LE_AUDIO_MSGLOG_I("[APP][ASE] config_qos, ase_id[%x]:%x source[%x] cig_id:%x cis_id:%x", 5,
                              ase_idx,
                              buf->param[idx].ase_id,
                              i,
                              buf->param[idx].cig_id,
                              buf->param[idx].cis_id);
            idx++;
        }
    } else {
        /* Media mode */
        if (0 == sink_ase_num) {
            return BT_STATUS_FAIL;
        }
        if (NULL == (buf = pvPortMalloc(APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_config_qos_param_t)))) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] config_qos, malloc failed", 0);
            return BT_STATUS_FAIL;
        }

        memset(buf, 0, APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_config_qos_param_t));

        LE_AUDIO_MSGLOG_I("[APP][ASE] config_qos, ase_num(total:%x sink:%x)", 2, ase_num, sink_ase_num);

        buf->opcode = ASE_OPCODE_CONFIG_QOS;
        buf->num_of_ase = ase_num;

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            ase_idx = i;
            buf->param[idx].cig_id = APP_LE_AUDIO_CIG_ID_1;
            buf->param[idx].ase_id = p_info->ase[ase_idx].id;
            if ((1 == g_lea_ucst_ctrl.cis_num) && (AUDIO_LOCATION_FRONT_LEFT == p_info->sink_location)) {
                buf->param[idx].cis_id = APP_LE_AUDIO_CIS_ID_1_MEDIA;
            } else {
                if (AUDIO_LOCATION_FRONT_LEFT == p_info->sink_location) {
                    buf->param[idx].cis_id = APP_LE_AUDIO_CIS_ID_1_MEDIA;
                } else if (AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location) {
                    buf->param[idx].cis_id = APP_LE_AUDIO_CIS_ID_2_MEDIA;
                } else {
                    buf->param[idx].cis_id = (APP_LE_AUDIO_CIS_ID_1_MEDIA + i);
                }
            }
            p_codec_state = &p_info->ase[ase_idx].codec_state;
            /*LE_AUDIO_MSGLOG_I("[APP][ASCS] handle:%x 111ASE[%d],sampling_frequency:%02x frame_duration:%02x audio_channel_allocation:%08x octets_per_codec_frame:%04x codec_frame_blocks_per_sdu:%02x", 7,
                p_info->handle,
                ase_idx,
                p_info->ase[ase_idx].codec_state.sampling_frequency,
                p_info->ase[ase_idx].codec_state.frame_duration,
                p_info->ase[ase_idx].codec_state.audio_channel_allocation,
                p_info->ase[ase_idx].codec_state.octets_per_codec_frame,
                p_info->ase[ase_idx].codec_state.codec_frame_blocks_per_sdu);
            */
            buf->param[idx].sdu_interval = (p_codec_state->frame_duration == FRAME_DURATION_7P5_MS) ? 7500 : 10000;
            buf->param[idx].framing = p_codec_state->framing;
            if (p_codec_state->preferred_phy & 0x02) {
                buf->param[idx].phy = 0x02;
            }
            else if (p_codec_state->preferred_phy & 0x04) {
                buf->param[idx].phy = 0x04;
            }
            else {
                buf->param[idx].phy = 0x01;
            }
            audio_location_channels = app_le_audio_ucst_get_location_count(p_codec_state->audio_channel_allocation);
            if (0 == audio_location_channels) {
                audio_location_channels = 1;
            }
            if (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4 == g_lea_ucst_ctrl.create_cis_mode) {
                audio_location_channels = 2;
            }
            buf->param[idx].maximum_sdu_size = p_codec_state->codec_frame_blocks_per_sdu * audio_location_channels * p_codec_state->octets_per_codec_frame;
            buf->param[idx].retransmission_number = p_codec_state->preferred_retransmission_number;
            buf->param[idx].transport_latency = p_codec_state->max_transport_latency;
            if (0 == p_codec_state->preferred_presentation_delay_min) {
                buf->param[idx].presentation_delay = p_codec_state->presentation_delay_min;
            }
            else {
                buf->param[idx].presentation_delay = p_codec_state->preferred_presentation_delay_min;
            }

            LE_AUDIO_MSGLOG_I("[APP][ASE] config_qos, ase_id[%x]:%x sink[%x] cig_id:%x cis_id:%x", 5,
                              ase_idx,
                              buf->param[idx].ase_id,
                              i,
                              buf->param[idx].cig_id,
                              buf->param[idx].cis_id);
            idx++;
        }
    }

    p_info->wait_event.wait_ase_event = buf->num_of_ase;
    p_info->wait_event.wait_ase_cp_event = 0;

    ret = ble_bap_ascs_config_qos(handle, buf);

    LE_AUDIO_MSGLOG_I("[APP][ASE] config_qos, handle:%x ret:%x w_ase:%x", 3, handle, ret, p_info->wait_event.wait_ase_event);

    vPortFree(buf);

    return ret;
}


static bt_status_t app_le_audio_ucst_enable_ase(bt_handle_t handle)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t *buf = NULL;
    bt_status_t ret;
    uint16_t context_type, metadata_len, idx = 0;
    uint8_t ase_num, i;
    uint8_t sink_ase_num = 0;
    uint16_t total_len = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_FAIL;
    }

    /* get context type */
    context_type = app_le_audio_ucst_get_audio_context_type();

    //Check Available Audio Context
    if (!g_lea_ucst_test_mode_flag) {
        if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
            //if(!((p_info->source_available_contexts & context_type)&&(p_info->sink_available_contexts & context_type))) {
            //CALL_MODE is determined by mic port, but recorder is not call mode and should not check sink
            if((!(p_info->source_available_contexts & context_type)) && (p_info->source_supported_contexts & context_type)) {
                return BT_STATUS_FAIL;
            }
        }
        else {
            if((!(p_info->sink_available_contexts & context_type)) && (p_info->sink_supported_contexts & context_type)) {
                return BT_STATUS_FAIL;
            }
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][ASE] sampling_freq(to_air:0x%x from_air:0x%x) context_type:0x%x", 3, g_lea_ucst_qos_params_spk_0.sampling_freq, g_lea_ucst_qos_params_mic_0.sampling_freq, context_type);

    /* get metadata len for all ASEs */
    metadata_len = app_le_audio_ucst_get_metadata_len();

    /* get the num of SINK ASE to be configured */
    sink_ase_num = app_le_audio_ucst_get_sink_ase_num(p_info);
    ase_num = sink_ase_num;

    if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
        uint8_t source_ase_num = 0;

        /* Call mode */

        /* get the num of SOURCE ASE to be configured */
        source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);
        ase_num += source_ase_num;

        /* total_len: | opcode(1B) | ase_num(1B) | ASE_0: ase_id(1B)+metadata_len(1B)+Metadata(Varies) | ASE_1....| */
        /* To do: when earch ASEs has different metadata */

        total_len = APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * (APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN + metadata_len);
        LE_AUDIO_MSGLOG_I("[APP][ASE] enable, len metadata:%x ase_len:%x total:%x", 3, metadata_len, (APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN + metadata_len), total_len);

        if (NULL == (buf = pvPortMalloc(total_len))) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] enable, malloc failed (call)", 0);
            return BT_STATUS_FAIL;
        }

        memset(buf, 0, total_len);
        LE_AUDIO_MSGLOG_I("[APP][ASE] enable, ase_num(total:%x sink:%x source:%x) (call)", 3, ase_num, sink_ase_num, source_ase_num);

        /* Opcode: Enable(0x03) */
        buf[0] = ASE_OPCODE_ENABLE;
        /* Number_of_ASEs */
        buf[1] = ase_num;
        idx = APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN;

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            /* ASE_ID[i] */
            if ((sink_ase_num == p_info->sink_ase_num) || (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1 <= g_lea_ucst_ctrl.create_cis_mode)) {
                buf[idx] = p_info->ase[i].id;
            } else {
                if (p_info->sink_ase_num >= (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) {
                    buf[idx] = p_info->ase[sink_ase_num + i].id;
                } else {
                    LE_AUDIO_MSGLOG_I("[APP][ASE] enable, failed, sink_ase_num:%x %x (call)", 2, sink_ase_num, p_info->sink_ase_num);
                    return BT_STATUS_FAIL;
                }
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] enable, ase_id:%x sink[%x]", 2, buf[idx], i);

            /* Metadata_Length[i] */
            buf[idx + 1] = metadata_len;
            idx += APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN;

            /* Metadata[i] */
            if (g_lea_ucst_test_mode_flag) {
                context_type = p_info->sink_available_contexts;
            }
            idx += app_le_audio_ucst_set_metadata(&buf[idx], context_type);

        }

        /* SORUCE ASE */
        for (i = 0; i < source_ase_num; i++) {
            /* ASE_ID[i] */
            buf[idx] = p_info->ase[p_info->source_ase_idx + i].id;
            LE_AUDIO_MSGLOG_I("[APP][ASE] enable, ase_id:%x source[%x]", 2, buf[idx], i);

            /* Metadata_Length[i] */
            buf[idx + 1] = metadata_len;
            idx += APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN;

            /* Metadata[i] */
            if (g_lea_ucst_test_mode_flag) {
                context_type = p_info->source_available_contexts;
            }
            idx += app_le_audio_ucst_set_metadata(&buf[idx], context_type);

        }
    } else {

        /* Media mode */
        if (0 == sink_ase_num) {
            return BT_STATUS_FAIL;
        }

        /* total_len: | opcode(1B) | ase_num(1B) | ASE_0: ase_id(1B)+metadata_len(1B)+Metadata(Varies) | ASE_1....| */
        /* To do: when earch ASEs has different metadata */

        total_len = APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * (APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN + metadata_len);
        LE_AUDIO_MSGLOG_I("[APP][ASE] enable, len metadata:%x ase_len:%x total:%x", 3, metadata_len, (APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN + metadata_len), total_len);

        if (NULL == (buf = pvPortMalloc(APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * (APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN + metadata_len)))) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] enable, malloc failed", 0);
            return BT_STATUS_FAIL;
        }

        memset(buf, 0, (APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * (APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN + metadata_len)));
        LE_AUDIO_MSGLOG_I("[APP][ASE] enable, ase_num(total:%x sink:%x)", 2, ase_num, sink_ase_num);

        /* Opcode: Enable(0x03) */
        buf[0] = ASE_OPCODE_ENABLE;
        /* Number_of_ASEs */
        buf[1] = ase_num;
        idx = APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN;

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            /* ASE_ID[i] */
            buf[idx] = p_info->ase[i].id;
            LE_AUDIO_MSGLOG_I("[APP][ASE] enable, ase_id:%x sink[%x]", 2, buf[idx], i);

            /* Metadata_Length[i] */
            buf[idx + 1] = metadata_len;
            idx += APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN;

            /* Metadata[i] */
            if (g_lea_ucst_test_mode_flag) {
                context_type = p_info->sink_available_contexts;
            }
            idx += app_le_audio_ucst_set_metadata(&buf[idx], context_type);

        }
    }

    p_info->wait_event.wait_ase_event = ase_num;
    p_info->wait_event.wait_ase_cp_event = 0;

    app_le_audio_ucst_send_prepare_vcmd(handle, APP_LE_AUDIO_PREPARE_VCMD_MODE_CONN, 1);
    ret = ble_bap_ascs_enable_ase_ex(handle, buf, total_len);
#ifndef AIR_VOLUME_CONTROL_BY_DONGLE
{// Sync volume for multi device, because volume only has been sent to active device
    uint8_t volume, mute;
    bt_status_t result = BT_STATUS_FAIL;
    app_le_audio_stream_port_mask_t streaming_port = app_le_audio_get_streaming_port();
    if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_SPK_0) {
        result = app_le_audio_usb_get_volume(APP_LE_AUDIO_USB_PORT_SPK_0, &volume, &mute);
    }
    else if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_SPK_1) {
        result = app_le_audio_usb_get_volume(APP_LE_AUDIO_USB_PORT_SPK_1, &volume, &mute);
    }
    /*
    else if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_LINE_IN) {
        result = app_le_audio_line_in_get_volume(&volume, &mute);
    }
    else if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_I2S_IN) {
        result = app_le_audio_i2s_in_get_volume(&volume, &mute);
    }
    */
    if (BT_STATUS_SUCCESS == result) {
        app_le_audio_vcp_set_volume_state(handle, volume, mute);
    }
}
#else
    app_le_audio_vcp_set_volume_state(handle, BLE_VCS_VOLUME_MAX, BLE_VCS_UNMUTE);
#endif
    LE_AUDIO_MSGLOG_I("[APP][ASE] enable, handle:%x ret:%x w_ase:%x", 3, p_info->handle, ret, p_info->wait_event.wait_ase_event);

    vPortFree(buf);


    return ret;
}


bt_status_t app_le_audio_ucst_update_metadata(bt_handle_t handle)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t *buf = NULL;
    bt_status_t ret;
    uint16_t context_type, metadata_len, idx = 0;
    uint8_t ase_num, i;
    uint8_t sink_ase_num = 0;
    uint16_t total_len = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_FAIL;
    }

    /* get context type */
    context_type = app_le_audio_ucst_get_audio_context_type();

    LE_AUDIO_MSGLOG_I("[APP][ASE] sampling_freq(to_air:0x%x from_air:0x%x) context_type:0x%x", 3, g_lea_ucst_qos_params_spk_0.sampling_freq, g_lea_ucst_qos_params_mic_0.sampling_freq, context_type);

    /* get metadata len for all ASEs */
    metadata_len = app_le_audio_ucst_get_metadata_len();

    /* get the num of SINK ASE to be configured */
    sink_ase_num = app_le_audio_ucst_get_sink_ase_num(p_info);
    ase_num = sink_ase_num;

    if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
        uint8_t source_ase_num = 0;

        /* Call mode */

        /* get the num of SOURCE ASE to be configured */
        source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);
        ase_num += source_ase_num;

        /* total_len: | opcode(1B) | ase_num(1B) | ASE_0: ase_id(1B)+metadata_len(1B)+Metadata(Varies) | ASE_1....| */
        /* To do: when earch ASEs has different metadata */

        total_len = APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * (APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN + metadata_len);
        LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata, len metadata:%x ase_len:%x total:%x", 3, metadata_len, (APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN + metadata_len), total_len);

        if (NULL == (buf = pvPortMalloc(total_len))) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata, malloc failed (call)", 0);
            return BT_STATUS_FAIL;
        }

        memset(buf, 0, total_len);
        LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata, ase_num(total:%x sink:%x source:%x) (call)", 3, ase_num, sink_ase_num, source_ase_num);

        /* Opcode: Update_metadat(0x07) */
        buf[0] = ASE_OPCODE_UPDATE_METADATA;
        /* Number_of_ASEs */
        buf[1] = ase_num;
        idx = APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN;

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            /* ASE_ID[i] */
            if ((sink_ase_num == p_info->sink_ase_num) || (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1 <= g_lea_ucst_ctrl.create_cis_mode)) {
                buf[idx] = p_info->ase[i].id;
            } else {
                if (p_info->sink_ase_num >= (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) {
                    buf[idx] = p_info->ase[sink_ase_num + i].id;
                } else {
                    LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata, failed, sink_ase_num:%x %x (call)", 2, sink_ase_num, p_info->sink_ase_num);
                    return BT_STATUS_FAIL;
                }
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata, ase_id:%x sink[%x]", 2, buf[idx], i);

            /* Metadata_Length[i] */
            buf[idx + 1] = metadata_len;
            idx += APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN;

            /* Metadata[i] */
            if (g_lea_ucst_test_mode_flag) {
                context_type = p_info->sink_available_contexts;
            }
            idx += app_le_audio_ucst_set_metadata(&buf[idx], context_type);

        }

        /* SORUCE ASE */
        for (i = 0; i < source_ase_num; i++) {
            /* ASE_ID[i] */
            buf[idx] = p_info->ase[p_info->source_ase_idx + i].id;
            LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata, ase_id:%x source[%x]", 2, buf[idx], i);

            /* Metadata_Length[i] */
            buf[idx + 1] = metadata_len;
            idx += APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN;

            /* Metadata[i] */
            if (g_lea_ucst_test_mode_flag) {
                context_type = p_info->source_available_contexts;
            }
            idx += app_le_audio_ucst_set_metadata(&buf[idx], context_type);

        }
    } else {

        /* Media mode */

        /* total_len: | opcode(1B) | ase_num(1B) | ASE_0: ase_id(1B)+metadata_len(1B)+Metadata(Varies) | ASE_1....| */
        /* To do: when earch ASEs has different metadata */

        total_len = APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * (APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN + metadata_len);
        LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata, len metadata:%x ase_len:%x total:%x", 3, metadata_len, (APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN + metadata_len), total_len);

        if (NULL == (buf = pvPortMalloc(total_len))) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata call, malloc failed", 0);
            return BT_STATUS_FAIL;
        }

        memset(buf, 0, total_len);
        LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata, ase_num(total:%x sink:%x)", 2, ase_num, sink_ase_num);

        /* Opcode: Update_metadat(0x07) */
        buf[0] = ASE_OPCODE_UPDATE_METADATA;
        /* Number_of_ASEs */
        buf[1] = ase_num;
        idx = APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN;

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            /* ASE_ID[i] */
            buf[idx] = p_info->ase[i].id;
            LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata, ase_id:%x sink[%x]", 2, buf[idx], i);

            /* Metadata_Length[i] */
            buf[idx + 1] = metadata_len;
            idx += APP_LE_AUDIO_UCST_ASE_METADATA_HDR_LEN;

            /* Metadata[i] */
            if (g_lea_ucst_test_mode_flag) {
                context_type = p_info->sink_available_contexts;
            }
            idx += app_le_audio_ucst_set_metadata(&buf[idx], context_type);

        }
    }

    p_info->wait_event.wait_ase_event = ase_num;
    p_info->wait_event.wait_ase_cp_event = 0;

    ret = ble_bap_ascs_update_metadata_ex(handle, buf, total_len);

    LE_AUDIO_MSGLOG_I("[APP][ASE] update_metadata, handle:%x ret:%x w_ase:%x", 3, p_info->handle, ret, p_info->wait_event.wait_ase_event);
    vPortFree(buf);

    return ret;
}


bt_status_t app_le_audio_ucst_set_receiver_start_ready(bt_handle_t handle)
{
    ble_bap_ascs_receiver_start_ready_operation_t *buf = NULL;
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret;
    uint8_t idx = 0, i;
    uint8_t source_ase_num = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_FAIL;
    }

    /* get the num of SOURCE ASE to be configured */
    source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);

    if (0 == source_ase_num) {
        return BT_STATUS_SUCCESS;
    }

    if (NULL == (buf = pvPortMalloc(APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + source_ase_num * sizeof(ble_bap_receiver_start_ready_param_t)))) {
        LE_AUDIO_MSGLOG_I("[APP][ASE] set_receiver_start_ready, malloc failed", 0);
        return BT_STATUS_FAIL;
    }

    memset(buf, 0, APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + source_ase_num * sizeof(ble_bap_receiver_start_ready_param_t));
    LE_AUDIO_MSGLOG_I("[APP][ASE] set_receiver_start_ready, ase_num(total:%x)", 1, source_ase_num);

    buf->opcode = ASE_OPCODE_RECEIVER_START_READY;
    buf->num_of_ase = source_ase_num;

    /* SORUCE ASE */
    for (i = 0; i < source_ase_num; i++) {
        buf->param[idx].ase_id = p_info->ase[p_info->source_ase_idx + i].id;
        LE_AUDIO_MSGLOG_I("[APP][ASE] set_receiver_start_ready, ase_id[%d]:%x source[%x]", 3, idx, buf->param[idx].ase_id, i);
        idx++;
    }

    p_info->wait_event.wait_ase_event = buf->num_of_ase;
    p_info->wait_event.wait_ase_cp_event = 0;

    ret = ble_bap_ascs_receiver_start_ready(handle, buf);

    LE_AUDIO_MSGLOG_I("[APP][ASE] set_receiver_start_ready, handle:%x ret:%x w_ase:%x", 3, p_info->handle, ret, p_info->wait_event.wait_ase_event);

    vPortFree(buf);
#ifndef AIR_VOLUME_CONTROL_BY_DONGLE
{// Sync volume for multi device, because volume only has been sent to active device
    uint8_t volume, mute;
    bt_status_t result = BT_STATUS_FAIL;
    app_le_audio_stream_port_mask_t streaming_port = app_le_audio_get_streaming_port();
    if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_MIC_0) {
        result = app_le_audio_usb_get_volume(APP_LE_AUDIO_USB_PORT_MIC_0, &volume, &mute);
    }
    if (BT_STATUS_SUCCESS == result) {
        app_le_audio_micp_set_mute_state(handle, (bool)mute);
    }
}
#else
    app_le_audio_micp_set_mute_state(handle, false);
#endif

    app_le_audio_ucst_set_mic_channel();

    return ret;
}


bt_status_t app_le_audio_ucst_disable_ase(bt_handle_t handle)
{
    ble_bap_ascs_disable_operation_t *buf = NULL;
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret;
    uint8_t ase_num, idx = 0, i;
    uint8_t sink_ase_num = 0, source_ase_num = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_FAIL;
    }

    /* get the num of SINK ASE to be configured */
    sink_ase_num = app_le_audio_ucst_get_sink_ase_num(p_info);
    ase_num = sink_ase_num;

    if ((APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target) ||
        (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
        /* Call mode */

        /* get the num of SOURCE ASE to be configured */
        source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);
        ase_num += source_ase_num;

        if (NULL == (buf = pvPortMalloc(APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_disable_param_t)))) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] disable, malloc failed (call)", 0);
            return BT_STATUS_FAIL;
        }

        memset(buf, 0, APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_disable_param_t));
        LE_AUDIO_MSGLOG_I("[APP][ASE] disable, ase_num(total:%x sink:%x source:%x) (call)", 3, ase_num, sink_ase_num, source_ase_num);

        buf->opcode = ASE_OPCODE_DISABLE;
        buf->num_of_ase = ase_num;

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            if ((sink_ase_num == p_info->sink_ase_num) || (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1 <= g_lea_ucst_ctrl.create_cis_mode)) {
                buf->param[idx].ase_id = p_info->ase[i].id;
            } else {
                if (p_info->sink_ase_num >= (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) {
                    buf->param[idx].ase_id = p_info->ase[sink_ase_num + i].id;
                } else {
                    LE_AUDIO_MSGLOG_I("[APP][ASE] disable, failed, sink_ase_num:%x %x (call)", 2, sink_ase_num, p_info->sink_ase_num);
                    return BT_STATUS_FAIL;
                }
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] disable, ase_id[%d]:%x sink[%x]", 3, idx, buf->param[idx].ase_id, i);
            idx++;
        }

        /* SORUCE ASE */
        for (i = 0; i < source_ase_num; i++) {
            buf->param[idx].ase_id = p_info->ase[p_info->source_ase_idx + i].id;
            LE_AUDIO_MSGLOG_I("[APP][ASE] disable, ase_id[%d]:%x source[%x]", 3, idx, buf->param[idx].ase_id, i);
            idx++;
        }
    } else {
        /* Media mode */

        if (NULL == (buf = pvPortMalloc(APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_disable_param_t)))) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] disable, malloc failed", 0);
            return BT_STATUS_FAIL;
        }

        memset(buf, 0, APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_disable_param_t));
        LE_AUDIO_MSGLOG_I("[APP][ASE] disable, ase_num(total:%x sink:%x)", 2, ase_num, sink_ase_num);

        buf->opcode = ASE_OPCODE_DISABLE;
        buf->num_of_ase = ase_num;

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            buf->param[idx].ase_id = p_info->ase[i].id;
            LE_AUDIO_MSGLOG_I("[APP][ASE] disable, ase_id[%d]:%x sink[%x]", 3, idx, buf->param[idx].ase_id, i);
            idx++;
        }
    }

    p_info->wait_event.wait_ase_event = buf->num_of_ase;
    p_info->wait_event.wait_ase_cp_event = 0;

    app_le_audio_ucst_send_prepare_vcmd(handle, APP_LE_AUDIO_PREPARE_VCMD_MODE_DISCONN, 1);
    ret = ble_bap_ascs_disable_ase(handle, buf);
    LE_AUDIO_MSGLOG_I("[APP][ASE] disable, handle:%x ret:%x w_ase:%x", 3, p_info->handle, ret, p_info->wait_event.wait_ase_event);
    vPortFree(buf);

    return ret;
}


bt_status_t app_le_audio_ucst_release_ase(bt_handle_t handle)
{
    ble_bap_ascs_release_operation_t *buf = NULL;
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret;
    uint8_t ase_num, idx = 0, i;
    uint8_t sink_ase_num = 0, source_ase_num = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_FAIL;
    }

    /* get the num of SINK ASE to be configured */
    sink_ase_num = app_le_audio_ucst_get_sink_ase_num(p_info);
    ase_num = sink_ase_num;

    if ((APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target) ||
        (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
        /* Call mode */

        /* get the num of SOURCE ASE to be configured */
        source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);
        ase_num += source_ase_num;

        if (NULL == (buf = pvPortMalloc(APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_disable_param_t)))) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] release, malloc failed (call)", 0);
            return BT_STATUS_FAIL;
        }

        memset(buf, 0, APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_disable_param_t));
        LE_AUDIO_MSGLOG_I("[APP][ASE] release, ase_num(total:%x sink:%x source:%x) (call)", 3, ase_num, sink_ase_num, source_ase_num);

        buf->opcode = ASE_OPCODE_RELEASE;
        buf->num_of_ase = ase_num;

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            if ((sink_ase_num == p_info->sink_ase_num) || (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1 <= g_lea_ucst_ctrl.create_cis_mode)) {
                buf->param[idx].ase_id = p_info->ase[i].id;
            } else {
                if (p_info->sink_ase_num >= (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) {
                    buf->param[idx].ase_id = p_info->ase[sink_ase_num + i].id;
                } else {
                    LE_AUDIO_MSGLOG_I("[APP][ASE] release, failed, sink_ase_num:%x %x (call)", 2, sink_ase_num, p_info->sink_ase_num);
                    return BT_STATUS_FAIL;
                }
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] release, ase_id[%d]:%x sink[%x]", 3, idx, buf->param[idx].ase_id, i);
            idx++;
        }

        /* SORUCE ASE */
        for (i = 0; i < source_ase_num; i++) {
            buf->param[idx].ase_id = p_info->ase[p_info->source_ase_idx + i].id;
            LE_AUDIO_MSGLOG_I("[APP][ASE] release, ase_id[%d]:%x source[%x]", 3, idx, buf->param[idx].ase_id, i);
            idx++;
        }
    } else {
        /* Media mode */

        if (NULL == (buf = pvPortMalloc(APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_disable_param_t)))) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] release, malloc failed", 0);
            return BT_STATUS_FAIL;
        }

        memset(buf, 0, APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + ase_num * sizeof(ble_bap_disable_param_t));
        LE_AUDIO_MSGLOG_I("[APP][ASE] release, ase_num(total:%x sink:%x)", 2, ase_num, sink_ase_num);

        buf->opcode = ASE_OPCODE_RELEASE;
        buf->num_of_ase = ase_num;

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            buf->param[idx].ase_id = p_info->ase[i].id;
            LE_AUDIO_MSGLOG_I("[APP][ASE] release, ase_id[%d]:%x sink[%x]", 3, idx, buf->param[idx].ase_id, i);
            idx++;
        }
    }

    p_info->wait_event.wait_ase_event = buf->num_of_ase;
    p_info->wait_event.wait_ase_cp_event = 0;

    ret = ble_bap_ascs_release_ase(handle, buf);
    LE_AUDIO_MSGLOG_I("[APP][ASE] release, handle:%x ret:%x w_ase:%x", 3, p_info->handle, ret, p_info->wait_event.wait_ase_event);
    vPortFree(buf);

    return ret;
}


bt_status_t app_le_audio_ucst_set_receiver_stop_ready(bt_handle_t handle)
{
    ble_bap_ascs_receiver_stop_ready_operation_t *buf = NULL;
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret;
    uint8_t idx = 0, i;
    uint8_t source_ase_num = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_FAIL;
    }

    /* get the num of SOURCE ASE to be configured */
    source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);

    if (NULL == (buf = pvPortMalloc(APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + source_ase_num * sizeof(ble_bap_receiver_stop_ready_param_t)))) {
        LE_AUDIO_MSGLOG_I("[APP][ASE] set_receiver_stop_ready, malloc failed", 0);
        return BT_STATUS_FAIL;
    }

    memset(buf, 0, APP_LE_AUDIO_UCST_ASE_CTRL_POINT_HDR_LEN + sizeof(ble_bap_receiver_stop_ready_param_t));
    LE_AUDIO_MSGLOG_I("[APP][ASE] set_receiver_stop_ready, ase_num(total:%x)", 1, source_ase_num);

    buf->opcode = ASE_OPCODE_RECEIVER_STOP_READY;
    buf->num_of_ase = source_ase_num;

    /* SORUCE ASE */
    for (i = 0; i < source_ase_num; i++) {
        buf->param[idx].ase_id = p_info->ase[p_info->source_ase_idx + i].id;
        LE_AUDIO_MSGLOG_I("[APP][ASE] set_receiver_stop_ready, ase_id[%d]:%x source[%x]", 3, idx, buf->param[idx].ase_id, i);
        idx++;
    }

    p_info->wait_event.wait_ase_event = buf->num_of_ase;
    p_info->wait_event.wait_ase_cp_event = 0;

    ret = ble_bap_ascs_receiver_stop_ready(handle, buf);

    LE_AUDIO_MSGLOG_I("[APP][ASE] set_receiver_stop_ready, handle:%x ret:%x w_ase:%x", 3, p_info->handle, ret, p_info->wait_event.wait_ase_event);

    vPortFree(buf);

    return ret;
}

static bool app_le_audio_ucst_need_cis_m_to_s(uint8_t i, uint8_t sink_ase_num)
{
    /* 0: cid_3, 1:cid_4*/
    switch (g_lea_ucst_ctrl.create_cis_mode) {
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_2: {
            if (0 == (i % 2)) {
                return false;
            }
            return true;
        }
        default: {
            if (0 != sink_ase_num) {
                return true;
            }
        }
    }
    return false;
}

static bool app_le_audio_ucst_need_cis_s_to_m(uint8_t i, uint8_t source_ase_num, uint8_t group_size)
{
    /* 0: cid_3, 1:cid_4*/
    switch (g_lea_ucst_ctrl.create_cis_mode) {
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_2:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_2: {
            if (0 == (i % 2)) {
                return true;
            }
            return false;
        }
        default: {
            if (0 != source_ase_num) {
                if ((1 == group_size) && (1 == source_ase_num) && (0 != (i % 2))) {
                    break;
                }
                return true;
            }
        }
    }
    return false;
}

static bt_status_t app_le_audio_ucst_test_mode_set_cig_param(void)
{
    bt_gap_le_cis_params_t *p_cis = NULL;
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret;
    uint8_t cis_num = 0, group_size = 0;
    uint8_t i, j;
    uint8_t sink_ase_num = 0, source_ase_num = 0;

    if (0 == app_le_audio_ucst_get_link_num_ex()) {
        LE_AUDIO_MSGLOG_I("[APP][U] test_mode_set_cig_param, no connection!", 0);
        return BT_STATUS_FAIL;
    }

    for (j = 0; j < APP_LE_AUDIO_UCST_LINK_MAX_NUM; j++) {
        p_info = &g_lea_ucst_link_info[j];
        if ((BT_HANDLE_INVALID != p_info->handle) && (0 != p_info->cis_num)) {
            group_size = p_info->group_size;
            cis_num += p_info->cis_num;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP] test_mode_set_cig_param, group_size:%x cis_num:%x", 2,
                      group_size, cis_num);

    /* Use set CIG parameters command */
    if (NULL == (p_cis = (bt_gap_le_cis_params_t *)pvPortMalloc(sizeof(bt_gap_le_cis_params_t) * cis_num))) {
        LE_AUDIO_MSGLOG_I("[APP] test_mode_set_cig_param, malloc failed!", 0);
        return BT_STATUS_OUT_OF_MEMORY;
    }

    memset(p_cis, 0, (sizeof(bt_gap_le_cis_params_t)*cis_num));

    for (i = 0; i < cis_num; i++) {
        for (j = 0; j < APP_LE_AUDIO_UCST_LINK_MAX_NUM; j++) {
            p_info = &g_lea_ucst_link_info[j];
            if (BT_HANDLE_INVALID != p_info->handle) {
                if (!p_info->cis_num) {
                    continue;
                }

                LE_AUDIO_MSGLOG_I("[APP] test_mode_set_cig_param, i:%x j:%x sink(location:%x num:%x) source(location:%x num:%x)", 6, i, j, p_info->sink_location, p_info->sink_location_num, p_info->source_location, p_info->source_location_num);

                if (0 == (i % 2)) {
                    if (((0 != p_info->sink_location_num) && (AUDIO_LOCATION_FRONT_LEFT == p_info->sink_location)) ||
                        ((0 != p_info->source_location_num) && (AUDIO_LOCATION_FRONT_LEFT == p_info->source_location))) {
                        break;
                    }
                } else {
                    if (((0 != p_info->sink_location_num) && (AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location)) ||
                        ((0 != p_info->source_location_num) && (AUDIO_LOCATION_FRONT_RIGHT == p_info->source_location))) {
                        break;
                    }
                }
            }
        }

        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == j) {
            break;
        }

        sink_ase_num = app_le_audio_ucst_get_sink_ase_num(p_info);
        source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);

        LE_AUDIO_MSGLOG_I("[APP] test_mode_set_cig_param, sink_ase_num:%x source_ase_num:%x", 2,
                          sink_ase_num, source_ase_num);

        if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
            p_cis[i].cis_id = ((0 == i) ? APP_LE_AUDIO_CIS_ID_3_CALL : APP_LE_AUDIO_CIS_ID_4_CALL);
            p_cis[i].rtn_m_to_s = g_lea_ucst_qos_params_spk_0.rtn;
            p_cis[i].rtn_s_to_m = g_lea_ucst_qos_params_mic_0.rtn;
            if (0 == sink_ase_num) {
                p_cis[i].max_sdu_m_to_s = 0x0000;
                LE_AUDIO_MSGLOG_I("[APP] test_mode_set_cig_param 1, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
            } else {
                p_cis[i].max_sdu_m_to_s = g_lea_ucst_qos_params_spk_0.sdu_size;
                LE_AUDIO_MSGLOG_I("[APP] test_mode_set_cig_param 2, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
            }

            if ((0 == source_ase_num) ||
                ((1 == group_size) && (1 == source_ase_num) && (0 != (i % 2)))) {
                p_cis[i].max_sdu_s_to_m = 0x0000;
                LE_AUDIO_MSGLOG_I("[APP] test_mode_set_cig_param 3, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
            } else {
                p_cis[i].max_sdu_s_to_m = g_lea_ucst_qos_params_mic_0.sdu_size;
                LE_AUDIO_MSGLOG_I("[APP] test_mode_set_cig_param 4, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
            }

        } else {
            p_cis[i].cis_id = ((0 == i) ? APP_LE_AUDIO_CIS_ID_1_MEDIA : APP_LE_AUDIO_CIS_ID_2_MEDIA);
            LE_AUDIO_MSGLOG_I("[APP] test_mode_set_cig_param 5, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
            p_cis[i].rtn_m_to_s = g_lea_ucst_qos_params_spk_1.rtn;
            p_cis[i].rtn_s_to_m = g_lea_ucst_qos_params_spk_1.rtn;
            if (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4 == g_lea_ucst_ctrl.create_cis_mode) {
                p_cis[i].max_sdu_m_to_s = (g_lea_ucst_qos_params_spk_1.sdu_size * 2);
            } else {
                p_cis[i].max_sdu_m_to_s = g_lea_ucst_qos_params_spk_1.sdu_size;
            }
            p_cis[i].max_sdu_s_to_m = 0x0000;
        }

        p_cis[i].phy_m_to_s = 0x02;
        p_cis[i].phy_s_to_m = 0x02;
    }

    bt_gap_le_set_cig_params_t param = {
        .cig_id = APP_LE_AUDIO_CIG_ID_1,
        .sca = 0x00,
        .packing = BT_GAP_LE_CIG_PACKING_INTERLEAVED,
        .framing = BT_GAP_LE_CIG_FRAMING_UNFRAMED,
        .cis_list = p_cis,
        .cis_count = cis_num,
    };

    if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
        param.cig_id = APP_LE_AUDIO_CIG_ID_2;
        param.sdu_interval_m_to_s = app_le_audio_ucst_get_sdu_interval(false);
        param.sdu_interval_s_to_m = app_le_audio_ucst_get_sdu_interval(true);
        param.max_transport_latency_m_to_s = g_lea_ucst_qos_params_spk_0.latency;
        param.max_transport_latency_s_to_m = g_lea_ucst_qos_params_mic_0.latency;

    } else {
        param.sdu_interval_m_to_s = app_le_audio_ucst_get_sdu_interval(false);
        param.sdu_interval_s_to_m = app_le_audio_ucst_get_sdu_interval(false);
        param.max_transport_latency_m_to_s = g_lea_ucst_qos_params_spk_1.latency;
        param.max_transport_latency_s_to_m = g_lea_ucst_qos_params_spk_1.latency;
    }

    g_lea_ucst_ctrl.cig_id = param.cig_id;

    ret = bt_gap_le_set_cig_parameters(&param);
    LE_AUDIO_MSGLOG_I("[APP] test_mode_set_cig_param, ret:%x cig_id:%x cis_num:%x", 3, ret, g_lea_ucst_ctrl.cig_id, cis_num);

    vPortFree(p_cis);

    return ret;
}


static bt_status_t app_le_audio_ucst_set_cig_parameters(void)
{
    bt_status_t ret;
    uint8_t cis_num = 0, group_size = 0;
    uint8_t i, tmp;
    uint8_t sink_ase_num = 0, source_ase_num = 0;

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= g_lea_ucst_ctrl.curr_group) {
        LE_AUDIO_MSGLOG_I("[APP][U] set_cig_parameters, no active group!", 0);
        return BT_STATUS_FAIL;
    }
#endif

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (0 == app_le_audio_ucst_get_group_link_num_ex(g_lea_ucst_ctrl.curr_group))
#else
    if (0 == app_le_audio_ucst_get_link_num_ex())
#endif
    {
        LE_AUDIO_MSGLOG_I("[APP][U] set_cig_parameters, no connection!", 0);
        return BT_STATUS_FAIL;
    }

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        if ((BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) && (0 != g_lea_ucst_link_info[i].cis_num)) {
            group_size = g_lea_ucst_link_info[i].group_size;
            cis_num = g_lea_ucst_link_info[i].cis_num;
            sink_ase_num = app_le_audio_ucst_get_sink_ase_num(&g_lea_ucst_link_info[i]);
            source_ase_num = app_le_audio_ucst_get_source_ase_num(&g_lea_ucst_link_info[i]);
            break;
        }
    }

    if (tmp == app_le_audio_ucst_get_max_link_num()) {
        LE_AUDIO_MSGLOG_I("[APP][U] set_cig_parameters ERROR, check link!", 0);
        return BT_STATUS_FAIL;
    }

    cis_num *= group_size;
    LE_AUDIO_MSGLOG_I("[APP][U] set_cig_parameters, cis_num:%x group_size:%x sink_ase_num:%x source_ase_num:%x", 4, cis_num, group_size, sink_ase_num, source_ase_num);

    if (NULL != g_lea_ucst_cig_params_test) {

        /* Use set CIG parameters test command */
        bt_gap_le_cis_params_test_t *p_cis = NULL;

        if (NULL == (p_cis = (bt_gap_le_cis_params_test_t *)pvPortMalloc(sizeof(bt_gap_le_cis_params_test_t) * cis_num))) {
            LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters_test, malloc failed!", 0);
            return BT_STATUS_OUT_OF_MEMORY;
        }

        memset(p_cis, 0, (sizeof(bt_gap_le_cis_params_test_t)*cis_num));

        for (i = 0; i < cis_num; i++) {
            if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
                /* Call mode */
                p_cis[i].cis_id = ((0 == i) ? APP_LE_AUDIO_CIS_ID_3_CALL : APP_LE_AUDIO_CIS_ID_4_CALL);
                p_cis[i].max_sdu_m_to_s = g_lea_ucst_qos_params_spk_0.sdu_size;
                p_cis[i].max_pdu_m_to_s = g_lea_ucst_qos_params_spk_0.sdu_size;

                if ((0 == source_ase_num) ||
                    ((1 == group_size) && (1 == source_ase_num) && (0 != (i % 2)))) {
                    LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters_test 1, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
                    p_cis[i].max_sdu_s_to_m = 0x0000;
                    p_cis[i].max_pdu_s_to_m = 0x0000;
                    p_cis[i].bn_s_to_m = 0;

                } else {
                    LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters_test 2, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
                    p_cis[i].max_sdu_s_to_m = g_lea_ucst_qos_params_mic_0.sdu_size;
                    p_cis[i].max_pdu_s_to_m = g_lea_ucst_qos_params_mic_0.sdu_size;
                    p_cis[i].bn_s_to_m = g_lea_ucst_cig_params_test->bn;
                }

            } else {
                /* Media mode */
                p_cis[i].cis_id = ((0 == i) ? APP_LE_AUDIO_CIS_ID_1_MEDIA : APP_LE_AUDIO_CIS_ID_2_MEDIA);
                LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters_test 3, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);

                if (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4 == g_lea_ucst_ctrl.create_cis_mode) {
                    p_cis[i].max_sdu_m_to_s = (g_lea_ucst_qos_params_spk_1.sdu_size * 2);
                    p_cis[i].max_pdu_m_to_s = (g_lea_ucst_qos_params_spk_1.sdu_size * 2);
                } else {
                    p_cis[i].max_sdu_m_to_s = g_lea_ucst_qos_params_spk_1.sdu_size;
                    p_cis[i].max_pdu_m_to_s = g_lea_ucst_qos_params_spk_1.sdu_size;
                }
                p_cis[i].max_sdu_s_to_m = 0x0000;
                p_cis[i].max_pdu_s_to_m = 0x0000;
                p_cis[i].bn_s_to_m = 0;
            }

            p_cis[i].nse = g_lea_ucst_cig_params_test->nse;
            p_cis[i].phy_m_to_s = 0x02;
            p_cis[i].phy_s_to_m = 0x02;
            p_cis[i].bn_m_to_s = g_lea_ucst_cig_params_test->bn;
        }

        bt_gap_le_set_cig_params_test_t param = {
            .cig_id = APP_LE_AUDIO_CIG_ID_1,
            .ft_m_to_s = g_lea_ucst_cig_params_test->ft,
            .ft_s_to_m = g_lea_ucst_cig_params_test->ft,
            .iso_interval = g_lea_ucst_cig_params_test->iso_interval,
            .sca = 0x00,
            .packing = BT_GAP_LE_CIG_PACKING_INTERLEAVED,
            .framing = BT_GAP_LE_CIG_FRAMING_UNFRAMED,
            .cis_list = p_cis,
            .cis_count = cis_num,
        };

        if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
            param.cig_id = APP_LE_AUDIO_CIG_ID_2;
            param.sdu_interval_m_to_s = app_le_audio_ucst_get_sdu_interval(false);
            param.sdu_interval_s_to_m = app_le_audio_ucst_get_sdu_interval(true);

        } else {
            param.sdu_interval_m_to_s = app_le_audio_ucst_get_sdu_interval(false);
            param.sdu_interval_s_to_m = app_le_audio_ucst_get_sdu_interval(false);
        }

        g_lea_ucst_ctrl.cig_id = param.cig_id;
        ret = bt_gap_le_set_cig_parameters_test(&param);
        LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters_test, ret:%x cig_id:%x cis_num:%x", 3, ret, g_lea_ucst_ctrl.cig_id, cis_num);

        vPortFree(p_cis);

    } else {

        /* Use set CIG parameters command */
        bt_gap_le_cis_params_t *p_cis = NULL;

        if (NULL == (p_cis = (bt_gap_le_cis_params_t *)pvPortMalloc(sizeof(bt_gap_le_cis_params_t) * cis_num))) {
            LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters, malloc failed!", 0);
            return BT_STATUS_OUT_OF_MEMORY;
        }

        memset(p_cis, 0, (sizeof(bt_gap_le_cis_params_t)*cis_num));
        app_le_audio_ucst_link_info_t *p_info = &g_lea_ucst_link_info[i];
        app_le_audio_ase_codec_t *p_sink_codec_state = NULL;
        app_le_audio_ase_codec_t *p_source_codec_state = &p_info->ase[p_info->source_ase_idx].codec_state;
        uint8_t ase_idx = 0;
        uint8_t audio_location_channels = 0;
        uint8_t source_audio_location_channels = 0;

        if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
            if (p_info->sink_ase_num >= (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) {
                ase_idx = sink_ase_num + 0;
            }
        }
        p_sink_codec_state = &p_info->ase[ase_idx].codec_state;

        audio_location_channels = app_le_audio_ucst_get_location_count(p_sink_codec_state->audio_channel_allocation);
        if (0 == audio_location_channels) {
            audio_location_channels = 1;
        }
        source_audio_location_channels = app_le_audio_ucst_get_location_count(p_source_codec_state->audio_channel_allocation);
        if (0 == source_audio_location_channels) {
            source_audio_location_channels = 1;
        }

        for (i = 0; i < cis_num; i++) {
            if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
                p_cis[i].cis_id = ((0 == i) ? APP_LE_AUDIO_CIS_ID_3_CALL : APP_LE_AUDIO_CIS_ID_4_CALL);
                p_cis[i].rtn_m_to_s = p_sink_codec_state->preferred_retransmission_number;//g_lea_ucst_qos_params_spk_0.rtn;
                p_cis[i].rtn_s_to_m = p_source_codec_state->preferred_retransmission_number;//g_lea_ucst_qos_params_mic_0.rtn;

                if (!app_le_audio_ucst_need_cis_m_to_s(i, sink_ase_num)) {
                    p_cis[i].max_sdu_m_to_s = 0x0000;
                    LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters 1, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
                } else {
                    p_cis[i].max_sdu_m_to_s = p_sink_codec_state->codec_frame_blocks_per_sdu * audio_location_channels * p_sink_codec_state->octets_per_codec_frame;//g_lea_ucst_qos_params_spk_0.sdu_size;
                    LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters 2, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
                }

                if (!app_le_audio_ucst_need_cis_s_to_m(i, source_ase_num, group_size)) {
                    LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters 3, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
                    p_cis[i].max_sdu_s_to_m = 0x0000;
                } else {
                    LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters 4, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
                    p_cis[i].max_sdu_s_to_m = p_source_codec_state->codec_frame_blocks_per_sdu * source_audio_location_channels * p_source_codec_state->octets_per_codec_frame;//g_lea_ucst_qos_params_mic_0.sdu_size;
                }

            } else {
                p_cis[i].cis_id = ((0 == i) ? APP_LE_AUDIO_CIS_ID_1_MEDIA : APP_LE_AUDIO_CIS_ID_2_MEDIA);
                LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters 5, cis_id[%x]:%x", 2, i, p_cis[i].cis_id);
                p_cis[i].rtn_m_to_s = p_sink_codec_state->preferred_retransmission_number;//g_lea_ucst_qos_params_spk_1.rtn;
                p_cis[i].rtn_s_to_m = p_sink_codec_state->preferred_retransmission_number;//g_lea_ucst_qos_params_spk_1.rtn;
                if (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4 == g_lea_ucst_ctrl.create_cis_mode) {
                    p_cis[i].max_sdu_m_to_s = p_sink_codec_state->octets_per_codec_frame * 2;//(g_lea_ucst_qos_params_spk_1.sdu_size * 2);
                } else {
                    p_cis[i].max_sdu_m_to_s = p_sink_codec_state->codec_frame_blocks_per_sdu * audio_location_channels * p_sink_codec_state->octets_per_codec_frame;//g_lea_ucst_qos_params_spk_1.sdu_size;
                }
                p_cis[i].max_sdu_s_to_m = 0x0000;
            }

            p_cis[i].phy_m_to_s = 0x02;
            p_cis[i].phy_s_to_m = 0x02;
        }

        bt_gap_le_set_cig_params_t param = {
            .cig_id = APP_LE_AUDIO_CIG_ID_1,
            .sca = 0x00,
            .packing = BT_GAP_LE_CIG_PACKING_INTERLEAVED,
            .framing = BT_GAP_LE_CIG_FRAMING_UNFRAMED,
            .cis_list = p_cis,
            .cis_count = cis_num,
        };

        if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
            param.cig_id = APP_LE_AUDIO_CIG_ID_2;
            param.sdu_interval_m_to_s = (p_sink_codec_state->frame_duration == FRAME_DURATION_7P5_MS) ? 7500 : 10000;//app_le_audio_ucst_get_sdu_interval(false);
            param.sdu_interval_s_to_m = (p_source_codec_state->frame_duration == FRAME_DURATION_7P5_MS) ? 7500 : 10000;//app_le_audio_ucst_get_sdu_interval(true);
            param.max_transport_latency_m_to_s = p_sink_codec_state->max_transport_latency;//g_lea_ucst_qos_params_spk_0.latency;
            param.max_transport_latency_s_to_m = p_source_codec_state->max_transport_latency;//g_lea_ucst_qos_params_mic_0.latency;

        } else {
            param.sdu_interval_m_to_s = (p_sink_codec_state->frame_duration == FRAME_DURATION_7P5_MS) ? 7500 : 10000;//app_le_audio_ucst_get_sdu_interval(false);
            param.sdu_interval_s_to_m = (p_sink_codec_state->frame_duration == FRAME_DURATION_7P5_MS) ? 7500 : 10000;//app_le_audio_ucst_get_sdu_interval(false);
            param.max_transport_latency_m_to_s = p_sink_codec_state->max_transport_latency;//g_lea_ucst_qos_params_spk_1.latency;
            param.max_transport_latency_s_to_m = p_sink_codec_state->max_transport_latency;//g_lea_ucst_qos_params_spk_1.latency;
        }

        g_lea_ucst_ctrl.cig_id = param.cig_id;

        ret = bt_gap_le_set_cig_parameters(&param);
        LE_AUDIO_MSGLOG_I("[APP] set_cig_parameters, ret:%x cig_id:%x cis_num:%x", 3, ret, g_lea_ucst_ctrl.cig_id, cis_num);

        vPortFree(p_cis);
    }

    return ret;
}


bt_status_t app_le_audio_ucst_create_cis(void)
{
    app_le_audio_ucst_link_info_t *p_info;
    bt_status_t ret;
    uint8_t i, tmp;
    bt_gap_le_cis_set_t hdl_list[APP_LE_AUDIO_UCST_CIS_MAX_NUM] = {{0}, {0}};
    bt_gap_le_create_cis_t param = {
        .cis_count = 0x00,
        .cis_list = hdl_list,
    };

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    app_le_audio_ucst_group_info_t *p_group_info = NULL;

    if (NULL == (p_group_info = app_le_audio_ucst_get_group_info(g_lea_ucst_ctrl.curr_group))) {
        LE_AUDIO_MSGLOG_I("[APP][U] create_cis, no active group!", 0);
        return BT_STATUS_FAIL;
    }
#endif

    LE_AUDIO_MSGLOG_I("[APP][U] create_cis, cig:%x total_cis_num:%x", 2, g_lea_ucst_ctrl.cig_id, g_lea_ucst_ctrl.cis_num);

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = p_group_info->link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        p_info = &g_lea_ucst_link_info[i];

        if ((BT_HANDLE_INVALID != p_info->handle) &&
            (APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS == p_info->next_state)) {

            LE_AUDIO_MSGLOG_I("[APP][U] create_cis, handle:%x state:%x->%x cis_num:%x location:(%x %x)", 6,
                              p_info->handle,
                              p_info->curr_state, p_info->next_state,
                              p_info->cis_num,
                              p_info->sink_location,
                              p_info->source_location);

            if (!p_info->sink_location) {
                LE_AUDIO_MSGLOG_E("[APP][U] ERROR! sink_location shall not be empty! handle:%x", 1, p_info->handle);
            }

            if (APP_LE_AUDIO_UCST_CIS_MAX_NUM == p_info->cis_num) {
                /* create 2 CIS in 1 LE link */
                if (APP_LE_AUDIO_UCST_CIS_IDLE == g_lea_ucst_cis_info[0].cis_status) {
                    LE_AUDIO_MSGLOG_I("[APP][U] create_cis, 0", 0);
                    hdl_list[0].acl_connection_handle = p_info->handle;
                    hdl_list[0].cis_connection_handle = g_lea_ucst_cis_info[0].cis_handle; /* Left */
                    g_lea_ucst_cis_info[0].acl_handle = p_info->handle;
                    g_lea_ucst_cis_info[0].cis_status = APP_LE_AUDIO_UCST_CIS_CREATING;
                    param.cis_count++;
                }

                if (APP_LE_AUDIO_UCST_CIS_IDLE == g_lea_ucst_cis_info[1].cis_status) {
                    hdl_list[1].acl_connection_handle = p_info->handle;
                    hdl_list[1].cis_connection_handle = g_lea_ucst_cis_info[1].cis_handle; /* Right */
                    g_lea_ucst_cis_info[1].acl_handle = p_info->handle;
                    g_lea_ucst_cis_info[1].cis_status = APP_LE_AUDIO_UCST_CIS_CREATING;
                    param.cis_count++;
                }
                break;
            }

            LE_AUDIO_MSGLOG_I("[APP][U] create_cis, cis_status:%x %x", 2,
                              g_lea_ucst_cis_info[0].cis_status,
                              g_lea_ucst_cis_info[1].cis_status);

            if ((1 == g_lea_ucst_ctrl.cis_num) &&
                ((AUDIO_LOCATION_FRONT_RIGHT== p_info->sink_location) ||
                 (AUDIO_LOCATION_FRONT_RIGHT == p_info->source_location))) {

                 if (APP_LE_AUDIO_UCST_CIS_IDLE == g_lea_ucst_cis_info[0].cis_status) {
                    g_lea_ucst_cis_info[0].cis_status = APP_LE_AUDIO_UCST_CIS_CREATING;
                    g_lea_ucst_cis_info[0].acl_handle = p_info->handle;
                    hdl_list[param.cis_count].acl_connection_handle = p_info->handle;
                    hdl_list[param.cis_count].cis_connection_handle = g_lea_ucst_cis_info[0].cis_handle;
                    LE_AUDIO_MSGLOG_I("[APP][U] create_cis, 3", 0);
                    param.cis_count++;
                 }

            } else {
                /* create 1 CIS in 1 LE link */
                if ((APP_LE_AUDIO_UCST_CIS_IDLE == g_lea_ucst_cis_info[0].cis_status) &&
                    (((0 == p_info->sink_location_num) && (0 == p_info->source_location_num)) ||
                    (AUDIO_LOCATION_FRONT_LEFT == p_info->sink_location) ||
                    (AUDIO_LOCATION_FRONT_LEFT == p_info->source_location))) {
                    g_lea_ucst_cis_info[0].cis_status = APP_LE_AUDIO_UCST_CIS_CREATING;
                    g_lea_ucst_cis_info[0].acl_handle = p_info->handle;
                    hdl_list[param.cis_count].acl_connection_handle = p_info->handle;
                    hdl_list[param.cis_count].cis_connection_handle = g_lea_ucst_cis_info[0].cis_handle;
                    LE_AUDIO_MSGLOG_I("[APP][U] create_cis, 1", 0);
                    param.cis_count++;

                } else if ((APP_LE_AUDIO_UCST_CIS_IDLE == g_lea_ucst_cis_info[1].cis_status) &&
                    ((AUDIO_LOCATION_FRONT_RIGHT== p_info->sink_location) ||
                    (AUDIO_LOCATION_FRONT_RIGHT == p_info->source_location))) {
                    g_lea_ucst_cis_info[1].cis_status = APP_LE_AUDIO_UCST_CIS_CREATING;
                    g_lea_ucst_cis_info[1].acl_handle = p_info->handle;
                    hdl_list[param.cis_count].acl_connection_handle = p_info->handle;
                    hdl_list[param.cis_count].cis_connection_handle = g_lea_ucst_cis_info[1].cis_handle;
                    LE_AUDIO_MSGLOG_I("[APP][U] create_cis, 2", 0);
                    param.cis_count++;
                }
            }
        }
    }

    LE_AUDIO_MSGLOG_I("[APP] create_cis, cis_count:%x [0](acl:%x cis:%x) [1](acl:%x cis:%x)", 5,
                      param.cis_count,
                      hdl_list[0].acl_connection_handle,
                      hdl_list[0].cis_connection_handle,
                      hdl_list[1].acl_connection_handle,
                      hdl_list[1].cis_connection_handle);

    ret = bt_gap_le_create_cis(&param);
    LE_AUDIO_MSGLOG_I("[APP] create_cis, ret:%x", 1, ret);

    return ret;
}

static bt_status_t app_le_audio_ucst_disconnect_cis(bt_handle_t handle)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t cis_idx, i;
    bool pending_action = false;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        LE_AUDIO_MSGLOG_I("[APP][U] disconnect_cis, link not exist (hdl:%x)", 1, handle);
        return BT_STATUS_FAIL;
    }

    i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
    cis_idx = APP_LE_AUDIO_UCST_CIS_MAX_NUM;

    LE_AUDIO_MSGLOG_I("[APP][U] disconnect_cis, handle:%x state:%x->%x cis_num:%x", 4,
                      p_info->handle,
                      p_info->curr_state, p_info->next_state,
                      p_info->cis_num);

    while (i > 0) {
        i--;
        if (BT_HANDLE_INVALID != g_lea_ucst_cis_info[i].cis_handle) {
            LE_AUDIO_MSGLOG_I("[APP] disconnect_cis, check handle:%x cis_handle[%x]:%x cis_status:%x", 4,
                              g_lea_ucst_cis_info[i].acl_handle,
                              i,
                              g_lea_ucst_cis_info[i].cis_handle,
                              g_lea_ucst_cis_info[i].cis_status);
            if (p_info->handle == g_lea_ucst_cis_info[i].acl_handle) {
                g_lea_ucst_cis_info[i].cis_status = APP_LE_AUDIO_UCST_CIS_DISCONNECTING;
                cis_idx = i;

            } else if (APP_LE_AUDIO_UCST_CIS_DISCONNECTING == g_lea_ucst_cis_info[i].cis_status) {
                pending_action = true;
            }
        }
    }

    if (APP_LE_AUDIO_UCST_CIS_MAX_NUM == cis_idx) {
        LE_AUDIO_MSGLOG_I("[APP][U] disconnect_cis, cis not found!", 0);
        return BT_STATUS_FAIL;
    }

    if (pending_action) {
        LE_AUDIO_MSGLOG_I("[APP][U] disconnect_cis, pending_action:%x", 1, pending_action);
        return BT_STATUS_SUCCESS;
    }

    return app_le_audio_ucst_disconnect(g_lea_ucst_cis_info[cis_idx].cis_handle);
}

static bt_status_t app_le_audio_ucst_remove_cig(void)
{
    bt_status_t ret;
    bt_gap_le_remove_cig_t param = {
        .cig_id = g_lea_ucst_ctrl.cig_id
    };

    ret = bt_gap_le_remove_cig(&param);

    LE_AUDIO_MSGLOG_I("[APP][U] remove_cig, ret:%x cig_id:%x", 2, ret, g_lea_ucst_ctrl.cig_id);

    return ret;
}

bool app_le_audio_ucst_check_pause_stream(void)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t i, tmp;
    app_le_audio_ucst_lock_stream_t lock_stream = APP_LE_AUDIO_UCST_LCOK_STREAM_NONE;
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    app_le_audio_ucst_group_info_t * p_group_info = NULL;
#endif

    if  (APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL <= g_lea_ucst_ctrl.pause_stream) {
        return true;
    }

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (NULL == (p_group_info = app_le_audio_ucst_get_group_info(g_lea_ucst_ctrl.curr_group))) {
            LE_AUDIO_MSGLOG_I("[APP][U] check_pause_stream, curr_group(%x) not exist", 1, g_lea_ucst_ctrl.curr_group);
            break;
        }
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = p_group_info->link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        p_info = &g_lea_ucst_link_info[i];
        if (BT_HANDLE_INVALID != p_info->handle) {
            LE_AUDIO_MSGLOG_I("[APP][U] check_pause_stream, handle:%x lock:%x", 2,
                              p_info->handle,
                              p_info->lock_stream);

            if (lock_stream < p_info->lock_stream) {
                lock_stream = p_info->lock_stream;
            }
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][U] check_pause_stream, p:%x lock_stream:%x", 2,
                      g_lea_ucst_ctrl.pause_stream,
                      lock_stream);

    if (APP_LE_AUDIO_UCST_LCOK_STREAM_ALL <= lock_stream) {
        return true;
    }

    if ((APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
        (APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == g_lea_ucst_ctrl.next_target)) {
        if ((APP_LE_AUDIO_UCST_PAUSE_STREAM_UNIDIRECTIONAL <= g_lea_ucst_ctrl.pause_stream) ||
            (APP_LE_AUDIO_UCST_LCOK_STREAM_UNIDIRECTIONAL <= lock_stream)) {
            return true;
        }
    }

    return false;
}

static bool app_le_audio_ucst_check_close_audio_stream(void)
{
    uint8_t i, tmp;
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    app_le_audio_ucst_group_info_t *p_group_info = app_le_audio_ucst_get_group_info(g_lea_ucst_ctrl.curr_group);
#endif

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if(NULL == p_group_info) {
            break;
        }
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = p_group_info->link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        if (BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) {
            if (!((APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC > g_lea_ucst_link_info[i].curr_state) ||
                  (((APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC == g_lea_ucst_link_info[i].curr_state) ||
                    (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS == g_lea_ucst_link_info[i].curr_state)) &&
                   (APP_LE_AUDIO_UCST_LINK_STATE_IDLE == g_lea_ucst_link_info[i].next_state) &&
                   (0 == g_lea_ucst_link_info[i].wait_event.wait_ase_event)))) {
                LE_AUDIO_MSGLOG_I("[APP][ASE] close_audio_stream, check handle:%x state:%x->%x w_ase:%x r:%x", 5,
                                  g_lea_ucst_link_info[i].handle,
                                  g_lea_ucst_link_info[i].curr_state, g_lea_ucst_link_info[i].next_state,
                                  g_lea_ucst_link_info[i].wait_event.wait_ase_event,
                                  g_lea_ucst_link_info[i].ase_releasing);
                return false;
            }
        }
    }
    if (0 != app_le_audio_ucst_get_cis_num()) {
        return false;
    }
    /* stop audio stream */
    if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE < g_lea_ucst_ctrl.curr_stream_state) &&
        (APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING >= g_lea_ucst_ctrl.curr_stream_state) &&
        (APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM != g_lea_ucst_ctrl.next_stream_state)) {
        g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM;
        if (BT_STATUS_SUCCESS == app_le_audio_close_audio_transmitter()) {
            return true;
        }
        g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
    }
    return false;
}

static void app_le_audio_ucst_check_set_ase(uint8_t link_idx)
{
    if (((APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE == g_lea_ucst_link_info[link_idx].curr_state) ||
         (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC == g_lea_ucst_link_info[link_idx].curr_state) ||
         (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS == g_lea_ucst_link_info[link_idx].curr_state)) &&
        (APP_LE_AUDIO_UCST_LINK_STATE_IDLE == g_lea_ucst_link_info[link_idx].next_state) &&
        (0 == g_lea_ucst_link_info[link_idx].wait_event.wait_ase_event)) {

        uint8_t ase_idx = APP_LE_AUDIO_UCST_ASE_IDX_0;

        LE_AUDIO_MSGLOG_I("[APP][U] check_set_ase", 0);

        if (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) {
            ase_idx = g_lea_ucst_link_info[link_idx].source_ase_idx;
        }

        if (ASE_STATE_QOS_CONFIGURED == g_lea_ucst_link_info[link_idx].ase[ase_idx].curr_state) {
            g_lea_ucst_link_info[link_idx].curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
            g_lea_ucst_link_info[link_idx].next_state = APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE;
            if (BT_STATUS_SUCCESS != app_le_audio_ucst_enable_ase(g_lea_ucst_link_info[link_idx].handle)) {
                g_lea_ucst_link_info[link_idx].next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
            }

        } else if (ASE_STATE_CODEC_CONFIGURED == g_lea_ucst_link_info[link_idx].ase[ase_idx].curr_state) {
            g_lea_ucst_link_info[link_idx].curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
            g_lea_ucst_link_info[link_idx].next_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
            if (BT_STATUS_SUCCESS != app_le_audio_ucst_config_qos(g_lea_ucst_link_info[link_idx].handle)) {
                g_lea_ucst_link_info[link_idx].next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
            }
        } else if (ASE_STATE_IDLE == g_lea_ucst_link_info[link_idx].ase[ase_idx].curr_state) {
            g_lea_ucst_link_info[link_idx].curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE;
            g_lea_ucst_link_info[link_idx].next_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
            if (BT_STATUS_SUCCESS != app_le_audio_ucst_config_codec(g_lea_ucst_link_info[link_idx].handle)) {
                g_lea_ucst_link_info[link_idx].next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
            }
        }
    }
}


static void app_le_audio_ucst_disable_ase_when_setup_iso_data_path(void)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t i, tmp;
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    app_le_audio_ucst_group_info_t *p_group_info = app_le_audio_ucst_get_group_info(g_lea_ucst_ctrl.curr_group);

    if(NULL == p_group_info) {
        return;
    }
#endif

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = p_group_info->link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        p_info = &g_lea_ucst_link_info[i];

        if (BT_HANDLE_INVALID != p_info->handle) {

            if ((((APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH == p_info->curr_state) ||
                  (APP_LE_AUDIO_UCST_LINK_STATE_STREAMING == p_info->curr_state)) &&
                 (APP_LE_AUDIO_UCST_LINK_STATE_IDLE == p_info->next_state)) ||
                (APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS == p_info->curr_state)) {

                LE_AUDIO_MSGLOG_I("[APP] disable_ase_when_setup_iso_data_path, handle:%x state:%x->%x", 3,
                                  p_info->handle,
                                  p_info->curr_state,
                                  p_info->next_state);

                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE;
                if (BT_STATUS_SUCCESS != app_le_audio_ucst_disable_ase(p_info->handle)) {
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                }
            }
        }
    }
}

static bool app_le_audio_ucst_disable_ase_when_state_match_streaming(bool check_release)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t i, tmp;
    uint8_t wait_ase_num = 0;
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    app_le_audio_ucst_group_info_t *p_group_info = app_le_audio_ucst_get_group_info(g_lea_ucst_ctrl.curr_group);
#endif

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if(NULL == p_group_info) {
            break;
        }

        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = p_group_info->link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        p_info = &g_lea_ucst_link_info[i];

        if (BT_HANDLE_INVALID != p_info->handle) {

            LE_AUDIO_MSGLOG_I("[APP][U] disable_ase_when_state_match_streaming, handle:%x state:%x->%x w_ase:%x check_release:%x", 5,
                              p_info->handle,
                              p_info->curr_state,
                              p_info->next_state,
                              p_info->wait_event.wait_ase_event,
                              check_release);

            if ((APP_LE_AUDIO_UCST_LINK_STATE_STREAMING == p_info->curr_state) &&
                (APP_LE_AUDIO_UCST_LINK_STATE_IDLE == p_info->next_state)) {

                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE;
                wait_ase_num++;

                if (BT_STATUS_SUCCESS != app_le_audio_ucst_disable_ase(p_info->handle)) {
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                    wait_ase_num--;
                }
            } else if (check_release && p_info->ase_releasing) {
                wait_ase_num++;
            }
        }
    }

    if (0 != wait_ase_num) {
        return true;
    }

    return false;
}

static void app_le_audio_ucst_disable_ase_when_state_match(app_le_audio_ucst_link_state_t target_state)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t i, tmp;
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    app_le_audio_ucst_group_info_t *p_group_info = app_le_audio_ucst_get_group_info(g_lea_ucst_ctrl.curr_group);
#endif

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if(NULL == p_group_info) {
            break;
        }

        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = p_group_info->link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        p_info = &g_lea_ucst_link_info[i];

        if ((BT_HANDLE_INVALID != p_info->handle) &&
            (target_state == p_info->curr_state) &&
            (APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE != p_info->next_state)) {

            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE;

            LE_AUDIO_MSGLOG_I("[APP] disable_ase_when_state_match, handle:%x state:%x->%x target_state:%x", 4,
                              p_info->handle,
                              p_info->curr_state,
                              p_info->next_state,
                              target_state);

            if (BT_STATUS_SUCCESS != app_le_audio_ucst_disable_ase(p_info->handle)) {
                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
            }
        }
    }
}



static void app_le_audio_ucst_handle_ase_idle(ble_bap_ase_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][ASE] IDLE, handle:%x state:%x->%x ase_id:%x w_ase:%x r:%x", 6,
                      p_info->handle,
                      p_info->curr_state, p_info->next_state,
                      p_notify->ase_id,
                      p_info->wait_event.wait_ase_event,
                      p_info->ase_releasing);

    /* update ase state */
    uint8_t ase_idx;
    if (APP_LE_AUDIO_UCST_ASE_MAX_NUM == (ase_idx = app_le_audio_ucst_get_ase_idx(p_info, p_notify->ase_id))) {
        return;
    }
    p_info->ase[ase_idx].curr_state = ASE_STATE_IDLE;

    if (p_info->ase_releasing) {
        if (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC == p_info->next_state) {
            /* handle ASE release */
            if (p_info->wait_event.wait_ase_event > 0) {
                p_info->wait_event.wait_ase_event--;

                if (0 != p_info->wait_event.wait_ase_event) {
                    return;
                }

                p_info->ase_releasing = false;
                p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE;
                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                //ASE_STATE_CODEC_CONFIGURED State: Receive ASE_STATE_RELEASING + ASE_STATE_CODEC_CONFIGURED
                if((false == app_le_audio_ucst_check_close_audio_stream())&&(0 != app_le_audio_ucst_get_cis_num())) {
                    app_le_audio_ucst_reset_release();
                }
            }
        }
        return;
    }

}

static void app_le_audio_ucst_handle_ase_codec_configured(ble_bap_ase_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint16_t index = 0;
    uint8_t codec_specific_configuration_length;
    uint8_t length,type;
    uint32_t value = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][ASE] CODEC_CONFIGURED, handle:%x state:%x->%x ase_id:%x w_ase:%x r:%x", 6,
                      p_info->handle,
                      p_info->curr_state, p_info->next_state,
                      p_notify->ase_id,
                      p_info->wait_event.wait_ase_event,
                      p_info->ase_releasing);

    /* update ase state */
    uint8_t ase_idx;
    if (APP_LE_AUDIO_UCST_ASE_MAX_NUM == (ase_idx = app_le_audio_ucst_get_ase_idx(p_info, p_notify->ase_id))) {
        return;
    }
    p_info->ase[ase_idx].curr_state = ASE_STATE_CODEC_CONFIGURED;
    p_info->ase[ase_idx].codec_state.codec_frame_blocks_per_sdu = 1;//Set default value
    p_info->ase[ase_idx].codec_state.audio_channel_allocation = 0;//Set default value
    p_info->ase[ase_idx].codec_state.framing = p_notify->additional_parameter[index++];
    p_info->ase[ase_idx].codec_state.preferred_phy = p_notify->additional_parameter[index++];
    p_info->ase[ase_idx].codec_state.preferred_retransmission_number = p_notify->additional_parameter[index++];
    memcpy(&p_info->ase[ase_idx].codec_state.max_transport_latency, &p_notify->additional_parameter[index], sizeof(uint16_t));
    index += sizeof(uint16_t);
    memcpy(&value, &p_notify->additional_parameter[index], 3);
    p_info->ase[ase_idx].codec_state.presentation_delay_min = value;
    index += 3;
    memcpy(&value, &p_notify->additional_parameter[index], 3);
    p_info->ase[ase_idx].codec_state.presentation_delay_max = value;
    index += 3;
    memcpy(&value, &p_notify->additional_parameter[index], 3);
    p_info->ase[ase_idx].codec_state.preferred_presentation_delay_min = value;
    index += 3;
    memcpy(&value, &p_notify->additional_parameter[index], 3);
    p_info->ase[ase_idx].codec_state.preferred_presentation_delay_max = value;
    index += 3;
    memcpy(p_info->ase[ase_idx].codec_state.codec_id, &p_notify->additional_parameter[index], AUDIO_CODEC_ID_SIZE);
    index += AUDIO_CODEC_ID_SIZE;
    LE_AUDIO_MSGLOG_I("[APP][ASCS] handle:%x ASE[%d],framing:%02x preferred_phy:%02x rtn:%02x max_transport_latency:%04x presentation_delay:%06x~%06x preferred_presentation_delay:%06x~%06x", 10,
        p_info->handle,
        ase_idx,
        p_info->ase[ase_idx].codec_state.framing,
        p_info->ase[ase_idx].codec_state.preferred_phy,
        p_info->ase[ase_idx].codec_state.preferred_retransmission_number,
        p_info->ase[ase_idx].codec_state.max_transport_latency,
        p_info->ase[ase_idx].codec_state.presentation_delay_min,
        p_info->ase[ase_idx].codec_state.presentation_delay_max,
        p_info->ase[ase_idx].codec_state.preferred_presentation_delay_min,
        p_info->ase[ase_idx].codec_state.preferred_presentation_delay_max);

    codec_specific_configuration_length = p_notify->additional_parameter[index++];
    while(codec_specific_configuration_length > 0) {
        length = p_notify->additional_parameter[index++];
        type = p_notify->additional_parameter[index++];
/*
        LE_AUDIO_MSGLOG_I("[APP][ASCS] L:%x T:%x D:%02x %02x %02x %02x", 6, length, type,
            p_notify->additional_parameter[index],p_notify->additional_parameter[index+1],
            p_notify->additional_parameter[index+2],p_notify->additional_parameter[index+3]);
*/
        switch (type) {
            case CODEC_CONFIGURATION_TYPE_SAMPLING_FREQUENCY: {
                p_info->ase[ase_idx].codec_state.sampling_frequency = p_notify->additional_parameter[index++];
                break;
            }
            case CODEC_CONFIGURATION_TYPE_FRAME_DURATIONS: {
                p_info->ase[ase_idx].codec_state.frame_duration = p_notify->additional_parameter[index++];
                break;
            }
            case CODEC_CONFIGURATION_TYPE_AUDIO_CHANNEL_ALLOCATION: {
                memcpy(&p_info->ase[ase_idx].codec_state.audio_channel_allocation, &(p_notify->additional_parameter[index]), sizeof(uint32_t));
                index += sizeof(uint32_t);
                break;
            }
            case CODEC_CONFIGURATION_TYPE_OCTETS_PER_CODEC_FRAME: {
                memcpy(&p_info->ase[ase_idx].codec_state.octets_per_codec_frame, &(p_notify->additional_parameter[index]), sizeof(uint16_t));
                index += sizeof(uint16_t);
                break;
            }
            case CODEC_CONFIGURATION_TYPE_CODEC_FRAME_BLOCKS_PER_SDU: {
                p_info->ase[ase_idx].codec_state.codec_frame_blocks_per_sdu = p_notify->additional_parameter[index++];
                break;
            }
            default:
                index += (length - 1);
                break;
        }
        codec_specific_configuration_length -= (length + 1);
    }

    LE_AUDIO_MSGLOG_I("[APP][ASCS] handle:%x ASE[%d],sampling_frequency:%02x frame_duration:%02x audio_channel_allocation:%08x octets_per_codec_frame:%04x codec_frame_blocks_per_sdu:%02x", 7,
        p_info->handle,
        ase_idx,
        p_info->ase[ase_idx].codec_state.sampling_frequency,
        p_info->ase[ase_idx].codec_state.frame_duration,
        p_info->ase[ase_idx].codec_state.audio_channel_allocation,
        p_info->ase[ase_idx].codec_state.octets_per_codec_frame,
        p_info->ase[ase_idx].codec_state.codec_frame_blocks_per_sdu);

    if (p_info->ase_releasing) {
        if (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC == p_info->next_state) {
            /* handle ASE release */
            if (p_info->wait_event.wait_ase_event > 0) {
                p_info->wait_event.wait_ase_event--;

                if (0 != p_info->wait_event.wait_ase_event) {
                    return;
                }

                p_info->ase_releasing = false;
                p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                //ASE_STATE_CODEC_CONFIGURED State: Receive ASE_STATE_RELEASING + ASE_STATE_CODEC_CONFIGURED
                if((false == app_le_audio_ucst_check_close_audio_stream())&&(0 != app_le_audio_ucst_get_cis_num())) {
                    app_le_audio_ucst_reset_release();
                }
            }
        }
        return;
    }

    switch (p_info->curr_state) {
        case APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC == p_info->next_state) {

                if (p_info->wait_event.wait_ase_event > 0) {
                    p_info->wait_event.wait_ase_event--;

                    if ((0 != p_info->wait_event.wait_ase_event) || (0 != p_info->wait_event.wait_ase_cp_event)) {
                        break;
                    }

                    char conn_string[60] = {0};

                    snprintf((char *)conn_string, 50, "Device config complete! handle:0x%04x\r\n", p_info->handle);
                    bt_app_common_at_cmd_print_report(conn_string);

                    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;

                    LE_AUDIO_MSGLOG_I("[APP][U] Device config complete! handle:%x target:%x->%x stream_state:%x->%x p:%x r:%x", 7,
                                      p_info->handle,
                                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);
                    if (app_le_audio_ucst_check_pause_stream() || (g_lea_ucst_ctrl.release) ||
#ifdef AIR_SILENCE_DETECTION_ENABLE
                        (APP_LE_AUDIO_SILENCE_DETECTION_MODE_SPECIAL == app_le_audio_silence_detection_get_silence_detection_mode()) ||
#endif
                        (((APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
                          (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) &&
                         (APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.next_target))) {
                        break;
                    }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                    LE_AUDIO_MSGLOG_I("[APP] codec_config, group:%x active:%x->%x latest:%x", 4,
                                      p_info->group_id,
                                      g_lea_ucst_ctrl.curr_group,
                                      g_lea_ucst_ctrl.next_group,
                                      g_lea_ucst_ctrl.latest_group);
                    if (!(((APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.curr_group) &&
                           (APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_ctrl.next_group) &&
                           (p_info->group_id == g_lea_ucst_ctrl.curr_group)) ||
                          ((APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_ctrl.curr_group) &&
                           (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.latest_group) &&
                           (p_info->group_id == g_lea_ucst_ctrl.latest_group)))) {
                        break;
                    }
#endif
                    if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
                        (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {
                        /* check usb state and start stream */
                        g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                        g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                        app_le_audio_ucst_start();
                        break;
                    }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                    if ((p_info->group_id != g_lea_ucst_ctrl.curr_group) ||
                        (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= g_lea_ucst_ctrl.curr_group)) {
                        break;
                    }
#endif

                    if ((APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
                        (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {

                        if ((APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM == g_lea_ucst_ctrl.curr_stream_state) &&
                               (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {
                             if (APP_LE_AUDIO_UCST_TEST_MODE_CIG_PARAM & g_lea_ucst_test_mode_flag) {
                                 if (BT_STATUS_SUCCESS == app_le_audio_ucst_test_mode_set_cig_param()) {
                                     g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_SET_CIG_PARAMETER;
                                 }
                             } else {
                                 if (BT_STATUS_SUCCESS == app_le_audio_ucst_set_cig_parameters()) {
                                     g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_SET_CIG_PARAMETER;
                                 }
                             }
                         }

                        if (((APP_LE_AUDIO_UCST_STREAM_STATE_SET_CIG_PARAMETER == g_lea_ucst_ctrl.curr_stream_state) ||
                             (APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING == g_lea_ucst_ctrl.curr_stream_state)) &&
                            (APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM != g_lea_ucst_ctrl.next_stream_state)) {
                            /* continue to config ase */
                            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
                            if (BT_STATUS_SUCCESS != app_le_audio_ucst_config_qos(p_info->handle)) {
                                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                            }
                            /* check group mate link (for after ase release) */
                            uint8_t i, tmp;
                            for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                                if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
                                    continue;
                                }
#else
                                i = tmp;
#endif
                                if ((p_info->handle != g_lea_ucst_link_info[i].handle) &&
                                    (BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle)) {
                                    app_le_audio_ucst_check_set_ase(i);
                                }
                            }
                            break;
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }

}


static void app_le_audio_ucst_handle_ase_qos_configured(ble_bap_ase_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint32_t value = 0;
    uint16_t index = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][ASE] QOS_CONFIGURED, handle:%x state:%x->%x ase_id:%x w_ase:%x r:%x", 6,
                      p_info->handle,
                      p_info->curr_state, p_info->next_state,
                      p_notify->ase_id,
                      p_info->wait_event.wait_ase_event,
                      p_info->ase_releasing);

    /* update ase state */
    uint8_t ase_idx;
    if (APP_LE_AUDIO_UCST_ASE_MAX_NUM == (ase_idx = app_le_audio_ucst_get_ase_idx(p_info, p_notify->ase_id))) {
        return;
    }
    p_info->ase[ase_idx].curr_state = ASE_STATE_QOS_CONFIGURED;
    p_info->ase[ase_idx].qos_state.cig_id = p_notify->additional_parameter[index++];
    p_info->ase[ase_idx].qos_state.cis_id = p_notify->additional_parameter[index++];
    memcpy(&value, &p_notify->additional_parameter[index], 3);
    p_info->ase[ase_idx].qos_state.sdu_interval = value;
    index += 3;
    p_info->ase[ase_idx].qos_state.framing = p_notify->additional_parameter[index++];
    p_info->ase[ase_idx].qos_state.phy = p_notify->additional_parameter[index++];
    memcpy(&p_info->ase[ase_idx].qos_state.max_sdu, &p_notify->additional_parameter[index], sizeof(uint16_t));
    index += sizeof(uint16_t);
    p_info->ase[ase_idx].qos_state.retransmission_number = p_notify->additional_parameter[index++];
    memcpy(&p_info->ase[ase_idx].qos_state.max_transport_latency, &p_notify->additional_parameter[index], sizeof(uint16_t));
    index += sizeof(uint16_t);
    memcpy(&value, &p_notify->additional_parameter[index], 3);
    p_info->ase[ase_idx].qos_state.presentation_delay = value;
    index += 3;

    LE_AUDIO_MSGLOG_I("[APP][ASCS] handle:%x ASE[%d],cig_id:%02x cis_id:%02x sdu_interval:%06x framing:%02x phy:%02x max_sdu:%04x rtn:%02x max_transport_latency:%04x presentation_delay:%06x", 11,
        p_info->handle,
        ase_idx,
        p_info->ase[ase_idx].qos_state.cig_id,
        p_info->ase[ase_idx].qos_state.cis_id,
        p_info->ase[ase_idx].qos_state.sdu_interval,
        p_info->ase[ase_idx].qos_state.framing,
        p_info->ase[ase_idx].qos_state.phy,
        p_info->ase[ase_idx].qos_state.max_sdu,
        p_info->ase[ase_idx].qos_state.retransmission_number,
        p_info->ase[ase_idx].qos_state.max_transport_latency,
        p_info->ase[ase_idx].qos_state.presentation_delay);


    switch (p_info->curr_state) {
        case APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS == p_info->next_state) {
                if (p_info->wait_event.wait_ase_event > 0) {
                    p_info->wait_event.wait_ase_event--;

                    if ((0 != p_info->wait_event.wait_ase_event) || (0 != p_info->wait_event.wait_ase_cp_event)) {
                        break;
                    }

                    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;

                    LE_AUDIO_MSGLOG_I("[APP] qos_config, group:%x active:%x->%x latest:%x", 4,
                                      p_info->group_id,
                                      g_lea_ucst_ctrl.curr_group,
                                      g_lea_ucst_ctrl.next_group,
                                      g_lea_ucst_ctrl.latest_group);

                    if ((!app_le_audio_ucst_is_active_group(p_info->group_id)) ||
                        app_le_audio_ucst_check_pause_stream() ||
                        (APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
                        (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
                        app_le_audio_ucst_check_close_audio_stream();
                        break;
                    }

                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE;
                    if (BT_STATUS_SUCCESS != app_le_audio_ucst_enable_ase(p_info->handle)) {
                        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                    }
                }
            }
            break;
        }
        case APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE: {
            /* handle switch bis or stop stream */
            if (APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE == p_info->next_state) {
                if (p_info->wait_event.wait_ase_event > 0) {
                    p_info->wait_event.wait_ase_event--;

                    if ((0 != p_info->wait_event.wait_ase_event) || (0 != p_info->wait_event.wait_ase_cp_event)) {
                        break;
                    }

                    LE_AUDIO_MSGLOG_I("[APP] QOS_CONFIGURED, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

                    LE_AUDIO_MSGLOG_I("[APP] qos_config, group:%x active:%x->%x latest:%x", 4,
                                      p_info->group_id,
                                      g_lea_ucst_ctrl.curr_group,
                                      g_lea_ucst_ctrl.next_group,
                                      g_lea_ucst_ctrl.latest_group);

                    if (((APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) ||
                         (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) &&
                        (0 != app_le_audio_ucst_get_source_ase_num(p_info))) {
                        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY;
                        if (BT_STATUS_SUCCESS != app_le_audio_ucst_set_receiver_stop_ready(p_info->handle)) {
                            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                        }

                    } else {
                        p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
                        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                        app_le_audio_ucst_check_close_audio_stream();
                    }
                }
                break;
            }

            if (APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY == p_info->next_state) {
                if (p_info->wait_event.wait_ase_event > 0) {
                    p_info->wait_event.wait_ase_event--;

                    if ((0 != p_info->wait_event.wait_ase_event) || (0 != p_info->wait_event.wait_ase_cp_event)) {
                        break;
                    }

                    LE_AUDIO_MSGLOG_I("[APP] QOS_CONFIGURED, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

                    LE_AUDIO_MSGLOG_I("[APP] qos_config, group:%x active:%x->%x latest:%x", 4,
                                      p_info->group_id,
                                      g_lea_ucst_ctrl.curr_group,
                                      g_lea_ucst_ctrl.next_group,
                                      g_lea_ucst_ctrl.latest_group);

                    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                    app_le_audio_ucst_check_close_audio_stream();
                }
                break;
            }
            break;
        }
        case APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS:
        case APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH:
        case APP_LE_AUDIO_UCST_LINK_STATE_STREAMING: {
            /* handle disable stream (sink ase) */
            if (APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE == p_info->next_state) {
                if (p_info->wait_event.wait_ase_event > 0) {
                    p_info->wait_event.wait_ase_event--;

                    if ((0 != p_info->wait_event.wait_ase_event) || (0 != p_info->wait_event.wait_ase_cp_event)) {
                        break;
                    }

                    LE_AUDIO_MSGLOG_I("[APP] QOS_CONFIGURED, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

                    LE_AUDIO_MSGLOG_I("[APP] qos_config, group:%x active:%x->%x latest:%x", 4,
                                      p_info->group_id,
                                      g_lea_ucst_ctrl.curr_group,
                                      g_lea_ucst_ctrl.next_group,
                                      g_lea_ucst_ctrl.latest_group);

                    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE;
                    if (((APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) ||
                         (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) &&
                        (0 != app_le_audio_ucst_get_source_ase_num(p_info))) {
                        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY;
                        if (BT_STATUS_SUCCESS != app_le_audio_ucst_set_receiver_stop_ready(p_info->handle)) {
                            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                        }
                    } else {
                        p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY;
                        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_CIS;
                        if (BT_STATUS_SUCCESS != app_le_audio_ucst_disconnect_cis(p_info->handle)) {
                            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                        }
                    }
                }
            }
            break;
        }
        case APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE: {
            /* handle disable stream (call mode) */
            if (APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY == p_info->next_state) {
                if (p_info->wait_event.wait_ase_event > 0) {
                    p_info->wait_event.wait_ase_event--;

                    if ((0 != p_info->wait_event.wait_ase_event) || (0 != p_info->wait_event.wait_ase_cp_event)) {
                        break;
                    }

                    LE_AUDIO_MSGLOG_I("[APP] QOS_CONFIGURED, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

                    LE_AUDIO_MSGLOG_I("[APP] qos_config, group:%x active:%x->%x latest:%x", 4,
                                      p_info->group_id,
                                      g_lea_ucst_ctrl.curr_group,
                                      g_lea_ucst_ctrl.next_group,
                                      g_lea_ucst_ctrl.latest_group);

                    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY;
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_CIS;
                    if (BT_STATUS_SUCCESS != app_le_audio_ucst_disconnect_cis(p_info->handle)) {
                        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                    }
                }
            }
            break;
        }
        default:
            if (p_info->wait_event.wait_ase_event > 0) {
                p_info->wait_event.wait_ase_event--;
            }
            break;
    }
}


static void app_le_audio_ucst_handle_ase_enabling(ble_bap_ase_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint16_t index = 0;
    uint8_t metadata_length = 0;
    uint8_t length,type;
    //uint32_t value = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][ASE] ENABLING, handle:%x state:%x->%x ase_id:%x w_ase:%x r:%x", 6,
                      p_info->handle,
                      p_info->curr_state, p_info->next_state,
                      p_notify->ase_id,
                      p_info->wait_event.wait_ase_event,
                      p_info->ase_releasing);

    /* update ase state */
    uint8_t ase_idx;
    if (APP_LE_AUDIO_UCST_ASE_MAX_NUM == (ase_idx = app_le_audio_ucst_get_ase_idx(p_info, p_notify->ase_id))) {
        return;
    }
    p_info->ase[ase_idx].curr_state = ASE_STATE_ENABLING;
    p_info->ase[ase_idx].metadata.cig_id = p_notify->additional_parameter[index++];
    p_info->ase[ase_idx].metadata.cis_id = p_notify->additional_parameter[index++];
    metadata_length = p_notify->additional_parameter[index++];
    while(metadata_length > 0) {
        length = p_notify->additional_parameter[index++];
        type = p_notify->additional_parameter[index++];
        switch (type) {
            case MATADATA_TYPE_STREAMING_AUDIO_CONTEXTS: {
                memcpy(&p_info->ase[ase_idx].metadata.streaming_audio_contexts, &p_notify->additional_parameter[index], sizeof(uint16_t));
                index += sizeof(uint16_t);
                break;
            }
            default:
                index += (length - 1);
                break;
        }
        metadata_length -= (length + 1);
    }

    LE_AUDIO_MSGLOG_I("[APP][ASCS] handle:%x ASE[%d],cig_id:%02x cis_id:%02x streaming_audio_contexts:%04x", 5,
        p_info->handle,
        ase_idx,
        p_info->ase[ase_idx].metadata.cig_id,
        p_info->ase[ase_idx].metadata.cis_id,
        p_info->ase[ase_idx].metadata.streaming_audio_contexts);

    switch (p_info->curr_state) {
        case APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE == p_info->next_state) {
                if (p_info->wait_event.wait_ase_event > 0) {
                    p_info->wait_event.wait_ase_event--;

                    if ((0 != p_info->wait_event.wait_ase_event) || (0 != p_info->wait_event.wait_ase_cp_event)) {
                        break;
                    }

                    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE;
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;

                    uint8_t i, tmp;
                    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
                            continue;
                        }
#else
                        i = tmp;
#endif
                        p_info = &g_lea_ucst_link_info[i];
                        if ((BT_HANDLE_INVALID != p_info->handle) && (p_info->handle != p_notify->handle)) {
                            LE_AUDIO_MSGLOG_I("[APP] ENABLING check link, handle:%x state:%x->%x w_ase:%x", 4, p_info->handle,
                                              p_info->curr_state, p_info->next_state,
                                              p_info->wait_event.wait_ase_event);

                            if ((APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE == p_info->next_state) ||
                                (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS == p_info->next_state)) {
                                return;
                            }
                        }
                    }

                    /* start audio stream */
                    LE_AUDIO_MSGLOG_I("[APP] ENABLING, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

                    if (app_le_audio_ucst_check_pause_stream() ||
                        (APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
                        (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
                        app_le_audio_ucst_disable_ase_when_state_match(APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE);
                        break;
                    }

                    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
                            continue;
                        }
#else
                        i = tmp;
#endif
                        p_info = &g_lea_ucst_link_info[i];
                        if ((BT_HANDLE_INVALID != p_info->handle) &&
                            (APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE == p_info->curr_state) &&
                            (APP_LE_AUDIO_UCST_LINK_STATE_IDLE == p_info->next_state) &&
                            (false == p_info->ase_releasing)) {
                            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS;
                            LE_AUDIO_MSGLOG_I("[APP] ENABLING, check handle:%x state:%x->%x", 3, p_info->handle,
                                              p_info->curr_state, p_info->next_state);
                        }
                    }
                    app_le_audio_ucst_create_cis();
                }
            }
            break;
        }
        default:
            break;
    }
}


static void app_le_audio_ucst_handle_ase_streaming(ble_bap_ase_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint16_t index = 0;
    uint8_t metadata_length = 0;
    uint8_t length,type;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][ASE] STREAMING, handle:%x state:%x->%x ase_id:%x w_ase:%x r:%x", 6,
                      p_info->handle,
                      p_info->curr_state, p_info->next_state,
                      p_notify->ase_id,
                      p_info->wait_event.wait_ase_event,
                      p_info->ase_releasing);
    /* update ase state */
    uint8_t ase_idx;
    if (APP_LE_AUDIO_UCST_ASE_MAX_NUM == (ase_idx = app_le_audio_ucst_get_ase_idx(p_info, p_notify->ase_id))) {
        return;
    }
    p_info->ase[ase_idx].curr_state = ASE_STATE_STREAMING;
    p_info->ase[ase_idx].metadata.cig_id = p_notify->additional_parameter[index++];
    p_info->ase[ase_idx].metadata.cis_id = p_notify->additional_parameter[index++];
    metadata_length = p_notify->additional_parameter[index++];
    while(metadata_length > 0) {
        length = p_notify->additional_parameter[index++];
        type = p_notify->additional_parameter[index++];
        switch (type) {
            case MATADATA_TYPE_STREAMING_AUDIO_CONTEXTS: {
                memcpy(&p_info->ase[ase_idx].metadata.streaming_audio_contexts, &p_notify->additional_parameter[index], sizeof(uint16_t));
                index += sizeof(uint16_t);
                break;
            }
            default:
                index += (length - 1);
                break;
        }
        metadata_length -= (length + 1);
    }

    LE_AUDIO_MSGLOG_I("[APP][ASCS] handle:%x ASE[%d],cig_id:%02x cis_id:%02x streaming_audio_contexts:%04x", 5,
        p_info->handle,
        ase_idx,
        p_info->ase[ase_idx].metadata.cig_id,
        p_info->ase[ase_idx].metadata.cis_id,
        p_info->ase[ase_idx].metadata.streaming_audio_contexts);

    switch (p_info->curr_state) {
        case APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS: {
            if (AUDIO_DIRECTION_SINK == p_notify->direction) {
                bt_gap_le_iso_data_path_id_t iso_data_path_id = 0x01;
                uint8_t cis_idx = 0;
                bt_status_t ret = BT_STATUS_FAIL;

                if (APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE == p_info->next_state) {
                    break;
                }

                if ((1 == g_lea_ucst_ctrl.cis_num) && (AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location)) {
                    iso_data_path_id = 0x02;
                } else if (((1 == p_info->sink_location_num) && (AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location)) ||
                    ((2 == p_info->sink_location_num) && (0 != (ase_idx % 2)))) {
                    iso_data_path_id = 0x02;
                    cis_idx = 1;
                }

                LE_AUDIO_MSGLOG_I("[APP] STREAMING, handle:%x sink_location:%x(num:%x) iso_data_path_id:%x cis_idx:%x", 5,
                                  p_info->handle,
                                  p_info->sink_location,
                                  p_info->sink_location_num,
                                  iso_data_path_id,
                                  cis_idx);

                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH;
                g_lea_ucst_cis_info[cis_idx].cis_status = APP_LE_AUDIO_UCST_CIS_SETUP_DATA_PATH;
                ret = app_le_audio_setup_iso_data_path(g_lea_ucst_cis_info[cis_idx].cis_handle, BT_GAP_LE_ISO_DATA_PATH_DIRECTION_OUTPUT, iso_data_path_id);
                if (BT_STATUS_SUCCESS != ret &&
                    BT_STATUS_PENDING != ret) {
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                    g_lea_ucst_cis_info[cis_idx].cis_status = APP_LE_AUDIO_UCST_CIS_CREATED;
                }
            }
            break;
        }
        case APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH: {
            if ((APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_START_READY == p_info->next_state) &&
                (AUDIO_DIRECTION_SOURCE == p_notify->direction)) {
                if (p_info->wait_event.wait_ase_event > 0) {
                    p_info->wait_event.wait_ase_event--;

                    if ((0 != p_info->wait_event.wait_ase_event) || (0 != p_info->wait_event.wait_ase_cp_event)) {
                        break;
                    }

                    LE_AUDIO_MSGLOG_I("[APP] STREAMING, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

                    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_STREAMING;
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                    g_lea_ucst_ctrl.curr_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING;
                    g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;

                    if (app_le_audio_ucst_check_pause_stream() ||
                        (APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
                        (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
                        app_le_audio_ucst_disable_ase_when_setup_iso_data_path();
                        return;
                    }

                }
            }
            break;
        }
        default:
            break;
    }
}

static void app_le_audio_ucst_handle_ase_disabling(ble_bap_ase_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint16_t index = 0;
    uint8_t metadata_length = 0;
    uint8_t length,type;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][ASE] DISABLING, handle:%x state:%x->%x ase_id:%x w_ase:%x r:%x", 6,
                      p_info->handle,
                      p_info->curr_state, p_info->next_state,
                      p_notify->ase_id,
                      p_info->wait_event.wait_ase_event,
                      p_info->ase_releasing);
    /* update ase state */
    uint8_t ase_idx;
    if (APP_LE_AUDIO_UCST_ASE_MAX_NUM == (ase_idx = app_le_audio_ucst_get_ase_idx(p_info, p_notify->ase_id))) {
        return;
    }
    p_info->ase[ase_idx].curr_state = ASE_STATE_DISABLING;
    p_info->ase[ase_idx].metadata.cig_id = p_notify->additional_parameter[index++];
    p_info->ase[ase_idx].metadata.cis_id = p_notify->additional_parameter[index++];
    metadata_length = p_notify->additional_parameter[index++];
    while(metadata_length > 0) {
        length = p_notify->additional_parameter[index++];
        type = p_notify->additional_parameter[index++];
        switch (type) {
            case MATADATA_TYPE_STREAMING_AUDIO_CONTEXTS: {
                memcpy(&p_info->ase[ase_idx].metadata.streaming_audio_contexts, &p_notify->additional_parameter[index], sizeof(uint16_t));
                index += sizeof(uint16_t);
                break;
            }
            default:
                index += (length - 1);
                break;
        }
        metadata_length -= (length + 1);
    }

    LE_AUDIO_MSGLOG_I("[APP][ASCS] handle:%x ASE[%d],cig_id:%02x cis_id:%02x streaming_audio_contexts:%04x", 5,
        p_info->handle,
        ase_idx,
        p_info->ase[ase_idx].metadata.cig_id,
        p_info->ase[ase_idx].metadata.cis_id,
        p_info->ase[ase_idx].metadata.streaming_audio_contexts);

    switch (p_info->curr_state) {
        case APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE: {
            /* handle switch bis or stop stream */
            if (APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE == p_info->next_state) {
                if (p_info->wait_event.wait_ase_event > 0) {
                    p_info->wait_event.wait_ase_event--;

                    if ((0 != p_info->wait_event.wait_ase_event) || (0 != p_info->wait_event.wait_ase_cp_event)) {
                        break;
                    }

                    LE_AUDIO_MSGLOG_I("[APP] DISABLING, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

                    if (((APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) ||
                         (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) &&
                        (0 != app_le_audio_ucst_get_source_ase_num(p_info))) {
                        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY;
                        if (BT_STATUS_SUCCESS != app_le_audio_ucst_set_receiver_stop_ready(p_info->handle)) {
                            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                        }

                    } else {
                        p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
                        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                        app_le_audio_ucst_check_close_audio_stream();
                    }

                }
            }
            break;
        }
        case APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS:
        case APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH:
        case APP_LE_AUDIO_UCST_LINK_STATE_STREAMING: {
            /* handle disable stream (source ase) */
            if (APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE == p_info->next_state) {
                if (p_info->wait_event.wait_ase_event > 0) {
                    p_info->wait_event.wait_ase_event--;

                    if ((0 != p_info->wait_event.wait_ase_event) || (0 != p_info->wait_event.wait_ase_cp_event)) {
                        break;
                    }

                    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE;
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;

                    LE_AUDIO_MSGLOG_I("[APP] DISABLING, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

                    if (((APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) ||
                         (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) &&
                        (0 != app_le_audio_ucst_get_source_ase_num(p_info))) {
                        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY;
                        if (BT_STATUS_SUCCESS != app_le_audio_ucst_set_receiver_stop_ready(p_info->handle)) {
                            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                        }
                    } else {
                        p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY;
                        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_CIS;
                        if (BT_STATUS_SUCCESS != app_le_audio_ucst_disconnect_cis(p_info->handle)) {
                            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }

}

static void app_le_audio_ucst_handle_ase_releasing(ble_bap_ase_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint16_t context_type;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][ASE] RELEASING, handle:%x state:%x->%x ase_id:%x w_ase:%x r:%x", 6,
                      p_info->handle,
                      p_info->curr_state, p_info->next_state,
                      p_notify->ase_id,
                      p_info->wait_event.wait_ase_event,
                      p_info->ase_releasing);

    /* update ase state */
    uint8_t ase_idx;
    if (APP_LE_AUDIO_UCST_ASE_MAX_NUM == (ase_idx = app_le_audio_ucst_get_ase_idx(p_info, p_notify->ase_id))) {
        return;
    }
    p_info->ase[ase_idx].curr_state = ASE_STATE_RELEASING;

    context_type = app_le_audio_ucst_get_audio_context_type();
    if (!(context_type & p_info->ase[ase_idx].codec_state.audio_contexts)) {
        return;
    }

    if (!p_info->ase_releasing) {
        p_info->ase_releasing = true;
        p_info->release = true;
        if ((APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC <= p_info->curr_state) ||
            (APP_LE_AUDIO_UCST_LINK_STATE_STREAMING >= p_info->curr_state)) {
            /*
            if ((APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) ||
                (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
                p_info->wait_event.wait_ase_event = 2;
            } else {
                p_info->wait_event.wait_ase_event = 1;
            }
            */
            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
        }
    }
    p_info->wait_event.wait_ase_event += 1;
    g_lea_ucst_ctrl.release = true;
}

static void app_le_audio_ucst_handle_ase_control_point_notify(ble_ascs_control_point_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    switch (p_notify->opcode) {
        case ASE_OPCODE_CONFIG_CODEC: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC != p_info->next_state) {
                return;
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] CONTROL_POINT_NOTIFY, CONFIG_CODEC handle:%x", 1, p_info->handle);
            break;
        }
        case ASE_OPCODE_CONFIG_QOS: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS != p_info->next_state) {
                return;
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] CONTROL_POINT_NOTIFY, CONFIG_QOS handle:%x", 1, p_info->handle);
            break;
        }
        case ASE_OPCODE_ENABLE: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE != p_info->next_state) {
                return;
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] CONTROL_POINT_NOTIFY, ENABLE handle:%x", 1, p_info->handle);
            uint8_t j;
            for (j = 0 ; j < p_notify->number_of_ases; j++) {
                if (RESPONSE_CODE_SUCCESS != p_notify->response[j].response_code) {
                    app_le_audio_ucst_send_prepare_vcmd(p_info->handle, APP_LE_AUDIO_PREPARE_VCMD_MODE_CONN, 0);
                }
                if (RESPONSE_CODE_INSUFFICIENT_RESOURCES == p_notify->response[j].response_code) {
                    LE_AUDIO_MSGLOG_I("[APP][ASE] CONTROL_POINT_NOTIFY, ENABLE ase_id:%d rsp:INSUFFICIENT_RESOURCES reason:0x%x", 2, p_notify->response[j].ase_id, p_notify->response[j].reason);
                    /* To do: change to qos */
                    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                    p_info->wait_event.wait_ase_cp_event = 0;
                    p_info->wait_event.wait_ase_event = 0;
                    app_le_audio_ucst_stop(false);

                    if (APP_LE_AUDIO_UCST_STREAM_STATE_STOP_STREAMING != g_lea_ucst_ctrl.next_stream_state) {
                        app_le_audio_ucst_check_close_audio_stream();
                    }
                    break;
                }
            }
            break;
        }
        case ASE_OPCODE_RECEIVER_START_READY: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_START_READY != p_info->next_state) {
                return;
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] CONTROL_POINT_NOTIFY, RECEIVER_START_READY handle:%x", 1, p_info->handle);
            break;
        }
        case ASE_OPCODE_DISABLE: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE != p_info->next_state) {
                return;
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] CONTROL_POINT_NOTIFY, DISABLE handle:%x", 1, p_info->handle);
            break;
        }
        case ASE_OPCODE_RECEIVER_STOP_READY: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY != p_info->next_state) {
                return;
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] CONTROL_POINT_NOTIFY, RECEIVER_STOP_READY handle:%x", 1, p_info->handle);
            break;
        }
        case ASE_OPCODE_UPDATE_METADATA: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_UPDATE_ASE_METADATA != p_info->next_state) {
                return;
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] CONTROL_POINT_NOTIFY, UPDATE_METADATA handle:%x", 1, p_info->handle);
            break;
        }
        case ASE_OPCODE_RELEASE: {
            if (APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE != p_info->next_state) {
                return;
            }
            LE_AUDIO_MSGLOG_I("[APP][ASE] CONTROL_POINT_NOTIFY, RELEASE handle:%x", 1, p_info->handle);
            break;
        }

        default: {
            return;
        }

    }

    uint8_t i;
    for (i = 0 ; i < p_notify->number_of_ases; i++) {
        if (RESPONSE_CODE_SUCCESS != p_notify->response[i].response_code) {
            LE_AUDIO_MSGLOG_I("[APP][ASE] CONTROL_POINT_NOTIFY ERROR!!, ase_id:%d rsp:0x%x reason:0x%x", 3, p_notify->response[i].ase_id, p_notify->response[i].response_code, p_notify->response[i].reason);
        }
    }

    if (p_info->wait_event.wait_ase_cp_event > 0) {
        p_info->wait_event.wait_ase_cp_event--;
    }
}

static void app_le_audio_ucst_handle_bap_discover_service_complete(ble_bap_discover_service_complete_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][U] handle_bap_discover_service_complete, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }
    //APP_LE_AUDIO_UCST_ASE_MAX_NUM = sink_ase_num + source_ase_num
    p_info->sink_ase_num = cnf->number_of_sink_ases;
    p_info->source_ase_num = cnf->number_of_source_ases;
    //p_info->source_ase_idx = p_info->sink_ase_num;

    LE_AUDIO_MSGLOG_I("[APP][U] handle_bap_discover_service_complete, handle:%x state:%x->%x ase_num(sink:%x source:%x)", 5, p_info->handle,
                      p_info->curr_state, p_info->next_state,
                      cnf->number_of_sink_ases, cnf->number_of_source_ases);

    bt_status_t ret;

    /* enter next step */
    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_AVAILABLE_AUDIO_CONTEXTS;
    if (BT_STATUS_SUCCESS != (ret = ble_bap_pacs_read_available_audio_contexts_req(p_info->handle))) {
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
        LE_AUDIO_MSGLOG_I("[APP] read_available_audio_contexts_req failed, handle:%x ret:%x", 2, p_info->handle, ret);
    }
}

static void app_le_audio_ucst_handle_bap_pacs_discovery_all(app_le_audio_ucst_link_info_t *p_info)
{
    bt_status_t ret;

    if (NULL == p_info) {
        return;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_SUPPORTED_AUDIO_CONTEXTS == p_info->next_state) {
        if (BT_STATUS_SUCCESS == (ret = ble_bap_pacs_read_supported_audio_contexts_req(p_info->handle))) {
            return;
        }
        LE_AUDIO_MSGLOG_I("[APP] read_supported_audio_contexts_req failed, handle:%x ret:%x", 2, p_info->handle, ret);
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_PAC;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_PAC == p_info->next_state) {
        if (BT_STATUS_SUCCESS == (ret = ble_bap_pacs_read_sink_pac_req(p_info->handle))) {
            return;
        }
        LE_AUDIO_MSGLOG_I("[APP] read_sink_pac_req failed, handle:%x ret:%x", 2, p_info->handle, ret);
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_LOCATION;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_LOCATION == p_info->next_state) {
        if (BT_STATUS_SUCCESS == (ret = ble_bap_pacs_read_sink_location_req(p_info->handle))) {
            return;
        }
        LE_AUDIO_MSGLOG_I("[APP] read_sink_location_req failed, handle:%x ret:%x", 2, p_info->handle, ret);
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_PAC;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_PAC == p_info->next_state) {
        if (BT_STATUS_SUCCESS == (ret = ble_bap_pacs_read_source_pac_req(p_info->handle))) {
            return;
        }
        LE_AUDIO_MSGLOG_I("[APP] read_source_pac_req failed, handle:%x ret:%x", 2, p_info->handle, ret);
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_LOCATION;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_LOCATION == p_info->next_state) {
        if (BT_STATUS_SUCCESS == (ret = ble_bap_pacs_read_source_location_req(p_info->handle))) {
            return;
        }
        LE_AUDIO_MSGLOG_I("[APP] read_source_location_req failed, handle:%x ret:%x", 2, p_info->handle, ret);
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE == p_info->next_state) {
        p_info->read_sink_ase_num = 1;
        p_info->read_source_ase_num = 1;
        if (BT_STATUS_SUCCESS == (ret = ble_bap_ascs_read_ase_value_req(p_info->handle, 1))) {
            return;
        }
        LE_AUDIO_MSGLOG_I("[APP] read_ase_value_req failed, handle:%x ret:%x", 2, p_info->handle, ret);
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
        p_info->read_sink_ase_num = 0;
        p_info->read_source_ase_num = 0;
    }
}

static void app_le_audio_ucst_handle_read_available_audio_contexts_cnf(ble_bap_read_available_audio_contexts_cnf_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][PACS] AVAILABLE_AUDIO_CONTEXTS, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }

    p_info->sink_available_contexts = cnf->sink_contexts;
    p_info->source_available_contexts = cnf->source_contexts;

    LE_AUDIO_MSGLOG_I("[APP][PACS] AVAILABLE_AUDIO_CONTEXTS, handle:%x sink:0x%x source:0x%x", 3, p_info->handle,
                      p_info->sink_available_contexts,
                      p_info->source_available_contexts);

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_AVAILABLE_AUDIO_CONTEXTS != p_info->next_state) {
        LE_AUDIO_MSGLOG_I("[APP] AVAILABLE_AUDIO_CONTEXTS state NOT match, handle:%x state:%x->%x", 2, p_info->handle, p_info->curr_state, p_info->next_state);
        return;
    }

    /* enter next step */
    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_AVAILABLE_AUDIO_CONTEXTS;
    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SUPPORTED_AUDIO_CONTEXTS;
    app_le_audio_ucst_handle_bap_pacs_discovery_all(p_info);

}

static void app_le_audio_ucst_handle_read_supported_audio_contexts_cnf(ble_bap_read_supported_audio_contexts_cnf_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][PACS] SUPPORTED_AUDIO_CONTEXTS, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }

    p_info->sink_supported_contexts = cnf->sink_contexts;
    p_info->source_supported_contexts = cnf->source_contexts;

    LE_AUDIO_MSGLOG_I("[APP][PACS] SUPPORTED_AUDIO_CONTEXTS, handle:%x sink:0x%x source:0x%x", 3, p_info->handle,
                      p_info->sink_supported_contexts,
                      p_info->source_supported_contexts);

    if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode) {
        /* [Switch streaming mode] UCST -> BCST */
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL;
        app_le_audio_ucst_disconnect(cnf->handle);
        return;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_SUPPORTED_AUDIO_CONTEXTS != p_info->next_state) {
        LE_AUDIO_MSGLOG_I("[APP] SUPPORTED_AUDIO_CONTEXTS state NOT match, handle:%x state:%x->%x", 2, p_info->handle, p_info->curr_state, p_info->next_state);
        return;
    }

    /* enter next step */
    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SUPPORTED_AUDIO_CONTEXTS;
    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_PAC;
    app_le_audio_ucst_handle_bap_pacs_discovery_all(p_info);
}

static void app_le_audio_ucst_handle_read_sink_pac_cnf(ble_bap_read_sink_pac_cnf_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    app_le_audio_pac_record_t *p_pac_record = NULL;
    uint16_t offset = 0;
    uint8_t i,j;
    uint8_t codec_specific_capabilities_len;
    uint8_t metadata_len;
    uint8_t length,type;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][PACS] SINK_PAC, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }

    //p_info->sink_pac.is_supported = true;
    p_info->sink_pac.num_of_record = cnf->num_of_record;
    //p_info->sink_pac.pac_record_length = cnf->pac_record_length;

    LE_AUDIO_MSGLOG_I("[APP][PACS] SINK_PAC, handle:%x pac_num:%x pac_len:%d", 3, p_info->handle, cnf->num_of_record, cnf->pac_record_length);

    if (NULL == (p_info->sink_pac.pac_record = (app_le_audio_pac_record_t *)pvPortMalloc(cnf->num_of_record * sizeof(app_le_audio_pac_record_t)))) {
        LE_AUDIO_MSGLOG_I("[APP][PACS] SINK_PAC, out of memory", 0);
        return;
    }

    p_pac_record = p_info->sink_pac.pac_record;
    memset(p_pac_record, 0, cnf->num_of_record * sizeof(app_le_audio_pac_record_t));
    for (i = 0; i < cnf->num_of_record; i++) {
        //set default value
        p_pac_record[i].supported_audio_channel_counts = 0x01;
        p_pac_record[i].supported_codec_frame_blocks_per_sdu = 0x01;
        //Codec_ID
        memcpy(p_pac_record[i].codec_id, &(cnf->pac_record[offset]), AUDIO_CODEC_ID_SIZE);
        offset += AUDIO_CODEC_ID_SIZE;
        //Codec_Specific_Capabilities
        codec_specific_capabilities_len = cnf->pac_record[offset];
        offset++;
        for (j = 0; j < codec_specific_capabilities_len; ) {
            length = cnf->pac_record[offset + j];
            j++;
            type = cnf->pac_record[offset + j];
            j++;

            //LE_AUDIO_MSGLOG_I("[APP][PACS] L:%x T:%x D:%02x %02x %02x %02x", 6, length, type, cnf->pac_record[offset + j],cnf->pac_record[offset + j+1],cnf->pac_record[offset + j+2],cnf->pac_record[offset + j+3]);
            switch (type) {
                case CODEC_SPECIFIC_CAPABILITIIES_TYPE_SAMPLING_FREQUENCY: {
                    memcpy(&p_pac_record[i].supported_sampling_frequencies, &(cnf->pac_record[offset + j]), sizeof(uint16_t));
                    j += 2;
                    break;
                }
                case CODEC_SPECIFIC_CAPABILITIIES_TYPE_FRAME_DURATIONS: {
                    p_pac_record[i].supported_frame_durations = cnf->pac_record[offset + j];
                    j++;
                    break;
                }
                case CODEC_SPECIFIC_CAPABILITIIES_TYPE_AUDIO_CHANNEL_COUNTS: {
                    p_pac_record[i].supported_audio_channel_counts = cnf->pac_record[offset + j];
                    j++;
                    break;
                }
                case CODEC_SPECIFIC_CAPABILITIIES_TYPE_OCTETS_PER_CODEC_FRAME: {
                    memcpy(&p_pac_record[i].supported_octets_per_codec_frame_min, &(cnf->pac_record[offset + j]), sizeof(uint16_t));
                    j += 2;
                    memcpy(&p_pac_record[i].supported_octets_per_codec_frame_max, &(cnf->pac_record[offset + j]), sizeof(uint16_t));
                    j += 2;
                    break;
                }
                case CODEC_SPECIFIC_CAPABILITIIES_TYPE_CODEC_FRAME_BLOCKS_PER_SDU: {
                    p_pac_record[i].supported_codec_frame_blocks_per_sdu = cnf->pac_record[offset + j];
                    j++;
                    break;
                }
                default:
                    j += (length - 1);
                    break;
            }
        }

        offset += codec_specific_capabilities_len;
        //Metadata
        metadata_len = cnf->pac_record[offset];
        offset++;
        for (j = 0; j < metadata_len; ) {
            length = cnf->pac_record[offset + j];
            j++;
            type = cnf->pac_record[offset + j];
            j++;
            if ((MATADATA_TYPE_PREFERRED_AUDIO_CONTEXTS == type) && (3 == length)) {
                memcpy(&p_pac_record[i].preferred_audio_contexts, &(cnf->pac_record[offset + j]), sizeof(uint16_t));
                j += 2;
            }
            else {
                j += (length - 1);
            }
        }
        offset += metadata_len;

    }

    for (i = 0; i < cnf->num_of_record; i++) {
        LE_AUDIO_MSGLOG_I("[APP][PACS] handle:%x SINK_PAC[%d],frame_durations:%02x channel_counts:%02x blocks_per_sdu:%02x sampling_freq:%04x CF_size:%04x~%04x preferred:%04x", 9,
            p_info->handle,
            i,
            p_pac_record[i].supported_frame_durations,
            p_pac_record[i].supported_audio_channel_counts,
            p_pac_record[i].supported_codec_frame_blocks_per_sdu,
            p_pac_record[i].supported_sampling_frequencies,
            p_pac_record[i].supported_octets_per_codec_frame_min,
            p_pac_record[i].supported_octets_per_codec_frame_max,
            p_pac_record[i].preferred_audio_contexts);
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_PAC != p_info->next_state) {
        LE_AUDIO_MSGLOG_I("[APP] SINK_PAC state NOT match, handle:%x state:%x->%x", 2, p_info->handle, p_info->curr_state, p_info->next_state);
        return;
    }

    /* enter next step */
    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_PAC;
    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_LOCATION;
    app_le_audio_ucst_handle_bap_pacs_discovery_all(p_info);
}

static void app_le_audio_ucst_handle_read_sink_location_cnf(ble_bap_read_sink_location_cnf_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][PACS] SINK_LOCATION, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }

    p_info->sink_location = cnf->location & 0x0F;
    p_info->sink_location_num = app_le_audio_ucst_get_location_count(p_info->sink_location);

    LE_AUDIO_MSGLOG_I("[APP][PACS] SINK_LOCATION, handle:%x location:0x%x(num:%x)", 3, p_info->handle, cnf->location, p_info->sink_location_num);

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_LOCATION != p_info->next_state) {
        LE_AUDIO_MSGLOG_I("[APP] SINK_LOCATION state NOT match, handle:%x state:%x->%x", 2, p_info->handle, p_info->curr_state, p_info->next_state);
        return;
    }

    /* enter next step */
    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_LOCATION;
    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_PAC;
    app_le_audio_ucst_handle_bap_pacs_discovery_all(p_info);
}

static void app_le_audio_ucst_handle_read_source_pac_cnf(ble_bap_read_source_pac_cnf_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    app_le_audio_pac_record_t *p_pac_record = NULL;
    uint16_t offset = 0;
    uint8_t i,j;
    uint8_t codec_specific_capabilities_len;
    uint8_t metadata_len;
    uint8_t length,type;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][PACS] SOURCE_PAC, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }

    p_info->source_pac.num_of_record = cnf->num_of_record;
    //p_info->source_pac.pac_record_length = cnf->pac_record_length;

    LE_AUDIO_MSGLOG_I("[APP][PACS] SOURCE_PAC, handle:%x pac_num:%x pac_len:%d", 3, p_info->handle, cnf->num_of_record, cnf->pac_record_length);
    if (NULL == (p_info->source_pac.pac_record = (app_le_audio_pac_record_t *)pvPortMalloc(cnf->num_of_record * sizeof(app_le_audio_pac_record_t)))) {
        LE_AUDIO_MSGLOG_I("[APP][PACS] SOURCE_PAC, out of memory", 0);
        return;
    }

    p_pac_record = p_info->source_pac.pac_record;
    memset(p_pac_record, 0, cnf->num_of_record * sizeof(app_le_audio_pac_record_t));
    for (i = 0; i < cnf->num_of_record; i++) {
        //set default value
        p_pac_record[i].supported_audio_channel_counts = 0;
        p_pac_record[i].supported_codec_frame_blocks_per_sdu = 0x01;
        //Codec_ID
        memcpy(p_pac_record[i].codec_id, &(cnf->pac_record[offset]), AUDIO_CODEC_ID_SIZE);
        offset += AUDIO_CODEC_ID_SIZE;
        //Codec_Specific_Capabilities
        codec_specific_capabilities_len = cnf->pac_record[offset];
        offset++;
        for (j = 0; j < codec_specific_capabilities_len; ) {
            length = cnf->pac_record[offset + j];
            j++;
            type = cnf->pac_record[offset + j];
            j++;
            switch (type) {
                case CODEC_SPECIFIC_CAPABILITIIES_TYPE_SAMPLING_FREQUENCY: {
                    memcpy(&p_pac_record[i].supported_sampling_frequencies, &(cnf->pac_record[offset + j]), sizeof(uint16_t));
                    j += 2;
                    break;
                }
                case CODEC_SPECIFIC_CAPABILITIIES_TYPE_FRAME_DURATIONS: {
                    p_pac_record[i].supported_frame_durations = cnf->pac_record[offset + j];
                    j++;
                    break;
                }
                case CODEC_SPECIFIC_CAPABILITIIES_TYPE_AUDIO_CHANNEL_COUNTS: {
                    p_pac_record[i].supported_audio_channel_counts = cnf->pac_record[offset + j];
                    j++;
                    break;
                }
                case CODEC_SPECIFIC_CAPABILITIIES_TYPE_OCTETS_PER_CODEC_FRAME: {
                    memcpy(&p_pac_record[i].supported_octets_per_codec_frame_min, &(cnf->pac_record[offset + j]), sizeof(uint16_t));
                    j += 2;
                    memcpy(&p_pac_record[i].supported_octets_per_codec_frame_max, &(cnf->pac_record[offset + j]), sizeof(uint16_t));
                    j += 2;
                    break;
                }
                case CODEC_SPECIFIC_CAPABILITIIES_TYPE_CODEC_FRAME_BLOCKS_PER_SDU: {
                    p_pac_record[i].supported_codec_frame_blocks_per_sdu = cnf->pac_record[offset + j];
                    j++;
                    break;
                }
                default:
                    j += (length - 1);
                    break;
            }
        }

        offset += codec_specific_capabilities_len;
        //Metadata
        metadata_len = cnf->pac_record[offset];
        offset++;
        for (j = 0; j < metadata_len; ) {
            length = cnf->pac_record[offset + j];
            j++;
            type = cnf->pac_record[offset + j];
            j++;
            if ((MATADATA_TYPE_PREFERRED_AUDIO_CONTEXTS == type) && (3 == length)) {
                memcpy(&p_pac_record[i].preferred_audio_contexts, &(cnf->pac_record[offset + j]), sizeof(uint16_t));
                j += 2;
            }
            else {
                j += (length - 1);
            }
        }
        offset += metadata_len;

    }

    for (i = 0; i < cnf->num_of_record; i++) {
            LE_AUDIO_MSGLOG_I("[APP][PACS] handle:%x SOURCE_PAC[%d],frame_durations:%02x channel_counts:%02x blocks_per_sdu:%02x sampling_freq:%04x CF_size:%04x~%04x preferred:%04x", 9,
            p_info->handle,
            i,
            p_pac_record[i].supported_frame_durations,
            p_pac_record[i].supported_audio_channel_counts,
            p_pac_record[i].supported_codec_frame_blocks_per_sdu,
            p_pac_record[i].supported_sampling_frequencies,
            p_pac_record[i].supported_octets_per_codec_frame_min,
            p_pac_record[i].supported_octets_per_codec_frame_max,
            p_pac_record[i].preferred_audio_contexts);
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_PAC != p_info->next_state) {
        LE_AUDIO_MSGLOG_I("[APP] SOURCE_PAC state NOT match, handle:%x state:%x->%x", 2, p_info->handle, p_info->curr_state, p_info->next_state);
        return;
    }

    /* enter next step */
    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_PAC;
    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_LOCATION;
    app_le_audio_ucst_handle_bap_pacs_discovery_all(p_info);
}

static void app_le_audio_ucst_handle_read_source_location_cnf(ble_bap_read_source_location_cnf_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][PACS] SOURCE_LOCATION, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }

    p_info->source_location = cnf->location & 0x0F;
    p_info->source_location_num = app_le_audio_ucst_get_location_count(p_info->source_location);

    LE_AUDIO_MSGLOG_I("[APP][PACS] SOURCE_LOCATION, handle:%x location:0x%x(num:%x)", 3, p_info->handle, cnf->location, p_info->source_location_num);

    if (!((APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING == g_lea_ucst_ctrl.curr_stream_state) &&
          (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state))) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (p_info->group_id == g_lea_ucst_ctrl.curr_group)
#endif
        {
            app_le_audio_ucst_set_mic_channel();
        }
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_LOCATION != p_info->next_state) {
        LE_AUDIO_MSGLOG_I("[APP] SOURCE_LOCATION state NOT match, handle:%x state:%x->%x", 2, p_info->handle, p_info->curr_state, p_info->next_state);
        return;
    }

    /* enter next step */
    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_LOCATION;
    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE;
    app_le_audio_ucst_handle_bap_pacs_discovery_all(p_info);
}

static void app_le_audio_ucst_handle_available_audio_contexts_notify(ble_bap_available_audio_contexts_notify_t *notify)
{
    uint8_t i, link_idx;
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(notify->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][PACS] AVAILABLE_AUDIO_CONTEXTS Notify, link not exist (hdl:%x)", 1, notify->handle);
        return;
    }

    p_info->sink_available_contexts = notify->sink_contexts;
    p_info->source_available_contexts = notify->source_contexts;

    LE_AUDIO_MSGLOG_I("[APP][PACS] AVAILABLE_AUDIO_CONTEXTS Notify, handle:%x sink:0x%x source:0x%x", 3, p_info->handle,
                      p_info->sink_available_contexts,
                      p_info->source_available_contexts);
      /* find the other link */
    for (i = 0; i < app_le_audio_ucst_get_max_link_num(); i++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (g_lea_ucst_ctrl.curr_group > APP_LE_AUDIO_UCST_GROUP_ID_MAX) {
            return;
        }
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (link_idx = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[i])) {
            continue;
        }
#else
        link_idx = i;
#endif
        if ((BT_HANDLE_INVALID != g_lea_ucst_link_info[link_idx].handle) && (notify->handle != g_lea_ucst_link_info[link_idx].handle)) {
            if ((p_info->sink_available_contexts != g_lea_ucst_link_info[link_idx].sink_available_contexts) ||
                (p_info->source_available_contexts != g_lea_ucst_link_info[link_idx].source_available_contexts)) {
                LE_AUDIO_MSGLOG_I("[APP][PACS] AVAILABLE_AUDIO_CONTEXTS other link, handle:%x sink:0x%x source:0x%x", 3, p_info->handle,
                                  p_info->sink_available_contexts,
                                  p_info->source_available_contexts);
                return;
            }
        }
    }
    app_le_audio_ucst_start();
}

static void app_le_audio_ucst_handle_read_ase_cnf(ble_ascs_read_ase_cnf_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][ASCS] READ_ASE_CNF, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE != p_info->next_state) {
        LE_AUDIO_MSGLOG_I("[APP][ASCS] READ_ASE_CNF state NOT match, handle:%x state:%x->%x", 2, p_info->handle, p_info->curr_state, p_info->next_state);
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][ASCS] READ_ASE_CNF, ase_id:%x ase_state:%x", 2, cnf->ase_id, cnf->ase_state);

    if ((p_info->read_sink_ase_num <= p_info->sink_ase_num) && (p_info->read_sink_ase_num <= 4)) {
        app_le_audio_ucst_set_ase(p_info, cnf->ase_id, cnf->ase_state, AUDIO_DIRECTION_SINK);
        p_info->read_sink_ase_num++;
    } else {
        app_le_audio_ucst_set_ase(p_info, cnf->ase_id, cnf->ase_state, AUDIO_DIRECTION_SOURCE);
        p_info->read_source_ase_num++;
    }

    if ((p_info->read_sink_ase_num <= p_info->sink_ase_num) && (p_info->read_sink_ase_num <= 4)) {
        ble_bap_ascs_read_ase_value_req(p_info->handle, p_info->read_sink_ase_num);
    }
    else if ((p_info->read_source_ase_num <= p_info->source_ase_num) && (p_info->read_source_ase_num <= (APP_LE_AUDIO_UCST_ASE_MAX_NUM - 4))) {
        ble_bap_ascs_read_ase_value_req(p_info->handle, (p_info->sink_ase_num + p_info->read_source_ase_num));
    }
    else {
        bt_status_t ret;
        if (p_info->sink_ase_num > 4) {
            p_info->sink_ase_num = 4;
        }

        if (p_info->source_ase_num > (APP_LE_AUDIO_UCST_ASE_MAX_NUM - 4)) {
            p_info->source_ase_num = (APP_LE_AUDIO_UCST_ASE_MAX_NUM - 4);
        }
        p_info->source_ase_idx = p_info->sink_ase_num;

        /* enter next step */
        p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE;
#ifdef AIR_LE_AUDIO_BA_ENABLE
        if ((APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode)
            || (APP_LE_AUDIO_MODE_ASIT == g_lea_ctrl.next_mode)) {
            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
            app_le_audio_ba_read_broadcast_receive_state(cnf->handle);
            return;
        }
#endif

#if 1
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
        if (g_lea_ucst_test_mode_flag & APP_LE_AUDIO_UCST_TEST_MODE_PAUSE_BEFORE_CONFIG_ASE) {
            bt_app_common_at_cmd_print_report("[TEST MODE] Service discovery complete!\r\n");
            return;
        }

        if (BT_STATUS_SUCCESS != (ret = app_le_audio_ucst_config_codec(p_info->handle))) {
            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
            LE_AUDIO_MSGLOG_I("[APP] config_codec failed, handle:%x ret:%x", 2, p_info->handle, ret);
        }
#else
        p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
        app_le_audio_ucst_start();
#endif
    }
}

static void app_le_audio_ucst_handle_bap_evt(ble_bap_event_t event, void *msg)
{
    if (NULL == msg) {
        return;
    }

    switch (event) {
        case BLE_BAP_ASCS_ASE_NOTIFY: {
            ble_bap_ase_notify_t *p_notify = (ble_bap_ase_notify_t *)msg;
            switch (p_notify->ase_state) {
                case ASE_STATE_IDLE: {
                    app_le_audio_ucst_handle_ase_idle(p_notify);
                    break;
                }
                case ASE_STATE_CODEC_CONFIGURED: {
                    app_le_audio_ucst_handle_ase_codec_configured(p_notify);
                    break;
                }
                case ASE_STATE_QOS_CONFIGURED: {
                    app_le_audio_ucst_handle_ase_qos_configured(p_notify);
                    break;
                }
                case ASE_STATE_ENABLING: {
                    app_le_audio_ucst_handle_ase_enabling(p_notify);
                    break;
                }
                case ASE_STATE_STREAMING: {
                    app_le_audio_ucst_handle_ase_streaming(p_notify);
                    break;
                }
                case ASE_STATE_DISABLING: {
                    app_le_audio_ucst_handle_ase_disabling(p_notify);
                    break;
                }
                case ASE_STATE_RELEASING: {
                    app_le_audio_ucst_handle_ase_releasing(p_notify);
                    break;
                }
                default:
                    break;
            }
            app_le_audio_ucst_check_link_idle(p_notify->handle);
            break;
        }
        case BLE_BAP_ASCS_ASE_CONTROL_POINT_NOTIFY: {
            app_le_audio_ucst_handle_ase_control_point_notify((ble_ascs_control_point_notify_t *)msg);
            break;
        }
        case BLE_BAP_ASCS_DISCOVER_SERVICE_COMPLETE_NOTIFY: {
            app_le_audio_ucst_handle_bap_discover_service_complete((ble_bap_discover_service_complete_t *)msg);
            break;
        }
        case BLE_BAP_PACS_READ_AVAILABLE_AUDIO_CONTEXTS_CNF: {
            app_le_audio_ucst_handle_read_available_audio_contexts_cnf((ble_bap_read_available_audio_contexts_cnf_t *)msg);
            break;
        }
        case BLE_BAP_PACS_READ_SUPPORTED_AUDIO_CONTEXTS_CNF: {
            app_le_audio_ucst_handle_read_supported_audio_contexts_cnf((ble_bap_read_supported_audio_contexts_cnf_t *)msg);
            break;
        }
        case BLE_BAP_PACS_AVAILABLE_AUDIO_CONTEXTS_NOTIFY: {
            app_le_audio_ucst_handle_available_audio_contexts_notify((ble_bap_available_audio_contexts_notify_t *)msg);
            break;
        }
        case BLE_BAP_PACS_READ_SINK_PAC_CNF: {
            app_le_audio_ucst_handle_read_sink_pac_cnf((ble_bap_read_sink_pac_cnf_t *)msg);
            break;
        }
        case BLE_BAP_PACS_READ_SINK_LOCATION_CNF: {
            app_le_audio_ucst_handle_read_sink_location_cnf((ble_bap_read_sink_location_cnf_t *)msg);
            break;
        }
        case BLE_BAP_PACS_READ_SOURCE_PAC_CNF: {
            app_le_audio_ucst_handle_read_source_pac_cnf((ble_bap_read_source_pac_cnf_t *)msg);
            break;
        }
        case BLE_BAP_PACS_READ_SOURCE_LOCATION_CNF: {
            app_le_audio_ucst_handle_read_source_location_cnf((ble_bap_read_source_location_cnf_t *)msg);
            break;
        }
        case BLE_BAP_ASCS_READ_ASE_CNF: {
            app_le_audio_ucst_handle_read_ase_cnf((ble_ascs_read_ase_cnf_t *)msg);
            break;
        }
        default:
            break;
    }
#ifdef AIR_LE_AUDIO_BA_ENABLE
    app_le_audio_ba_handle_bap_client_evt(event, msg);
#endif
}

static uint16_t app_le_audio_ucst_handle_source_evt(bt_le_audio_source_event_t event, void *msg)
{
    switch (event) {
        /* Call Event: call state */
        case BT_LE_AUDIO_SOURCE_EVENT_CALL_INCOMING: {
            ble_tbs_event_incoming_call_t *param = (ble_tbs_event_incoming_call_t *)msg;

            bt_app_common_at_cmd_print_report("call state: INCOMING\r\n");
            LE_AUDIO_MSGLOG_I("[APP] CALL INCOMING, call_idx:%x", 1, param->call_idx);
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_CALL_DIALING: {
            bt_app_common_at_cmd_print_report("call state: DIALING\r\n");
            LE_AUDIO_MSGLOG_I("[APP] CALL DIALING", 0);
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_CALL_ALERTING: {
            bt_app_common_at_cmd_print_report("call state: ALERTIN\r\n");
            LE_AUDIO_MSGLOG_I("[APP] CALL ALERTING", 0);
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_CALL_ACTIVE: {
            bt_app_common_at_cmd_print_report("call state: ACTIVE\r\n");
            LE_AUDIO_MSGLOG_I("[APP] CALL ACTIVE", 0);
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_CALL_HELD: {
            bt_app_common_at_cmd_print_report("call state: HELD\r\n");
            LE_AUDIO_MSGLOG_I("[APP] CALL HELD", 0);
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_CALL_ENDED: {
            bt_app_common_at_cmd_print_report("call state: END\r\n");
            LE_AUDIO_MSGLOG_I("[APP] CALL ENDED", 0);
            break;
        }
        /* Call Event: call control */
        case BT_LE_AUDIO_SOURCE_EVENT_CALL_ACCEPT: {
            ble_tbs_event_parameter_t *param = (ble_tbs_event_parameter_t *)msg;
            bt_le_audio_call_state_t call_state;

            bt_app_common_at_cmd_print_report("call action: ACCEPT\r\n");

            call_state =  bt_le_audio_source_call_get_state(param->service_idx, param->call_idx);
            LE_AUDIO_MSGLOG_I("[APP] CALL ACCEPT, call_idx:%x state:%x", 2, param->call_idx, call_state);

            if (BLE_TBS_STATE_INCOMING == call_state) {
#ifdef AIR_MS_TEAMS_ENABLE
                usb_hid_srv_send_action(USB_HID_SRV_ACTION_ACCEPT_CALL, NULL);

                if (true == g_lea_ucst_pts_remote_call_service) {
                    g_curr_call_idx = param->call_idx;
                    app_le_audio_usb_hid_handle_call_active();
                }
#endif
            }
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_CALL_TERMINATE: {
            ble_tbs_event_parameter_t *param = (ble_tbs_event_parameter_t *)msg;
            bt_le_audio_call_state_t call_state;

            bt_app_common_at_cmd_print_report("call action: TERMINATE\r\n");
            call_state =  bt_le_audio_source_call_get_state(param->service_idx, param->call_idx);

            LE_AUDIO_MSGLOG_I("[APP] CALL TERMINATE, call_idx:%x state:%x", 2, param->call_idx, call_state);
#ifdef AIR_MS_TEAMS_ENABLE
            if (BLE_TBS_STATE_INCOMING == call_state) {
                usb_hid_srv_send_action(USB_HID_SRV_ACTION_REJECT_CALL, NULL);
            } else if (BLE_TBS_STATE_IDLE > call_state) {
                usb_hid_srv_send_action(USB_HID_SRV_ACTION_TERMINATE_CALL, NULL);
            }

            if (true == g_lea_ucst_pts_remote_call_service) {
                g_curr_call_idx = param->call_idx;
                app_le_audio_usb_hid_handle_call_end();
            }
#endif
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_HOLD_CALL: {
            ble_tbs_event_parameter_t *param = (ble_tbs_event_parameter_t *)msg;
            bt_le_audio_call_state_t call_state;

            bt_app_common_at_cmd_print_report("call action: HOLD\r\n");

            call_state =  bt_le_audio_source_call_get_state(param->service_idx, param->call_idx);
            LE_AUDIO_MSGLOG_I("[APP] CALL HOLD, call_idx:%x state:%x", 2, param->call_idx, call_state);

#ifdef AIR_MS_TEAMS_ENABLE
            usb_hid_srv_send_action(USB_HID_SRV_ACTION_HOLD_CALL, NULL);

            if (true == g_lea_ucst_pts_remote_call_service) {
                g_curr_call_idx = param->call_idx;
                app_le_audio_usb_hid_handle_call_hold();
            }
#endif
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_UNHOLD_CALL: {
            ble_tbs_event_parameter_t *param = (ble_tbs_event_parameter_t *)msg;
            bt_le_audio_call_state_t call_state;

            bt_app_common_at_cmd_print_report("call action: UNHOLD\r\n");

            call_state =  bt_le_audio_source_call_get_state(param->service_idx, param->call_idx);
            LE_AUDIO_MSGLOG_I("[APP] CALL UNHOLD, call_idx:%x state:%x", 2, param->call_idx, call_state);

#ifdef AIR_MS_TEAMS_ENABLE
            usb_hid_srv_send_action(USB_HID_SRV_ACTION_UNHOLD_CALL, NULL);

            if (true == g_lea_ucst_pts_remote_call_service) {
                g_curr_call_idx = param->call_idx;
                app_le_audio_usb_hid_handle_call_unhold();
            }
#endif
            break;
        }
        /* Media Event */
        case BT_LE_AUDIO_SOURCE_EVENT_MEDIA_PLAYING: {
            ble_mcs_event_parameter_t *param = (ble_mcs_event_parameter_t *)msg;
            if ((BT_HANDLE_INVALID != param->handle) && (!APP_LE_AUDIO_UCST_IS_CALL_MODE)) {
                USB_HID_PlayPause();
            }
            bt_app_common_at_cmd_print_report("media state: PLAYING\r\n");
            LE_AUDIO_MSGLOG_I("[APP] MEDIA PLAYING", 0);
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_MEDIA_PAUSED: {
            ble_mcs_event_parameter_t *param = (ble_mcs_event_parameter_t *)msg;
            if ((BT_HANDLE_INVALID != param->handle) && (!APP_LE_AUDIO_UCST_IS_CALL_MODE)) {
                USB_HID_PlayPause();
            }
            bt_app_common_at_cmd_print_report("media state: PAUSED\r\n");
            LE_AUDIO_MSGLOG_I("[APP] MEDIA PAUSED", 0);
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_MEDIA_PREVIOUS_TRACK: {
            ble_mcs_event_parameter_t *param = (ble_mcs_event_parameter_t *)msg;
            if ((BT_HANDLE_INVALID != param->handle) && (!APP_LE_AUDIO_UCST_IS_CALL_MODE)) {
                USB_HID_ScanPreviousTrack();
            }
            bt_app_common_at_cmd_print_report("media action: PREVIOUS\r\n");
            LE_AUDIO_MSGLOG_I("[APP] MEDIA PREVIOUS TRACK", 0);
            break;
        }
        case BT_LE_AUDIO_SOURCE_EVENT_MEDIA_NEXT_TRACK: {
            ble_mcs_event_parameter_t *param = (ble_mcs_event_parameter_t *)msg;
            if ((BT_HANDLE_INVALID != param->handle) && (!APP_LE_AUDIO_UCST_IS_CALL_MODE)) {
                USB_HID_ScanNextTrack();
            }
            bt_app_common_at_cmd_print_report("media action: NEXT\r\n");
            LE_AUDIO_MSGLOG_I("[APP] MEDIA NEXT TRACK", 0);
            break;
        }
        default:
            break;
    }
    return 0;
}


/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void app_le_audio_ucst_handle_bonding_complete_ind(bt_status_t ret, bt_gap_le_bonding_complete_ind_t *ind)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(ind->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] LE_BONDING_COMPLETE_IND, handle:%x ret:%x", 2, ind->handle, ret);
    if (BT_STATUS_SUCCESS != ret) {
        return;
    }
    bt_device_manager_le_bonded_info_t * p_bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext(&p_info->addr.addr);
    bt_bd_addr_t empty_addr = {0};

    if ((NULL != p_bonded_info) && (0 != memcmp(p_bonded_info->info.identity_addr.address.addr, empty_addr, sizeof(bt_bd_addr_t)))) {
        //ret = bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &p_bonded_info->info.identity_addr.address);
        ret = bt_gap_le_srv_operate_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &p_bonded_info->info.identity_addr.address, NULL);
        LE_AUDIO_MSGLOG_I("[APP][U] add_white_list(IDA), ret:%x addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 8, ret,
              p_bonded_info->info.identity_addr.address.type,
              p_bonded_info->info.identity_addr.address.addr[5],
              p_bonded_info->info.identity_addr.address.addr[4],
              p_bonded_info->info.identity_addr.address.addr[3],
              p_bonded_info->info.identity_addr.address.addr[2],
              p_bonded_info->info.identity_addr.address.addr[1],
              p_bonded_info->info.identity_addr.address.addr[0]);
    }

    /* enter next step */
    app_le_audio_ucst_exchange_mtu(p_info->handle);
}

void app_le_audio_ucst_handle_exchange_mtu_rsp(bt_status_t ret, bt_gatt_exchange_mtu_rsp_t *rsp)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(rsp->connection_handle))) {
        return;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_EXCHANGE_MTU != p_info->next_state) {
        LE_AUDIO_MSGLOG_I("[APP][U] exchange_mtu_rsp state NOT match, handle:%x state:%x->%x", 3,
                          rsp->connection_handle,
                          p_info->curr_state,
                          p_info->next_state);
        return;
    }

    if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode) {
        /* [Switch streaming mode] UCST -> BCST */
        g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                            APP_LE_AUDIO_EVENT_STOP_SCAN_NEW_DEVICE, NULL, 0,
                            NULL, 0);
        return;
    }

    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_EXCHANGE_MTU;
    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;

    LE_AUDIO_MSGLOG_I("[APP][U] exchange_mtu_rsp, handle:%x state:%x->%x MTU:%d", 4,
                      rsp->connection_handle,
                      p_info->curr_state,
                      p_info->next_state,
                      rsp->server_rx_mtu);

    /* enter next step */
    bt_gattc_discovery_status_t status;
    status = bt_gattc_discovery_start(BT_GATTC_DISCOVERY_USER_LE_AUDIO, rsp->connection_handle, FALSE);

    LE_AUDIO_MSGLOG_I("[APP] exchange_mtu_rsp, start discovery ret:%x", 1, status);

}

void app_le_audio_ucst_handle_set_cig_parameter_cnf(bt_status_t ret, bt_gap_le_set_cig_params_cnf_t *cnf)
{
    if (BT_STATUS_SUCCESS != ret) {
        /* To do: handle error case */
        LE_AUDIO_MSGLOG_I("[APP][U] LE_SET_CIG_PARAMETERS_CNF fail, ret:%x", 1, ret);
        g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] LE_SET_CIG_PARAMETERS_CNF, target:%x->%x stream_state:%x->%x", 4,
                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state);

    if (APP_LE_AUDIO_UCST_STREAM_STATE_SET_CIG_PARAMETER != g_lea_ucst_ctrl.next_stream_state) {
        /* To do: check */
        return;
    }

    g_lea_ucst_ctrl.curr_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_SET_CIG_PARAMETER;
    g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;

    g_lea_ucst_ctrl.is_cig_created = true;

    if (APP_LE_AUDIO_UCST_CIS_MAX_NUM < cnf->cis_count) {
        cnf->cis_count = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
    }

    g_lea_ucst_ctrl.cis_num = cnf->cis_count;

    uint8_t i, tmp;
    for (i = 0; i < g_lea_ucst_ctrl.cis_num; i++) {
        g_lea_ucst_cis_info[i].cis_handle = cnf->cis_connection_handle[i];
        g_lea_ucst_cis_info[i].cis_status = APP_LE_AUDIO_UCST_CIS_IDLE;
        g_lea_ucst_cis_info[i].acl_handle = BT_HANDLE_INVALID;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] LE_SET_CIG_PARAMETERS_CNF, cig_id:%x cis_num:%x cis_handle:%x %x", 4, cnf->cig_id, cnf->cis_count,
                      g_lea_ucst_cis_info[0].cis_handle, g_lea_ucst_cis_info[1].cis_handle);

    /* check target */
    if (app_le_audio_ucst_check_pause_stream() ||
        (APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
        (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
        g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM;
        if (BT_STATUS_SUCCESS != app_le_audio_close_audio_transmitter()) {
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
        }
        return;
    }

    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t ase_idx = APP_LE_AUDIO_UCST_ASE_IDX_0;

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        p_info = &g_lea_ucst_link_info[i];

        if ((BT_HANDLE_INVALID != p_info->handle) &&
                    ((APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE == p_info->curr_state) ||
                     (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC == p_info->curr_state) ||
                     (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS == p_info->curr_state)) &&
                    (APP_LE_AUDIO_UCST_LINK_STATE_IDLE == p_info->next_state) &&
                    (0 == p_info->wait_event.wait_ase_event)) {

            ase_idx = APP_LE_AUDIO_UCST_ASE_IDX_0;

            LE_AUDIO_MSGLOG_I("[APP] LE_SET_CIG_PARAMETERS_CNF, i:%x handle:%x state:%x->%x source_ase_num:%x", 5,
                              i, p_info->handle, p_info->curr_state, p_info->next_state, p_info->source_ase_num);
            if ((APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) &&
                (!((AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location) &&
                   ((APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_2 == g_lea_ucst_ctrl.create_cis_mode) ||
                    (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_2 == g_lea_ucst_ctrl.create_cis_mode))))) {
                if (p_info->source_ase_num) {
                    ase_idx = p_info->source_ase_idx;
                }
            }

            LE_AUDIO_MSGLOG_I("[APP] LE_SET_CIG_PARAMETERS_CNF, ase[%x]:%x ase_state:%x", 3, ase_idx, p_info->ase[ase_idx].id, p_info->ase[ase_idx].curr_state);

            if (ASE_STATE_QOS_CONFIGURED == p_info->ase[ase_idx].curr_state) {
                p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE;
                if (BT_STATUS_SUCCESS != app_le_audio_ucst_enable_ase(p_info->handle)) {
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                }


            } else if (ASE_STATE_CODEC_CONFIGURED == p_info->ase[ase_idx].curr_state) {
                p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
                if (BT_STATUS_SUCCESS != app_le_audio_ucst_config_qos(p_info->handle)) {
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                }
            } else if (ASE_STATE_IDLE == p_info->ase[ase_idx].curr_state) {
                p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE;
                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
                if (BT_STATUS_SUCCESS != app_le_audio_ucst_config_codec(p_info->handle)) {
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                }
            }
        }
    }
}


void app_le_audio_ucst_handle_cis_established_ind(bt_status_t ret, bt_gap_le_cis_established_ind_t *ind)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t link_idx, cis_idx, i;
    static uint8_t retry_create_cis = 0;
    if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == (link_idx = app_le_audio_ucst_get_link_idx_by_cis(ind->connection_handle, &cis_idx))) {
        LE_AUDIO_MSGLOG_I("[APP][U] LE_CIS_ESTABLISHED_IND, link not exist (cis_hdl:%x)", 1, ind->connection_handle);
        return;
    }

    p_info = &g_lea_ucst_link_info[link_idx];

    LE_AUDIO_MSGLOG_I("[APP][U] LE_CIS_ESTABLISHED_IND, handle:%x state:%x->%x cis_handle[%x]:%x cis_status:%x", 6,
                      p_info->handle,
                      p_info->curr_state,
                      p_info->next_state,
                      cis_idx,
                      g_lea_ucst_cis_info[cis_idx].cis_handle,
                      g_lea_ucst_cis_info[cis_idx].cis_status);

    app_le_audio_ucst_send_prepare_vcmd(g_lea_ucst_link_info[link_idx].handle, APP_LE_AUDIO_PREPARE_VCMD_MODE_CONN, 0);

    if (BT_STATUS_SUCCESS != ret) {
        LE_AUDIO_MSGLOG_I("[APP][U] LE_CIS_ESTABLISHED_IND fail, cis_handle:%x ret:%x", 2, ind->connection_handle, ret);
        g_lea_ucst_cis_info[cis_idx].cis_status = APP_LE_AUDIO_UCST_CIS_IDLE;
        g_lea_ucst_cis_info[cis_idx].acl_handle = BT_HANDLE_INVALID;
        if ((p_info->ase_releasing) &&
            (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC == p_info->next_state)) {
            if (0 != p_info->wait_event.wait_ase_event) {
                return;
            }

            p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
            p_info->ase_releasing = false;

        } else if (APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE <= p_info->curr_state) {
            if ((BT_HCI_STATUS_CONNECTION_FAILED_TO_BE_ESTABLISHED == ret) && (retry_create_cis < 3)) {
                retry_create_cis++;
                if (BT_STATUS_SUCCESS == app_le_audio_ucst_create_cis()){
                    return;
                }
                else {
                    retry_create_cis = 0;
                }
            }
            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE;
            if (BT_STATUS_SUCCESS != app_le_audio_ucst_disable_ase(p_info->handle)) {
                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
            }

            /* check the other CIS in the same LE link, and disconnect it. */
            if (APP_LE_AUDIO_UCST_CIS_MAX_NUM == p_info->cis_num) {
                i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
                while (0 != i) {
                    i--;
                    if (BT_HANDLE_INVALID != g_lea_ucst_cis_info[i].cis_handle &&
                        p_info->handle == g_lea_ucst_cis_info[i].acl_handle &&
                        (APP_LE_AUDIO_UCST_CIS_IDLE != g_lea_ucst_cis_info[i].cis_status &&
                         APP_LE_AUDIO_UCST_CIS_DISCONNECTING != g_lea_ucst_cis_info[i].cis_status)) {
                        app_le_audio_ucst_disconnect(g_lea_ucst_cis_info[i].cis_handle);
                    }
                }
            }
        }

        if (0 == app_le_audio_ucst_get_cis_num()) {
            if (app_le_audio_ucst_check_close_audio_stream()) {
                return;
            }

            if ((APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM == g_lea_ucst_ctrl.curr_stream_state) &&
                (APP_LE_AUDIO_UCST_STREAM_STATE_REMOVE_CIG != g_lea_ucst_ctrl.next_stream_state)) {
                if (g_lea_ucst_ctrl.is_cig_created) {
                    /* remove cig */
                    g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_REMOVE_CIG;
                    if (BT_STATUS_SUCCESS != app_le_audio_ucst_remove_cig()) {
                        g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
                    }
                }
            }
            return;
        }

        cis_idx = APP_LE_AUDIO_UCST_CIS_MAX_NUM;

    } else {
        char conn_string[40] = {0};
        retry_create_cis = 0;

        snprintf((char *)conn_string, 40, "CIS connected, handle: 0x%04x\r\n", ind->connection_handle);
        bt_app_common_at_cmd_print_report(conn_string);

        g_lea_ucst_cis_info[cis_idx].cis_status = APP_LE_AUDIO_UCST_CIS_CREATED;

        /* if other cis in same LE link is in creating state, wait for it */
        if (APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS == p_info->next_state) {
            i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
            while (0 != i) {
                i--;
                if ((p_info->handle == g_lea_ucst_cis_info[i].acl_handle) &&
                    (APP_LE_AUDIO_UCST_CIS_CREATING == g_lea_ucst_cis_info[i].cis_status)) {
                    return;
                }
            }

            p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS;
            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
        }
    }

    /* Check setup ISO data path */
    uint8_t j, tmp;
    bool found = false;

    i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;

    while (0 != i) {
        i--;
        if (BT_HANDLE_INVALID != g_lea_ucst_cis_info[i].cis_handle) {
            LE_AUDIO_MSGLOG_I("[APP] CIS_ESTABLISHED_IND, check handle:%x cis_handle[%x]:%x cis_status:%x", 4,
                              g_lea_ucst_cis_info[i].acl_handle,
                              i,
                              g_lea_ucst_cis_info[i].cis_handle,
                              g_lea_ucst_cis_info[i].cis_status);

            if (APP_LE_AUDIO_UCST_CIS_CREATING == g_lea_ucst_cis_info[i].cis_status) {
                /* wait the other cis established */
                return;
            }

            if (APP_LE_AUDIO_UCST_CIS_CREATED == g_lea_ucst_cis_info[i].cis_status) {
                for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (j = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
                        continue;
                    }
#else
                    j = tmp;
#endif
                    /* check link state */
                    p_info = &g_lea_ucst_link_info[j];
                    if (p_info->handle == g_lea_ucst_cis_info[i].acl_handle) {
                        if ((APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS == p_info->curr_state) &&
                            (APP_LE_AUDIO_UCST_LINK_STATE_IDLE == p_info->next_state) &&
                            (!p_info->ase_releasing)) {
                            if (i != cis_idx) {
                                cis_idx = i;
                                link_idx = j;
                                found = true;
                                break;
                            }
                        } else if (i == cis_idx) {
                            cis_idx = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
                        }
                        break;
                    }
                }
                if (found) {
                    break;
                }
            }
        }
    }

    if (APP_LE_AUDIO_UCST_CIS_MAX_NUM == cis_idx) {
        /* no cis created? */
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP] CIS_ESTABLISHED_IND, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

    if (app_le_audio_ucst_check_pause_stream() || (g_lea_ucst_ctrl.release) ||
        (APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
        (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
        app_le_audio_ucst_disable_ase_when_state_match(APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS);
        return;
    }

    uint8_t sink_ase_num = app_le_audio_ucst_get_sink_ase_num(p_info);
    uint8_t source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);
    bt_gap_le_iso_data_path_id_t iso_data_path_id = 0x01;
    p_info = &g_lea_ucst_link_info[link_idx];

    if (0 != sink_ase_num) {

        LE_AUDIO_MSGLOG_I("[APP] CIS_ESTABLISHED_IND, sink_ase_num:%x %x", 2, sink_ase_num, p_info->sink_ase_num);

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            if ((APP_LE_AUDIO_UCST_IS_CALL_MODE) && (p_info->sink_ase_num >= (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) &&
                (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1 > g_lea_ucst_ctrl.create_cis_mode)) {
                LE_AUDIO_MSGLOG_I("[APP] CIS_ESTABLISHED_IND, sink_ase_num+i:%x curr_state:%x", 2, sink_ase_num + i, p_info->ase[sink_ase_num + i].curr_state);
                if (ASE_STATE_STREAMING == p_info->ase[sink_ase_num + i].curr_state) {
                    break;
                }
            } else {
                LE_AUDIO_MSGLOG_I("[APP] CIS_ESTABLISHED_IND, i:%x curr_state:%x", 2, i, p_info->ase[i].curr_state);
                if (ASE_STATE_STREAMING == p_info->ase[i].curr_state) {
                    break;
                }
            }
        }

        LE_AUDIO_MSGLOG_I("[APP] CIS_ESTABLISHED_IND, i:%x sink_ase_num:%x", 2, i, sink_ase_num);
        if (i == sink_ase_num) {
            return;
        }

        if (((1 == p_info->sink_location_num) && (AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location)) ||
            ((2 == p_info->sink_location_num) && (0 != cis_idx))) {
            iso_data_path_id = 0x02;
        }
        LE_AUDIO_MSGLOG_I("[APP] CIS_ESTABLISHED_IND, handle:%x sink_location:%x(num:%x) iso_data_path_id:%x", 4,
                          p_info->handle,
                          p_info->sink_location,
                          p_info->sink_location_num,
                          iso_data_path_id);

    } else if (0 != source_ase_num) {

        if (((1 == p_info->source_location_num) && (AUDIO_LOCATION_FRONT_RIGHT == p_info->source_location)) ||
            ((2 == p_info->source_location_num) && (0 != cis_idx))) {
            iso_data_path_id = 0x02;
        }

        LE_AUDIO_MSGLOG_I("[APP] CIS_ESTABLISHED_IND, handle:%x source_location:%x(num:%x) iso_data_path_id:%x", 4,
                          p_info->handle,
                          p_info->source_location,
                          p_info->source_location_num,
                          iso_data_path_id);
    }

    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH;
    g_lea_ucst_cis_info[cis_idx].cis_status = APP_LE_AUDIO_UCST_CIS_SETUP_DATA_PATH;
    app_le_audio_setup_iso_data_path(g_lea_ucst_cis_info[cis_idx].cis_handle, BT_GAP_LE_ISO_DATA_PATH_DIRECTION_OUTPUT, iso_data_path_id);
}


void app_le_audio_ucst_handle_setup_iso_data_path_cnf(bt_status_t ret, bt_gap_le_setup_iso_data_path_cnf_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t link_idx, cis_idx;

    if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == (link_idx = app_le_audio_ucst_get_link_idx_by_cis(cnf->handle, &cis_idx))) {
        LE_AUDIO_MSGLOG_I("[APP][U] LE_SETUP_ISO_DATA_PATH_CNF, link not exist (cis_hdl:%x)", 1, cnf->handle);
        return;
    }

    p_info = &g_lea_ucst_link_info[link_idx];

    LE_AUDIO_MSGLOG_I("[APP][U] LE_SETUP_ISO_DATA_PATH_CNF, handle:%x state:%x->%x cis_handle[%x]:%x cis_status:%x", 6,
                      p_info->handle,
                      p_info->curr_state,
                      p_info->next_state,
                      cis_idx,
                      g_lea_ucst_cis_info[cis_idx].cis_handle,
                      g_lea_ucst_cis_info[cis_idx].cis_status);

    if (BT_STATUS_SUCCESS != ret) {
        /* To do: handle error case */
        LE_AUDIO_MSGLOG_I("[APP][U] LE_SETUP_ISO_DATA_PATH_CNF fail, cis_handle:%x ret:%x", 2, cnf->handle, ret);
        return;
    }

    g_lea_ucst_cis_info[cis_idx].cis_status = APP_LE_AUDIO_UCST_CIS_STREAMING;

    /* Check ASE release */
    if (p_info->ase_releasing) {
        return;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH != p_info->next_state) {
        return;
    }

    p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH;
    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;

    /* check pause stream */
    uint8_t i, tmp;
    if (app_le_audio_ucst_check_pause_stream() ||
        (APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
        (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
        app_le_audio_ucst_disable_ase_when_setup_iso_data_path();
        return;
    }

    /* check if other cis in same LE link needed to setup iso data path */
    i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
    cis_idx = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
    while (0 != i) {
        i--;
        if ((APP_LE_AUDIO_UCST_CIS_CREATED == g_lea_ucst_cis_info[i].cis_status) &&
            (p_info->handle == g_lea_ucst_cis_info[i].acl_handle)) {
            cis_idx = i;
            break;
        }
    }

    uint8_t source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);

    if (APP_LE_AUDIO_UCST_CIS_MAX_NUM == cis_idx) {
        if ((APP_LE_AUDIO_UCST_IS_CALL_MODE) && (0 != source_ase_num)) {
            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_START_READY;
            app_le_audio_ucst_set_receiver_start_ready(p_info->handle);

        } else {
            p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_STREAMING;
            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
            g_lea_ucst_ctrl.curr_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING;
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
        }

        if (NULL != p_info->conn_interval_timer_handle) {
            app_le_audio_timer_stop(p_info->conn_interval_timer_handle);
            p_info->conn_interval_timer_handle = NULL;
        }

        app_le_audio_ucst_update_connection_interval(p_info, APP_LE_AUDIO_CONN_INTERVAL_STREAMING);

        /* Check other link's setup ISO data path */
        cis_idx = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
        for (i = 0; i < APP_LE_AUDIO_UCST_CIS_MAX_NUM; i++) {
            if ((BT_HANDLE_INVALID != g_lea_ucst_cis_info[i].cis_handle) &&
                (APP_LE_AUDIO_UCST_CIS_CREATED == g_lea_ucst_cis_info[i].cis_status)) {
                for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (link_idx = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
                        continue;
                    }
#else
                    link_idx = tmp;
#endif
                    p_info = &g_lea_ucst_link_info[link_idx];
                    if ((p_info->handle == g_lea_ucst_cis_info[i].acl_handle) &&
                        (false == p_info->ase_releasing)) {
                        cis_idx = i;
                        break;
                    }
                }
            }
        }

        if (APP_LE_AUDIO_UCST_CIS_MAX_NUM == cis_idx) {
            return;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP] SETUP_ISO_DATA_PATH_CNF, handle:%x state:%x->%x cis_handle[%x]:%x cis_status:%x", 6,
                      p_info->handle,
                      p_info->curr_state,
                      p_info->next_state,
                      cis_idx,
                      g_lea_ucst_cis_info[cis_idx].cis_handle,
                      g_lea_ucst_cis_info[cis_idx].cis_status);

    uint8_t sink_ase_num = app_le_audio_ucst_get_sink_ase_num(p_info);
    bt_gap_le_iso_data_path_id_t iso_data_path_id = 0x01;

    if (0 != sink_ase_num) {

        /* SINK ASE */
        for (i = 0; i < sink_ase_num; i++) {
            if ((APP_LE_AUDIO_UCST_IS_CALL_MODE) && (p_info->sink_ase_num >= (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) &&
                (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1 > g_lea_ucst_ctrl.create_cis_mode)) {
                if (ASE_STATE_STREAMING == p_info->ase[sink_ase_num + i].curr_state) {
                    break;
                }
            } else if (ASE_STATE_STREAMING == p_info->ase[i].curr_state) {
                break;
            }
        }

        if (i == sink_ase_num) {
            return;
        }

        if (((1 == p_info->sink_location_num) && (AUDIO_LOCATION_FRONT_RIGHT == p_info->sink_location)) ||
            ((2 == p_info->sink_location_num) && (0 != cis_idx))) {
            iso_data_path_id = 0x02;
        }
        LE_AUDIO_MSGLOG_I("[APP] SETUP_ISO_DATA_PATH_CNF, handle:%x sink_location:%x(num:%x) iso_data_path_id:%x", 4,
                          p_info->handle,
                          p_info->sink_location,
                          p_info->sink_location_num,
                          iso_data_path_id);

    } else if (0 != source_ase_num) {
        if (((1 == p_info->source_location_num) && (AUDIO_LOCATION_FRONT_RIGHT == p_info->source_location)) ||
            ((2 == p_info->source_location_num) && (0 != cis_idx))) {
            iso_data_path_id = 0x02;
        }
        LE_AUDIO_MSGLOG_I("[APP] SETUP_ISO_DATA_PATH_CNF, handle:%x source_location:%x(num:%x) iso_data_path_id:%x", 4,
                          p_info->handle,
                          p_info->source_location,
                          p_info->source_location_num,
                          iso_data_path_id);
    }

    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH;
    g_lea_ucst_cis_info[cis_idx].cis_status = APP_LE_AUDIO_UCST_CIS_SETUP_DATA_PATH;
    app_le_audio_setup_iso_data_path(g_lea_ucst_cis_info[cis_idx].cis_handle, BT_GAP_LE_ISO_DATA_PATH_DIRECTION_OUTPUT, iso_data_path_id);
}


void app_le_audio_ucst_handle_cis_terminated_ind(bt_status_t ret, bt_gap_le_cis_terminated_ind_t *ind)
{
    uint8_t link_idx, cis_idx;
    if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == (link_idx = app_le_audio_ucst_get_link_idx_by_cis(ind->connection_handle, &cis_idx))) {
        LE_AUDIO_MSGLOG_I("[APP][U] LE_CIS_TERMINATED_IND, link not exist (cis_hdl:%x)", 1, ind->connection_handle);
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] LE_CIS_TERMINATED_IND, handle:%x state:%x->%x cis_handle[%x]:%x cis_status:%x reason:%x cis_num:%x", 8,
                      g_lea_ucst_link_info[link_idx].handle,
                      g_lea_ucst_link_info[link_idx].curr_state,
                      g_lea_ucst_link_info[link_idx].next_state,
                      cis_idx,
                      g_lea_ucst_cis_info[cis_idx].cis_handle,
                      g_lea_ucst_cis_info[cis_idx].cis_status,
                      ind->reason,
                      g_lea_ucst_link_info[link_idx].cis_num);

    uint8_t cis_num = 0;

    app_le_audio_ucst_send_prepare_vcmd(g_lea_ucst_link_info[link_idx].handle, APP_LE_AUDIO_PREPARE_VCMD_MODE_DISCONN, 0);

    if (BT_STATUS_SUCCESS != ret) {
        LE_AUDIO_MSGLOG_I("[APP][U] LE_CIS_TERMINATED_IND fail, cis_handle:%x ret:%x", 2, ind->connection_handle, ret);
        cis_num = app_le_audio_ucst_get_cis_num();

    } else {
        char conn_string[40] = {0};
        snprintf((char *)conn_string, 40, "CIS disconnected, handle: 0x%04x\r\n", ind->connection_handle);
        bt_app_common_at_cmd_print_report(conn_string);

        g_lea_ucst_cis_info[cis_idx].acl_handle = BT_HANDLE_INVALID;
        g_lea_ucst_cis_info[cis_idx].cis_status = APP_LE_AUDIO_UCST_CIS_IDLE;

        uint8_t tmp_cis_idx, i;

        i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
        cis_idx = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
        tmp_cis_idx = APP_LE_AUDIO_UCST_CIS_MAX_NUM;

        while (i > 0) {
            i--;
            if ((BT_HANDLE_INVALID != g_lea_ucst_cis_info[i].cis_handle) &&
                (BT_HANDLE_INVALID != g_lea_ucst_cis_info[i].acl_handle)) {
                cis_num++;

                LE_AUDIO_MSGLOG_I("[APP] CIS_TERMINATED_IND, check handle:%x cis_handle:%x cis_status:%x", 3,
                                  g_lea_ucst_cis_info[i].acl_handle,
                                  g_lea_ucst_cis_info[i].cis_handle,
                                  g_lea_ucst_cis_info[i].cis_status);

                if (APP_LE_AUDIO_UCST_CIS_DISCONNECTING == g_lea_ucst_cis_info[i].cis_status) {
                    if (g_lea_ucst_link_info[link_idx].handle == g_lea_ucst_cis_info[i].acl_handle) {
                        cis_idx = i;
                    } else {
                        tmp_cis_idx = i;
                    }
                }
            }
        }

        if (APP_LE_AUDIO_UCST_CIS_MAX_NUM != cis_idx) {
            /* BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST: delete the other CIS in the same LE link */
            LE_AUDIO_MSGLOG_I("[APP] disconnect CIS, handle:%x cis_handle[%x]:%x", 3,
                              g_lea_ucst_cis_info[cis_idx].acl_handle,
                              cis_idx,
                              g_lea_ucst_cis_info[cis_idx].cis_handle);

            app_le_audio_ucst_disconnect(g_lea_ucst_cis_info[cis_idx].cis_handle);
            return;
        } else if (BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST != ind->reason) {
            /* When the LE link creates more than one CIS link, disconnect LE link if one of the CIS links disconnects because of CONNTECTION_TIMEOUT.
             * It may not be re-created successfully. Therefore, disconnect LE link and start over simply.
             */
            g_lea_ucst_link_info[link_idx].next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL;
            app_le_audio_ucst_disconnect(g_lea_ucst_link_info[link_idx].handle);
            return;
        }

        if ((APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_CIS == g_lea_ucst_link_info[link_idx].next_state) ||
            (APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL == g_lea_ucst_link_info[link_idx].next_state)) {
            g_lea_ucst_link_info[link_idx].curr_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS;
            g_lea_ucst_link_info[link_idx].next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
        }

        if (APP_LE_AUDIO_UCST_CIS_MAX_NUM != tmp_cis_idx) {
            /* BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST:delete the other CIS in group mate LE link */
            LE_AUDIO_MSGLOG_I("[APP] disconnect CIS, handle:%x cis_handle[%x]:%x", 3,
                              g_lea_ucst_cis_info[tmp_cis_idx].acl_handle,
                              tmp_cis_idx,
                              g_lea_ucst_cis_info[tmp_cis_idx].cis_handle);

            app_le_audio_ucst_disconnect(g_lea_ucst_cis_info[tmp_cis_idx].cis_handle);
            return;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP] CIS_TERMINATED_IND, cis_num:%x target:%x->%x stream_state:%x->%x", 5, cis_num,
                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state);
    if (0 < cis_num) {
        return;
    }

    /* no cis exist */
    if (app_le_audio_ucst_check_close_audio_stream()) {
        return;
    }

    if ((APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM == g_lea_ucst_ctrl.curr_stream_state) &&
        (APP_LE_AUDIO_UCST_STREAM_STATE_REMOVE_CIG != g_lea_ucst_ctrl.next_stream_state)) {
        if (g_lea_ucst_ctrl.is_cig_created) {
            /* remove cig */
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_REMOVE_CIG;
            if (BT_STATUS_SUCCESS != app_le_audio_ucst_remove_cig()) {
                g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
            }
        }
    }
}


void app_le_audio_ucst_handle_remove_cig_cnf(bt_status_t ret, bt_gap_le_remove_cig_cnf_t *cnf)
{
    uint8_t i, tmp;
    if (BT_STATUS_SUCCESS != ret) {
        /* To do: handle error case */
        LE_AUDIO_MSGLOG_I("[APP][U] LE_REMOVE_CIG_CNF fail, ret:%x", 1, ret);
        g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] LE_REMOVE_CIG_CNF, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

    for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
        if (BT_HANDLE_INVALID != g_lea_ucst_prepare_vcmd_disconn_handle[i]) {
            app_le_audio_ucst_send_prepare_vcmd(g_lea_ucst_prepare_vcmd_disconn_handle[i], APP_LE_AUDIO_PREPARE_VCMD_MODE_DISCONN, 0);
        }
        if (BT_HANDLE_INVALID != g_lea_ucst_prepare_vcmd_conn_handle[i]) {
            app_le_audio_ucst_send_prepare_vcmd(g_lea_ucst_prepare_vcmd_conn_handle[i], APP_LE_AUDIO_PREPARE_VCMD_MODE_CONN, 0);
        }
    }
    if (APP_LE_AUDIO_UCST_STREAM_STATE_REMOVE_CIG != g_lea_ucst_ctrl.next_stream_state) {
        /* To do: check */
        return;
    }

    g_lea_ucst_ctrl.curr_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
    g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;

    LE_AUDIO_MSGLOG_I("[APP][U] LE_REMOVE_CIG_CNF, cig_id:%x", 1, cnf->cig_id);

    g_lea_ucst_ctrl.is_cig_created = false;
    g_lea_ucst_ctrl.cis_num = 0;
    g_lea_ucst_ctrl.set_endpoint_tx_ready = false;

    i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
    while (i > 0) {
        i--;
        g_lea_ucst_cis_info[i].cis_handle = BT_HANDLE_INVALID;
        g_lea_ucst_cis_info[i].acl_handle = BT_HANDLE_INVALID;
        g_lea_ucst_cis_info[i].cis_status = APP_LE_AUDIO_UCST_CIS_IDLE;
    }

    if (app_le_audio_ucst_check_pause_stream() || (g_lea_ucst_ctrl.release) ||
        (((APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
          (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) &&
         ((APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.next_target) ||
          (APP_LE_AUDIO_MODE_DISABLE == g_lea_ctrl.next_mode)||
          (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode)))) {
        g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        app_le_audio_ucst_reset_release();
        if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode) {
            /* [Switch streaming mode] UCST -> BCST */
            app_le_audio_start_broadcast();

        } else if (APP_LE_AUDIO_MODE_DISABLE == g_lea_ctrl.next_mode) {
            g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_NONE;
            g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
            app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_CIS;
            app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, &lea_mode);
        }
        return;
    }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    bool change_active_device = false;
    LE_AUDIO_MSGLOG_I("[APP][U] REMOVE_CIG_CNF, active:%x->%x", 2,
                      g_lea_ucst_ctrl.curr_group,
                      g_lea_ucst_ctrl.next_group);
    if ((APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.curr_group) &&
        (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.next_group)) {
        change_active_device = true;
    }
#endif

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        if (BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) {
            LE_AUDIO_MSGLOG_I("[APP] REMOVE_CIG_CNF, check handle:%x state:%x->%x w_ase:%x", 4, g_lea_ucst_link_info[i].handle,
                              g_lea_ucst_link_info[i].curr_state, g_lea_ucst_link_info[i].next_state,
                              g_lea_ucst_link_info[i].wait_event.wait_ase_event);
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
            if (change_active_device && (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC > g_lea_ucst_link_info[i].curr_state)) {
                continue;
            }
#endif
            if (!(((APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC == g_lea_ucst_link_info[i].curr_state) ||
                   (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS == g_lea_ucst_link_info[i].curr_state)) &&
                  ((APP_LE_AUDIO_UCST_LINK_STATE_IDLE == g_lea_ucst_link_info[i].next_state) ||
                   (APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL == g_lea_ucst_link_info[i].next_state)) &&
                  (0 == g_lea_ucst_link_info[i].wait_event.wait_ase_event))) {
                return;
            }
        }
    }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (change_active_device) {
        g_lea_ucst_ctrl.curr_group = g_lea_ucst_ctrl.next_group;
        g_lea_ucst_ctrl.next_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                            APP_LE_AUDIO_EVENT_ACTIVE_DEVICE_CHANGED, NULL, 0,
                            NULL, 0);
        ble_tbs_switch_device_completed();
    } else {
        g_lea_ucst_ctrl.next_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    }
#endif

#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE == g_lea_ucst_ctrl.next_target) {
        app_le_audio_silence_detection_handle_event(APP_LE_AUDIO_SILENCE_DETECTION_EVENT_START_SPECIAL_SILENCE_DETECTION, NULL);
    } else
#endif
    {
        g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
            (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state) &&
            (APP_LE_AUDIO_UCST_CONN_NONE == g_lea_ucst_ctrl.curr_conn) &&
            (0 == app_le_audio_ucst_get_link_num_ex())) {
            g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_NONE;
            g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
            app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_CIS;
            app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, &lea_mode);
        } else {
            app_le_audio_ucst_start();
        }
    }
}


void app_le_audio_ucst_open_audio_transmitter_cb(void)
{
    LE_AUDIO_MSGLOG_I("[APP][U] open_audio_transmitter_cb, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

    if (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state &&
        APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM == g_lea_ucst_ctrl.next_stream_state) {
        /* On receiving DISCONNECT_IND, if there is no LE link and audio transmitter is opening, next_stream_state is set to
               * APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM. open_audio_transmitter_cb need close audio transmitter. */
        g_lea_ucst_ctrl.curr_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM;
        if (BT_STATUS_SUCCESS != app_le_audio_close_audio_transmitter()) {
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
        } else {
            /* Change the curr_target only */
            app_le_audio_ucst_stop(false);
        }
        return;
    }

    if (APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM != g_lea_ucst_ctrl.next_stream_state) {
        return;
    }

    g_lea_ucst_ctrl.curr_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM;
    g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;

    /* If it is APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE, do nothing. */
    switch (g_lea_ucst_ctrl.curr_target) {
        case APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE:
        case APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE: {
            if ((!app_le_audio_ucst_check_pause_stream()) && (APP_LE_AUDIO_MODE_BCST != g_lea_ctrl.next_mode)) {
                if (!g_lea_ucst_ctrl.is_cig_created) {
                    g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_SET_CIG_PARAMETER;
                    if (APP_LE_AUDIO_UCST_TEST_MODE_CIG_PARAM & g_lea_ucst_test_mode_flag) {
                        if (BT_STATUS_SUCCESS != app_le_audio_ucst_test_mode_set_cig_param()) {
                            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
                        }
                    } else {
                        if (BT_STATUS_SUCCESS != app_le_audio_ucst_set_cig_parameters()) {
                            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
                        }
                    }
                }
                break;
            }
        }
#ifdef AIR_SILENCE_DETECTION_ENABLE
        case APP_LE_AUDIO_UCST_TARGET_STOP_SPECIAL_SILENCE_DETECTION_MODE:
#endif
        case APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE:
        case APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE: {
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM;
            if (BT_STATUS_SUCCESS != app_le_audio_close_audio_transmitter()) {
                g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
            }
            break;
        }

        default:
            break;
    }
}

void app_le_audio_ucst_close_audio_transmitter_cb(void)
{
    LE_AUDIO_MSGLOG_I("[APP][U] close_audio_transmitter_cb, target:%x->%x stream_state:%x->%x p:%x r:%x", 6,
                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release);

    if (APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM != g_lea_ucst_ctrl.next_stream_state) {
        return;
    }

    g_lea_ucst_ctrl.curr_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM;
    g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;

    switch (g_lea_ucst_ctrl.curr_target) {
        case APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE:
        case APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE:
        case APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE:
        case APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE: {
            if (g_lea_ucst_ctrl.is_cig_created) {
                g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_REMOVE_CIG;
                if (BT_STATUS_SUCCESS != app_le_audio_ucst_remove_cig()) {
                    g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
                }
                return;
            }
            g_lea_ucst_ctrl.curr_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;

            if (app_le_audio_ucst_check_pause_stream() || (g_lea_ucst_ctrl.release) ||
                (((APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
                  (APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target)) &&
                 (APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.next_target))) {
                g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                app_le_audio_ucst_reset_release();
                if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode) {
                    /* [Switch streaming mode] UCST -> BCST */
                    app_le_audio_start_broadcast();

                } else if (APP_LE_AUDIO_MODE_DISABLE == g_lea_ctrl.next_mode) {
                    g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_NONE;
                    g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
                    app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_CIS;
                    app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, &lea_mode);
                }
                return;
            }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
            LE_AUDIO_MSGLOG_I("[APP][U] close_audio_transmitter_cb, active:%x->%x", 2,
                              g_lea_ucst_ctrl.curr_group,
                              g_lea_ucst_ctrl.next_group);
            if ((APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.curr_group) &&
                (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.next_group)) {
                if (g_lea_ucst_ctrl.curr_group != g_lea_ucst_ctrl.next_group) {
                    g_lea_ucst_ctrl.curr_group = g_lea_ucst_ctrl.next_group;
                    g_lea_ucst_ctrl.next_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                        APP_LE_AUDIO_EVENT_ACTIVE_DEVICE_CHANGED, NULL, 0,
                                        NULL, 0);
                    ble_tbs_switch_device_completed();
                } else {
                    g_lea_ucst_ctrl.next_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
                }
            }
#endif

#ifdef AIR_SILENCE_DETECTION_ENABLE
            if (APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE == g_lea_ucst_ctrl.next_target) {
                app_le_audio_silence_detection_handle_event(APP_LE_AUDIO_SILENCE_DETECTION_EVENT_START_SPECIAL_SILENCE_DETECTION, NULL);
            }
            else
#endif
            {
                g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                app_le_audio_ucst_start();
            }
            break;
        }

#ifdef AIR_SILENCE_DETECTION_ENABLE
        /* Audio transmitter may be closed because of LE link disconnected. */
        case APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE:
        case APP_LE_AUDIO_UCST_TARGET_STOP_SPECIAL_SILENCE_DETECTION_MODE: {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
            LE_AUDIO_MSGLOG_I("[APP][SD] close_audio_transmitter_cb, active:%x->%x", 2,
                              g_lea_ucst_ctrl.curr_group,
                              g_lea_ucst_ctrl.next_group);
            if ((APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.curr_group) &&
                (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.next_group)) {
                if (g_lea_ucst_ctrl.curr_group != g_lea_ucst_ctrl.next_group) {
                    g_lea_ucst_ctrl.curr_group = g_lea_ucst_ctrl.next_group;
                    g_lea_ucst_ctrl.next_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                        APP_LE_AUDIO_EVENT_ACTIVE_DEVICE_CHANGED, NULL, 0,
                                        NULL, 0);
                    ble_tbs_switch_device_completed();
                } else {
                    g_lea_ucst_ctrl.next_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
                }
            }
#endif
            g_lea_ucst_ctrl.curr_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
            app_le_audio_silence_detection_handle_event(APP_LE_AUDIO_SILENCE_DETECTION_EVENT_SPECIAL_SILENCE_DETECTION_STOPPED, NULL);
            break;
        }
#endif
        default:
            break;
    }
}

#ifdef AIR_LE_AUDIO_DO_NOT_STOP_CALL_MODE_WHEN_CALL_EXIST
/* When there is any active call when MIC0 USB port is disabled, call mode is not stopped. Therefore, need to check
  * if call mode need be stopped when all call ends.
  */
void app_le_audio_ucst_handle_call_end_event(void)
{
    uint8_t streaming_port = app_le_audio_get_streaming_port();//app_le_audio_ucst_get_streaming_port

    app_le_audio_ucst_target_t curr_target = app_le_audio_ucst_get_curr_target();
    bool timer_exist = app_le_audio_ucst_is_delay_disconnect_timer_exist();
    bool call_exist = app_le_audio_usb_hid_call_existing();

    LE_AUDIO_MSGLOG_I("[APP][U] handle call end: call_exist:%d streaming_port:%x curr_target:%x timer_exist:%d", 4,
                      call_exist, streaming_port, curr_target, timer_exist);

    if (!call_exist &&
        !(APP_LE_AUDIO_USB_PORT_MASK_MIC_0 & streaming_port) &&
        !timer_exist &&
        APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == curr_target) {
        if (APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL != app_le_audio_ucst_get_create_cis_mode()) {
            app_le_audio_ucst_stop(true);
        }
    }
}
#endif


void app_le_audio_ucst_reset_all_cis_info(void)
{
    uint8_t i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;
    while (i > 0) {
        i--;
        g_lea_ucst_cis_info[i].cis_handle = BT_HANDLE_INVALID;
        g_lea_ucst_cis_info[i].acl_handle = BT_HANDLE_INVALID;
        g_lea_ucst_cis_info[i].cis_status = APP_LE_AUDIO_UCST_CIS_IDLE;
    }
}

uint8_t app_le_audio_ucst_get_cis_num(void)
{
    uint8_t i, cis_num = 0;

    i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;

    while (i > 0) {
        i--;
        if ((BT_HANDLE_INVALID != g_lea_ucst_cis_info[i].cis_handle) &&
            (BT_HANDLE_INVALID != g_lea_ucst_cis_info[i].acl_handle)) {
            cis_num++;
        }
    }

    return cis_num;
}

bool app_le_audio_ucst_is_streaming(void)
{
    if (APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING == g_lea_ucst_ctrl.curr_stream_state) {
        return true;
    }
    return false;
}


/* Check the enable status only. Do not check usb ready. Decide the unicast mode (media or call) based on usb port enable status only. */
uint8_t app_le_audio_ucst_get_streaming_port(void)
{
    uint8_t streaming_port = app_le_audio_get_streaming_port();

    switch (g_lea_ucst_ctrl.create_cis_mode) {
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_6_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_6_2:
        case APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_UNIDIRECTIONAL: {
            streaming_port = (APP_LE_AUDIO_USB_PORT_MASK_SPK_0);
#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
            streaming_port |= APP_LE_AUDIO_STREAM_PORT_MASK_LINE_IN;
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
            streaming_port |= APP_LE_AUDIO_STREAM_PORT_MASK_I2S_IN;
#endif
            break;
        }

        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_2:
        //case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_9_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_9_2:
            //case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_10:
        {
            streaming_port = (APP_LE_AUDIO_USB_PORT_MASK_MIC_0);
            break;
        }
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_3:
        //case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_5:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_2:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_2:
        //case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_11_1:
        case APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_11_2:
        case APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL: {
            streaming_port = (APP_LE_AUDIO_USB_PORT_MASK_SPK_0 | APP_LE_AUDIO_USB_PORT_MASK_MIC_0);
            break;
        }
        default:
            break;
    }

    return streaming_port;
}

uint16_t app_le_audio_ucst_get_audio_context_type(void)
{
    uint16_t context_type;

    /* get context type */
    if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
        context_type = (g_lea_ucst_qos_params_spk_0.sampling_freq == g_lea_ucst_qos_params_mic_0.sampling_freq) ? AUDIO_CONTENT_TYPE_CONVERSATIONAL : AUDIO_CONTENT_TYPE_GAME;
    } else {
        context_type = AUDIO_CONTENT_TYPE_MEDIA;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] get_audio_context_type, context_type:%04x", 1, context_type);
    return context_type;
}

app_le_audio_ase_codec_t * app_le_audio_ucst_get_ase_codec_config(uint16_t context_type, bt_le_audio_direction_t direction)
{
    uint8_t i, tmp, ase_idx;

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= g_lea_ucst_ctrl.curr_group) {
        LE_AUDIO_MSGLOG_I("[APP][U] get_ase_codec_config, no active group!", 0);
        return NULL;
    }
#endif

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (0 == app_le_audio_ucst_get_group_link_num_ex(g_lea_ucst_ctrl.curr_group))
#else
    if (0 == app_le_audio_ucst_get_link_num_ex())
#endif
    {
        LE_AUDIO_MSGLOG_I("[APP][U] get_ase_codec_config, no connection!", 0);
        return NULL;
    }

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        if ((BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) &&
            (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC <= g_lea_ucst_link_info[i].curr_state)) {
            for (ase_idx = 0; ase_idx < APP_LE_AUDIO_UCST_ASE_MAX_NUM; ase_idx++) {
                if ((ASE_STATE_IDLE != g_lea_ucst_link_info[i].ase[ase_idx].curr_state) &&
                    (direction == g_lea_ucst_link_info[i].ase[ase_idx].direction) &&
                    (context_type & g_lea_ucst_link_info[i].ase[ase_idx].codec_state.audio_contexts)) {
                    return &g_lea_ucst_link_info[i].ase[ase_idx].codec_state;
                }
            }
        }
    }

    return NULL;
}

bt_status_t app_le_audio_ucst_check_ase_codec_config(bt_handle_t handle)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret = BT_STATUS_SUCCESS;
    uint16_t context_type;
    uint8_t ase_idx = 0, sink_ase_num = 0, source_ase_num = 0;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }

    if (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS < p_info->curr_state) {
        return BT_STATUS_BUSY;
    }

    /* get context type */
    context_type = app_le_audio_ucst_get_audio_context_type();

    /* get the num of SINK ASE to be configured */
    sink_ase_num = app_le_audio_ucst_get_sink_ase_num(p_info);
    p_info->cis_num = sink_ase_num;

#if 0 // For Airoha device
    if ((APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1 > g_lea_ucst_ctrl.create_cis_mode) &&
        (0 != p_info->sink_location_num) && (p_info->sink_ase_num >= (p_info->sink_location_num * 2))) {
        sink_ase_num = (p_info->sink_location_num * 2); /* call mode and media mode */
    }
#endif
    /* get the num of SOURCE ASE to be configured */
    source_ase_num = app_le_audio_ucst_get_source_ase_num(p_info);

    if (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_1 == g_lea_ucst_ctrl.create_cis_mode) {
        p_info->cis_num = 2;

    } else if (p_info->cis_num < source_ase_num) {
        p_info->cis_num = source_ase_num;
    }

    uint8_t i;

    /* SINK ASE */
    for (i = 0; i < sink_ase_num; i++) {
        if (p_info->sink_ase_num < (sink_ase_num * APP_LE_AUDIO_UCST_MAX_MODE_NUM)) {
            ase_idx = i;
        } else {
            /* config SINK ASE for media and call */
            /* earbuds: media(ase[0]), call(ase[1]) */
            /* headset: media(ase[0], ase[1]), call(ase[2], ase[3]) */
            if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
                /* call mode */
                ase_idx = sink_ase_num + i;
            } else {
                /* media mode */
                ase_idx = i;
            }

        }
        LE_AUDIO_MSGLOG_I("[APP][ASE] check_ase_codec_config, handle:%x ASE[%d]: audio_contexts:%04x context_type:%04x", 4,
                            handle,
                            ase_idx,
                            p_info->ase[ase_idx].codec_state.audio_contexts,
                            context_type);
        if (p_info->ase[ase_idx].curr_state > ASE_STATE_QOS_CONFIGURED) {
            return BT_STATUS_UNSUPPORTED;
        }

        if ((p_info->ase[ase_idx].curr_state == ASE_STATE_IDLE) ||
            (p_info->ase[ase_idx].codec_state.audio_contexts != context_type)) {
            return BT_STATUS_FAIL;
        }
    }

    /* SOURCE ASE */
    if (APP_LE_AUDIO_UCST_IS_CALL_MODE) {
        for (i = 0; i < source_ase_num; i++) {
            ase_idx = p_info->source_ase_idx + i;
            LE_AUDIO_MSGLOG_I("[APP][ASE] check_ase_codec_config, handle:%x ASE[%d] audio_contexts:%04x context_type:%04x", 4,
                                handle,
                                ase_idx,
                                p_info->ase[ase_idx].codec_state.audio_contexts,
                                context_type);
            if (p_info->ase[ase_idx].curr_state > ASE_STATE_QOS_CONFIGURED) {
                return BT_STATUS_UNSUPPORTED;
            }

            if ((p_info->ase[ase_idx].curr_state == ASE_STATE_IDLE) ||
                (p_info->ase[ase_idx].codec_state.audio_contexts != context_type)) {
                return BT_STATUS_FAIL;
            }
        }
    }

    return ret;
}


void app_le_audio_ucst_start(void)
{
    uint8_t streaming_port = app_le_audio_ucst_get_streaming_port();
    uint8_t i, link_idx;
    bool bidirectional = false, is_ready = false;

    if (APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL <= g_lea_ucst_ctrl.pause_stream) {
        LE_AUDIO_MSGLOG_I("[APP][U] start, p:%x", g_lea_ucst_ctrl.pause_stream);
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] start, target:%x->%x stream_state:%x->%x p:%x r:%x c:%x port:%x", 8,
                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release,
                      g_lea_ucst_ctrl.create_cis_mode, streaming_port);

#ifdef AIR_SILENCE_DETECTION_ENABLE
    /* Stop special silence detection mode first if it exists. */
    if (app_le_audio_silence_detection_is_speical_silence_detection_ongoing()) {
        app_le_audio_silence_detection_handle_event(APP_LE_AUDIO_SILENCE_DETECTION_EVENT_START_OTHER_MODE, NULL);
        return;
    }
#endif

    if (!streaming_port) {
        //if (APP_LE_AUDIO_UCST_TARGET_NONE != g_lea_ucst_ctrl.curr_target) {
        //    g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        //}
        return;
    }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    LE_AUDIO_MSGLOG_I("[APP] start, active_group:%x->%x latest:%x", 3,
                      g_lea_ucst_ctrl.curr_group,
                      g_lea_ucst_ctrl.next_group,
                      g_lea_ucst_ctrl.latest_group);

    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.next_group) {
        /* change active group */
        return;
    }

    if ((APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_ctrl.curr_group) &&
        (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.latest_group)) {
        app_le_audio_ucst_set_active_group(g_lea_ucst_ctrl.latest_group);
        return;
    }

    app_le_audio_ucst_group_info_t *p_group_info = NULL;

    if (NULL == (p_group_info = app_le_audio_ucst_get_group_info(g_lea_ucst_ctrl.curr_group))) {
        LE_AUDIO_MSGLOG_I("[APP][U] start, invalid group:%x", 1, g_lea_ucst_ctrl.curr_group);
        return;
    }

#if 0
    if (!app_le_audio_ucst_is_group_device_all_connected(g_lea_ucst_ctrl.curr_group)) {
        app_le_audio_ucst_connect_group_device(g_lea_ucst_ctrl.curr_group);
    }
#endif

    for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM; i++) {
        if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID != p_group_info->link_idx[i]) {
            break;
        }
    }

    if (APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM == i) {
        LE_AUDIO_MSGLOG_I("[APP][U] start, group:%x no link exist", 1, g_lea_ucst_ctrl.curr_group);
        return;
    }
#endif

    app_le_audio_ucst_lock_stream_t lock_stream = APP_LE_AUDIO_UCST_LCOK_STREAM_NONE;
    uint16_t source_supported_contexts = 0;
    uint16_t sink_supported_contexts = 0;
    uint16_t tmap_role = 0;

    for (i = 0; i < app_le_audio_ucst_get_max_link_num(); i++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (link_idx = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[i])) {
            continue;
        }
#else
        link_idx = i;
#endif

        if (BT_HANDLE_INVALID != g_lea_ucst_link_info[link_idx].handle) {
            LE_AUDIO_MSGLOG_I("[APP][U] start, check handle:%x state:%x->%x r:%x lock:%x", 5,
                              g_lea_ucst_link_info[link_idx].handle,
                              g_lea_ucst_link_info[link_idx].curr_state, g_lea_ucst_link_info[link_idx].next_state,
                              g_lea_ucst_link_info[link_idx].ase_releasing,
                              g_lea_ucst_link_info[link_idx].lock_stream);

            if ((g_lea_ucst_link_info[link_idx].ase_releasing) ||
                (APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL == g_lea_ucst_link_info[link_idx].next_state)) {
                return;
            }

            if (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC <= g_lea_ucst_link_info[link_idx].curr_state) {
                is_ready = true;
            }
            else {
                app_le_audio_ucst_check_set_ase(link_idx);
                return;
            }

#ifdef AIR_LE_AUDIO_BA_ENABLE
            if ((APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE == g_lea_ucst_link_info[link_idx].curr_state)
                && (APP_LE_AUDIO_UCST_LINK_STATE_IDLE == g_lea_ucst_link_info[link_idx].next_state)) {
                bt_status_t ret;
                g_lea_ucst_link_info[link_idx].next_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
                if (BT_STATUS_SUCCESS != (ret = app_le_audio_ucst_config_codec(g_lea_ucst_link_info[link_idx].handle))) {
                    g_lea_ucst_link_info[link_idx].next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                    LE_AUDIO_MSGLOG_I("[APP][U] codec_config failed, handle:%x ret:%x", 2, g_lea_ucst_link_info[link_idx].handle, ret);
                }
            }
#endif
            if (lock_stream < g_lea_ucst_link_info[link_idx].lock_stream) {
                lock_stream = g_lea_ucst_link_info[link_idx].lock_stream;
            }
            tmap_role |= g_lea_ucst_link_info[link_idx].tmap_role;
            source_supported_contexts |= g_lea_ucst_link_info[link_idx].source_supported_contexts;
            sink_supported_contexts |= g_lea_ucst_link_info[link_idx].sink_supported_contexts;
        }
    }

    if ((!is_ready) || (APP_LE_AUDIO_UCST_LCOK_STREAM_ALL <= lock_stream)) {
        return;
    }

    if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_MIC_0) {
        if ((AUDIO_CONTENT_TYPE_CONVERSATIONAL & source_supported_contexts) ||
            (AUDIO_CONTENT_TYPE_GAME & source_supported_contexts)) {
            bidirectional = true;
        }
        else {
            bidirectional = true;
            if (!(streaming_port & (~APP_LE_AUDIO_STREAM_PORT_MASK_MIC_0))) {
                return;
            }
        }
    }

    if ((!bidirectional) &&(!((~(AUDIO_CONTENT_TYPE_CONVERSATIONAL | AUDIO_CONTENT_TYPE_GAME)) & sink_supported_contexts))) {
        return;
    }


    if (bidirectional) {
        switch (g_lea_ucst_ctrl.curr_target) {
            case APP_LE_AUDIO_UCST_TARGET_NONE: {
                g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE;
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                break;
            }
            case APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE:
            case APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE: {
                g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE;
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE;
                break;
            }
            case APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE:
            case APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE: {
                g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE;
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                break;
            }
            default:
                break;
        }
    } else {
        if (APP_LE_AUDIO_UCST_PAUSE_STREAM_UNIDIRECTIONAL == g_lea_ucst_ctrl.pause_stream) {
            return;
        }
        if (APP_LE_AUDIO_UCST_LCOK_STREAM_UNIDIRECTIONAL == lock_stream) {
            return;
        }
        switch (g_lea_ucst_ctrl.curr_target) {
            case APP_LE_AUDIO_UCST_TARGET_NONE: {
                g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE;
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                break;
            }
            case APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE:
            case APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE: {
                g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE;
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                break;
            }
            case APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE:
            case APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE: {
                g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE;
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE;
                break;
            }
            default:
                break;
        }
    }

    if (0 != tmap_role) {
        if ((BLE_TMAP_ROLE_MASK_CT != (tmap_role & BLE_TMAP_ROLE_MASK_CT)) && (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
            g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][U] start, target:%x->%x", 2, g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target);

    {
        app_le_audio_ucst_link_info_t *p_info = NULL;
        uint8_t i, tmp;
        bt_status_t ret = BT_STATUS_SUCCESS;

        for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
            if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= g_lea_ucst_ctrl.curr_group) {
                break;
            }
            if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
                continue;
            }
#else
            i = tmp;
#endif
            p_info = &g_lea_ucst_link_info[i];

            if ((APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH > p_info->curr_state) || (APP_LE_AUDIO_UCST_TARGET_NONE != g_lea_ucst_ctrl.next_target)) {
                app_le_audio_ucst_increase_connection_config_speed(p_info->handle);
            }
            ret = app_le_audio_ucst_check_ase_codec_config(p_info->handle);
            if (BT_STATUS_FAIL == ret) {//APP_LE_AUDIO_UCST_IS_CALL_MODE is confirmed, check ASE's codec
                p_info->curr_state = APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE;
                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC;
                if (BT_STATUS_SUCCESS == app_le_audio_ucst_config_codec(p_info->handle)) {
                    is_ready = false;
                }
            }
            else {
            }
        }
    }

    if (!is_ready) {
        return;
    }

    if ((APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
        (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
#ifdef AIR_SILENCE_DETECTION_ENABLE
        if (APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) {
            app_le_audio_silence_detection_set_status(APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_SILENCE);
        }
#endif
        if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
            (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM;
            if (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) {
                streaming_port |= (APP_LE_AUDIO_USB_PORT_MASK_SPK_0 | APP_LE_AUDIO_USB_PORT_MASK_MIC_0);
            }
            if (BT_STATUS_SUCCESS != app_le_audio_open_audio_transmitter(bidirectional, streaming_port)) {
                g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
            }
            return;
        }

        if (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state &&
            APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM == g_lea_ucst_ctrl.next_stream_state) {
            /* This should not happen. Opening audio transmitter should be finished before a new LE link is connected and codec configured on that link. */
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM;
            return;
        }

        if (APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.next_target &&
            APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state &&
            APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM == g_lea_ucst_ctrl.next_stream_state) {
            /* Clear the STOP/DEINIT state for next_transmitter_state, if port stop is received before the current port play. */
            /* SPK_0 */
            if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_SPK_0)) {
                app_le_audio_start_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_SPK_0);
            }

            /* SPK_1 */
            if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_SPK_1)) {
                app_le_audio_start_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_SPK_1);
            }
#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
            /* Line-In */
            if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_LINE_IN)) {
                app_le_audio_start_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_LINE_IN);
            }

            /* I2S-In */
            if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_I2S_IN)) {
                app_le_audio_start_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_I2S_IN);
            }

#endif
            /* MIC_0 */
            if (bidirectional) {
                if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_MIC_0)) {
                    app_le_audio_start_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_MIC_0);
                }
            }

            return;
        }

        if (((APP_LE_AUDIO_UCST_STREAM_STATE_SET_CIG_PARAMETER == g_lea_ucst_ctrl.curr_stream_state) ||
            (APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM == g_lea_ucst_ctrl.curr_stream_state) ||
            (APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING == g_lea_ucst_ctrl.curr_stream_state)) &&
            (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {

            /* check the all link */
            for (i = 0; i < app_le_audio_ucst_get_max_link_num(); i++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (link_idx = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[i])) {
                    continue;
                }
#else
                link_idx = i;
#endif
                if (BT_HANDLE_INVALID != g_lea_ucst_link_info[link_idx].handle) {
                    app_le_audio_ucst_check_set_ase(link_idx);
                }
            }

            if (APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.next_target) {
                app_le_audio_stream_port_t port;
                for (port = APP_LE_AUDIO_STREAM_PORT_SPK_0; port < APP_LE_AUDIO_STREAM_PORT_MAX; port++) {
                    if (streaming_port & (0x01 << port)) {
                        if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(port)) {
                            app_le_audio_start_audio_transmitter(port);
                        }
                    }
                }
            }
            return;
        }
    }

    if ((APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE == g_lea_ucst_ctrl.curr_target) ||
        (APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == g_lea_ucst_ctrl.curr_target)) {
        if ((APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING == g_lea_ucst_ctrl.curr_stream_state) &&
            (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {

            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_STREAMING;

            if (0 != app_le_audio_ucst_get_cis_num()) {
                if (app_le_audio_ucst_disable_ase_when_state_match_streaming(false)) {
                    return;
                }
            }

            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM;
            if (BT_STATUS_SUCCESS != app_le_audio_close_audio_transmitter()) {
                g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
            }
            return;
        }
        else if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
            (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {
            //Error handle: curr_target start fail
            g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
            g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
            app_le_audio_ucst_start();
        }
    }
}


bool app_le_audio_ucst_stop(bool restart)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t i, tmp;

    LE_AUDIO_MSGLOG_I("[APP][U] stop, target:%x->%x stream_state:%x->%x p:%x r:%x c:%x restart:%x", 8,
                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                      g_lea_ucst_ctrl.pause_stream, g_lea_ucst_ctrl.release,
                      g_lea_ucst_ctrl.create_cis_mode, restart);

    app_le_audio_ucst_stop_delay_disconnect_timer();

    if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
        (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {
        if (restart) {
            app_le_audio_ucst_start();
            return true;
        }
        return false;
    }
    {
        for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
            if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
                continue;
            }
#else
            i = tmp;
#endif
            p_info = &g_lea_ucst_link_info[i];
            if (BT_HANDLE_INVALID != p_info->handle) {
                LE_AUDIO_MSGLOG_I("[APP][U] stop, check handle:%x state:%x->%x w_ase:%x r:%x lock:%x", 6,
                                  p_info->handle,
                                  p_info->curr_state, p_info->next_state,
                                  p_info->wait_event.wait_ase_event,
                                  p_info->ase_releasing,
                                  p_info->lock_stream);
            }
        }
    }

    /*if ((APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL == g_lea_ucst_ctrl.create_cis_mode) &&
        (true == restart)) {
        return true;
    }*/

#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (app_le_audio_silence_detection_is_speical_silence_detection_ongoing()) {
        app_le_audio_silence_detection_handle_event(APP_LE_AUDIO_SILENCE_DETECTION_EVENT_STOP_ANY_MODE, (void *)restart);
        return true;
    } else {
        app_le_audio_silence_detection_stop_delay_stop_timer();
    }
#endif

    switch (g_lea_ucst_ctrl.curr_target) {
        case APP_LE_AUDIO_UCST_TARGET_NONE: {
            if ((restart) && (APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.next_target)) {
                app_le_audio_ucst_start();
                return true;
            }
            break;
        }
        case APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE: {
            g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE;
            if (restart) {
                /* start progress will correct start mode based on streaming port. */
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE;
            } else {
#ifdef AIR_SILENCE_DETECTION_ENABLE
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE;
#else
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
#endif
            }
            break;
        }
        case APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE: {
            g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE;
            if (restart) {
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE;
            } else {
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
            }
            break;
        }
        case APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE: {
            if (restart) {
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE;
            } else {
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
            }
            break;
        }
        case APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE: {
            if (restart) {
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE;
            } else {
                g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
            }
            break;
        }
        default:
            break;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] stop, target:%x->%x", 2, g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target);

    if ((APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING == g_lea_ucst_ctrl.curr_stream_state) &&
        (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {

        g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_STREAMING;
        //New interval will be updated after interval*8 = 480ms, CIS is disconnecting at that time.
        app_le_audio_ucst_increase_active_device_config_speed();

        if (app_le_audio_ucst_disable_ase_when_state_match_streaming(true)) {
            return true;
        }

        if (0 != app_le_audio_ucst_get_cis_num()) {
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM;
            if (BT_STATUS_SUCCESS != app_le_audio_close_audio_transmitter()) {
                g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
            }
        }

    }

    return true;
}



void app_le_audio_ucst_pause(void)
{
    LE_AUDIO_MSGLOG_I("[APP][U] pause, p:%x", 1, g_lea_ucst_ctrl.pause_stream);

    if (APP_LE_AUDIO_UCST_PAUSE_STREAM_DONGLE_FOTA == g_lea_ucst_ctrl.pause_stream) {
        return;
    }

    if (!app_le_audio_ucst_check_pause_stream()) {
        return;
    }

    app_le_audio_ucst_stop(false);
}

bool app_le_audio_ucst_pause_ex(app_le_audio_ucst_pause_stream_t mode)
{
    LE_AUDIO_MSGLOG_I("[APP][U] pause, p:%x->%x", 2, g_lea_ucst_ctrl.pause_stream, mode);

    if (!(g_lea_ucst_ctrl.pause_stream & mode)) {
        g_lea_ucst_ctrl.pause_stream |= mode;
#if AIR_MS_TEAMS_ENABLE
        if (APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL <= mode) {
            app_le_audio_usb_hid_handle_busy_call(true);
        }
#endif
    }

    if (!app_le_audio_ucst_check_pause_stream()) {
        return false;
    }

    return app_le_audio_ucst_stop(false);
}

void app_le_audio_ucst_resume_ex(app_le_audio_ucst_pause_stream_t mode)
{
    LE_AUDIO_MSGLOG_I("[APP][U] resume, target:%x->%x stream_state:%x->%x p:%x->%x r:%x", 7,
                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state,
                      g_lea_ucst_ctrl.pause_stream, mode,
                      g_lea_ucst_ctrl.release);

    app_le_audio_ucst_reset_release();

    if (g_lea_ucst_ctrl.pause_stream & mode) {
        g_lea_ucst_ctrl.pause_stream &= ~mode;
#if AIR_MS_TEAMS_ENABLE
        if (APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL > mode) {
            app_le_audio_usb_hid_handle_busy_call(false);
        }
#endif
    }

    if (app_le_audio_ucst_check_pause_stream()) {
        return;
    }

    if (!((APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING == g_lea_ucst_ctrl.curr_stream_state) &&
          (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state))) {
        app_le_audio_ucst_start();
    }
}

void app_le_audio_ucst_resume(void)
{
    if (app_le_audio_ucst_check_pause_stream()) {
        return;
    }

    app_le_audio_ucst_reset_release();

    if (!((APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING == g_lea_ucst_ctrl.curr_stream_state) &&
          (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state))) {
        app_le_audio_ucst_start();
    }
}

void app_le_audio_ucst_reset_param(void)
{
    /* reset ucst info */
    memset(&g_lea_ucst_ctrl, 0, sizeof(app_le_audio_ucst_ctrl_t));
    g_lea_ucst_ctrl.curr_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    g_lea_ucst_ctrl.next_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    g_lea_ucst_ctrl.latest_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;



    /* reset cis info */
    app_le_audio_ucst_reset_all_cis_info();

    app_le_audio_aird_init();

    uint8_t i = 0;
    for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
        g_lea_ucst_prepare_vcmd_conn_handle[i] = BT_HANDLE_INVALID;
        g_lea_ucst_prepare_vcmd_disconn_handle[i] = BT_HANDLE_INVALID;
    }
}

void app_le_audio_ucst_init(void)
{
    app_le_audio_ucst_reset_param();
    app_le_audio_csip_reset();

    bt_le_audio_source_init((BLE_TMAP_ROLE_MASK_CG | BLE_TMAP_ROLE_MASK_UMS | BLE_TMAP_ROLE_MASK_BMS),
                            app_le_audio_ucst_handle_source_evt,
                            APP_LE_AUDIO_UCST_LINK_MAX_NUM);

    ble_csip_init(app_le_audio_ucst_handle_csip_evt, APP_LE_AUDIO_UCST_LINK_MAX_NUM);

    ble_tmap_init(app_le_audio_ucst_handle_tmap_evt, APP_LE_AUDIO_UCST_LINK_MAX_NUM);

    ble_vcp_enhance_init(app_le_audio_vcp_handle_evt, APP_LE_AUDIO_UCST_LINK_MAX_NUM);

    ble_micp_enhance_init(app_le_audio_micp_handle_evt, APP_LE_AUDIO_UCST_LINK_MAX_NUM);

    ble_bap_client_init(app_le_audio_ucst_handle_bap_evt, APP_LE_AUDIO_UCST_LINK_MAX_NUM);
#ifdef AIR_LE_AUDIO_HAPC_ENABLE
    ble_hapc_init(app_le_audio_ucst_handle_hapc_evt, APP_LE_AUDIO_UCST_LINK_MAX_NUM);
    ble_iac_init(app_le_audio_ucst_handle_iac_evt, APP_LE_AUDIO_UCST_LINK_MAX_NUM);
#endif
}

#endif  /* AIR_LE_AUDIO_ENABLE */


/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "hal_gpt.h"
#include "FreeRTOS.h"
#include "serial_port.h"
#include "nvdm.h"
#include "atci.h"
#include "at_command_bt.h"

//BT releated
#include "bt_avm.h"
#include "bt_hfp.h"
//BT Sink releated
#include "bt_sink_srv.h"
#include "bt_sink_srv_utils.h"
#include "bt_sink_srv_common.h"
#include "bt_sink_srv_music.h"
#include "bt_sink_srv_call_audio.h"
#include "bt_sink_srv_a2dp.h"
#ifdef MTK_BT_HSP_ENABLE
#include "bt_sink_srv_hsp.h"
#endif /*MTK_BT_HSP_ENABLE*/
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
#include "bt_sink_srv_aws_mce_a2dp.h"
#endif
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#endif
#ifdef MTK_BT_SPEAKER_ENABLE
#include "speaker_fec.h"
#endif
#include "bt_utils.h"

#ifdef FPGA_ENV
#include "bt_gap_le.h"
#include "bt_hci.h"
#endif
#include "bt_sink_srv_hf.h"
#ifdef AIR_FEATURE_SINK_AVRCP_BQB
#include "bt_sink_srv_avrcp.h"
#endif
//extern global various.
extern uint32_t g_sink_srv_hf_delay_for_acqua;
extern bt_hfp_audio_codec_type_t g_sink_srv_hf_audio_codec;
bool g_sink_srv_pts_on;
extern bt_status_t bt_sink_srv_hf_switch(bool value);

//extern functions.
extern void bt_driver_trigger_controller_codedump(void);
extern uint8_t *bt_driver_get_n9_git_version(uint8_t *len);
extern uint8_t *bt_driver_get_n9_build_date(uint8_t *len);
#ifdef MTK_BT_HSP_ENABLE
extern void bt_sink_srv_hsp_button_press(void);
#endif/*MTK_BT_HSP_ENABLE*/

#ifdef MTK_BT_A2DP_AAC_ENABLE
extern void bt_sink_srv_a2dp_enable_aac(bool open_flag);
#endif /*MTK_BT_A2DP_AAC_ENABLE*/
extern bool bt_sink_srv_hf_mic_volume_change_handler(bt_sink_srv_call_audio_volume_act_t vol_act, bool min_max);
extern bt_sink_srv_state_t bt_sink_srv_hf_get_call_state(void);
extern void bt_sink_srv_avrcp_send_stop_command(void *param);
extern void bt_sink_srv_avrcp_send_vup_vdn_command(uint8_t type);
//static functions.
static atci_status_t bt_sink_srv_atci_it_handler(atci_parse_cmd_param_t *parse_cmd);
static void bt_sink_srv_atci_get_music_state(void);
static void bt_sink_srv_atci_get_call_state(void);
#ifdef AIR_FEATURE_SINK_AVRCP_BQB
extern atci_status_t bt_sink_srv_avrcp_bqb_atci_callback(atci_parse_cmd_param_t *parse_cmd);
#endif
extern bt_sink_srv_hf_context_t *g_sink_srv_hf_last_device_p;
bt_sink_srv_send_dtmf_t bt_sink_srv_dtmf_t;

//defined.
#define CMD_PARAM(s) s, bt_sink_srv_strlen(s)
#define STRCPYN(dest, source) strncpy(dest, source, strlen(source)+1);

static atci_cmd_hdlr_item_t bt_sink_srv_atci_cmd[] = {
    {
        .command_head = "AT+BTSINKIT",    /* INTERNAL USE, IT TEST */
        .command_hdlr = bt_sink_srv_atci_it_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    }
};
#ifdef AIR_FEATURE_SINK_AVRCP_BQB
static atci_cmd_hdlr_item_t bt_sink_srv_avrcp_bqb_atci_cmd[] = {
    {
        .command_head = BT_SINK_SRV_AVRCP_BQB_HEADER,
        .command_hdlr = bt_sink_srv_avrcp_bqb_atci_callback,
        .hash_value1 = 0,
        .hash_value2 = 0,
    }
};
#endif


#ifdef MTK_AWS_MCE_ENABLE
// AWS_MCE internal UT test.
#define AWS_TEST_TYPE_CMD   (0x00)
#define AWS_TEST_TYPE_RSP  (0x01)
#define AWS_TEST_REPEART_TIMES (500)

const uint32_t test_timer_dur = 100;

static uint32_t repeat_times = AWS_TEST_REPEART_TIMES;
static uint32_t aws_run_count = 0;
static bool sender = false;
static uint32_t max_rx_diff = 0;
static uint32_t sum_rx_diff = 0;
static uint32_t fail_num = 0;
static uint32_t aws_mce_sink_rx_count = 0;

static void bt_sink_srv_clear_test_context(void)
{
    repeat_times = AWS_TEST_REPEART_TIMES;
    aws_run_count = 0;
    sender = false;
    max_rx_diff = 0;
    sum_rx_diff = 0;
    fail_num = 0;
}

static void bt_sink_srv_aws_mce_test(uint8_t type)
{
    static uint32_t last_rx_count = 0;
    bt_status_t ret = BT_STATUS_SUCCESS;
    uint32_t rx_period = 0;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &aws_mce_sink_rx_count);
    if (aws_run_count == 0) {
        last_rx_count = aws_mce_sink_rx_count;
    }
    bt_utils_assert(((sender && type == AWS_TEST_TYPE_CMD) || (!sender && type == AWS_TEST_TYPE_RSP)) && "TYPE error");
    
    bt_sink_srv_report("[AWS_TEST] send aws_test, aws_run_count:%d, repeat_times:%d, ave: %d, max: %d, fail_num: %d", 5, 
        aws_run_count, repeat_times, sum_rx_diff / repeat_times, max_rx_diff, fail_num);
    if (aws_run_count < repeat_times) {
    } else if (repeat_times) {
        atci_response_t *output = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
        bt_utils_assert(output && "Error: No extra heap");
        if (NULL != output) {
            output->response_len = strlen("AWS_TEST PASS");
            memcpy((void *)(output->response_buf), "AWS_TEST PASS", output->response_len);
            output->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
            atci_send_response(output);
            vPortFree(output);
            bt_sink_srv_clear_test_context();
        }
    }

    rx_period = aws_mce_sink_rx_count - last_rx_count;
    last_rx_count = aws_mce_sink_rx_count;
    {
        sum_rx_diff += rx_period;
        if (rx_period > max_rx_diff) {
            max_rx_diff = rx_period;
        }
        if (rx_period > test_timer_dur * 1000) {
            fail_num++;
        }
    }

    bt_sink_srv_report_id("[AWS_TEST] send aws_test, type: %d, rx_period: %d, test_timer_dur:%d",
        3, type, rx_period, test_timer_dur);
    if (ret == BT_STATUS_SUCCESS) {
        ++aws_run_count;
    }
}

static void bt_sink_srv_aws_mce_test_callback(bt_aws_mce_report_info_t *para)
{
    uint8_t *payload = (uint8_t *)para->param;
    uint8_t test_type = payload[0];
    uint32_t remote_sn = payload[1] | (payload[2] << 8) | (payload[3] << 16) | (payload[4] << 24);
    bt_sink_srv_report_id("[AWS_TEST]aws_mce_test_callback, rmt_sn: %d", 1, remote_sn);

    if (test_type == AWS_TEST_TYPE_CMD) {
        bt_sink_srv_aws_mce_test(AWS_TEST_TYPE_RSP);
    } else {
        if (aws_run_count != remote_sn + 1) {
            return;
        }
        bt_sink_srv_aws_mce_test(AWS_TEST_TYPE_CMD);
    }
}

#define BT_SINK_SRV_AWS_MCE_ACTION_FOR_DEBUG_PAYLOAD_LEN (5)
static void bt_sink_srv_aws_mce_send_action_for_debug(void)
{
    bt_aws_mce_report_info_t info = {0};
    uint8_t payload[BT_SINK_SRV_AWS_MCE_ACTION_FOR_DEBUG_PAYLOAD_LEN] = {0xFF, 0x01, 0x02, 0x03, 0x04};

    info.module_id = BT_AWS_MCE_REPORT_MODULE_SINK_CALL;
    info.param_len = BT_SINK_SRV_AWS_MCE_ACTION_FOR_DEBUG_PAYLOAD_LEN;
    info.param = (void *)payload;
    bt_aws_mce_report_send_event(&info);
}

static uint16_t bt_sink_srv_parse_digital(const uint8_t *string, uint8_t **result)
{
    uint16_t MBp = 0;
    uint8_t *ptr = (uint8_t *)string;

    uint8_t ext = *ptr++ - '0';
    while (ext < 10) {
        MBp = MBp * 10 + ext;
        ext = (*ptr++ - '0');
    }
    if (result) {
        *result = ptr;
    }

    return MBp;
}
#endif /*MTK_AWS_MCE_ENABLE*/

static void bt_sink_srv_cmd_copy_str_to_addr(uint8_t *addr, const char *str)
{
    unsigned int i = 0, value = 0;
    int using_long_format = 0;
    int using_hex_sign = 0;
    int result = 0;
    if (str[2] == ':' || str[2] == '-') {
        using_long_format = 1;
    }
    if (str[1] == 'x') {
        using_hex_sign = 2;
    }
    for (i = 0; i < 6; i++) {
        result = sscanf(str + using_hex_sign + i * (2 + using_long_format), "%02x", &value);
        if (result <= 0) {
            bt_sink_srv_report("bt gatt demo addr convert fail");
        }
        addr[5 - i] = (uint8_t) value;
    }
}

#ifdef MTK_BT_AT_COMMAND_ENABLE
extern atci_bt_context_struct_t *atci_bt_cntx_p;
#endif

static int16_t bt_sink_srv_cmd_entry(const char *string)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_SP, NULL);

    bt_sink_srv_report("[SINK][ATCI] string:%s", string);

    /*  FOR BQB */
    if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("ATD01"))) { //DIAL NUMBER
        char *number = "1234567";
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_DIAL_NUMBER, number);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("ATD02"))) { //DIAL memory
        uint8_t memory = (uint8_t)((*(string + 6)) - '0');
        bt_sink_srv_hf_dial_memory(&memory);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("AVT"))) { //attach voice tag
        bt_sink_srv_hf_attach_voice_tag();
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("DTMF"))) { //attach voice tag
        uint8_t dtmf = (uint8_t)((*(string + 5)) - '0');
        bt_sink_srv_memcpy(bt_sink_srv_dtmf_t.address, (const void*)&(g_sink_srv_hf_last_device_p->link.address), BT_BD_ADDR_LEN);
        bt_sink_srv_dtmf_t.code = dtmf + '0';
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_DTMF, &bt_sink_srv_dtmf_t);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("AT+BIEV"))) {
        bt_hfp_send_command(g_sink_srv_hf_last_device_p->link.handle, (uint8_t *)(string), strlen(string));
    }
    /* ACTION PART */
    if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("ANS"))) { //ANSWER
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_ANSWER, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("REJ"))) { //REJECT
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_REJECT, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("HAG"))) { //HANG UP
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_HANG_UP, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("DLN"))) { //DIAL LAST NUMBER
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_DIAL_LAST, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("QCL"))) { //QUERY CALL LIST
        uint8_t bd_address[6] = {0};
        const char *address_str = string + strlen("QCL,");
        bt_sink_srv_cmd_copy_str_to_addr(bd_address, address_str);
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_QUERY_CALL_LIST, &bd_address);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("RAH"))) { // RELEASE ALL HELD
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("RAA"))) { // RELEASE AND ACCEPT
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_3WAY_RELEASE_ACTIVE_ACCEPT_OTHER, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("HAA"))) { // HOLD AND ACCEPT
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SAP"))) { // SWITCH AUDIO PATH
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_SWITCH_AUDIO_PATH, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SAD"))) { // SWITCH AUDIO DEVICE
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_SWITCH_AUDIO_DEVICE, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("RSI"))) { // RELEASE SPECIAL INDEX
        uint8_t index = (uint8_t)((*(string + 3)) - '0');
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_3WAY_RELEASE_SPECIAL, &index);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("HSI"))) { // HOLD SPECIAL INDEX
        uint8_t index = (uint8_t)((*(string + 3)) - '0');
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_3WAY_HOLD_SPECIAL, &index);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("AHTC"))) { // ADD HELD TO CONVERSATION
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_3WAY_ADD_HELD_CALL_TO_CONVERSATION, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("ECT"))) { // EXPLICIT CALL TRANSFER
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_3WAY_EXPLICIT_CALL_TRANSFER, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("VRA"))) { // VOICE RECOGNITION ACTIVATE
        bool active = (bool)((*(string + 3)) - '0');
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_VOICE_RECOGNITION_ACTIVATE, &active);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("ECNR"))) {
        bool active = (bool)((*(string + 4)) - '0');
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_HF_ECNR_ACTIVATE, &active);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("CALLVUP"))) { //CALL VOLUME UP
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_CALL_VOLUME_UP, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("CALLVDN"))) { // CALL VOLUME DOWN
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_CALL_VOLUME_DOWN, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("CALLVMIN"))) { // CALL VOLUME MIN
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_CALL_VOLUME_MIN, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("CALLVMAX"))) { //CALL VOLUME MAX
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_CALL_VOLUME_MAX, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("EHFP"))) {
        bt_hfp_enable_service_record(true);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("DHFP"))) {
        bt_hfp_enable_service_record(false);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("ACQUA,"))) {
        g_sink_srv_hf_delay_for_acqua = (uint32_t)strtoul(string + 6, NULL, 10);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("APLBAT,"))) {
        uint8_t bat_lev = (uint8_t)((*(string + 7)) - '0');
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_REPORT_BATTERY, (void *)&bat_lev);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SIRI"))) {
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_HF_GET_SIRI_STATE, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("VUP"))) { //BT  MUSIC VOLUME UP
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_VOLUME_UP, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("VDN"))) { // BT MUSIC VOLUME DOWN
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_VOLUME_DOWN, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("VMIN"))) { // BT MUSIC VOLUME MIN
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_VOLUME_MIN, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("VMAX"))) { //MUSIC VOLUME MAX
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_VOLUME_MAX, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("NEXT"))) { // BT MUSIC NEXT
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_NEXT_TRACK, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("PREV"))) { //BT MUSIC RREV
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PREV_TRACK, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("PLAY_PAUSE"))) { //BT MUSIC One key
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PLAY_PAUSE, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("PLAY"))) { // BT MUSIC PLAY
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PLAY, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("PAUSE"))) { //BT MUSIC PAUSE
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PAUSE, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("STOP"))) { //BT MUSIC STOP
        bt_sink_srv_avrcp_send_stop_command(NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("PT-VUP"))) { //BT MUSIC VUP BY PASS THROUGH CMD
        bt_sink_srv_avrcp_send_vup_vdn_command(0);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("PT-VDN"))) { //BT MUSIC VUP BY PASS THROUGH CMD
        bt_sink_srv_avrcp_send_vup_vdn_command(1);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("LMP_REJ_1M"))) {
        bt_sink_srv_music_reject_a2dp_1M();
        bt_sink_srv_report_id("Disable LMP 1M", 0);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("ASSERT"))) {
        bt_utils_assert(0);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("CVSD"))) {
        g_sink_srv_hf_audio_codec = (bt_hfp_audio_codec_type_t)BT_HFP_CODEC_TYPE_CVSD;
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("MSBC"))) {
        g_sink_srv_hf_audio_codec = (bt_hfp_audio_codec_type_t)BT_HFP_CODEC_TYPE_MSBC;
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("BATEXT"))) {
        uint8_t battery_level = atoi(string + 7);
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_REPORT_BATTERY_EXT, &battery_level);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("FW-DUMP"))) {
        bt_driver_trigger_controller_codedump();
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("REWIND_PUSH"))) { // BT MUSIC REWIND PUSH
        bt_sink_srv_avrcp_operation_state_t operation_state = BT_SINK_SRV_AVRCP_OPERATION_PRESS;
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_REWIND, &operation_state);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("REWIND_RELEASED"))) { // BT MUSIC REWIND RELEASED
        bt_sink_srv_avrcp_operation_state_t operation_state = BT_SINK_SRV_AVRCP_OPERATION_RELEASE;
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_REWIND, &operation_state);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("FAST_FORWARD_PUSH"))) { // BT MUSIC FAST FORWARD PUSH
        bt_sink_srv_avrcp_operation_state_t operation_state = BT_SINK_SRV_AVRCP_OPERATION_PRESS;
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_FAST_FORWARD, &operation_state);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("FAST_FORWARD_RELEASED"))) { // BT MUSIC FAST FORWARD RELEASED
        bt_sink_srv_avrcp_operation_state_t operation_state = BT_SINK_SRV_AVRCP_OPERATION_RELEASE;
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_FAST_FORWARD, &operation_state);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("GEA,"))) { // GET ELEMENT ATTRIBUTES
        bt_sink_srv_avrcp_get_element_attributes_parameter_t attribute_params = {0};
        attribute_params.attribute_size = 28;
        bt_avrcp_get_element_attributes_t attribute_list[7] = {{1}, {2}, {3}, {4}, {5}, {6}, {7}};
        attribute_params.attribute_list = attribute_list;
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_GET_ELEMENT_ATTRIBUTE, &attribute_params);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("GPS"))) { // GET PLAY STATUS
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_GET_PLAY_STATUS, NULL);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("GETC,"))) { // GET PLAY STATUS
        bt_avrcp_capability_types_t capability_type = (bt_avrcp_capability_types_t)((*(string + 5)) - '0');
        bt_sink_srv_avrcp_get_capability_parameter_t capability_parameter = {0};
        bt_sink_srv_memset(&capability_parameter, 0, sizeof(bt_sink_srv_avrcp_get_capability_parameter_t));
        capability_parameter.type = capability_type;
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_GET_CAPABILITY, &capability_parameter);
#ifdef MTK_BT_AVRCP_BROWSE_ENABLE
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("GET_NAME"))) {
        bt_sink_srv_avrcp_get_folder_items_parameter_t param = {0};
        param.address = NULL;
        param.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_LIST;
        param.start_item = 0;
        param.end_item = 5;
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_GET_FOLDER_ITEM, &param);
#endif
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("MICVUP"))) {
        bt_sink_srv_hf_mic_volume_change_handler(BT_SINK_SRV_CALL_AUDIO_VOL_ACT_UP, false);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("MICVDN"))) {
        bt_sink_srv_hf_mic_volume_change_handler(BT_SINK_SRV_CALL_AUDIO_VOL_ACT_DOWN, false);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("MICVMAX"))) {
        bt_sink_srv_hf_mic_volume_change_handler(BT_SINK_SRV_CALL_AUDIO_VOL_ACT_UP, true);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("MICVMIN"))) {
        bt_sink_srv_hf_mic_volume_change_handler(BT_SINK_SRV_CALL_AUDIO_VOL_ACT_DOWN, true);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("PTS-ON"))) {
        g_sink_srv_pts_on = true;
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("CALL_STATE"))) {
        bt_sink_srv_atci_get_call_state();
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("AG-ON"))) {
        bt_hfp_enable_ag_service_record(true);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("AG-OFF"))) {
        bt_hfp_enable_ag_service_record(false);
#ifdef MTK_AWS_MCE_ENABLE
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SEND_IF_APP"))) {
        bt_sink_srv_aws_mce_send_action_for_debug();
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("AWS_CHECK"))) {
        bt_sink_srv_clear_test_context();
        repeat_times = (uint32_t)bt_sink_srv_parse_digital((const uint8_t *)(string + bt_sink_srv_strlen("AWS_CHECK ")), NULL);
        sender = true;
        bt_sink_srv_aws_mce_test(AWS_TEST_TYPE_CMD);
#endif
#ifdef MTK_BT_HSP_ENABLE
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("HSP-ON"))) {
        bt_hfp_enable_service_record(false);
        bt_hsp_enable_service_record(true);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("HSP-OFF"))) {
        bt_hfp_enable_service_record(true);
        bt_hsp_enable_service_record(false);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("HSP-PRESS"))) {
        bt_sink_srv_hsp_button_press();
#endif /*MTK_BT_HSP_ENABLE*/
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("REINIT_MUSIC"))) {
        bt_sink_srv_a2dp_reinitial_sync();
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SET_SINK_LATENCY,"))) {
        uint32_t sink_lt = (uint32_t)strtoul(string + 17, NULL, 10);
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_SET_LATENCY, (void *)&sink_lt);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SET_A2DP_MTU,"))) {
        uint32_t a2dp_mtu = (uint32_t)strtoul(string + 13, NULL, 10);
        bt_a2dp_set_mtu_size(a2dp_mtu);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SET_MAX_BP,"))) {
        uint32_t max_bp = (uint32_t)strtoul(string + 11, NULL, 10);
        bt_sink_srv_music_set_max_bit_pool(max_bp);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("GET_SINK_LATENCY"))) {
        bt_sink_srv_report_id("[SINK][ATCI]get sink lt: %d", 1, bt_sink_srv_music_get_sink_latency());
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SET_ALC_STATE,"))) {
        bt_sink_srv_music_set_ALC_enable((uint8_t)strtoul(string + 14, NULL, 10));
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("GET_VOLUME,"))) {
        bt_sink_srv_get_volume(NULL, (uint8_t)strtoul(string + 11, NULL, 10));
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SET_VOLUME,"))) {
        uint8_t volume = (uint8_t)strtoul(string + 11, NULL, 10);
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_SET_VOLUME, &volume);
        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_CALL_SET_VOLUME, &volume);
#ifdef MTK_BT_SPEAKER_ENABLE
#ifdef MTK_BT_SPEAKER_FEC_ENABLE
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SET_FEC_COUNT,"))) {
        speaker_fec_set_fec_gen_count((uint8_t)strtoul(string + 14, NULL, 10));
#endif
#endif
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("PARTNER_REINIT"))) {
        bt_sink_srv_aws_mce_trigger_agent_reinitial_sync(BT_SINK_SRV_MUSIC_TYPE_CONTROL_TRIGGER_ALC);
#endif
#ifdef MTK_BT_A2DP_AAC_ENABLE
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("AAC_ON"))) {
        bt_sink_srv_a2dp_enable_aac(true);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("AAC_OFF"))) {
        bt_sink_srv_a2dp_enable_aac(false);
#endif
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SET_NO_RETRANS"))) {
        bt_sink_srv_a2dp_set_no_retransmission_mode(0, 0xffff);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("REG_VOL_CHANGE")) && dev) {
        bt_avrcp_register_notification(dev->avrcp_hd, BT_AVRCP_EVENT_VOLUME_CHANGED, 0);
    }  else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SET_ABSOLUTE")) && dev) {
        bt_avrcp_set_absolute_volume(dev->avrcp_hd, 10);
#ifdef MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("MUSIC_STOP"))) {
        bt_sink_srv_music_audio_switch(false, BT_SINK_SRV_ACTION_PAUSE);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("MUSIC_RESUME"))) {
        bt_sink_srv_music_audio_switch(true, BT_SINK_SRV_ACTION_PLAY);
#endif
#ifdef AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("HFP_STOP"))) {
        bt_sink_srv_hf_switch(false);
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("HFP_START"))) {
        bt_sink_srv_hf_switch(true);
#endif
    }
#ifdef MTK_BT_AVRCP_BROWSE_ENABLE
    else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("DISCONN_BROW"))) {
        if (dev) {
            bt_status_t status = BT_STATUS_FAIL;
            if (dev->a2dp_hd) {
                status = bt_a2dp_disconnect(dev->a2dp_hd);
            }
            if (dev->avrcp_browse_hd) {
                status = bt_avrcp_browse_disconnect(dev->avrcp_browse_hd);
                if (status == BT_STATUS_SUCCESS && dev->avrcp_hd) {
                    bt_avrcp_disconnect(dev->avrcp_hd);
                }
            }
            bt_sink_srv_report_id("[SINK][ATCI]DISCONN_BROW,br_hd: %x,hd:%x, status :%x",
                3, dev->avrcp_browse_hd, dev->avrcp_hd, status);
        }
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("BROW_SUPPORT"))) {
        bt_avrcp_init_t init;
        init.role = BT_AVRCP_ROLE_CT;
        init.support_browse = true;
        bt_avrcp_init(&init);
    }
#endif
    else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("GET_MUSIC_STATE"))) {
        bt_sink_srv_atci_get_music_state();
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("START_STREAMING"))) {
        bt_sink_srv_music_device_list_t *device_list = bt_sink_srv_music_get_played_device_list(true);
        bt_sink_srv_music_device_t *dev;
        if (device_list && device_list->number) {
            dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, &(device_list->device_list[0]));
        } else {
            dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_SP, NULL);
        }
        if(dev){
            bt_a2dp_start_streaming(dev->a2dp_hd);
        }
    } else if (0 == bt_sink_srv_memcmp(string, CMD_PARAM("SUSPEND_STREAMING"))) {
        bt_sink_srv_music_device_list_t *device_list = bt_sink_srv_music_get_played_device_list(true);
        bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
        bt_sink_srv_music_device_t *dev = ctx->run_dev;
        if(!dev){
            if (device_list && device_list->number) {
                dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, &(device_list->device_list[0]));
            } else {
                dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_SP, NULL);
            }
        }
        if(dev){
            bt_a2dp_suspend_streaming(dev->a2dp_hd);
        }
    } 
#ifdef FPGA_ENV
    else if(0 == bt_sink_srv_memcmp(string, CMD_PARAM("SET_ADV_ENABLE"))){
        bt_hci_cmd_le_set_advertising_enable_t adv_enable = {

            .advertising_enable = BT_HCI_ENABLE,

        };

        bt_hci_cmd_le_set_advertising_parameters_t adv_para = {

            .advertising_interval_min = 0x080,

            .advertising_interval_max = 0x080,

            .advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED,

            .advertising_channel_map = 7,

            .advertising_filter_policy = 0

        };

        bt_gap_le_set_advertising(&adv_enable, &adv_para, NULL, NULL);
    }
#endif
    else {
        //int16_t ret = bt_connection_manager_cmd_entry(string);
        //return ret;
    }
    bt_sink_srv_report_id("[SINK][ATCI]bt_sink_srv_cmd_entry(e)", 0);
    return 0;
}

atci_status_t bt_sink_srv_atci_it_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)bt_sink_srv_memory_alloc(sizeof(atci_response_t));
    if (NULL == response) {
        bt_sink_srv_report_id("[SINK][ATCI] malloc heap memory fail.", 0);
        return ATCI_STATUS_ERROR;
    }

    bt_sink_srv_memset(response, 0, sizeof(atci_response_t));
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+BTSINKIT=<ACTION, PARAMS> */
            if (0 == bt_sink_srv_memcmp(parse_cmd->string_ptr + 5, "SINKIT", 6)) {
                int16_t result = bt_sink_srv_cmd_entry(parse_cmd->string_ptr + 12);
                if (result == 0) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else {
                    snprintf((char *)response->response_buf,
                             ATCI_UART_TX_FIFO_BUFFER_SIZE,
                             "command error:%d\r\n",
                             result);
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                }
            } else {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;

        default :
            /* others are invalid command format */
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;
    }
    bt_sink_srv_memory_free(response);
    return ATCI_STATUS_OK;
}

void bt_sink_srv_atci_init(void)
{
    atci_status_t ret = atci_register_handler(bt_sink_srv_atci_cmd, sizeof(bt_sink_srv_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
#ifdef AIR_FEATURE_SINK_AVRCP_BQB
    bt_sink_srv_report_id("[SINK][ATCI]ret:0x%x!", 1, ret);
    if (ret == ATCI_STATUS_OK) {
        ret = atci_register_handler(bt_sink_srv_avrcp_bqb_atci_cmd, sizeof(bt_sink_srv_avrcp_bqb_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
    }
#endif
    if (ret != ATCI_STATUS_OK) {
        bt_sink_srv_report_id("[SINK][ATCI]Register fail!", 0);
    }
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_TEST, bt_sink_srv_aws_mce_test_callback);
#endif
}

void bt_sink_srv_atci_get_play_status(bt_sink_srv_avrcp_get_play_status_cnf_t *params)
{
#ifdef MTK_BT_AT_COMMAND_ENABLE
    //if (atci_bt_cntx_p->ata == true) {
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        return;
    }
    bt_sink_srv_memset(response, 0, sizeof(atci_response_t));
    snprintf((char *)response->response_buf, sizeof(response->response_buf), "+BTSINKIT:%ld,%ld,%ld,%d\r\n",
             params->status,
             params->song_length,
             params->song_position,
             params->play_status);
    response->response_len = strlen((char *)response->response_buf);
    response->response_flag = 0 | ATCI_RESPONSE_FLAG_URC_FORMAT;

    atci_send_response(response);
    vPortFree(response);
    //}
#endif
}

uint32_t bt_sink_srv_atci_get_element_attributes(bt_sink_srv_avrcp_get_element_attributes_cnf_t *params, uint32_t *parsered_num)
{
    uint32_t parsered_length = 0;
#ifdef MTK_BT_AT_COMMAND_ENABLE
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        return 0;
    }
    bt_sink_srv_memset(response, 0, sizeof(atci_response_t));
    uint32_t i = 0;
    snprintf((char *)response->response_buf, sizeof(response->response_buf), "+BTSINKIT:%ld,%d,%d,%d\r\n",
             params->status,
             params->packet_type,
             params->length,
             params->number);

    response->response_len = strlen((char *)response->response_buf);
    response->response_flag = 0 | ATCI_RESPONSE_FLAG_URC_FORMAT;

    atci_send_response(response);
    bt_sink_srv_report_id("attribute_list:0x%08x, status:0x%08x, type:0x%04x, length:0x%04x, number:0x%02x",
        5, params->attribute_list, params->status, params->packet_type, params->length, params->number);

    uint32_t parser_length = 0;
    for (i = 0; (i < params->number) && (parser_length < params->length); i++) {
        bt_avrcp_get_element_attributes_response_value_t *attribute = (bt_avrcp_get_element_attributes_response_value_t *)(((void *)(params->attribute_list)) + parser_length);
        parser_length += (sizeof(bt_avrcp_get_element_attributes_response_value_t) + attribute->attribute_value_length - 1);
        bt_sink_srv_report_id("attribute_value_length:0x%04x, parser_length:%d, attribute:0x%x", 3,
            attribute->attribute_value_length, parser_length, attribute);
        if (parser_length <= params->length) {
            parsered_length = parser_length;
            if (parsered_num) {
                (*parsered_num)++;
                bt_sink_srv_report_id("get_element_attributes-parsered_num:%d", 1, (*parsered_num));
            }
            if (attribute->attribute_value_length) {
                uint8_t *data = (uint8_t *)bt_sink_srv_memory_alloc(attribute->attribute_value_length + 1); // Add 1 byte for NULL.
                bt_utils_assert(data);
                bt_sink_srv_memset(data, 0, attribute->attribute_value_length + 1); // Add 1 byte for NULL.
                bt_sink_srv_memcpy(data, attribute->attribute_value, attribute->attribute_value_length);
                snprintf((char *)response->response_buf, sizeof(response->response_buf), "+BTSINKIT:%d,%d,%d,%s\r\n",
                         (int)attribute->attribute_id,
                         attribute->character_set_id,
                         attribute->attribute_value_length,
                         data);
                bt_sink_srv_memory_free(data);
            } else {
                snprintf((char *)response->response_buf, sizeof(response->response_buf), "+BTSINKIT:%d,%d,%d\r\n",
                         (int)attribute->attribute_id,
                         attribute->character_set_id,
                         attribute->attribute_value_length);
            }
            response->response_len = strlen((char *)response->response_buf);
            response->response_flag = 0 | ATCI_RESPONSE_FLAG_URC_FORMAT;

            atci_send_response(response);
        }
    }
    vPortFree(response);
#endif
    bt_sink_srv_report_id("get_element_attributes-attribute_list:0x%08x, status:0x%08x, type:0x%04x, length:0x%04x, number:0x%02x, parsered_length:%d",
        6, params->attribute_list, params->status, params->packet_type, params->length, params->number, parsered_length);
    return parsered_length;
}

static void bt_sink_srv_atci_get_music_state(void)
{
    bt_sink_srv_music_playback_state_t state = bt_sink_srv_get_music_state();
    atci_response_t *response = (atci_response_t *)bt_sink_srv_memory_alloc(sizeof(atci_response_t));
    if (response) {
        snprintf((char *)response->response_buf, sizeof(response->response_buf), "+Get music state:%d \r\n", state);
        response->response_len = strlen((char *)response->response_buf);
        response->response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
        bt_sink_srv_memory_free(response);
    }
    bt_sink_srv_report("music state:%d", state);
}

static void bt_sink_srv_atci_get_call_state(void)
{
    bt_sink_srv_state_t call_state = bt_sink_srv_hf_get_call_state();
    atci_response_t *response = (atci_response_t *)bt_sink_srv_memory_alloc(sizeof(atci_response_t));
    bt_sink_srv_report_id("[Sink][ATCI]call_state: 0x%x\r\n", 1, call_state);

    if (response != NULL) {
        snprintf((char *)response->response_buf, sizeof(response->response_buf), "call state: 0x%x", call_state);
        response->response_len = bt_sink_srv_strlen((char *)response->response_buf);
        response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
        bt_sink_srv_memory_free(response);
    }
}


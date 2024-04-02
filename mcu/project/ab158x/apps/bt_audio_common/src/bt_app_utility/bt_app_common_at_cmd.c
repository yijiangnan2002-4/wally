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


#include <stdlib.h>
#include "FreeRTOS.h"
#include "atci.h"

#include "bt_app_common.h"
#ifdef MTK_FOTA_VIA_RACE_CMD
#include "race_fota.h"
#endif
#ifdef MTK_IAP2_PROFILE_ENABLE
#include "mfi_coprocessor_api.h"
#endif

#if defined(SUPPORT_ROLE_HANDOVER_SERVICE)
#include "bt_role_handover.h"
#endif
#ifdef AIR_BT_FAST_PAIR_ENABLE
#include "app_fast_pair.h"
#endif
#include "bt_gap_le.h"
#include "bt_customer_config.h"
#include "bt_device_manager.h"
#include "bt_device_manager_le.h"
#include "bt_gatt_over_bredr.h"
#include "bt_callback_manager.h"
#include "bt_gattc_discovery.h"

#include "multi_ble_adv_manager.h"
#include "ui_shell_manager.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "nvkey.h"
#include "nvkey_id_list.h"

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "app_lea_service_event.h"
#endif
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio.h"
#include "app_le_audio_bis.h"
#include "app_lea_service_conn_mgr.h"
#include "bt_sink_srv_le_cap_stream.h"
#include "bt_sink_srv_le_cap.h"
#include "bt_sink_srv_le.h"
#include "ble_csis.h"
#include "bt_le_audio_sink.h"
#include "bt_sink_srv_le_volume.h"
#include "app_lea_service.h"

#include "ble_bass.h"
#include "ble_bap.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_sink_srv_le_volume.h"
#include "app_lea_service_config.h"
#endif
#ifdef AIR_LE_AUDIO_HAPS_ENABLE
#include "ble_has_def.h"
#include "ble_haps.h"
#endif
#endif

#ifdef MTK_IAP2_PROFILE_ENABLE
#include "aacp_i2c.h"
#endif
#ifdef AIR_SPEAKER_ENABLE
#include "app_speaker_le_association.h"
#include "apps_config_event_list.h"
#include "apps_events_key_event.h"
#endif
#include "bt_device_manager_le.h"

// richard for customer UI spec
#include "apps_config_event_list.h"
#include "apps_events_key_event.h"
#include "bt_sink_srv_ami.h"
#include "apps_events_interaction_event.h"
#include "app_hall_sensor_activity.h"
#include "app_customer_common.h"

#ifdef MTK_AWS_MCE_ENABLE
extern uint8_t at_config_adv_flag;
extern bt_status_t bt_app_common_advtising_stop(void);
#endif
#ifdef AIR_HEADSET_ENABLE
extern bt_status_t bt_app_common_advtising_stop(void);
#endif
static atci_status_t bt_app_comm_at_cmd_ble_adv_hdl(atci_parse_cmd_param_t *parse_cmd);
#ifdef MTK_AWS_MCE_ENABLE
static atci_status_t bt_app_comm_at_cmd_nrpa_adv_hdl(atci_parse_cmd_param_t *parse_cmd);
#endif
static atci_status_t bt_app_comm_at_cmd_fota_hdl(atci_parse_cmd_param_t *parse_cmd);
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
static atci_status_t bt_app_comm_at_cmd_rho_hdl(atci_parse_cmd_param_t *parse_cmd);
#endif
extern void bt_app_common_start_scan();
extern void bt_app_common_stop_scan();
extern const bt_bd_addr_t *bt_app_common_get_local_random_addr(void);
extern bt_status_t bt_app_common_ext_advertising_start_test(uint8_t instance);
extern bt_status_t bt_app_common_ext_advertising_stop_test(uint8_t instance);

static atci_status_t bt_app_comm_at_cmd_ble_scan_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_ble_cancel_conn_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_ble_random_addr_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_lea_adv_addr_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_set_fast_pair_tx_power_level(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_gatt_over_bredr(atci_parse_cmd_param_t *parse_cmd);

#ifdef MTK_IAP2_PROFILE_ENABLE
static atci_status_t bt_app_comm_at_cmd_get_mfi_cert_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_get_mfi_response_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_check_mfi_chip_hdl(atci_parse_cmd_param_t *parse_cmd);
#endif
static atci_status_t bt_app_comm_at_cmd_sniff_mode(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_ext_ble_adv_hdl(atci_parse_cmd_param_t *parse_cmd);


#ifdef AIR_LE_AUDIO_ENABLE
#define BLE_CCP_PARM_MAX_LEN     20

extern bt_le_sink_srv_music_active_handle g_music_active_handle;
extern uint8_t app_lea_adv_cas_announcement_type;
extern bool bt_le_audio_pts_test_enable;
extern bt_status_t ble_csis_get_sirk(bt_key_t *sirk);
extern void ble_csis_write_nvkey_sirk(bt_key_t *sirk);
extern bt_status_t bt_app_common_remove_ltk();
extern bt_status_t bt_app_common_reset_ltk();
extern bt_status_t bt_sink_srv_le_volume_pts_set_paras(uint8_t volume, uint8_t mute, uint8_t flags);
#endif

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
static atci_status_t bt_app_comm_at_cmd_le_audio_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_ble_remove_bonded_device_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_ble_random_addr_hdl(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef AIR_BT_HID_ENABLE
extern bt_status_t bt_hid_io_callback(atci_parse_cmd_param_t *parse_cmd);
#endif
#ifdef AIR_SPEAKER_ENABLE
static atci_status_t bt_app_comm_at_cmd_bt_aws_le_associate(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_sink_app_command_get_mode();
static atci_status_t bt_sink_app_command_spk_mode(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_sink_app_command_set_mode(atci_parse_cmd_param_t *parse_cmd);
static void          bt_app_comm_at_cmd_copy_str_to_addr(uint8_t *addr, const char *str);
#endif
static atci_status_t bt_app_comm_at_cmd_hfp_codec_set_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_ext_ble_ltk_2_linkkey(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_le_bond_hdl(atci_parse_cmd_param_t *parse_cmd);

#if 1	// richard for customer UI spec
static atci_status_t bt_app_comm_at_cmd_check_fw_version(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_enable_discoverable(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_enable_dut(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_factory_reset(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_channel_checking(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_enter_shipping_mode(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_HALL_check(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_power_off(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_touch_test(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_touch_check(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_wrire_HW_ver(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_IR_check(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_mem_check(atci_parse_cmd_param_t *parse_cmd);
#endif
static atci_cmd_hdlr_item_t bt_app_comm_at_cmd[] = {
    {
        .command_head = "AT+BLEADV",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ble_adv_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#ifdef MTK_AWS_MCE_ENABLE
    {
        .command_head = "AT+NRPA",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_nrpa_adv_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#endif
    {
        .command_head = "AT+FOTA",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_fota_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
    {
        .command_head = "AT+RHO",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_rho_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#endif
    {
        .command_head = "AT+BLESCAN",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ble_scan_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+BLECANCELCONN",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ble_cancel_conn_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+BLERANDOMADDR",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ble_random_addr_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+LEAADVADDR",
        .command_hdlr = bt_app_comm_at_cmd_lea_adv_addr_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+FPSTXPOWER",
        .command_hdlr = bt_app_comm_at_cmd_set_fast_pair_tx_power_level,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#ifdef MTK_IAP2_PROFILE_ENABLE
    {
        .command_head = "AT+GETMFICERT",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_get_mfi_cert_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+GETMFIRESP",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_get_mfi_response_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+CHECKMFICHIP",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_check_mfi_chip_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#endif
    {
        .command_head = "AT+SNIFF",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_sniff_mode,
        .hash_value1 = 0,
        .hash_value2 = 0,
    }
    ,
    {
        .command_head = "AT+EXTBLEADV", /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ext_ble_adv_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+GATTOVEREDR", /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_gatt_over_bredr,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+HFPCODEC", /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_hfp_codec_set_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+CTKD", /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ext_ble_ltk_2_linkkey,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    {
        .command_head = "AT+LEAUDIO",       /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_le_audio_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+BLERMBOND",     /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ble_remove_bonded_device_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#endif
    {
        .command_head = "AT+LEBOND", /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_le_bond_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#ifdef AIR_SPEAKER_ENABLE
    {
        .command_head = "AT+BTMODESET", /**< AT command string. */
        .command_hdlr = bt_sink_app_command_set_mode,
        .hash_value1 = 0,
        .hash_value2 = 0
    },
    {
        .command_head = "AT+SPKMODE", /**< AT command string. */
        .command_hdlr = bt_sink_app_command_spk_mode,
        .hash_value1 = 0,
        .hash_value2 = 0
    },
#endif
#if 1		// richard for customer UI spec
	{
		.command_head = "AT+VERSION",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_check_fw_version,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},
	{
		.command_head = "AT+DISCOVERABLE",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_enable_discoverable,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},
	{
		.command_head = "AT+DUT",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_enable_dut,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},
	{
		.command_head = "AT+FACTORYRESET",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_factory_reset,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},
	{
		.command_head = "AT+CHANNELCHECK",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_channel_checking,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},	
	{
		.command_head = "AT+SHIPPINGMODE",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_enter_shipping_mode,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},
	{
		.command_head = "AT+HALLCHECK",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_HALL_check,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},	
	{
		.command_head = "AT+PWROFF",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_power_off,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},	
	{
		.command_head = "AT+TOUCHTEST",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_touch_test,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},
	{
		.command_head = "AT+TOUCHCHECK",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_touch_check,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},
	{
		.command_head = "AT+WRITEHWVER",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_wrire_HW_ver,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},
	{
		.command_head = "AT+IRCHECK",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_IR_check,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},
	{
		.command_head = "AT+MEM",	  /**< AT command string. */
		.command_hdlr = bt_app_comm_at_cmd_mem_check,
		.hash_value1 = 0,
		.hash_value2 = 0,
	},		
#endif
};
	
#if 1	// richard for customer UI spec
static atci_status_t bt_app_comm_at_cmd_check_fw_version(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("factory_test_sw_version_read\r\n", 0);

	//strcpy((char *)(response.response_buf), FOTA_DEFAULT_VERSION);

    uint8_t version[FOTA_VERSION_MAX_SIZE] = {0};
	int32_t ret = 0;

    ret = fota_version_get(version, FOTA_VERSION_MAX_SIZE, FOTA_VERSION_TYPE_STORED);	

	if(ret == 0)
	{
		strcpy((char *)(response.response_buf), version);
		strcat((char *)(response.response_buf), "\r\n");
	}
	else
		response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_enable_discoverable(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("factory_test_enable_discoverable\r\n", 0);

    uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

//	*p_key_action = KEY_SIGNAL_DISCOVERABLE;
	*p_key_action = KEY_DISCOVERABLE;

    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
                        sizeof(uint16_t), NULL, 100);	

	strcpy((char *)(response.response_buf), "Enable Discoverable Done!\r\n");
	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_enable_dut(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("factory_test_enable_dut\r\n", 0);

    uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

	*p_key_action = KEY_ENABLE_DUT_TEST;

    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
                        sizeof(uint16_t), NULL, 100);	

	strcpy((char *)(response.response_buf), "Enable DUT Done!\r\n");
	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_factory_reset(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("factory_test_factory_reset\r\n", 0);

    uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

	*p_key_action = KEY_TEST_FACTORY_RESET;

    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
                        sizeof(uint16_t), NULL, 100);	

	strcpy((char *)(response.response_buf), "Factory Reset Done!\r\n");
	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_channel_checking(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("factory_test_channel_checking\r\n", 0);

	if(ami_get_audio_channel() == AUDIO_CHANNEL_L)
	{
#if 0
	    snprintf((char *)response.response_buf,
	             ATCI_UART_TX_FIFO_BUFFER_SIZE,
	             "Earbuds Channel: %d\r\n",
	             ami_get_audio_channel());	
#endif
		strcpy((char *)(response.response_buf), "Earbuds Channel: L\r\n");

	}
	else if(ami_get_audio_channel() == AUDIO_CHANNEL_R)
	{
		strcpy((char *)(response.response_buf), "Earbuds Channel: R\r\n");
	}
	else
	{
		strcpy((char *)(response.response_buf), "Earbuds Channel: ERROR\r\n");
		
		response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
	}

	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_enter_shipping_mode(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("enter shipping mode\r\n", 0);
	
	app_enter_shipping_mode_flag_set(true);

	ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
					APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF, NULL, 0,
					NULL, 500);

	strcpy((char *)(response.response_buf), "Enter shipping mode... wait few seconds.\r\n");
	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_HALL_check(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("hall sensor status checking\r\n", 0);

	snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
			 "HALL sensor status is %d.", get_hall_sensor_status());
	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_power_off(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("system power off...\r\n", 0);
	
	ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
						APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF, NULL, 0,
						NULL, 500);

	strcpy((char *)(response.response_buf), "power off... wait few seconds.\r\n");
	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_touch_test(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("enter touch test mode\r\n", 0);

	app_touch_key_test_status_set(0x01);
	
	strcpy((char *)(response.response_buf), "enter touch test mode.\r\n");
	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_touch_check(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("touch_check\r\n", 0);
	
	snprintf((char *)response.response_buf,
			 ATCI_UART_TX_FIFO_BUFFER_SIZE,
			 "Touch count = %d\r\n",
			 app_touch_key_test_read());	

	app_touch_key_test_status_set(0x00);

	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_wrire_HW_ver(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	APPS_LOG_MSGID_I("write hw version, parse_pos=%d\r\n", 1, parse_cmd->parse_pos);

	//race_debug_print((uint8_t *)(parse_cmd->string_ptr+parse_cmd->parse_pos), 5, "HW_version");
	app_nvkey_hw_version_set((uint8_t *)(parse_cmd->string_ptr+parse_cmd->parse_pos), 5);
	
	strcpy((char *)(response.response_buf), "write HW version done.\r\n");
	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_IR_check(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	uint16_t ir_ps = bsp_px31bf_Threshold_Factory_Calibrate();
	
	APPS_LOG_MSGID_I("IR_check = %d\r\n", 1, ir_ps);

	snprintf((char *)response.response_buf,
			 ATCI_UART_TX_FIFO_BUFFER_SIZE,
			 "IR status = %d, IR PS = %d\r\n",
			 app_get_ir_isr_status(), ir_ps);	
	

	response.response_len = strlen((char *)response.response_buf);
	atci_send_response(&response);	

    return ATCI_STATUS_OK;
}

static void at_cmd_system_query_mem(uint8_t *buf)
{
    int32_t pos = 0;
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+SYSTEM:\r\nheap information: \r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "\ttotal: %d\r\n",
                    configTOTAL_HEAP_SIZE);
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "\tcurrent free: %d\r\n",
                    xPortGetFreeHeapSize());
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "\tmini free: %d\r\n",
                    xPortGetMinimumEverFreeHeapSize());
}

static atci_status_t bt_app_comm_at_cmd_mem_check(atci_parse_cmd_param_t *parse_cmd)
{

    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

	at_cmd_system_query_mem(response.response_buf);
	response.response_len = strlen((char *)(response.response_buf));
	response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
	atci_send_response(&response);
    return ATCI_STATUS_OK;

}
#endif

#ifdef AIR_SPEAKER_ENABLE
static int16_t bt_app_common_at_cmd_convert_bt_data(const char *index, uint8_t *bt_data, uint32_t bt_data_len)
{
    int16_t result = 0;
    uint32_t total_num = strlen(index), bt_count = bt_data_len, bt_bit = 1;
    const char *temp_index = index;
    memset(bt_data, 0, bt_data_len);
    while (total_num) {
        if (*temp_index <= '9' && *temp_index >= '0') {
            bt_data[bt_count - 1] += ((*temp_index - '0') * (bt_bit * 15 + 1));
            if (bt_bit == 0) {
                bt_count--;
                bt_bit = 1;
            } else {
                bt_bit--;
            }

        } else if (*temp_index <= 'F' && *temp_index >= 'A') {
            bt_data[bt_count - 1] += ((*temp_index - 'A' + 10) * (bt_bit * 15 + 1));
            if (bt_bit == 0) {
                bt_count--;
                bt_bit = 1;
            } else {
                bt_bit--;
            }
        } else if (*temp_index <= 'f' && *temp_index >= 'a') {
            bt_data[bt_count - 1] += ((*temp_index - 'a' + 10) * (bt_bit * 15 + 1));
            if (bt_bit == 0) {
                bt_count--;
                bt_bit = 1;
            } else {
                bt_bit--;
            }
        }
        if (!bt_count) {
            break;
        }
        total_num--;
        temp_index++;
    }

    if (bt_count) {
        memset(bt_data, 0, bt_data_len);
        result = -1;
        LOG_MSGID_I(BT_APP, "[APP_SPK][BTAWS_ATCMD]convert fail", 0);
    }

    if (bt_data_len == sizeof(bt_bd_addr_t)) {
        LOG_MSGID_I(BT_APP, "[APP_SPK][BTAWS_ATCMD]LS addr: %02X:%02X:%02X:%02X:%02X:%02X", 6,
                    bt_data[5], bt_data[4], bt_data[3], bt_data[2], bt_data[1], bt_data[0]);
    } else if (bt_data_len == sizeof(bt_key_t)) {
        LOG_MSGID_I(BT_APP, "[APP_SPK][BTAWS_ATCMD]AWS key: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 16,
                    bt_data[15], bt_data[14], bt_data[13], bt_data[12], bt_data[11], bt_data[10],
                    bt_data[9], bt_data[8], bt_data[7], bt_data[6], bt_data[5], bt_data[4], bt_data[3],
                    bt_data[2], bt_data[1], bt_data[0]);
    }
    return result;
}


#define STRNCPY(dest, source) strncpy(dest, source, strlen(source)+1);
static atci_status_t bt_sink_app_command_get_mode()
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    do {
        if (mode == BT_AWS_MCE_SRV_MODE_NORMAL || mode == BT_AWS_MCE_SRV_MODE_SINGLE) {
            STRNCPY((char *)response.response_buf, "+Mode: SINGLE, ");
        } else if (mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
            STRNCPY((char *)response.response_buf, "+Mode: BROADCAST, ");
        } else if (mode == BT_AWS_MCE_SRV_MODE_DOUBLE) {
            STRNCPY((char *)response.response_buf, "+Mode: DOUBLE, ");
        } else {
            STRNCPY((char *)response.response_buf, "+Mode: Error Mode!\r\n");
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
        }

        if (role == BT_AWS_MCE_ROLE_NONE) {
            strcat((char *)response.response_buf, "Role: None.\r\n");
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (role == BT_AWS_MCE_ROLE_AGENT) {
            strcat((char *)response.response_buf, "Role: Agent.\r\n");
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (role == BT_AWS_MCE_ROLE_CLINET) {
            strcat((char *)response.response_buf, "Role: Client.\r\n");
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (role == BT_AWS_MCE_ROLE_PARTNER) {
            strcat((char *)response.response_buf, "Role: Partner.\r\n");
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else {
            strcat((char *)response.response_buf, "Role: error Role!\r\n");
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    } while (0);

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t  bt_sink_app_command_set_mode(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+BTMODESET=<action>
             * action:"mode, [role,agent address]"
             */
            uint32_t string_ptr_offset = parse_cmd->name_len + 1;
            if (0 == memcmp(parse_cmd->string_ptr + string_ptr_offset, "SINGLE", strlen("SINGLE"))) {
                bt_aws_mce_srv_mode_t mode = BT_AWS_MCE_SRV_MODE_SINGLE;
                bt_aws_mce_srv_mode_switch_t param = {0};
                param.role = BT_AWS_MCE_ROLE_NONE;
                if (bt_aws_mce_srv_switch_mode(mode, &param) == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + string_ptr_offset, "DOUBLE,AGENT", strlen("DOUBLE,AGENT"))) {
                bt_aws_mce_srv_mode_t mode = BT_AWS_MCE_SRV_MODE_DOUBLE;
                bt_aws_mce_srv_mode_switch_t param = {0};
                param.role = BT_AWS_MCE_ROLE_AGENT;
                if (bt_aws_mce_srv_switch_mode(mode, &param) == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + string_ptr_offset, "DOUBLE,PARTNER", strlen("DOUBLE,PARTNER"))) {
                bt_aws_mce_srv_mode_t mode = BT_AWS_MCE_SRV_MODE_DOUBLE;
                bt_aws_mce_srv_mode_switch_t param = {0};
                const char *addr_str = parse_cmd->string_ptr + string_ptr_offset + strlen("DOUBLE,PARTNER,");
                bt_app_comm_at_cmd_copy_str_to_addr(param.addr, addr_str);
                param.role = BT_AWS_MCE_ROLE_PARTNER;
                if (bt_aws_mce_srv_switch_mode(mode, &param) == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + string_ptr_offset, "BROADCAST,AGENT", strlen("BROADCAST,AGENT"))) {
                bt_aws_mce_srv_mode_t mode = BT_AWS_MCE_SRV_MODE_BROADCAST;
                bt_aws_mce_srv_mode_switch_t param = {0};
                param.role = BT_AWS_MCE_ROLE_AGENT;
                if (bt_aws_mce_srv_switch_mode(mode, &param) == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + string_ptr_offset, "BROADCAST,CLIENT", strlen("BROADCAST,CLIENT"))) {
                bt_aws_mce_srv_mode_t mode = BT_AWS_MCE_SRV_MODE_BROADCAST;
                bt_aws_mce_srv_mode_switch_t param = {0};
                const char *addr_str = parse_cmd->string_ptr + string_ptr_offset + strlen("BROADCAST,CLIENT,");
                bt_app_comm_at_cmd_copy_str_to_addr(param.addr, addr_str);
                param.role = BT_AWS_MCE_ROLE_CLINET;
                if (bt_aws_mce_srv_switch_mode(mode, &param) == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            }
            break;
        }

        default:
            break;
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_bt_aws_le_associate(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        LOG_MSGID_E(BT_APP, "[APP_SPK] AT CMD malloc fail", 0);
        return ATCI_STATUS_OK;
    }

    memset(response, 0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            uint16_t *key_id = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
            if (key_id == NULL) {
                break;
            }
            memset(key_id, 0, sizeof(sizeof(uint16_t)));
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "CLWL", 4)) {
                bt_device_manager_le_clear_all_bonded_info();
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "DOUBLE,", 7)) {
                if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 8, "AGENT", 5)) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                    *key_id = KEY_DOUBLE_AGENT;
                } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 8, "PARTNER", 7))  {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                    *key_id = KEY_DOUBLE_PARTNER;
                } else {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    vPortFree(key_id);
                    break;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "BROADCAST,", 10)) {
                if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 11, "AGENT", 5)) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                    *key_id = KEY_BROADCAST_AGENT;
                } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 11, "CLIENT", 6)) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                    *key_id = KEY_BROADCAST_CLIENT;
                } else {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    vPortFree(key_id);
                    break;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "UNGROUP", 7)) {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                *key_id = KEY_SPK_UNGROUP;
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "KEY,", 4)) {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                bt_key_t aws_key = {0};
                int16_t result = bt_app_common_at_cmd_convert_bt_data(parse_cmd->string_ptr + parse_cmd->name_len + 5,
                                                                      (uint8_t *)&aws_key, sizeof(bt_key_t));
                if (result == 0) {
                    app_speaker_le_ass_set_aws_key(aws_key);
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "LSADDR,", 7)) {
                /*
                bt_bd_addr_t ls_addr = {0};
                int16_t result = bt_app_common_at_cmd_convert_bt_data(parse_cmd->string_ptr + parse_cmd->name_len + 8,
                        (uint8_t *)&ls_addr, sizeof(bt_bd_addr_t));
                if (result == 0){
                    //bt_aws_ls_association_write_address(&ls_addr);
                    nvdm_status_t status = nvdm_write_data_item(NVDM_GROUP_BT_APP, NVDM_BT_APP_ITEM_FIX_ADDR, NVDM_DATA_ITEM_TYPE_RAW_DATA, &ls_addr, sizeof(bt_bd_addr_t));
                    if (status != NVDM_STATUS_OK) {
                        response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    } else{
                        response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                    }
                }
                */
            }
            if (response->response_flag == ATCI_RESPONSE_FLAG_APPEND_OK) {
                ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, key_id, sizeof(uint16_t), NULL, 0);
            } else {
                vPortFree(key_id);
            }
            break;
        }

        default:
            break;
    }

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

static atci_status_t  bt_sink_app_command_spk_mode(atci_parse_cmd_param_t *parse_cmd)
{
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_READ: {
            /* AT+SPKMODESET? */
            return bt_sink_app_command_get_mode();
        }
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+SPKMODESET=<mode>,<role>
             * mode:SINGLE,DOUBLE,BROADCAST
             * role:NONE,AGENT,PARTNER,CLIENT
             */
            uint32_t string_ptr_offset = parse_cmd->name_len + 1;
            if (0 == memcmp(parse_cmd->string_ptr + string_ptr_offset, "SINGLE", strlen("SINGLE"))) {
                atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
                if (response == NULL) {
                    LOG_MSGID_E(BT_APP, "[APP_SPK] AT CMD malloc fail", 0);
                    break;
                }
                uint16_t *key_id = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
                if (key_id == NULL) {
                    vPortFree(response);
                    break;
                }
                memset(response, 0, sizeof(atci_response_t));
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                memset(key_id, 0, sizeof(sizeof(uint16_t)));
                *key_id = KEY_SPK_SINGLE;
                ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, key_id, sizeof(uint16_t), NULL, 0);

                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((char *)response->response_buf);
                atci_send_response(response);
                vPortFree(response);
            } else {
                return bt_app_comm_at_cmd_bt_aws_le_associate(parse_cmd);
            }
            break;
        }

        default:
            break;
    }

    return ATCI_STATUS_OK;
}
#endif

static atci_status_t bt_app_comm_at_cmd_sniff_mode(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_OK};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    bt_init_feature_mask_t bt_customer_config_feature = bt_customer_config_get_feature_mask_configuration();
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+SNIFF=<action>
                * action: "ON" / "OFF"
                */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                bt_customer_config_feature |= BT_INIT_FEATURE_MASK_DISABLE_SNIFF_MODE;
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                bt_customer_config_feature &= ~BT_INIT_FEATURE_MASK_DISABLE_SNIFF_MODE;
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}


static atci_status_t bt_app_comm_at_cmd_ble_adv_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+BLEADV=<action>
                        * action: "ON" / "OFF"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                /* BLE ADV OFF */
                bt_status_t status = bt_app_common_advtising_stop();
                if (status == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                /* BLE ADV ON */
                bt_status_t status = bt_app_common_start_ble_adv_with_default_interval();
                if (status == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}
#ifdef MTK_AWS_MCE_ENABLE
static atci_status_t bt_app_comm_at_cmd_nrpa_adv_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+NRPA=<action>
                        * action: "ON" / "OFF"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                /* NRPA ADV OFF */
                bt_status_t status = bt_app_common_advtising_stop();
                if (status == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                /* NRPA ADV ON */
                bt_bd_addr_t addr;
                bt_app_common_generate_non_resolvable_private_address(addr);
                bt_app_common_store_local_random_address(&addr);
                bt_gap_le_set_random_address((bt_bd_addr_ptr_t)addr);
                bt_status_t status = bt_app_common_nrpa_adv_start();
                if (status == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}
#endif


static atci_status_t bt_app_comm_at_cmd_fota_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+FOTA=<action>
                        * action: "CANCEL"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "CANCEL", 6)) {
#if (defined MTK_FOTA_VIA_RACE_CMD) && (defined MTK_RACE_CMD_ENABLE)
                /* STOP FOTA */
                if (RACE_ERRCODE_SUCCESS == race_fota_cancel()) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
#endif
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

void bt_app_common_at_cmd_print_report(char *string)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    if (response != NULL) {
        memset(response, 0, sizeof(*response));
    } else {
        return;
    }
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
             "%s\n", (char *)string);
    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    response->response_len = strlen((char *)response->response_buf);
    if (response->response_len > 0) {
        atci_send_response(response);
    }
    vPortFree(response);
}

#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
void bt_app_common_at_cmd_rho_srv_status_callback(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    LOG_MSGID_I(BT_APP, "RHO srv status_callback role 0x%x, event %d, status 0x%x", 3, role, event, status);
    if (response != NULL) {
        memset(response, 0, sizeof(*response));
    } else {
        return;
    }
    switch (event) {
        case BT_ROLE_HANDOVER_PREPARE_REQ_IND: {
            snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                     "Alert:RHO prepare\r\n");
            break;
        }
        case BT_ROLE_HANDOVER_START_IND: {
            snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                     "Alert:RHO start\r\n");
            break;
        }
        case BT_ROLE_HANDOVER_COMPLETE_IND: {
            uint8_t role_string[20] = {0};
            if (status == BT_STATUS_SUCCESS) {
                if (role == BT_AWS_MCE_ROLE_AGENT) {
                    strcpy((char *)role_string, "(Partner now).\r\n");
                } else if (role == BT_AWS_MCE_ROLE_PARTNER) {
                    strcpy((char *)role_string, "(Agent now).\r\n");
                } else {
                    strcpy((char *)role_string, "(error now)!\r\n");
                }
                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                         "Alert:RHO succuss%s", (char *)role_string);
            } else {
                if (role == BT_AWS_MCE_ROLE_AGENT) {
                    strcpy((char *)role_string, "(Agent now).\r\n");
                } else if (role == BT_AWS_MCE_ROLE_PARTNER) {
                    strcpy((char *)role_string, "(Partner now).\r\n");
                } else {
                    strcpy((char *)role_string, "(error now)!\r\n");
                }
                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                         "Alert:RHO fail%s", (char *)role_string);
            }
            break;
        }
    }

    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    response->response_len = strlen((char *)response->response_buf);
    if (response->response_len > 0) {
        atci_send_response(response);
    }
    vPortFree(response);
}

#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
#include "bt_timer_external.h"
#define BT_EXT_TIMER_TEST_TIMER_ID             (BT_TIMER_EXT_GROUP_SINK(9))
#define BT_EXT_TIMER_TEST_TIMER_DUR            (20 * 1000)
const uint32_t g_timer_data = 0x1234;

void bt_app_common_timer_test_cb(uint32_t timer_id, uint32_t data)
{
    LOG_MSGID_I(BT_APP, "bt_app_common_timer_test_cb, id is 0x%8x, data is 0x%4x\r\n", 2, timer_id, data);
}
#endif

static atci_status_t bt_app_comm_at_cmd_rho_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    uint32_t response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    LOG_MSGID_I(BT_APP, "bt_app_comm_at_cmd_rho_hdl", 0);

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE: {
            /* AT+RHO */
            bt_status_t status = bt_role_handover_start();
            if (BT_STATUS_SUCCESS == status || BT_STATUS_BUSY == status) {
                response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                LOG_MSGID_I(BT_APP, "bt_app_comm_at_cmd_rho_hdl fail 0x%x", 1, status);
            }

            break;
        }

        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+RHO=<action>
                        * action: "TIMER"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "TIMER", 5)) {
                /* START A TIMER */
                LOG_MSGID_I(BT_APP, "start an 20s test timer!", 0);
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
                bt_timer_ext_status_t timer_ret = BT_TIMER_EXT_STATUS_SUCCESS;
                timer_ret = bt_timer_ext_start(BT_EXT_TIMER_TEST_TIMER_ID, (uint32_t)g_timer_data, BT_EXT_TIMER_TEST_TIMER_DUR, bt_app_common_timer_test_cb);
                if (timer_ret != BT_TIMER_EXT_STATUS_SUCCESS) {
                    LOG_MSGID_I(BT_APP, "start test timer fail!", 0);
                }
                LOG_MSGID_I(BT_APP, "start test timer success, id is 0x%8x, data is 0x%4x\r\n", 2, BT_EXT_TIMER_TEST_TIMER_ID, g_timer_data);
                response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
#endif
            }

            break;
        }

        default:
            break;
    }

    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    if (response != NULL) {
        memset(response, 0, sizeof(*response));
    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    response->response_flag = response_flag;
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}
#endif

static atci_status_t bt_app_comm_at_cmd_ble_scan_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+BLESCAN=<action>
                        * action: "ON" / "OFF"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                /* BLE SCAN OFF */
                bt_app_common_stop_scan();
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                /* BLE SCAN ON */
                bt_app_common_start_scan();
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_ble_cancel_conn_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE: {
            /* AT+BLECANCELCONN*/
            bt_gap_le_cancel_connection();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_ble_random_addr_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+BLERANDOMADDR=<action>
                        * action: "GET"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "GET", 3)) {
                uint8_t temp_str[30] = {0};
                bt_bd_addr_t *addr = NULL;

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                bt_gap_le_advertising_handle_t adv_handle = 0;
                bt_bd_addr_t adv_addr = {0};
                if (!multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, &adv_addr, &adv_handle)) {
                    break;
                }
                addr = &adv_addr;
#else
                addr = (bt_bd_addr_t *)bt_app_common_get_local_random_addr();
#endif
                snprintf((char *)temp_str, sizeof(temp_str), "0x%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                         (*addr)[5], (*addr)[4], (*addr)[3], (*addr)[2], (*addr)[1], (*addr)[0]);
                snprintf((char *)response.response_buf, sizeof(response.response_buf), "+Get addrss:%s\r\n", (char *)temp_str);
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_lea_adv_addr_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+LEAADVADDR=<GET> */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "GET", 3)) {
                uint8_t temp_str[30] = {0};
                bt_bd_addr_t *addr = NULL;
#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
#ifdef AIR_TWS_ENABLE
                addr = (bt_bd_addr_t *)bt_device_manager_aws_local_info_get_fixed_address();
#else
                addr = (bt_bd_addr_t *)bt_device_manager_get_local_address();
#endif
#else
                bt_gap_le_advertising_handle_t adv_handle = 0;
                bt_bd_addr_t adv_addr = {0};
                if (!multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, &adv_addr, &adv_handle)) {
                    break;
                }
                addr = &adv_addr;
#endif
                snprintf((char *)temp_str, sizeof(temp_str), "0x%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                         (*addr)[5], (*addr)[4], (*addr)[3], (*addr)[2], (*addr)[1], (*addr)[0]);
                snprintf((char *)response.response_buf, sizeof(response.response_buf), "+Get addrss:%s\r\n", (char *)temp_str);
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            }
            break;
        }
        default:
            break;
    }
#endif

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_set_fast_pair_tx_power_level(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)malloc(sizeof(atci_response_t));
    if (NULL == response) {
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0 , sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

#ifdef AIR_BT_FAST_PAIR_ENABLE
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            int8_t tx_power_level = atoi(parse_cmd->string_ptr + parse_cmd->name_len + 1);
            LOG_MSGID_I(BT_APP, "set fast pair tx power level :%d", 1, tx_power_level);

            app_fast_pair_set_tx_power_level(tx_power_level);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            break;
    }
#endif

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    free(response);
    return ATCI_STATUS_OK;
}

#ifdef MTK_IAP2_PROFILE_ENABLE

#define MAX_MFI_CERT_DATA_LEN 640
static atci_status_t bt_app_comm_at_cmd_get_mfi_cert_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    int                 r;
    uint8_t             *accessory_cert;
    uint32_t            accessory_cert_len = MAX_MFI_CERT_DATA_LEN;
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            accessory_cert = (uint8_t *)pvPortMalloc(MAX_MFI_CERT_DATA_LEN);
            if (!accessory_cert) {
                APPS_LOG_MSGID_I("malloc mfi cert data buf failed.", 0);
                break;
            }
            memset(accessory_cert, 0, MAX_MFI_CERT_DATA_LEN);

            r = mfi_coprocessor_get_accessory_cert(&accessory_cert, &accessory_cert_len);
            if (r < 0) {
                APPS_LOG_MSGID_I("get mfi cert data failed.", 0);
            } else {
                APPS_LOG_MSGID_I("get mfi cert data len: %d.", 1, accessory_cert_len);
                APPS_LOG_DUMP_I("mfi data:", accessory_cert, accessory_cert_len);
            }
            vPortFree(accessory_cert);
        }
        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}


#define MAX_MFI_RESP_DATA_LEN 64
#define MAX_MFI_CHALLENGE_DATA_LEN 32
static const uint8_t mfi_challenge_data[MAX_MFI_CHALLENGE_DATA_LEN] = {0};
static atci_status_t bt_app_comm_at_cmd_get_mfi_response_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    int                 r;
    uint8_t             *resp_data;
    uint32_t            resp_data_len = MAX_MFI_RESP_DATA_LEN;
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            resp_data = (uint8_t *)pvPortMalloc(MAX_MFI_RESP_DATA_LEN);
            if (!resp_data) {
                APPS_LOG_MSGID_I("malloc mfi resp data buf failed.", 0);
                break;
            }
            memset(resp_data, 0, MAX_MFI_CERT_DATA_LEN);

            r = mfi_coprocessor_gen_challenge_response(mfi_challenge_data, MAX_MFI_CHALLENGE_DATA_LEN, &resp_data, &resp_data_len);
            if (r < 0) {
                APPS_LOG_MSGID_I("get mfi resp data failed.", 0);
            } else {
                APPS_LOG_MSGID_I("get mfi resp data len: %d.", 1, resp_data_len);
                APPS_LOG_DUMP_I("mfi data:", resp_data, resp_data_len);
            }
            vPortFree(resp_data);
        }
        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}


#define REG_DATA_LEN_1 1
#define REG_DATA_LEN_4 4
static bool check_mfi_version_data(atci_response_t *response)
{
    uint8_t temp_buf[4] = {0};
    int32_t ret = 0;
    char *ret_str = NULL;

    /* get device version */
    ret = aacp_i2c_read(AACP_DEVICE_VERSION, temp_buf, REG_DATA_LEN_1);
    if (ret != REG_DATA_LEN_1) {
        ret_str = "FAIL: get device version fail.";
        goto func_ret;
    }
    if (temp_buf[0] != 0x07) {
        ret_str = "FAIL: invalid device version";
        goto func_ret;
    }

    /* get authentication revision */
    memset(temp_buf, 0x00, 4);
    ret = aacp_i2c_read(AACP_FIRMWARE_VERSION, temp_buf, REG_DATA_LEN_1);
    if (ret != REG_DATA_LEN_1) {
        ret_str = "FAIL: get authentication version fail.";
        goto func_ret;
    }
    if (temp_buf[0] != 0x01) {
        ret_str = "FIAL: invalid authentication version.";
        goto func_ret;
    }

    /* get authentication protocal major version */
    memset(temp_buf, 0x00, 4);
    ret = aacp_i2c_read(AACP_AUTH_PROTO_MAJOR_VERSION, temp_buf, REG_DATA_LEN_1);
    if (ret != REG_DATA_LEN_1) {
        ret_str = "FAIL: get authentication protocal major version fail.";
        goto func_ret;
    }
    if (temp_buf[0] != 0x03) {
        ret_str = "FIAL: invalid authentication protocal major version.";
        goto func_ret;
    }

    /* get authentication protocal minor version */
    memset(temp_buf, 0x00, 4);
    ret = aacp_i2c_read(AACP_AUTH_PROTO_MINOR_VERSION, temp_buf, REG_DATA_LEN_1);
    if (ret != REG_DATA_LEN_1) {
        ret_str = "FAIL: get authentication protocal minor version fail.";
        goto func_ret;
    }
    if (temp_buf[0] != 0x00) {
        ret_str = "FIAL: invalid authentication protocal minor version.";
        goto func_ret;
    }

    /* get device id */
    memset(temp_buf, 0x00, 4);
    ret = aacp_i2c_read(AACP_DEVICE_ID, temp_buf, REG_DATA_LEN_4);
    if (ret != REG_DATA_LEN_4) {
        ret_str = "FAIL: get device id fail.";
        goto func_ret;
    }
    if (temp_buf[0] != 0x00 || temp_buf[1] != 0x00 ||
        temp_buf[2] != 0x03 || temp_buf[3] != 0x00) {
        ret_str = "FIAL: invalid device id.";
        goto func_ret;
    }

func_ret:
    if (ret_str != NULL) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, ret_str);
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        return false;
    }
    return true;
}


static bool check_mfi_certifacation_data(atci_response_t *response)
{
    uint32_t idx = 0;
    bool check_result = false;
    int32_t ret = 0;
    uint8_t *accessory_cert;
    uint32_t accessory_cert_len = MAX_MFI_CERT_DATA_LEN;

    /* request certification data */
    accessory_cert = (uint8_t *)pvPortMalloc(MAX_MFI_CERT_DATA_LEN);
    if (!accessory_cert) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: malloc memory fail.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    memset(accessory_cert, 0x00, MAX_MFI_CERT_DATA_LEN);
    ret = mfi_coprocessor_get_accessory_cert(&accessory_cert, &accessory_cert_len);
    if (ret < 0) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: get certification data fail.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    /* check certification data */
    //on aacp 3.0, the certification data length is between 607 and 609, inclusive.
    if (accessory_cert_len < 607 || accessory_cert_len > 609) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: certification data length is %d.", (int)accessory_cert_len);
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    for (idx = 0; idx < accessory_cert_len; idx++) {
        if (accessory_cert[idx] != 0x00) {
            break;
        }
    }
    if (idx == accessory_cert_len) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: certification data is padding by 0x00.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    if (accessory_cert != NULL) {
        vPortFree(accessory_cert);
        accessory_cert = NULL;
    }
    check_result = true;

func_ret:
    if (accessory_cert != NULL) {
        vPortFree(accessory_cert);
        accessory_cert = NULL;
    }
    return check_result;
}


static bool check_mfi_challenge_response(atci_response_t *response)
{
    bool check_result = false;
    int32_t ret = 0;
    uint32_t idx = 0;
    uint8_t *challenge_data = NULL;
    uint8_t *challenge_response_data = NULL;
    uint32_t challenge_response_len = 0;

    /* send dummy challenge data to chip */
    challenge_data = (uint8_t *)pvPortMalloc(MAX_MFI_CHALLENGE_DATA_LEN);
    if (!challenge_data) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: malloc memory fail.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    } else {
        memset(challenge_data, 0x77, MAX_MFI_CHALLENGE_DATA_LEN);
    }

    challenge_response_data = (uint8_t *)pvPortMalloc(MAX_MFI_RESP_DATA_LEN);
    if (!challenge_response_data) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: malloc memory fail.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    } else {
        memset(challenge_response_data, 0x00, MAX_MFI_RESP_DATA_LEN);
    }

    ret = mfi_coprocessor_gen_challenge_response(challenge_data,
                                                 MAX_MFI_CHALLENGE_DATA_LEN,
                                                 &challenge_response_data,
                                                 &challenge_response_len);
    if (ret < 0) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: get challenge response data fail.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    /* check challenge response data */
    // on aacp 3.0, the challenge response data length is 64
    if (challenge_response_len != MAX_MFI_RESP_DATA_LEN) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: challenge response data length is %d.", (int)challenge_response_len);
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    for (idx = 0; idx < challenge_response_len; idx++) {
        if (challenge_response_data[idx] != 0x00) {
            break;
        }
    }
    if (idx == challenge_response_len) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: challenge response data is padding by 0x00.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    if (challenge_data != NULL) {
        vPortFree(challenge_data);
        challenge_data = NULL;
    }

    if (challenge_response_data != NULL) {
        vPortFree(challenge_response_data);
        challenge_response_data = NULL;
    }

    check_result = true;

func_ret:
    if (challenge_data != NULL) {
        vPortFree(challenge_data);
        challenge_data = NULL;
    }
    if (challenge_response_data != NULL) {
        vPortFree(challenge_response_data);
        challenge_response_data = NULL;
    }
    return check_result;
}


static atci_status_t bt_app_comm_at_cmd_check_mfi_chip_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

    do {
        if (!check_mfi_version_data(&response)) {
            break;
        }

        if (!check_mfi_certifacation_data(&response)) {
            break;
        }

        if (!check_mfi_challenge_response(&response)) {
            break;
        }
    } while (0);

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

#endif

static atci_status_t bt_app_comm_at_cmd_ext_ble_adv_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        return ATCI_STATUS_ERROR;
    }
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+EXTBLEADV=<action>,<instance>
                        * action: "ON" / "OFF"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                /* BLE ADV OFF */
                uint8_t instance = (uint8_t)strtoul(parse_cmd->string_ptr + parse_cmd->name_len + 5, NULL, 16);
                bt_status_t status = bt_app_common_ext_advertising_stop_test(instance);
                if (status == BT_STATUS_SUCCESS) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                /* BLE ADV ON */
                uint8_t instance = (uint8_t)strtoul(parse_cmd->string_ptr + parse_cmd->name_len + 4, NULL, 16);
                bt_status_t status = bt_app_common_ext_advertising_start_test(instance);
                if (status == BT_STATUS_SUCCESS) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ALL_OFF", 7)) {
                if (bt_gap_le_srv_clear_adv(NULL) == BT_GAP_LE_SRV_SUCCESS) {
                    multi_ble_adv_manager_pause_ble_adv();
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            }
            break;
        }

        default:
            break;
    }

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
static atci_status_t bt_app_comm_at_cmd_ble_remove_bonded_device_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    char *pChar = NULL;
    uint8_t buf[BT_BD_ADDR_LEN + 1] = {0};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    if (parse_cmd->mode == ATCI_CMD_MODE_EXECUTION) {
        pChar = parse_cmd->string_ptr + parse_cmd->name_len + 1;

        if (0 == memcmp(pChar, "ADDR", 4)) {
            uint8_t i;

            /* AT+BLERMBOND=ADDR,<addrType>,<addr0, addr1, addr2, addr3, addr4, addr5> */
            pChar = strtok(pChar, ",");
            for (i = 0; i < (BT_BD_ADDR_LEN + 1); i++) {
                pChar = strtok(NULL, ",");
                buf[i] = atoi(pChar);
            }
            LOG_MSGID_I(BT_APP, "[AT_CMD] BLE Remove bonded device, type:%x Addr:%x %x %x %x %x %x", 7, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

        } else if (0 == memcmp(pChar, "ALL", 3)) {
            bt_device_manager_le_clear_all_bonded_info();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

        } else if (0 == memcmp(pChar, "GET", 3)) {
            uint8_t count = bt_device_manager_le_get_bonded_number();
            bt_bd_addr_t *peer_addr = (bt_bd_addr_t *)pvPortMalloc(count * sizeof(bt_bd_addr_t));
            if (peer_addr) {
                bt_device_manager_le_get_bonded_list(peer_addr, &count);
            } else {
                count = 0;
            }
            vPortFree(peer_addr);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        if (response.response_flag == ATCI_RESPONSE_FLAG_APPEND_OK) {
            bt_device_manager_le_remove_bonded_device((bt_addr_t *)buf);
        }
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_le_audio_adv_hdl(char *pChar)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    uint8_t low_power_mode = 0;
    uint32_t size = sizeof(low_power_mode);

    /* Read system mode is in low power mode or not. */
    if ((NVKEY_STATUS_OK != nvkey_read_data(NVID_APP_SYSTEM_MODE_SETTING, &low_power_mode, &size)) || (0 == low_power_mode)) {
#ifdef MTK_AWS_MCE_ENABLE
        /* AT+LEAUDIO=ADV,<ACTION> */
        /* <ACTION>: PTS_ON, PTS_OFF, PTS_RESUME */
        if (0 == memcmp(pChar, "PTS_ON", 6)) {
            multi_ble_adv_manager_start_ble_adv();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "PTS_OFF", 7)) {
            multi_ble_adv_manager_pause_ble_adv();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "PTS_RESUME", 10)) {
            multi_ble_adv_manager_resume_ble_adv();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else {
            LOG_MSGID_I(BT_APP, "[AT_CMD] NOT in low power mode! AT+LEAUDIO=ADV can only be used in low power mode", 0);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
#else

        LOG_MSGID_I(BT_APP, "[AT_CMD] NOT in low power mode! AT+LEAUDIO=ADV can only be used in low power mode", 0);
        response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
    } else {
        /* AT+LEAUDIO=ADV,<ACTION> */
        /* <ACTION>: ON, OFF, RESUME */
        if (0 == memcmp(pChar, "ON", 2)) {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_GENERAL_ADV_FOR_TEST);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_GENERAL_ADV_FOR_TEST,
                                NULL, 0, NULL, 0);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "OFF", 3)) {
            multi_ble_adv_manager_pause_ble_adv();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "RESUME", 6)) {
            multi_ble_adv_manager_resume_ble_adv();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        }
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
extern void app_le_ull_get_sirk(bt_key_t *sirk);
extern void app_le_ull_write_nvkey_sirk(bt_key_t *sirk);
#endif
static atci_status_t bt_app_comm_at_cmd_le_audio_sirk_hdl(char *pChar)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    bt_key_t sirk = {0};

    /* SIRK */
    /* AT+LEAUDIO=SIRK,<ACTION> */
    /* <ACTION>: SET, GET */
    if (0 == memcmp(pChar, "GET", 3)) {
        uint8_t temp_str[50] = {0};
#ifdef AIR_LE_AUDIO_ENABLE
        ble_csis_get_sirk(&sirk);
#else
        app_le_ull_get_sirk(&sirk);
#endif
        snprintf((char *)temp_str, sizeof(temp_str), "%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X",
                 sirk[0], sirk[1], sirk[2], sirk[3], sirk[4], sirk[5], sirk[6], sirk[7],
                 sirk[8], sirk[9], sirk[10], sirk[11], sirk[12], sirk[13], sirk[14], sirk[15]);
        snprintf((char *)response.response_buf, sizeof(response.response_buf), "SIRK:%s\r\n", (char *)temp_str);
        response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

    } else if (0 == memcmp(pChar, "SET", 3)) {
        /* AT+LEAUDIO=SIRK,SET,<B0>,<B1>,<B2>,<B3>,<B4>,<B5>,<B6>,<B7>,<B8>,<B9>,<B10>,<B11>,<B12>,<B13>,<B14>,<B15> */
        pChar = strchr(pChar, ',');
        pChar++;

        if (sscanf(pChar, "%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x",
                   (unsigned int *)&sirk[0], (unsigned int *)&sirk[1], (unsigned int *)&sirk[2], (unsigned int *)&sirk[3],
                   (unsigned int *)&sirk[4], (unsigned int *)&sirk[5], (unsigned int *)&sirk[6], (unsigned int *)&sirk[7],
                   (unsigned int *)&sirk[8], (unsigned int *)&sirk[9], (unsigned int *)&sirk[10], (unsigned int *)&sirk[11],
                   (unsigned int *)&sirk[12], (unsigned int *)&sirk[13], (unsigned int *)&sirk[14], (unsigned int *)&sirk[15]) > 0) {

            LOG_MSGID_I(common, "[SIRK] SET:%x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[0], sirk[1], sirk[2], sirk[3], sirk[4], sirk[5], sirk[6], sirk[7]);
            LOG_MSGID_I(common, "[SIRK]:%x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[8], sirk[9], sirk[10], sirk[11], sirk[12], sirk[13], sirk[14], sirk[15]);
#ifdef AIR_LE_AUDIO_ENABLE
            ble_csis_set_sirk(sirk);
            ble_csis_write_nvkey_sirk(&sirk);
#else
            app_le_ull_write_nvkey_sirk(&sirk);
#endif
        }
        response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

#ifdef AIR_LE_AUDIO_CIS_ENABLE
#ifdef MTK_AWS_MCE_ENABLE
#define BT_PTS_STATE_IDLE                    (0)
uint8_t bt_app_pts_state = BT_PTS_STATE_IDLE;
#endif
static atci_status_t bt_app_comm_at_cmd_le_audio_media_hdl(char *pChar)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    bt_status_t result = BT_STATUS_UNSUPPORTED;
    bt_handle_t handle = g_music_active_handle.handle;//bt_sink_srv_cap_get_link_handle(0xFF);
    bt_le_audio_sink_action_param_t le_param = {
        .service_idx = BLE_MCP_GMCS_INDEX,
    };

    /* AT+LEAUDIO=MEDIA,<ACTION> */
    /* <ACTION>: PLAY, PAUSE, NEXT, PREV */

    if (0 == memcmp(pChar, "PLAY", 4)) {
        result = bt_le_audio_sink_send_action(handle, BT_LE_AUDIO_SINK_ACTION_MEDIA_PLAY, &le_param);

    } else if (0 == memcmp(pChar, "PAUSE", 5)) {
        result = bt_le_audio_sink_send_action(handle, BT_LE_AUDIO_SINK_ACTION_MEDIA_PAUSE, &le_param);

    } else if (0 == memcmp(pChar, "NEXT", 4)) {
        result = bt_le_audio_sink_send_action(handle, BT_LE_AUDIO_SINK_ACTION_MEDIA_NEXT_TRACK, &le_param);

    } else if (0 == memcmp(pChar, "PREV", 4)) {
        result = bt_le_audio_sink_send_action(handle, BT_LE_AUDIO_SINK_ACTION_MEDIA_PREVIOUS_TRACK, &le_param);
    }

    LOG_MSGID_I(BT_APP, "[AT_CMD] Media result:%x", 1, result);

    if (BT_STATUS_SUCCESS == result) {
        response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static bt_status_t bt_app_comm_at_cmd_le_audio_call_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;

    /* AT+LEAUDIO=CALL,<ACTION> */
    /* <ACTION>: ACCEPT, TERMINATE, PLACE */
    if (0 == memcmp(pChar, "ACCEPT", 6)) {
        result = bt_sink_srv_le_call_action_handler(BT_SINK_SRV_ACTION_ANSWER, NULL);
    } else if (0 == memcmp(pChar, "TERMINATE", 9)) {
        result = bt_sink_srv_le_call_action_handler(BT_SINK_SRV_ACTION_REJECT, NULL);
        if (result != BT_STATUS_SUCCESS) {
            result = bt_sink_srv_le_call_action_handler(BT_SINK_SRV_ACTION_HANG_UP, NULL);
        }
    } else if (0 == memcmp(pChar, "PLACE", 5)) {
        result = bt_sink_srv_le_call_action_handler(BT_SINK_SRV_ACTION_DIAL_LAST, NULL);
        if (result != BT_STATUS_SUCCESS) {
            uint8_t temp_uri[] = "tel:+l50";
            result = bt_sink_srv_le_place_call(temp_uri, 8);
        }
    }
    LOG_MSGID_I(BT_APP, "[AT_CMD] Call result:%x \r\n", 1, result);
    return result;
}

static bool bt_app_comm_at_cmd_le_audio_volume_hdl(char *pChar)
{
    bt_handle_t handle = bt_sink_srv_cap_stream_get_service_ble_link();

    /* AT+LEAUDIO=VOLUME,<ACTION> */
    /* <ACTION>: MUTE, UNMUTE, UP, DOWN, SET */
    if (0 == memcmp(pChar, "MUTE", 4)) {
        bt_sink_srv_le_volume_vcp_send_action(handle, BT_SINK_SRV_LE_VCS_ACTION_MUTE, NULL);
    } else if (0 == memcmp(pChar, "UNMUTE", 6)) {
        bt_sink_srv_le_volume_vcp_send_action(handle, BT_SINK_SRV_LE_VCS_ACTION_UNMUTE, NULL);

    } else if (0 == memcmp(pChar, "UP", 2)) {

        bt_sink_srv_le_volume_vcp_send_action(handle, BT_SINK_SRV_LE_VCS_ACTION_UNMUTE_RELATIVE_VOLUME_UP, NULL);

    } else if (0 == memcmp(pChar, "DOWN", 4)) {
        bt_sink_srv_le_volume_vcp_send_action(handle, BT_SINK_SRV_LE_VCS_ACTION_UNMUTE_RELATIVE_VOLUME_DOWN, NULL);

    } else if (0 == memcmp(pChar, "SET", 3)) {
        /* AT+LEAUDIO=VOLUME,SET,<volume> */
        bt_sink_srv_le_set_absolute_volume_t param;
        char *buf = (pChar + 4);
        param.volume = (uint8_t)strtoul(buf, NULL, 10);
        if (param.volume > 15) {
            return false;
        }
        param.volume *= BT_SINK_LE_VOLUME_VALUE_STEP;
        if (BT_STATUS_SUCCESS != bt_sink_srv_le_volume_vcp_send_action(handle, BT_SINK_SRV_LE_VCS_ACTION_SET_ABSOLUTE_VOLUME, &param)) {
            return false;
        }
    } else if (0 == memcmp(pChar, "PTS_SET", 7)) {
        uint8_t volume, mute, flag;

        pChar = strchr(pChar, ',');
        pChar++;
        volume = atoi(pChar);

        pChar = strchr(pChar, ',');
        pChar++;
        mute = atoi(pChar);

        pChar = strchr(pChar, ',');
        pChar++;
        flag = atoi(pChar);

        bt_sink_srv_le_volume_pts_set_paras(volume, mute, flag);
    }
    return true;
}
#endif

#ifdef AIR_LE_AUDIO_BIS_ENABLE
static bool bt_app_comm_at_cmd_le_audio_broadcast_hdl(char *pChar)
{
#ifdef AIR_SPEAKER_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_mode_t aws_mode = bt_aws_mce_srv_get_mode();
    if (aws_mode == BT_AWS_MCE_SRV_MODE_BROADCAST && role == BT_AWS_MCE_ROLE_CLINET) {
        LOG_MSGID_I(common, "[LEA][BIS] broadcast AT CMD, fail Client role", 0);
        return FALSE;
    }
#endif

    /* AT+LEAUDIO=BROADCAST,<ACTION> */
    if (0 == memcmp(pChar, "SCAN", 4)) {
        /* AT+LEAUDIO=BROADCAST,SCAN */
        bt_sink_srv_cap_stream_bmr_scan_param_ex_t scan_params = {0};
        unsigned int val = 0;
        bt_addr_t addr = {0};

        if (NULL != (pChar = strchr(pChar, ','))) {
            pChar ++;
            if (0 == memcmp(pChar, "ADDR,", 5)) {
                /* AT+LEAUDIO=BROADCAST,SCAN,ADDR,<Address>,<Address type> */
                unsigned int value[6] = {0};
                pChar += 5;
                if (sscanf(pChar, "%2x:%2x:%2x:%2x:%2x:%2x", &value[5], &value[4], &value[3], &value[2], &value[1], &value[0]) > 0) {
                    for (uint8_t i = 0; i < 6; i++) {
                        addr.addr[i] = (uint8_t)value[i];
                    }
                } else {
                    return false;
                }

                if (NULL == (pChar = strchr(pChar, ','))) {
                    return false;
                } else {
                    pChar ++;
                }

                if (sscanf(pChar, "%2x", &val) > 0) {
                    addr.type = (uint8_t)val;
                    memcpy(&scan_params.bms_address, &addr, sizeof(bt_addr_t));
                    scan_params.scan_type = BT_HCI_SCAN_FILTER_ACCEPT_ONLY_ADVERTISING_PACKETS_IN_WHITE_LIST;
                    LOG_MSGID_I(common, "[BIS_CMD] START SCAN addr=%02X:%02X:%02X:%02X:%02X:%02X type=%02X", 7
                                , addr.addr[5], addr.addr[4], addr.addr[3], addr.addr[2], addr.addr[1], addr.addr[0], addr.type);
                } else {
                    return false;
                }

                if (NULL != (pChar = strchr(pChar, ','))) {
                    pChar ++;
                    if (0 == memcmp(pChar, "IDX,", 4)) {
                        /* AT+LEAUDIO=BROADCAST,SCAN,ADDR,<Address>,<Address type>,IDX,<BIS index> */
                        pChar += 4;
                        if (sscanf(pChar, "%2x", &val) > 0) {
                            if (val) {
                                scan_params.bis_sync_state = 1 << (val - 1);
#ifdef AIR_HEADSET_ENABLE
                                if (NULL != (pChar = strchr(pChar, ','))) {
                                    pChar ++;
                                    if (sscanf(pChar, "%2x", &val) > 0) {
                                        scan_params.bis_sync_state |= 1 << (val - 1);
                                    }
                                }
#endif
                                LOG_MSGID_I(common, "[BIS_CMD] START SCAN index=%02X", 1, scan_params.bis_sync_state);
                            }
                        } else {
                            return false;
                        }
                    } else if (0 == memcmp(pChar, "GROUP,", 6)) {
                        /* AT+LEAUDIO=BROADCAST,SCAN,ADDR,<Address>,<Address type>,GROUP,<Group index> */
                        pChar += 6;
                        if (sscanf(pChar, "%2x", &val) > 0) {
                            bt_sink_srv_cap_stream_set_bis_subgroup_idx((uint8_t)val);
                            LOG_MSGID_I(common, "[BIS_CMD] START SCAN Group index=%02X", 1, (uint8_t)val);
                        } else {
                            return false;
                        }
                    }
                }
            } else if (0 == memcmp(pChar, "RSSI,", 5)) {
                /* AT+LEAUDIO=BROADCAST,SCAN,RSSI,<Scan timer (s)> */
                pChar += 5;
                if (sscanf(pChar, "%2x", &val) > 0) {
                    if (val) {
                        scan_params.duration = 100 * ((uint16_t)val);
                        scan_params.sync_policy = BT_SINK_SRV_CAP_STREAM_SYNC_POLICY_MAX_RSSI;
                        LOG_MSGID_I(common, "[BIS_CMD] START SCAN MAX RSSI timer=%02X(s)", 1, val);
                    }
                } else {
                    return false;
                }
            } else if (0 == memcmp(pChar, "NEXT", 4)) {
                /* AT+LEAUDIO=BROADCAST,SCAN,NEXT */
                scan_params.sync_policy = BT_SINK_SRV_CAP_STREAM_SYNC_POLICY_NEXT;
                LOG_MSGID_I(common, "[BIS_CMD] START SCAN NEXT", 0);
            } else {
                return false;
            }
        }
        app_le_audio_config_bis_scan_params(&scan_params);
        app_le_audio_bis_start(TRUE);

    } else if (0 == memcmp(pChar, "STOP", 4)) {
        app_le_audio_config_bis_scan_params(NULL);
#ifdef MTK_AWS_MCE_ENABLE
        uint8_t bis_indices[1] = {0};
        app_le_audio_bis_start(FALSE);
        bt_sink_srv_cap_stream_set_big_sync_info(1, 1, bis_indices);
#else
        uint8_t bis_indices[2] = {0};
        app_le_audio_bis_start(FALSE);
        bt_sink_srv_cap_stream_set_big_sync_info(1, 2, bis_indices);
#endif
    } else if (0 == memcmp(pChar, "ENCRYPT", 7)) {
        bool encryption;
        pChar = strchr(pChar, ',');
        pChar++;
        if (sscanf(pChar, "%2x", (unsigned int *)&encryption) > 0) {
            LOG_MSGID_I(common, "[BRST CODE] Encrypt:%x", 1, encryption);
            ble_bap_set_broadcast_encryption(encryption);
        }

    } else if (0 == memcmp(pChar, "CODE,", 5)) {
        uint8_t code[16];
        pChar = strchr(pChar, ',');
        pChar++;

        if (sscanf(pChar, "%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x",
                   (unsigned int *)&code[0], (unsigned int *)&code[1], (unsigned int *)&code[2], (unsigned int *)&code[3],
                   (unsigned int *)&code[4], (unsigned int *)&code[5], (unsigned int *)&code[6], (unsigned int *)&code[7],
                   (unsigned int *)&code[8], (unsigned int *)&code[9], (unsigned int *)&code[10], (unsigned int *)&code[11],
                   (unsigned int *)&code[12], (unsigned int *)&code[13], (unsigned int *)&code[14], (unsigned int *)&code[15]) > 0) {

            LOG_MSGID_I(common, "[BRST CODE] SET:%x-%x-%x-%x-%x-%x-%x-%x", 8, code[0], code[1], code[2], code[3], code[4], code[5], code[6], code[7]);
            LOG_MSGID_I(common, "[BRST CODE]:%x-%x-%x-%x-%x-%x-%x-%x", 8, code[8], code[9], code[10], code[11], code[12], code[13], code[14], code[15]);
            bt_sink_srv_cap_stream_set_broadcast_code(code);
        }

    } else if (0 == memcmp(pChar, "CONFIG", 6)) {
        uint8_t big_handle = 0;
        uint8_t bis_indices[2] = {0};

        pChar = strchr(pChar, ',');
        pChar++;

        big_handle = atoi(pChar);

        if (big_handle > 4) {
            return false;
        }

        pChar = strchr(pChar, ',');
        pChar++;

        bis_indices[0] = atoi(pChar);

        LOG_MSGID_I(common, "[BIS] config sync big info big_handle[%d] bis_indices[%d]",
                    2, big_handle, bis_indices[0]);

        bt_sink_srv_cap_stream_set_big_sync_info(big_handle, 1, bis_indices);
    } else if (0 == memcmp(pChar, "NOTIFY_ALL", 10)) {
        ble_bap_bass_notify_all_clients();

    } else if (0 == memcmp(pChar, "KEEP_PA", 7)) {
        bool is_keep_pa = true;
        pChar = strchr(pChar, ',');
        pChar++;
        if (sscanf(pChar, "%2x", (unsigned int *)&is_keep_pa) > 0) {
            bt_sink_srv_cap_stream_keep_pa(is_keep_pa);
        }

    } else {
        return false;
    }

    return true;
}
#endif
#ifdef AIR_LE_AUDIO_HAPS_ENABLE
static atci_status_t bt_app_comm_at_cmd_le_audio_haps_hdl(uint16_t len, char *pChar)
{
    extern bt_status_t ble_haps_change_feature(ble_ha_features_t features);

    /* AT+LEAUDIO=HAPS,<ACTION> */
    /* <ACTION>: ADD, REMOVE, CHANGE, ACTIVE */
    bt_status_t status = BT_STATUS_SUCCESS;
    char *e = pChar + len;
    uint8_t i;

    *e = 0;

    printf("[HAPS AT] len = %d\r\n", len);

    if (len == 4 && 0 == memcmp(pChar, "INIT", 4)) {
        printf("[HAPS AT] INIT\r\n");

        if (ble_haps_init_server(APP_LE_AUDIO_MAX_LINK_NUM)) {
            status = ATCI_STATUS_ERROR;
        }
    } else if (len > 8 && 0 == memcmp(pChar, "ADD", 3)) {
        // index, property, name
        bt_status_t rtv;
        char *para[3];
        uint8_t index, property;

        pChar += 3;

        for (i = 0; i < 3; i++) {
            pChar = strchr(pChar, ',');

            if (!pChar || pChar >= e) {
                return ATCI_STATUS_ERROR;
            }

            pChar++;
            para[i] = pChar;

            if (pChar >= e) {
                return ATCI_STATUS_ERROR;
            }
        }

        index = atoi(para[0]);
        property = atoi(para[1]);

        rtv = ble_haps_add_new_preset(index, property, e - para[2], para[2]);

        if (rtv) {
            status = ATCI_STATUS_ERROR;
        }
    } else if (len > 7 && 0 == memcmp(pChar, "REMOVE", 6)) {
        // index
        bt_status_t rtv;
        uint8_t index;

        pChar += 6;
        pChar = strchr(pChar, ',');

        if (!pChar || pChar >= e) {
            return ATCI_STATUS_ERROR;
        }

        pChar++;
        index = atoi(pChar);

        if (pChar >= e) {
            return ATCI_STATUS_ERROR;
        }

        rtv = ble_haps_remove_preset(index);

        if (rtv) {
            status = ATCI_STATUS_ERROR;
        }
    } else if (len > 11 && 0 == memcmp(pChar, "CHANGE", 6)) {
        // index, flag, property, name
        bt_status_t rtv;
        char *para[4];
        uint8_t index, flag, property = 0;

        pChar += 6;

        for (i = 0; i < 4; i++) {
            pChar = strchr(pChar, ',');

            if (!pChar) {
                return ATCI_STATUS_ERROR;
            }

            pChar++;
            para[i] = pChar;

            if (pChar >= e) {
                return ATCI_STATUS_ERROR;
            }
        }

        index = atoi(para[0]);
        flag = atoi(para[1]);
        property = atoi(para[2]);

        if (!(flag & HAS_CHANGE_FLAG_PRESET_NAME)) {
            para[3] = e;
        }

        rtv = ble_haps_change_preset(index, flag, property, e - para[3], para[3]);

        if (rtv) {
            status = ATCI_STATUS_ERROR;
        }
    } else if (len > 7 && 0 == memcmp(pChar, "ACTIVE", 6)) {
        // index
        bt_status_t rtv;
        uint8_t index;

        pChar += 6;
        pChar = strchr(pChar, ',');

        if (!pChar) {
            return ATCI_STATUS_ERROR;
        }

        pChar++;
        index = atoi(pChar);

        if (pChar >= e) {
            return ATCI_STATUS_ERROR;
        }

        rtv = ble_haps_set_active_preset(index);

        if (rtv) {
            status = ATCI_STATUS_ERROR;
        }
    } else if (len > 8 && 0 == memcmp(pChar, "FEATURE", 7)) {
        ble_ha_features_t features = {HEARING_AID_TYPE_BINAURAL, 0, 1, 1, 1, 0};
        bt_status_t rtv;
        uint8_t param[5], count = 0;

        pChar += 7;

        for (i = 0; i < 5; i++) {
            pChar = strchr(pChar, ',');

            if (!pChar || pChar >= e) {
                break;
            }

            count++;
            pChar++;
            param[i] = atoi(pChar);
            pChar++;

            if (pChar >= e) {
                break;
            }
        }

        if (!count) {
            return ATCI_STATUS_ERROR;
        }

        if (count > 4) {
            features.wr_support = !!param[4];
        }
        if (count > 3) {
            features.dynamic = !!param[3];
        }
        if (count > 2) {
            features.independent = !!param[2];
        }
        if (count > 1) {
            features.sync_support = !!param[1];
        }
        if (param[0] > 2) {
            return ATCI_STATUS_ERROR;
        }

        features.hearing_aid_type = param[0];

        rtv = ble_haps_change_feature(features);

        if (rtv) {
            status = ATCI_STATUS_ERROR;
        }
    } else {
        status = ATCI_STATUS_ERROR;
    }

    return status;
}
#endif

extern bt_status_t bt_app_common_set_pairing_config(bool is_bond);

static atci_status_t bt_app_comm_at_cmd_le_audio_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_status_t ret_at = ATCI_STATUS_ERROR;
    char *pChar = NULL;
    atci_response_t *response = NULL;
    response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        return ret_at;
    }
    memset(response, 0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

    if (parse_cmd->mode == ATCI_CMD_MODE_EXECUTION) {
        pChar = parse_cmd->string_ptr + parse_cmd->name_len + 1;

        if (0 == memcmp(pChar, "SIRK", 4)) {
            /* SIRK */
            /* AT+LEAUDIO=SIRK,<ACTION> */
            /* <ACTION>: SET, GET */
            pChar = strchr(pChar, ',');
            pChar++;
            ret_at = bt_app_comm_at_cmd_le_audio_sirk_hdl(pChar);
#ifdef AIR_LE_AUDIO_CIS_ENABLE
        } else if (0 == memcmp(pChar, "MEDIA", 5)) {
            /* Media */
            /* AT+LEAUDIO=MEDIA,<ACTION> */
            /* <ACTION>: PLAY, PAUSE, NEXT, PREV */
            pChar = strchr(pChar, ',');
            pChar++;
            ret_at = bt_app_comm_at_cmd_le_audio_media_hdl(pChar);
        } else if (0 == memcmp(pChar, "VOLUME", 6)) {
            /* Volume */
            /* AT+LEAUDIO=VOLUME,<ACTION> */
            /* <ACTION>: MUTE, UNMUTE, UP, DOWN, SET */
            pChar = strchr(pChar, ',');
            pChar++;
            if (!bt_app_comm_at_cmd_le_audio_volume_hdl(pChar)) {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            } else {
                ret_at = ATCI_STATUS_OK;
            }
        } else if (0 == memcmp(pChar, "CALL", 4)) {
            /* Call */
            /* AT+LEAUDIO=CALL,<ACTION> */
            /* <ACTION>: ACCEPT, TERMINATE, PLACE */
            pChar = strchr(pChar, ',');
            pChar++;
            if (BT_STATUS_SUCCESS == bt_app_comm_at_cmd_le_audio_call_hdl(pChar)) {
                ret_at = ATCI_STATUS_OK;
            }
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        } else if (0 == memcmp(pChar, "BROADCAST", 9)) {
            /* Broadcast */
            /* AT+LEAUDIO=BROADCAST,<ACTION> */
            /* <ACTION>: SCAN, STOP */
            pChar = strchr(pChar, ',');
            pChar++;
            if (!bt_app_comm_at_cmd_le_audio_broadcast_hdl(pChar)) {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            } else {
                ret_at = ATCI_STATUS_OK;
            }
#endif
        } else if (0 == memcmp(pChar, "ADV", 3)) {
            /* Enalbe LE Audio ADV, low power mode use only */
            /* AT+LEAUDIO=ADV,<ACTION> */
            /* <ACTION>: ON, OFF, RESUME */
            pChar = strchr(pChar, ',');
            pChar++;
            ret_at = bt_app_comm_at_cmd_le_audio_adv_hdl(pChar);
#ifdef AIR_LE_AUDIO_ENABLE
        } else if (0 == memcmp(pChar, "RESET_LEA_DONGLE", 16)) {
            app_lea_service_reset_lea_dongle();
        } else if (0 == memcmp(pChar, "FEATURE", 7)) {
            /* AT+LEAUDIO=FEATURE,ON/OFF/DUAL */
            pChar = strchr(pChar, ',');
            pChar++;
            if (0 == memcmp(pChar, "OFF", 3)) {
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_SET_FEATURE_MODE);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                    EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_SET_FEATURE_MODE,
                                    (void *)(int)APP_LEA_FEATURE_MODE_OFF, 0, NULL, 0);
            } else if (0 == memcmp(pChar, "ON", 2)) {
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_SET_FEATURE_MODE);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                    EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_SET_FEATURE_MODE,
                                    (void *)(int)APP_LEA_FEATURE_MODE_ON, 0, NULL, 0);
            } else if (0 == memcmp(pChar, "DUAL", 4)) {
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_SET_FEATURE_MODE);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                    EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_SET_FEATURE_MODE,
                                    (void *)(int)APP_LEA_FEATURE_MODE_DUAL_MODE, 0, NULL, 0);
            }
        } else if (0 == memcmp(pChar, "START_ADV", 9)) {
            app_lea_service_start_advertising(APP_LEA_ADV_MODE_GENERAL, FALSE, 0);
        } else if (0 == memcmp(pChar, "STOP_ADV", 8)) {
            app_lea_service_stop_advertising(FALSE);
        } else if (0 == memcmp(pChar, "CAS_ADV_TYPE", 12)) {
            /* AT+LEAUDIO=CAS_ADV_TYPE,<ACTION> */
            /* <ACTION>: 0 (ANNOUNCEMENT_TYPE_GENERAL), 1 (ANNOUNCEMENT_TYPE_TARGETED) */
            unsigned int val = 0;
            pChar = strchr(pChar, ',');
            pChar++;
            if (sscanf(pChar, "%2x", &val) > 0) {
                if (0 == val || 1 == val) {
                    app_lea_adv_cas_announcement_type = (uint8_t)val;
                }
            }
        } else if (0 == memcmp(pChar, "PTS_TEST_ENABLE", 15)) {
            /* AT+LEAUDIO=PTS_TEST_ENABLE,<ACTION> */
            /* <ACTION>: 0 (Disable), 1 (Enable) */
            unsigned int val = 0;
            pChar = strchr(pChar, ',');
            pChar++;
            if (sscanf(pChar, "%2x", &val) > 0) {
                if (0 == val || 1 == val) {
                    bt_le_audio_pts_test_enable = (bool)val;
                }
            }
#ifdef MTK_AWS_MCE_ENABLE
        } else if (0 == memcmp(pChar, "REMLTK", 6)) {
            /* AT+LEAUDIO=REMLTK */
            bt_app_common_remove_ltk();
#endif
        } else if (0 == memcmp(pChar, "PTS_PAIRING_SET", 15)) {
            /* PTS CCP */
            /* AT+LEAUDIO=PTS_PAIRING_SET,<cmdType>
               <cmdType>: BOND, UNBOND
            */
            pChar = strchr(pChar, ',');
            pChar++;

            if (0 == memcmp(pChar, "BOND", 4)) {
                bt_app_common_set_pairing_config(true);
                ret_at = ATCI_STATUS_OK;
            } else if (0 == memcmp(pChar, "UNBOND", 6)) {
                bt_app_common_set_pairing_config(false);
                ret_at = ATCI_STATUS_OK;
            }
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
        } else if (0 == memcmp(pChar, "VSCODEC", 7)) {

            pChar = strchr(pChar, ',');

            if (pChar && (parse_cmd->string_ptr + parse_cmd->string_len - 2) > pChar) {
                bt_le_audio_direction_t direction;
                ble_ascs_config_codec_operation_t *parm;
                uint8_t buf[19], codec[] = CODEC_ID_LC3PLUS_CBR, *p;
                bt_handle_t conn_hdl = g_music_active_handle.handle;

                pChar++;
                direction = !!atoi(pChar);
                direction++;
                parm = (ble_ascs_config_codec_operation_t *)buf;

                if (direction == AUDIO_DIRECTION_SINK) {
                    parm->ase_id = 1;
                } else
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
                    parm->ase_id = 5;
#else
                    parm->ase_id = 3;
#endif
                parm->target_latency = 10;
                parm->target_phy = 2;
                memcpy(parm->codec_id, codec, AUDIO_CODEC_ID_SIZE);
                parm->codec_specific_configuration_length = 10;
                p = parm->codec_specific_configuration;
                *p++ = 2;
                *p++ = CODEC_CONFIGURATION_TYPE_SAMPLING_FREQUENCY;
                *p++ = CODEC_CONFIGURATION_SAMPLING_FREQ_96KHZ;
                *p++ = 2;
                *p++ = CODEC_CONFIGURATION_TYPE_FRAME_DURATIONS;
                *p++ = FRAME_DURATIONS_10_MS;
                *p++ = 3;
                *p++ = CODEC_CONFIGURATION_TYPE_OCTETS_PER_CODEC_FRAME;
                *p++ = 0xBE;
                *p++ = 0;

                LOG_MSGID_I(BT_APP, "[AT CMD] conn_hdl = %d", 1, conn_hdl);

                if (!bt_sink_srv_cap_stream_config_codec_autonomously(conn_hdl, direction, parm)) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                }
            } else {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
#endif
#endif
#ifdef AIR_LE_AUDIO_HAPS_ENABLE
        } else if (0 == memcmp(pChar, "HAPS", 4)) {
            /* LE Audio HAPS */
            /* AT+LEAUDIO=HAPS,<ACTION> */
            /* <ACTION>: ADD, REMOVE, CHANGE, ACTIVE */
            pChar = strchr(pChar + 4, ',');

            if (pChar && (parse_cmd->string_ptr + parse_cmd->string_len - 2) > pChar) {
                pChar++;
                ret_at = bt_app_comm_at_cmd_le_audio_haps_hdl(parse_cmd->string_ptr + parse_cmd->string_len - 2 - pChar, pChar);
            }

            if (ret_at) {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
#endif
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    }

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ret_at;
}

#endif  /* defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE) */

static atci_status_t bt_app_comm_at_cmd_hfp_codec_set_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    char *token = strtok(parse_cmd->string_ptr, ",");
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR};

    if (strstr(token, "set") != NULL) {
        token = strtok(NULL, ",");

        response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        if (strstr(token, "CVSD") != NULL) {
            update_hfp_audio_codec(BT_HFP_CODEC_TYPE_CVSD);
        } else if (strstr(token, "MSBC") != NULL) {
            update_hfp_audio_codec(BT_HFP_CODEC_TYPE_MSBC);
        } else {
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            char *pstr = (char *)response.response_buf;
            snprintf(pstr, ATCI_UART_TX_FIFO_BUFFER_SIZE, "unknown type, ");
        }

        response.response_len = strlen((char *)response.response_buf);
    } else {
        bt_hfp_audio_codec_type_t type = get_hfp_audio_codec();
        char *pstr = (char *)response.response_buf;
        snprintf(pstr, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "codec mask: 0x%x", type);

        response.response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        response.response_len = strlen((char *)response.response_buf);
    }
    atci_send_response(&response);
    return ATCI_STATUS_OK;

}

bool bt_app_ctkd_enable = false;

static atci_status_t bt_app_comm_at_cmd_ext_ble_ltk_2_linkkey(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        return ATCI_STATUS_ERROR;
    }
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+CTKD=<action> */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                bt_app_ctkd_enable = false;
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                bt_app_ctkd_enable = true;
                extern void bt_app_common_set_ctkd_config();
                bt_app_common_set_ctkd_config();
            }
            break;
        }

        default:
            break;
    }
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}



static bt_status_t bt_app_common_at_cmd_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_GATT_OVER_BREDR_CONNECT_CNF: {
            bt_gatt_over_bredr_connect_cnf_t *cnf = (bt_gatt_over_bredr_connect_cnf_t *)buff;
            LOG_MSGID_I(BT_APP, "gatt over bredr connect confirmation,connection_handle = %02x,remote_mtu = %d",
                        2, cnf->connection_handle, cnf->remote_rx_mtu);
            uint8_t i = 0;
            for (i = 0; i < 6; i++) {
                LOG_MSGID_I(BT_APP, "%02x", 1, cnf->address[i]);
            }
        }
        break;
        case BT_GATT_OVER_BREDR_DISCONNECT_IND: {
            LOG_MSGID_I(BT_APP, "gatt over bredr disconnect indication", 0);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}
static void bt_app_comm_at_cmd_copy_str_to_addr(uint8_t *addr, const char *str)
{
    unsigned int i, value;
    int using_long_format = 0;
    int using_hex_sign = 0;

    if (str[2] == ':' || str[2] == '-') {
        using_long_format = 1;
    }

    if (str[1] == 'x') {
        using_hex_sign = 2;
    }

    for (i = 0; i < 6; i++) {
        sscanf(str + using_hex_sign + i * (2 + using_long_format), "%02x", &value);
        addr[5 - i] = (uint8_t) value;
    }
}
bt_gatt_over_bredr_service_handle_t service_handle_ctx = {
    .gap_start_handle = 0x0001,
    .gap_end_handle = 0x0009,
    .gatt_start_handle = 0x0011,
    .gatt_end_handle = 0x0011
};

static atci_status_t bt_app_comm_at_cmd_gatt_over_bredr(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        LOG_MSGID_I(BT_APP, "gatt over bredr connect response is null.", 0);
        return ATCI_STATUS_ERROR;
    }
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "CONNECT", 7)) {
                LOG_MSGID_I(BT_APP, "gatt over bredr connect.", 0);
                uint8_t addr[6];
                const char *addr_str = parse_cmd->string_ptr + parse_cmd->name_len + 1 + strlen("CONNECT ");
                bt_app_comm_at_cmd_copy_str_to_addr(addr, addr_str);
                bt_gatt_over_bredr_connect((const bt_bd_addr_t *)addr);
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                LOG_MSGID_I(BT_APP, "gatt over bredr switch on.", 0);
                bt_gatt_over_bredr_switch(true);
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                LOG_MSGID_I(BT_APP, "gatt over bredr switch off.", 0);
                bt_gatt_over_bredr_switch(false);
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "DISCONNECT", 10)) {
                LOG_MSGID_I(BT_APP, "gatt over bredr disconnect.", 0);
                const char *str = parse_cmd->string_ptr + parse_cmd->name_len + 1 + strlen("DISCONNECT ");
                bt_gatt_over_bredr_disconnect(strtoul(str, NULL, 16));
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "SETSERVICE", 10)) {
                LOG_MSGID_I(BT_APP, "gatt over bredr set service.", 0);
                const char *str = parse_cmd->string_ptr + parse_cmd->name_len + 1 + strlen("SETSERVICE ");
                service_handle_ctx.gap_start_handle = strtoul(str, NULL, 16);
                LOG_MSGID_I(BT_APP, "gap_start_handle = %02x.", 1, service_handle_ctx.gap_start_handle);
                str += 5;
                service_handle_ctx.gap_end_handle =  strtoul(str, NULL, 16);
                LOG_MSGID_I(BT_APP, "gap_end_handle = %02x.", 1, service_handle_ctx.gap_end_handle);
                str += 5;
                service_handle_ctx.gatt_start_handle =  strtoul(str, NULL, 16);
                LOG_MSGID_I(BT_APP, "gatt_start_handle = %02x.", 1, service_handle_ctx.gatt_start_handle);
                str += 5;
                service_handle_ctx.gatt_end_handle = strtoul(str, NULL, 16);
                LOG_MSGID_I(BT_APP, "gatt_end_handle = %02x.", 1, service_handle_ctx.gatt_end_handle);
            } else {

            }
        }
        break;
        default:
            break;
    }
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

extern bt_status_t bt_device_manager_le_get_bonding_info_by_link_type(bt_gap_le_srv_link_t link_type, bt_device_manager_le_bonded_info_t *infos, uint8_t *count);
static atci_status_t bt_app_comm_at_cmd_le_bond_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (NULL == response) {
        return ATCI_STATUS_ERROR;
    }
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "GET LEA", strlen("GET LEA"))) {
        bt_device_manager_le_bonded_info_t bond_info[8] = {0};
        uint32_t bond_info_count = 8;
        if (bt_device_manager_le_get_bonding_info_by_link_type(BT_GAP_LE_SRV_LINK_TYPE_LE_AUDIO, bond_info, (uint8_t *)&bond_info_count) == BT_STATUS_SUCCESS) {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            uint32_t response_offset = 0;
            uint8_t *p_remote_address = NULL;
            uint8_t *p_ltk = NULL;
            bt_bd_addr_t default_address = {0};
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            for (uint32_t i = 0; i < bond_info_count; i++) {
                if (memcmp(&bond_info[i].info.identity_addr.address.addr, default_address, sizeof(bt_bd_addr_t)) == 0) {
                    p_remote_address = (uint8_t *)&bond_info[i].bt_addr.addr;
                } else {
                    p_remote_address = (uint8_t *)&bond_info[i].info.identity_addr.address.addr;
                }
                p_ltk = (uint8_t *)&bond_info[i].info.local_key.encryption_info.ltk;
                response_offset += snprintf((char *)response->response_buf + response_offset, sizeof(response->response_buf) - response_offset, "address %02x:%02x:%02x:%02x:%02x:%02x\r\n",\
                                        p_remote_address[0], p_remote_address[1], p_remote_address[2], p_remote_address[3], p_remote_address[4], p_remote_address[5]);
                if (response_offset >= ATCI_UART_TX_FIFO_BUFFER_SIZE) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                response_offset += snprintf((char *)response->response_buf + response_offset, sizeof(response->response_buf) - response_offset, "LTK %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",\
                                        p_ltk[0], p_ltk[1], p_ltk[2], p_ltk[3], p_ltk[4], p_ltk[5], p_ltk[6], p_ltk[7], p_ltk[8], p_ltk[9], p_ltk[10], p_ltk[11],\
                                        p_ltk[12], p_ltk[13], p_ltk[14], p_ltk[15]);
                if (response_offset >= ATCI_UART_TX_FIFO_BUFFER_SIZE) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
            }
        }
    }

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

void bt_app_comm_at_cmd_init(void)
{
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
    bt_role_handover_callbacks_t callbacks = {0};
    callbacks.status_cb = bt_app_common_at_cmd_rho_srv_status_callback;
    bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_BT_CMD, &callbacks);
#endif
    atci_register_handler(bt_app_comm_at_cmd, sizeof(bt_app_comm_at_cmd) / sizeof(atci_cmd_hdlr_item_t));
    bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_GAP | MODULE_MASK_SYSTEM | MODULE_MASK_GATT, (void *)bt_app_common_at_cmd_event_callback);
}



/**
  * @file    app_customer_common.c
  *
  * @author  wayne.xiong
  *
  * @date    2022/3/29
  *
  * @brief   customer common handle
**/

#include "app_customer_common.h"
#include "apps_debug.h"
#include "bt_power_on_config.h"
#include "bt_device_manager_test_mode.h"
#include "bt_sink_srv_a2dp.h"
#include "app_customer_nvkey_operation.h"

#include "apps_config_event_list.h"
#include "ui_shell_activity.h"
#include "apps_events_event_group.h"
#include "apps_events_key_event.h"
#include "ui_shell_manager.h"

#include "bt_device_manager.h"
#include "bt_aws_mce.h"
#include "apps_aws_sync_event.h"
#include "app_customer_common_activity.h"
#include "race_xport.h"
#include "mux_iap2.h"
//#include "iAP2.h"
#include "nvdm_config_factory_reset.h"
#ifdef BLE_ZOUND_ENABLE
#include "gatt_zound_service_tool.h"
#endif
#include "app_preproc_activity.h"
#include "bt_device_manager_internal.h"
#include "apps_events_interaction_event.h"
#include "bt_connection_manager_utils.h"
#include "battery_management.h"

static char case_verison[8] = "v3.00.00";
//static char ab1571d_verison[4] = "v0.0";
static uint8_t isShippingMode = 0;
volatile static uint8_t g_limit_status = 0;
volatile static uint8_t sn_cmp_result = 0x0;
static uint8_t touch_key_test_status = 0;


#if defined (PROJECT_GAHAN)
static const char go_client_id[] = "a56a9ca4c3224149844c062bbebf64a4"; //33
static const char go_model[] = "Motif II A.N.C."; //16
static const char go_brand[] = "Marshall"; //9
#elif defined (PROJECT_KALLA)
static const char go_client_id[] = "388bfcbe1e64455c83266f10a62f2490"; //33
static const char go_model[] = "Z.N.E. 02 ANC"; //14
static const char go_brand[] = "adidas"; //7
#else
static const char go_client_id[] = " ";
static const char go_model[] = " ";
static const char go_brand[] = " ";
#endif

#define SPOTIFY_GO_TYPE (0x01)
#define GO_TYPE_INDEX (0)
#define GO_DATA_LENGHT_INDEX (1)
#define GO_DATA_INDEX (2)
#define GO_CMD_DATA_LENGHT_MIN (37)//33+2+2
#define GO_CMD_LENGHT_MAX (64)


void app_spotify_tap_triggle(void)
{
	uint8_t go_cmd_buf[GO_CMD_LENGHT_MAX] = {0};
//	uint8_t ret=IAP2_STATUS_FAIL;

	go_cmd_buf[GO_TYPE_INDEX] = SPOTIFY_GO_TYPE;
	go_cmd_buf[GO_DATA_LENGHT_INDEX] = strlen(go_client_id) + strlen(go_model) + strlen(go_brand) +3;

	memcpy(&go_cmd_buf[GO_DATA_INDEX], (void*)go_client_id, strlen(go_client_id));
	memcpy(&go_cmd_buf[GO_DATA_INDEX+strlen(go_client_id)+1], (void*)go_model, strlen(go_model));
	memcpy(&go_cmd_buf[GO_DATA_INDEX+strlen(go_client_id)+strlen(go_model)+2], (void*)go_brand, strlen(go_brand));

	race_debug_print(go_cmd_buf, go_cmd_buf[GO_DATA_LENGHT_INDEX]+2,"Spotify Tap Buffer:");
	if(GO_CMD_DATA_LENGHT_MIN < go_cmd_buf[GO_DATA_LENGHT_INDEX])
	{
#ifdef MTK_IAP2_PROFILE_ENABLE
//		ret=iap2_send_data_to_spotify(go_cmd_buf, go_cmd_buf[GO_DATA_LENGHT_INDEX] + 2);
#endif
//		race_port_send_spotify_tap_data(go_cmd_buf, go_cmd_buf[GO_DATA_LENGHT_INDEX] + 2);
	}
	else
	{
		APPS_LOG_MSGID_E(" Spotify Go Command Invalid!", 0);
	}
}


void app_customer_common_setting_to_partner(void)
{
	app_customer_setting_sync_t sync_data;

	sync_data.eco_charging_status = app_nvkey_eco_charging_profile_status_read();
	sync_data.shipping_mode_state = app_nvkey_shipping_mode_read();
	sync_data.color_id = app_nvkey_ble_color_id_read();
	sync_data.mini_ui = app_nvkey_ble_mini_ui_read();
	sync_data.bat_heathy_times = app_nvkey_battery_heathy_read();

#ifdef BLE_ZOUND_ENABLE
	sync_data.touch_lock = app_nvkey_customer_all().touch_lock;
	sync_data.ui_sounds = app_nvkey_customer_all().ui_sounds;
	sync_data.anc_level = app_nvkey_customer_all().anc_level;
	sync_data.tra_level = app_nvkey_customer_all().tra_level;
	memmove(sync_data.wear_sensor_action,app_nvkey_customer_all().wear_sensor_action,sizeof(sync_data.wear_sensor_action));
	memmove(sync_data.action_button_configuration, app_nvkey_customer_all().action_button_configuration, sizeof(app_nvkey_customer_all().action_button_configuration));
	memmove(sync_data.eq_settings,app_nvkey_customer_all().eq_settings,sizeof(app_nvkey_customer_all().eq_settings));
#endif

	uint8_t eco_setting[8] = {0};
	app_nvkey_eco_setting_read(eco_setting);
	memcpy(sync_data.eco_charging_setting, eco_setting, 8);

	apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_CUSTOMER_COMMON_CUSTOMER_DATA_SYNC,
								   (void*)&sync_data, sizeof(app_customer_setting_sync_t));
}

void app_customer_common_sync_aws_data(void* p_data)
{
	app_nvkey_sync_aws_data(p_data);
}


void app_customer_test_enable_dut(void)
{
    bt_power_on_set_config_type(BT_POWER_ON_DUT);
	bt_device_manager_set_test_mode(BT_DEVICE_MANAGER_TEST_MODE_DUT_ONLY);
    APPS_LOG_MSGID_I( " KEY_TEST_DUT_FORCE_ON", 0);
}

void app_customer_common_tws_clean(void)
{
	app_home_screen_fact_rst_nvdm_flag(FACTORY_RESET_FLAG);
	bt_device_manager_aws_reset();

    uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

	*p_key_action = KEY_SYSTEM_REBOOT;

    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
                        sizeof(uint16_t), NULL, 500);	

}


void app_set_charger_case_version(uint8_t version_data)
{
//	case_verison[1] = ((version_data>>4) & 0xF) + 0x30;
//	case_verison[3] = (version_data & 0xF) + 0x30;
	case_verison[3] = (version_data/10)+0x30;
	case_verison[4] = (version_data%10)+0x30;
}

void app_get_charger_case_version(uint8_t* p_version)
{
//	race_debug_print((uint8_t*)case_verison, 4,"customer case version:");
//	memcpy((void*)p_version, (void*)case_verison, 4);
	race_debug_print((uint8_t*)case_verison, 8,"customer case version:");
	memcpy((void*)p_version, (void*)case_verison, 8);	
}

void app_set_ab1571d_version(uint8_t version_data)
{
//	ab1571d_verison[1] = ((version_data>>4) & 0xF) + 0x30;
//	ab1571d_verison[3] = (version_data & 0xF) + 0x30;
	case_verison[6] = (version_data&0xf)/10+0x30;
	case_verison[7] = (version_data&0xf)%10+0x30;
}

#if 0
void app_get_ab1571d_version(uint8_t* p_version)
{
	race_debug_print((uint8_t*)ab1571d_verison, 4,"ab1571d version:");
	memcpy((void*)p_version, (void*)ab1571d_verison, 4);
}
#endif

void app_set_eco_charing_profile_switch(uint8_t profile_status)
{
	app_nvkey_eco_charging_profile_status_write(profile_status);

	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_ECO_CHARGING_STATUS_SYNC,(void*)&profile_status ,1);
	}

}


uint8_t app_get_eco_charing_profile_switch(void)
{
	return app_nvkey_eco_charging_profile_status_read();
}

void app_set_a2dp_volume(uint8_t volume_level)
{
	bt_sink_srv_send_action(BT_SINK_SRV_ACTION_SET_VOLUME, &volume_level);
}

void app_system_factory_reset(uint8_t reset_mode)
{
	uint8_t delay_ms = 200;
    uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

	if(reset_mode == 0x1)
	    *p_key_action = KEY_TEST_FACTORY_RESET;
	else if(reset_mode == 0x2)
		*p_key_action = KEY_TEST_FACTORY_RESET_AND_POWEROFF;
	else if(reset_mode == 0x3)
	    *p_key_action = KEY_FACTORY_RESET;
	else if(reset_mode == 0x4)
		*p_key_action = KEY_FACTORY_RESET_AND_POWEROFF;

	if(reset_mode == 0x03 || reset_mode == 0x04)
		delay_ms = 2000;

    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
                        sizeof(uint16_t), NULL, delay_ms);

}

/******************Shipping Mode Start*****************************************************************/
void app_set_shipping_mode_state(uint8_t state)
{

	if(state == 0)
	{
		app_enter_shipping_mode_flag_set(0);
	}
	
	app_nvkey_shipping_mode_set((state&0x01));

	if(state>0x80)
	{
		app_enter_shipping_mode_flag_set(true);
		ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
							APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF, NULL, 0,
							NULL, 2000);
	}

	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_SHIPPING_MODE_STATE_SYNC,(void*)&state ,1);
	}
}

uint8_t app_get_shipping_mode_state(void)
{
	return app_nvkey_shipping_mode_read();
}

void app_enter_shipping_mode_flag_set(uint8_t shipping_mode)
{
	isShippingMode = shipping_mode;
	//APPS_LOG_MSGID_I("app_customer_common.c:: Set isShippingMode = %d",1 , shipping_mode);
}

uint8_t app_enter_shipping_mode_flag_read(void)
{
	//APPS_LOG_MSGID_I("app_customer_common.c:: Read isShippingMode = %d",1 , isShippingMode);
	return isShippingMode;
}

void app_shipping_mode_enter(void)
{
	//APPS_LOG_MSGID_I("app_customer_common.c:: Enter Shipping Mode!!!",0);
	hal_gpio_set_output(HAL_GPIO_15, HAL_GPIO_DATA_HIGH);
	hal_gpt_delay_ms(500);
}

void app_shipping_mode_exit(void)
{
	//APPS_LOG_MSGID_I("app_customer_common.c:: Exit Shipping Mode!!!",0);
	hal_gpio_set_output(HAL_GPIO_15, HAL_GPIO_DATA_LOW);
	hal_gpt_delay_ms(40);
}
/******************Shipping Mode End*****************************************************************/

void app_set_ble_device_color_state(uint8_t state)
{
	app_nvkey_ble_color_id_set(state);

	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_DEVICE_COLOR_STATE_SYNC,(void*)&state ,1);
	}
	else if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                            APPS_EVENTS_INTERACTION_REQUEST_REBOOT, NULL, 0,
                                            NULL, 100);
	}
	
}

void app_set_ble_device_mini_ui_state(uint8_t state)
{
	app_nvkey_ble_mini_ui_set(state);
	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_DEVICE_MINI_UI_STATE_SYNC,(void*)&state ,1);
	}

}

void app_set_lea_disable_state(uint8_t state)
{
	if(bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		app_nvkey_is_lea_disable_set(state);
		apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_DEVICE_LEA_DISABLE_STATE_SYNC,(void*)&state ,1);

	    uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

		*p_key_action = KEY_SYSTEM_REBOOT;

	    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
	                        sizeof(uint16_t), NULL, 2000);		

	}

}


void app_set_anc_status_bk(uint8_t status)
{
	app_nvkey_anc_status_bk_set(status);
	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_ANC_BK_STATUS_SYNC,(void*)&status ,1);
	}

}


void app_set_ble_write_SN(uint8_t *sn, uint8_t len)
{
	app_nvkey_sn_set(sn, len);
	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT)
	{
		sn_cmp_result = 0x0;
		apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_DEVICE_SN_SYNC,(void*)sn ,len);
	}
	else
	{
		uint8_t local_sn[22] = {0};
		app_nvkey_sn_read(local_sn, 22);
		race_debug_print((uint8_t*)local_sn, 22,"sn local1:");
		apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_DEVICE_SN_COMPARE, (void*)local_sn, 22);
	}
}

void app_set_hw_version(uint8_t *ver, uint8_t len)
{
	app_nvkey_hw_version_set(ver, len);
	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_DEVICE_HW_VER_SYNC,(void*)ver ,len);
	}	

	ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
									APPS_EVENTS_INTERACTION_REQUEST_REBOOT, NULL, 0,
									NULL, 1000);		
}

void app_write_sn_compare(uint8_t *sn, uint8_t len)
{

	if(len == 22)
	{
		uint8_t local_sn[22] = {0};
		app_nvkey_sn_read(local_sn, 22);
		race_debug_print((uint8_t*)local_sn, 22,"sn local2:");
		
		if(!memcmp(local_sn, sn, len))
			sn_cmp_result = 1;
		else
			sn_cmp_result = 0;
	}
	else
	{
		sn_cmp_result = 0;
	}
}

uint8_t app_write_sn_compare_result_read(void)
{
	return sn_cmp_result;
}

void app_set_hx300x_debug_state(uint8_t state)
{
	if(state != app_nvkey_get_hx300x_log_status())
	{
		if(state == 0x01)
			hx300x_spp_open_log();
		else
			hx300x_spp_stop_log();
		
		app_nvkey_set_hx300x_log_status(state);
	}

	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_HX300X_DEBUG_SYNC,(void*)&state ,1);
	}	
}

void app_spp_debug_print(char *fmt, uint8_t len)
{
	race_port_send_debug_data(fmt, len);
}

void app_read_all_reg(void)
{
	ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON,
						EVENT_ID_HX300X_READ_ALL_REG, NULL, 0, NULL, 10); 		
}


void app_battery_heathy_send2peer(uint16_t times)
{
	apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_BATTERY_HEATHY_SYNC,(void*)&times ,2);
}

uint8_t app_smcharger_get_current_limit_status(void)
{
	return g_limit_status;
}

void app_smcharger_set_current_limit_status(uint8_t status)
{
	g_limit_status = status;
}

uint8_t app_touch_key_test_status_get(void)
{
	return touch_key_test_status;
}

void app_touch_key_test_status_set(uint8_t status)
{
//	if(status == 0x00)
//		app_touch_key_test_clean();
	
	touch_key_test_status = status;
}

uint8_t app_bt_connected_number(void)
{
	uint32_t conn_num = 0;
	bt_bd_addr_t addr_list[3] ={{0}};
	conn_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE, addr_list, 3);
	if(conn_num>1)
		conn_num = (conn_num-1);
	return (conn_num&0xff);
}

#if 0
void app_force_disconnect_bt_connection_before_pairing(void)
{
	return;
	
	uint32_t conn_num = 0;
	bt_bd_addr_t *local_addr = bt_device_manager_get_local_address();
	bt_bd_addr_t addr_list[3];
	bt_cm_connect_t dis_conn = {
		.profile = BT_CM_PROFILE_SERVICE_MASK_ALL
	};
	conn_num = bt_cm_get_connected_devices(~(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)), addr_list, sizeof(addr_list) / sizeof(bt_bd_addr_t));
	//bt_cmgr_report_id("[BT_CM][ATCI] Disconnect all connected device num %d!", 1, conn_num);
	for (uint32_t i = 0; i < conn_num; i++) {
		if (!bt_cm_memcmp(local_addr, &addr_list[i], sizeof(bt_bd_addr_t))) {
			continue;
		}
		//bt_cmgr_report_id("[BT_CM][ATCI] To disconnect 0x%x!", 1, *(uint32_t *)(&addr_list[i]));
		bt_cm_memcpy(&(dis_conn.address), &(addr_list[i]), sizeof(bt_bd_addr_t));
		bt_cm_disconnect(&dis_conn);
	}

}
#endif

#define TRACKING_INDEX_MAX 32
static uint8_t tracking_log[TRACKING_INDEX_MAX] = {0};
static uint8_t tracking_index = 0;


void app_common_tracking_log_reset(void)
{
	memset(tracking_log, 0x00, TRACKING_INDEX_MAX);
	tracking_index = 0;
}


void app_common_add_tracking_log(uint8_t data)
{
	tracking_log[tracking_index++] = data;
	if(tracking_index >= TRACKING_INDEX_MAX)
		tracking_index = 0;
}

void app_common_get_tracking_log(uint8_t *data)
{	
	memcpy(data, tracking_log, TRACKING_INDEX_MAX);
}

static uint8_t g_eco_chraging_soc = 0;
void app_common_set_eco_charging_soc(uint8_t status)
{
	g_eco_chraging_soc = status;
}

uint8_t app_common_get_eco_charging_soc(void)
{
	return g_eco_chraging_soc;
}

void app_eco_charge_soc_limit_handle(void* eco_setting)
{
	typedef struct
	{
		uint8_t EcoChargingState;
		uint8_t ChargeLevelLimit;
		uint8_t ChargeRate;
		uint8_t ChargeCondition;
		uint8_t LowTempLimit;
		uint8_t HighTempLimit;
		uint8_t SmartCharging;
		uint8_t reserved1;
	}eco_charge_tem_t;

	eco_charge_tem_t* p_eco_setting = (eco_charge_tem_t*)eco_setting;
	int soc = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);

	if( p_eco_setting->EcoChargingState == 1
		|| (p_eco_setting->EcoChargingState == 2 && p_eco_setting->ChargeLevelLimit && p_eco_setting->ChargeLevelLimit != 0x64))
	{//enable charge soc
		if(!app_nvkey_eco_soc_limit_status_read() && soc== p_eco_setting->ChargeLevelLimit)
		{
			app_nvkey_eco_soc_limit_status_write(true);
		}
	}
	else
	{//disable charge soc
		if(app_nvkey_eco_soc_limit_status_read() && soc < 90)
		{
			app_nvkey_eco_soc_limit_status_write(false);
		}			
	}
}

uint8_t app_is_eco_charge_level_limit_enable(void)
{
	return app_nvkey_eco_soc_limit_status_read();
}



/**
  * @file    app_customer_nvkey_operation.c
  *
  * @author  wayne.xiong
  *
  * @date    2022/3/29
  *
  * @brief   customer nvkey operation
**/

#include "app_customer_nvkey_operation.h"


#include "nvkey_id_list.h"
#include "nvkey.h"
#include "apps_debug.h"
#include "apps_config_event_list.h"
#include "ui_shell_activity.h"
#include "apps_events_event_group.h"
#include "apps_events_key_event.h"
#include "ui_shell_manager.h"
#include "app_customer_common.h"
#include "bt_aws_mce.h"
#include "app_power_save_utils.h"
#include "anc_control_api.h"
#include "apps_events_interaction_event.h"


static app_customer_nvkey_t g_customer_nvkey_setting;
static bool ota_upgrade_nvkey_flag = 0;

extern bt_bd_addr_t *bt_sink_srv_cm_get_aws_connected_device(void);


void ota_update_cus_nvkey_set(void)
{
	ota_upgrade_nvkey_flag = true;
}


void app_nvkey_setting_init(void)
{
	uint32_t size = 0;
	app_customer_nvkey_t* default_setting = NULL;

	nvkey_status_t ret = nvkey_data_item_length(NVID_CUSTOMER_DEFAULT_SETTING, &size);

	APPS_LOG_MSGID_I("app_nvkey_setting_init ret=%d, size=%d\n",2, ret, size);

	default_setting = (app_customer_nvkey_t *)pvPortMalloc(size);

	ret = nvkey_read_data(NVID_CUSTOMER_DEFAULT_SETTING,(uint8_t *)default_setting, &size);
	memcpy(&g_customer_nvkey_setting, (uint8_t *)default_setting, size);

	if (default_setting)
	{
		vPortFree(default_setting);
		default_setting = NULL;
	}

	if(ret != NVKEY_STATUS_OK)
		APPS_LOG_MSGID_E("app_nvkey_setting_init()	error!!!!\n", 0);

	/**If need update customer nvkey after OTA*/
	if(ota_upgrade_nvkey_flag)
	{
		app_csutomer_nvkey_ota_update();
		ota_upgrade_nvkey_flag = false;
	}

	if(g_customer_nvkey_setting.factory_reset_flag)
	{
		app_customer_custom_eq_reset();
		app_nvkey_factory_reset_flag_write(false);
	}

}
void app_nvkey_setting_set(app_customer_nvkey_t* p_Setting)
{
	uint32_t size = 0;
	app_customer_nvkey_t* default_setting = NULL;

	nvkey_status_t ret = nvkey_data_item_length(NVID_CUSTOMER_DEFAULT_SETTING, &size);

	APPS_LOG_MSGID_I("app_nvkey_setting_set ret=%d, size=%d\n",2, ret, size);

	if (NVKEY_STATUS_OK == ret)
	{
		default_setting = (app_customer_nvkey_t *)pvPortMalloc(size);
		if (default_setting)
		{
			if(p_Setting == NULL)
			{
				memcpy((uint8_t *)default_setting, &g_customer_nvkey_setting, size);
			}
			else
			{
				memcpy((uint8_t *)default_setting, (uint8_t *)p_Setting, size);
			}

			ret = nvkey_write_data(NVID_CUSTOMER_DEFAULT_SETTING,(uint8_t *)default_setting, size);
			vPortFree(default_setting);
			default_setting = NULL;
		}

	}

}

void app_nvkey_sync_aws_data(void* p_data)
{
	app_customer_nvkey_t* p_customer_nvkey_setting = &g_customer_nvkey_setting;
	app_customer_setting_sync_t* pSync_data = (app_customer_setting_sync_t*)p_data;

	p_customer_nvkey_setting->eco_charging_profile_status = pSync_data->eco_charging_status;
	p_customer_nvkey_setting->shipping_mode_state = pSync_data->shipping_mode_state;
	p_customer_nvkey_setting->device_color = pSync_data->color_id;
	p_customer_nvkey_setting->mini_ui = pSync_data->mini_ui;

#ifdef BLE_ZOUND_ENABLE
	LOG_MSGID_I(GATT_ZOUND_SERVICE, "app_nvkey_sync_aws_data", 0);
	p_customer_nvkey_setting->touch_lock = pSync_data->touch_lock;
	p_customer_nvkey_setting->ui_sounds = pSync_data->ui_sounds;
	p_customer_nvkey_setting->anc_level = pSync_data->anc_level;
	p_customer_nvkey_setting->tra_level = pSync_data->tra_level;
	memmove(p_customer_nvkey_setting->eq_settings_custom_preset,pSync_data->eq_settings_custom_preset,sizeof(pSync_data->eq_settings_custom_preset));
	memmove(p_customer_nvkey_setting->wear_sensor_action,pSync_data->wear_sensor_action,sizeof(pSync_data->wear_sensor_action));
	memmove(p_customer_nvkey_setting->eq_settings,pSync_data->eq_settings,sizeof(pSync_data->eq_settings));
	memmove(p_customer_nvkey_setting->action_button_configuration, pSync_data->action_button_configuration, sizeof(p_customer_nvkey_setting->action_button_configuration));
#endif

	memmove(p_customer_nvkey_setting->eco_charging_setting, pSync_data->eco_charging_setting, sizeof(p_customer_nvkey_setting->eco_charging_setting));

	app_nvkey_setting_set(NULL);//customer setting reset to nvkey
}

void app_customer_nvkey_backup(void)
{
	app_customer_nvkey_t* p_customer_nvkey_setting = &g_customer_nvkey_setting;
	APPS_LOG_MSGID_I("backup nvkey!\n", 0);
    uint8_t anc_enable;
    audio_anc_control_filter_id_t anc_current_filter_id;
    audio_anc_control_type_t anc_current_type;
    int16_t anc_runtime_gain;
    uint8_t support_hybrid_enable;
    bool control_ret = FALSE;
	
    audio_anc_control_get_status(&anc_enable, &anc_current_filter_id, &anc_current_type, &anc_runtime_gain, &support_hybrid_enable, NULL);

	if(anc_enable)
	{
		if(anc_current_filter_id == AUDIO_ANC_CONTROL_ANC_FILTER_DEFAULT)
		{
			p_customer_nvkey_setting->anc_status_bk = 1;//anc on
		}
		else
		{
			p_customer_nvkey_setting->anc_status_bk = 2;//passthru on
		}
	}
	else
	{
		p_customer_nvkey_setting->anc_status_bk = 0;// off
	}
	p_customer_nvkey_setting->anc_status_bk |= 0x80;
	
	app_nvkey_setting_set(NULL);	

}

void app_customer_custom_eq_reset(void)
{
	APPS_LOG_MSGID_I("app_customer_custom_eq_reset\n", 0);

#if defined (PROJECT_GAHAN)
	// customer eq 101
	const uint8_t cus_eq_e42c[] = {0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0D, 0x00, 0x01, 0x00, 0x90, 0x6B, 0x00, 0x22, 0xA1, 0xBD, 0x00, 0xB9, 0x57, 0x58, 0x00, 0x6A, 0x9E, 0xB3, 0x00, 0x28, 0x76, 0x61, 0x00, 0xA3, 0x9A, 0x04, 0x00, 0x81, 0x02, 0x00, 0x0D, 0x00, 0x01, 0x00, 0x90, 0x6B, 0x00, 0x22, 0x89, 0xB8, 0x00, 0xF6, 0x78, 0x59, 0x00, 0x22, 0x98, 0xAD, 0x00, 0xFE, 0x33, 0x63, 0x00, 0x17, 0x9C, 0x04, 0x00, 0xC8, };
	nvkey_write_data(NVID_DSP_ALG_PEQ_COF_29, cus_eq_e42c, sizeof(cus_eq_e42c) / sizeof(uint8_t));

	const uint8_t cus_eq_ef00[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0xF0, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC5, 0x7A, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x01, 0x02, 0x40, 0x9C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x9C, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x01, 0x02, 0xA0, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x86, 0x01, 0x00, 0x64, 0x00, 0x00, 0x00, 0x01, 0x02, 0x90, 0xD0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0xD0, 0x03, 0x00, 0x64, 0x00, 0x00, 0x00, 0x01, 0x02, 0xD0, 0xDD, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0xCF, 0x09, 0x00, 0x46, 0x00, 0x00, 0x00, 0x01, 0x02, 0xA0, 0xD9, 0x08, 0x00, 0xE8, 0xFE, 0xFF, 0xFF, 0x3A, 0xC6, 0x02, 0x00, 0x3F, 0x01, 0x00, 0x00, 0x00, 0x02, 0x40, 0x0D, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x86, 0x01, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x02, 0x80, 0x1A, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x0D, 0x03, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x35, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1A, 0x06, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x6A, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35, 0x0C, 0x00, 0xC8, 0x00, 0x00, 0x00, };
	nvkey_write_data(NVID_TOOL_DSP_A2DP_PEQ_29, cus_eq_ef00, sizeof(cus_eq_ef00) / sizeof(uint8_t));
#elif defined (PROJECT_KALLA)
	// customer eq 101
       const uint8_t cus_eq_e42c[] = {0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x35, 0x00, 0x05, 0x00, 0x90, 0x6B, 0x00, 0x22, 0xCC, 0x94, 0x00, 0x0F, 0xD8, 0x6A, 0x00, 0x4D, 0x6D, 0x80, 0x00, 0xB6, 0x25, 0x7F, 0x00, 0x3D, 0xFF, 0x7F, 0x00, 0xFF, 0xB2, 0x81, 0x00, 0xE7, 0xA4, 0x7C, 0x00, 0x74, 0xB2, 0x81, 0x00, 0xE7, 0xA4, 0x7C, 0x00, 0x74, 0xFF, 0x7F, 0x00, 0xFF, 0xC3, 0x86, 0x00, 0xB0, 0x16, 0x73, 0x00, 0x76, 0xC3, 0x86, 0x00, 0xB0, 0x16, 0x73, 0x00, 0x76, 0xFF, 0x7F, 0x00, 0xFF, 0x76, 0x9A, 0x00, 0x69, 0x9B, 0x53, 0x00, 0xA3, 0x76, 0x9A, 0x00, 0x69, 0x9B, 0x53, 0x00, 0xA3, 0xFF, 0x7F, 0x00, 0xFF, 0x21, 0xE0, 0x00, 0xE7, 0x8F, 0x18, 0x00, 0x73, 0x21, 0xE0, 0x00, 0xE7, 0x8F, 0x18, 0x00, 0x73, 0xC2, 0x04, 0x00, 0x8F, 0x02, 0x00, 0x35, 0x00, 0x05, 0x00, 0x90, 0x6B, 0x00, 0x22, 0xC4, 0x94, 0x00, 0x92, 0xE7, 0x6A, 0x00, 0x31, 0x64, 0x80, 0x00, 0xCD, 0x36, 0x7F, 0x00, 0xF5, 0xFF, 0x7F, 0x00, 0xFF, 0x8F, 0x81, 0x00, 0x9D, 0xE9, 0x7C, 0x00, 0x71, 0x8F, 0x81, 0x00, 0x9D, 0xE9, 0x7C, 0x00, 0x71, 0xFF, 0x7F, 0x00, 0xFF, 0x37, 0x86, 0x00, 0xD1, 0x16, 0x74, 0x00, 0x28, 0x37, 0x86, 0x00, 0xD1, 0x16, 0x74, 0x00, 0x28, 0xFF, 0x7F, 0x00, 0xFF, 0x60, 0x98, 0x00, 0x53, 0x8E, 0x56, 0x00, 0xF1, 0x60, 0x98, 0x00, 0x53, 0x8E, 0x56, 0x00, 0xF1, 0xFF, 0x7F, 0x00, 0xFF, 0x1F, 0xD9, 0x00, 0xE9, 0x80, 0x1B, 0x00, 0x5B, 0x1F, 0xD9, 0x00, 0xE9, 0x80, 0x1B, 0x00, 0x5B, 0xC2, 0x04, 0x00, 0x8F, };
    nvkey_write_data(NVID_DSP_ALG_PEQ_COF_29, cus_eq_e42c, sizeof(cus_eq_e42c) / sizeof(uint8_t));

       const uint8_t cus_eq_ef00[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x4E, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5D, 0x12, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x01, 0x02, 0xD4, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE1, 0x48, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x01, 0x02, 0x50, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x23, 0x01, 0x00, 0x43, 0x00, 0x00, 0x00, 0x01, 0x02, 0x40, 0x0D, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x8E, 0x04, 0x00, 0x43, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x35, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2E, 0x38, 0x12, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00, 0x02, 0xA0, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0xC3, 0x00, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x0D, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x86, 0x01, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x02, 0x80, 0x1A, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x0D, 0x03, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x35, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1A, 0x06, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x6A, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35, 0x0C, 0x00, 0xC8, 0x00, 0x00, 0x00, };
    nvkey_write_data(NVID_TOOL_DSP_A2DP_PEQ_29, cus_eq_ef00, sizeof(cus_eq_ef00) / sizeof(uint8_t));
#else
#endif

}

void app_csutomer_nvkey_ota_update(void)
{
	app_customer_nvkey_t* p_customer_nvkey_setting = &g_customer_nvkey_setting;
	APPS_LOG_MSGID_I("app_csutomer_nvkey_ota_update\n", 0);
	//TO DO:
	//re-write customer nvkey in below

	if(p_customer_nvkey_setting->cus_eq101_set != 0x03)
	{
		p_customer_nvkey_setting->cus_eq101_set = 0x03;
		app_customer_custom_eq_reset();
	}

	app_nvkey_setting_set(NULL);

	
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
	//app_power_save_timeout_update_via_ota();
#endif

}

void app_nvkey_factory_reset(void)
{
	app_customer_nvkey_t* p_customer_nvkey_setting = &g_customer_nvkey_setting;

	p_customer_nvkey_setting->factory_test_mode = 0x0;
	p_customer_nvkey_setting->eco_charging_profile_status = 0x0;
	//p_customer_nvkey_setting->shipping_mode_state = 0x1;
	p_customer_nvkey_setting->mini_ui = 0x00;
	p_customer_nvkey_setting->hx300x_log_debug = 0;
#ifdef BLE_ZOUND_ENABLE
	LOG_MSGID_I(GATT_ZOUND_SERVICE, "app_nvkey_factory_reset", 0);
	p_customer_nvkey_setting->touch_lock = 0x0;
	p_customer_nvkey_setting->anc_level = 0x64;
	p_customer_nvkey_setting->tra_level = 0x64;

	uint8_t eq_settings_custom_preset[]={0x05,0x05,0x05,0x05,0x05};
	memmove(p_customer_nvkey_setting->eq_settings_custom_preset,eq_settings_custom_preset,sizeof(p_customer_nvkey_setting->eq_settings_custom_preset));

	uint8_t action_button_default[]={0x00,0x12,0x13,0x0E,0x06,0x09,0x01,0x12,0x13,0x0E,0x06,0x09};
	memmove(p_customer_nvkey_setting->action_button_configuration,action_button_default,sizeof(p_customer_nvkey_setting->action_button_configuration));
	p_customer_nvkey_setting->ui_sounds = 0x0001;
	memset(p_customer_nvkey_setting->eq_settings,0,sizeof(p_customer_nvkey_setting->eq_settings));

	uint8_t wear_sensor_action[]={0x03,0x03,0x02};
	memmove(p_customer_nvkey_setting->wear_sensor_action,wear_sensor_action,sizeof(wear_sensor_action));
	
	uint8_t eco_tmp[] = {0x02, 0x64, 0x04, 0x00, 0x80, 0x80, 0x00, 0x00};
	memmove(p_customer_nvkey_setting->eco_charging_setting, eco_tmp, sizeof(eco_tmp));

	//Set ANC on after factory reset
	Set_ANC_On_After_Factory_Reset();
	
#endif
	app_nvkey_setting_set(NULL);//customer setting reset to nvkey
}



void app_nvkey_psensor_calibration_cfg_read(uint8_t* PsDacCtrl_val, uint8_t* PsFineCt_Val, uint16_t* PsData, uint8_t* Calibrate_Status)
{
	app_customer_nvkey_t* p_customer_nvkey_setting = &g_customer_nvkey_setting;

	*PsDacCtrl_val = p_customer_nvkey_setting->psensor_PsDacCtrl;
	*PsFineCt_Val = p_customer_nvkey_setting->psensor_PsFineCt;
	*PsData = p_customer_nvkey_setting->psensor_PsCal;
	*Calibrate_Status = p_customer_nvkey_setting->psensor_calibrate_status;

}

void app_nvkey_psensor_calibration_cfg_write(uint8_t PsDacCtrl_val, uint8_t PsFineCt_Val, uint16_t PsData, uint8_t Calibrate_Status)
{
	app_customer_nvkey_t* p_customer_nvkey_setting = &g_customer_nvkey_setting;

    p_customer_nvkey_setting->psensor_PsDacCtrl = PsDacCtrl_val;
	p_customer_nvkey_setting->psensor_PsFineCt = PsFineCt_Val;
	p_customer_nvkey_setting->psensor_PsCal = PsData;
	p_customer_nvkey_setting->psensor_calibrate_status = Calibrate_Status;

	//reset threshold
	p_customer_nvkey_setting->PsensorPsThresholdLow = 0;
	p_customer_nvkey_setting->PsensorPsThresholdHigh = 0;
    app_nvkey_setting_set(NULL);
}

void app_nvkey_psensor_threshold_read(uint8_t* Crosstalk, uint16_t* thresholdHigh, uint16_t* thresholdLow)
{
	app_customer_nvkey_t* p_customer_nvkey_setting = &g_customer_nvkey_setting;

	*thresholdHigh = p_customer_nvkey_setting->PsensorPsThresholdHigh;
	*thresholdLow = p_customer_nvkey_setting->PsensorPsThresholdLow;
	*Crosstalk = p_customer_nvkey_setting->psensor_PsDacCtrl;
}

void app_nvkey_psensor_threshold_write(uint16_t threshold, uint8_t type)
{
	app_customer_nvkey_t* p_customer_nvkey_setting = &g_customer_nvkey_setting;

	if(type == 0x01)
	{
		if(threshold < 100)
			threshold = 100;

    	p_customer_nvkey_setting->PsensorPsThresholdLow = threshold;
	}
	else if(type == 0x02)
	{
		if(threshold >= 1023)
			threshold = 1000;

		p_customer_nvkey_setting->PsensorPsThresholdHigh = threshold;
	}

    app_nvkey_setting_set(NULL);
}



uint8_t app_nvkey_factory_test_mode_read(void)
{
    return g_customer_nvkey_setting.factory_test_mode;
}

void app_nvkey_factory_test_mode_write(uint8_t test_mode)
{
    g_customer_nvkey_setting.factory_test_mode = test_mode;
    app_nvkey_setting_set(NULL);
}

uint8_t app_nvkey_shipping_mode_read(void)
{
    return g_customer_nvkey_setting.shipping_mode_state;
}

void app_nvkey_shipping_mode_set(uint8_t shipping_mode)
{
    g_customer_nvkey_setting.shipping_mode_state = shipping_mode;
    app_nvkey_setting_set(NULL);
}

uint8_t app_nvkey_anc_status_bk_read(void)
{
    return g_customer_nvkey_setting.anc_status_bk;
}
void app_nvkey_anc_status_bk_set(uint8_t status)
{
    g_customer_nvkey_setting.anc_status_bk = status;
    app_nvkey_setting_set(NULL);
}


uint8_t app_nvkey_ble_color_id_read(void)
{
    return g_customer_nvkey_setting.device_color;
}
void app_nvkey_ble_color_id_set(uint8_t color_id)
{
    g_customer_nvkey_setting.device_color = color_id;
    app_nvkey_setting_set(NULL);
}

uint8_t app_nvkey_ble_mini_ui_read(void)
{
	return g_customer_nvkey_setting.mini_ui;
}
void app_nvkey_ble_mini_ui_set(uint8_t mini_ui)
{
	g_customer_nvkey_setting.mini_ui = mini_ui;
	app_nvkey_setting_set(NULL);
}

uint8_t app_nvkey_is_lea_disable(void)
{
	return g_customer_nvkey_setting.lea_disable;
}
void app_nvkey_is_lea_disable_set(uint8_t lea_disable)
{
	g_customer_nvkey_setting.lea_disable = lea_disable;
	app_nvkey_setting_set(NULL);
}


uint8_t app_nvkey_ble_touch_limit_disable_read(void)
{
	return g_customer_nvkey_setting.touch_limit_disable;
}
void app_nvkey_ble_touch_limit_disable_set(uint8_t touch_limit_disable)
{
	g_customer_nvkey_setting.touch_limit_disable = touch_limit_disable;
	app_nvkey_setting_set(NULL);
}

void app_nvkey_psensor_setting_read(uint8_t * data)
{
	memcpy(data, g_customer_nvkey_setting.psensor_setting, 4);
}

void app_nvkey_psensor_setting_set(uint8_t *data)
{
	uint8_t tem_data[4] = {0};
	
	if(data[0])
	{
		tem_data[0] = data[0];
		tem_data[0] |= 0x80;
	}

	if(data[1])
	{
		tem_data[1] = data[1];
		tem_data[1] |= 0x80;
	}

	if(data[2])
	{
		tem_data[2] = data[2];
		tem_data[2] |= 0x80;
	}

	if(data[3])
	{
		tem_data[3] = data[3];
		tem_data[3] |= 0x80;
	}	

	memcpy(g_customer_nvkey_setting.psensor_setting, tem_data,4);
	g_customer_nvkey_setting.psensor_calibrate_status = 0x0;//clear calibrate status, will auto calibrate in power on
	app_nvkey_setting_set(NULL);

	if(data[3])
	{
	    uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

		*p_key_action = KEY_SYSTEM_REBOOT;

	    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
	                        sizeof(uint16_t), NULL, 500);
	}
}



uint8_t app_nvkey_psensor_power_read(void)
{
	return g_customer_nvkey_setting.psensor_power;
}
void app_nvkey_psensor_power_set(uint8_t PsW)
{
	if(PsW)
	{
		PsW |= 0x80;
	}

	g_customer_nvkey_setting.psensor_power = PsW;
	g_customer_nvkey_setting.psensor_calibrate_status = 0x0;//clear calibrate status, will auto calibrate in power on
	app_nvkey_setting_set(NULL);

    uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

	*p_key_action = KEY_SYSTEM_REBOOT;

    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
                        sizeof(uint16_t), NULL, 100);
}
void app_nvkey_eq_settings_set(uint8_t* eqs, uint8_t len){
	memmove(g_customer_nvkey_setting.eq_settings, eqs, len);
	app_nvkey_setting_set(NULL);
}
void app_nvkey_eq_settings_read(uint8_t* eqs, uint8_t len){
	memmove(eqs, g_customer_nvkey_setting.eq_settings, len);
}

void app_nvkey_wear_sensor_action_set(uint8_t* action, uint8_t len){
	memmove(g_customer_nvkey_setting.wear_sensor_action, action, len);
	app_nvkey_setting_set(NULL);
}
void app_nvkey_wear_sensor_action_read(uint8_t* action, uint8_t len){
	memmove(action, g_customer_nvkey_setting.wear_sensor_action, len);
}
void app_nvkey_sn_set(uint8_t* sn, uint8_t len){
	memmove(g_customer_nvkey_setting.sn, sn, len);
	app_nvkey_setting_set(NULL);
}
void app_nvkey_sn_read(uint8_t* buf, uint8_t len){
	memmove(buf, g_customer_nvkey_setting.sn, len);
}
void app_nvkey_hw_version_read(uint8_t* buf,uint8_t len)
{
	if(len>5)
		len = 5;
	
	memmove(buf, &g_customer_nvkey_setting.customer_hw_version[1],len);
}

void app_nvkey_hw_version_set(uint8_t* ver, uint8_t len)
{
	memmove(&g_customer_nvkey_setting.customer_hw_version[1],ver,len);
	app_nvkey_setting_set(NULL);
}

void app_nvkey_eco_setting_read(void* setting)
{
	memcpy(setting, g_customer_nvkey_setting.eco_charging_setting, 8);
}

void app_nvkey_eco_setting_set(void* setting)
{
	app_eco_charge_soc_limit_handle(setting);
	memcpy(g_customer_nvkey_setting.eco_charging_setting, setting, 8);
	app_nvkey_setting_set(NULL);
}


void app_nvkey_hx300x_read_cali_setting(void *pEvt)
{
	typedef struct
	{
		uint8_t status;
		uint8_t data_reg10;
		uint8_t data_reg12;
		uint8_t data_reg13;
		uint16_t data_h_thod;
		uint16_t data_l_thod;
		uint16_t data_read_ps3;
	}PACKED RSP;

	RSP* pRsp = (RSP*)pEvt;

	pRsp->data_reg10 = g_customer_nvkey_setting.hx300x_reg10;
	pRsp->data_reg12 = g_customer_nvkey_setting.hx300x_reg12;
	pRsp->data_reg13 = g_customer_nvkey_setting.hx300x_reg13;
	pRsp->data_h_thod = g_customer_nvkey_setting.hx300x_THOD_H;
	pRsp->data_l_thod = g_customer_nvkey_setting.hx300x_THOD_L;
	pRsp->data_read_ps3 = g_customer_nvkey_setting.hx300x_read_ps;	

}

uint8_t app_nvkey_hx300x_Gain_read(void)
{
	return g_customer_nvkey_setting.hx300x_reg12;
}

void app_nvkey_hx300x_Thres_read(uint16_t* h_thrd, uint16_t* l_thrd)
{
	h_thrd = &g_customer_nvkey_setting.hx300x_THOD_H;
	l_thrd = &g_customer_nvkey_setting.hx300x_THOD_L;
}

void app_nvkey_hx300x_cur_read(uint16_t* reg_10, uint8_t* reg_13)
{
	reg_10 = &g_customer_nvkey_setting.hx300x_reg10;
	reg_13 = &g_customer_nvkey_setting.hx300x_reg13;
}


void app_nvkey_hx300x_setting_save(uint8_t reg10, uint8_t reg12, uint8_t reg13, uint16_t THOD_H, uint16_t THOD_L, uint16_t read_ps)
{
	g_customer_nvkey_setting.hx300x_reg10 = reg10;
	g_customer_nvkey_setting.hx300x_reg12 = reg12;
	g_customer_nvkey_setting.hx300x_reg13 = reg13;
	g_customer_nvkey_setting.hx300x_THOD_H = THOD_H;		
	g_customer_nvkey_setting.hx300x_THOD_L = THOD_L;	
	g_customer_nvkey_setting.hx300x_read_ps = read_ps;
	app_nvkey_setting_set(NULL);
}

uint8_t app_nvkey_get_hx300x_log_status(void)
{
	return g_customer_nvkey_setting.hx300x_log_debug;
}
void app_nvkey_set_hx300x_log_status(uint8_t log_state)
{
	g_customer_nvkey_setting.hx300x_log_debug = log_state;
	app_nvkey_setting_set(NULL);
}



uint8_t app_nvkey_eco_soc_limit_status_read(void)
{
    return g_customer_nvkey_setting.eco_soc_display_limit;
}

void app_nvkey_eco_soc_limit_status_write(uint8_t limit_enable)
{
    g_customer_nvkey_setting.eco_soc_display_limit = limit_enable;
    app_nvkey_setting_set(NULL);
}


uint8_t app_nvkey_eco_charging_profile_status_read(void)
{
    return g_customer_nvkey_setting.eco_charging_profile_status;
}

void app_nvkey_eco_charging_profile_status_write(uint8_t profile_status)
{
    g_customer_nvkey_setting.eco_charging_profile_status = profile_status;
    app_nvkey_setting_set(NULL);
}
void app_nvkey_ui_sounds_write(uint16_t uis){

	g_customer_nvkey_setting.ui_sounds = uis;
	app_nvkey_setting_set(NULL);
}
void app_nvkey_action_button_configuration_write(uint8_t* abc){
	memcpy(g_customer_nvkey_setting.action_button_configuration, abc, sizeof(g_customer_nvkey_setting.action_button_configuration));
	app_nvkey_setting_set(NULL);
}
void app_nvkey_action_touch_lock_write(uint8_t state) {
	g_customer_nvkey_setting.touch_lock = state;
	app_nvkey_setting_set(NULL);
}
app_customer_nvkey_t app_nvkey_customer_all(void){
	return g_customer_nvkey_setting;
}
void app_nvkey_anc_level_write(uint8_t level) {
	g_customer_nvkey_setting.anc_level = level;
	app_nvkey_setting_set(NULL);
}
void app_nvkey_tra_level_write(uint8_t level) {
	g_customer_nvkey_setting.tra_level = level;
	app_nvkey_setting_set(NULL);
}

uint8_t app_nvkey_battery_heathy_percent2time_read(void)
{
	return g_customer_nvkey_setting.battery_heathy_percent_to_time;
}
void app_nvkey_battery_heathy_percent2time_set(uint8_t percent2time)
{
	g_customer_nvkey_setting.battery_heathy_percent_to_time = percent2time;
	app_nvkey_setting_set(NULL);
}

uint8_t app_nvkey_factory_reset_flag_read(void)
{
	return g_customer_nvkey_setting.factory_reset_flag;
}


void app_nvkey_factory_reset_flag_write(uint8_t flag)
{
	g_customer_nvkey_setting.factory_reset_flag = flag;
	app_nvkey_setting_set(NULL);
}

uint16_t app_nvkey_battery_heathy_read(void)
{
	uint16_t times = 0;

	if(g_customer_nvkey_setting.battery_heathy_percent_to_time == 0)
		g_customer_nvkey_setting.battery_heathy_percent_to_time = 90;

	times = (uint16_t)(((uint32_t)g_customer_nvkey_setting.battery_heathy_times*100 + g_customer_nvkey_setting.battery_heathy_percent)/g_customer_nvkey_setting.battery_heathy_percent_to_time);

	APPS_LOG_MSGID_I("nvkey battery heathy READ times=%d+%d%, uint=%d%\n",3, g_customer_nvkey_setting.battery_heathy_times, g_customer_nvkey_setting.battery_heathy_percent, g_customer_nvkey_setting.battery_heathy_percent_to_time);

	if(times == 0 && g_customer_nvkey_setting.battery_heathy_percent)
		times = 1;

	return times;
}
void app_nvkey_battery_heathy_write(uint8_t percent, uint8_t is_reset)
{
	g_customer_nvkey_setting.battery_heathy_percent += percent;

	if(g_customer_nvkey_setting.battery_heathy_percent>= 100)
	{
		uint8_t times = g_customer_nvkey_setting.battery_heathy_percent/100;

		g_customer_nvkey_setting.battery_heathy_times += times;
		g_customer_nvkey_setting.battery_heathy_percent -= (times*100);
	}

	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
			&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		uint16_t heathy_times = app_nvkey_battery_heathy_read();
		
		if(heathy_times)
			app_battery_heathy_send2peer(heathy_times);
	}

	if(is_reset)
	{
		g_customer_nvkey_setting.battery_heathy_times = 0;
		g_customer_nvkey_setting.battery_heathy_percent = 0;

		if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
				&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
		{
			app_battery_heathy_send2peer(0);
		}

	}

	APPS_LOG_MSGID_I("nvkey battery heathy times=%d+%d%\n",2, g_customer_nvkey_setting.battery_heathy_times, g_customer_nvkey_setting.battery_heathy_percent);

	app_nvkey_setting_set(NULL);
}



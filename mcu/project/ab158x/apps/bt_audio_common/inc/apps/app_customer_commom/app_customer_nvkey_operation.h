
/**
  * @file    app_customer_nvkey_operation.h
  *
  * @author  wayne.xiong
  *
  * @date    2022/3/29
  *
  * @brief   customer nvkey operation
**/

#ifndef __APP_CUSTOMER_NVKEY_OPERATION_H__
#define __APP_CUSTOMER_NVKEY_OPERATION_H__

#include <stdint.h>
#include <string.h>

#define PACKED __attribute__((packed))


//LSB first
typedef struct {
	uint8_t psensor_PsDacCtrl;					//Byte 1
	uint8_t psensor_PsFineCt;					//Byte 2
	uint16_t psensor_PsCal;					//Byte 3-4
	uint8_t psensor_calibrate_status;		//Byte 5
	uint8_t factory_test_mode;					//Byte 6
	uint8_t eco_charging_profile_status;		//byte 7
	uint8_t device_color;		//byte 8
	uint16_t ui_sounds; // byte 9-10
	uint8_t shipping_mode_state;				//byte 11
	uint8_t action_button_configuration[12];//12-23
	//uint8_t touch_lock;//24
	uint8_t factory_autopoweroff;//24
	uint8_t anc_level;//25
	uint8_t tra_level;//26
	uint8_t mini_ui;//27
	//uint8_t reserved1;//28
	uint8_t nv_btname;//28  harry use
	uint16_t PsensorPsThresholdHigh;//29-30
	uint16_t PsensorPsThresholdLow;//31-32
	uint8_t psensor_power;//33
	uint8_t customer_hw_version[6];//34-39
	uint8_t sn[22];//40-61
	uint8_t eq_settings[3];//62-64
	uint8_t battery_heathy_percent_to_time;//65
	uint8_t battery_heathy_percent;//66
	uint16_t battery_heathy_times;//67-68
	uint8_t eq_settings_custom_preset[5]; //69-73
	uint8_t touch_limit_disable;
	uint8_t anc_gain_backup[8];
	uint8_t wear_sensor_action[3];
	uint8_t eco_charging_setting[8];
	uint8_t factory_reset_flag;
	uint8_t psensor_setting[4];
	uint8_t cus_eq101_set;
	uint8_t anc_status_bk;
	uint8_t lea_disable;
	uint8_t eco_soc_display_limit;
	uint8_t hx300x_reg10;
	uint8_t hx300x_reg12;
	uint8_t hx300x_reg13;
	uint16_t hx300x_THOD_H;
	uint16_t hx300x_THOD_L;
	uint16_t hx300x_read_ps;
	uint8_t hx300x_log_debug;
	uint8_t reserved[16];
}PACKED app_customer_nvkey_t;

//LSB first
typedef struct {
	uint8_t eco_charging_status;
	uint8_t shipping_mode_state;
	uint16_t ui_sounds;
	uint8_t touch_lock;
	uint8_t action_button_configuration[12];
	uint8_t anc_level;
	uint8_t tra_level;
	uint8_t color_id;
	uint8_t mini_ui;
	uint16_t bat_heathy_times;
	uint8_t eq_settings[3];
	uint8_t eq_settings_custom_preset[5];
	uint8_t wear_sensor_action[3];
	uint8_t eco_charging_setting[8];
} PACKED app_customer_setting_sync_t;

void app_nvkey_sync_aws_data(void* p_data);
void app_nvkey_factory_reset(void);
void app_nvkey_setting_init(void);
void app_nvkey_psensor_calibration_cfg_read(uint8_t* PsDacCtrl_val, uint8_t* PsFineCt_Val, uint16_t* PsData, uint8_t* Calibrate_Status);
void app_nvkey_psensor_calibration_cfg_write(uint8_t PsDacCtrl_val, uint8_t PsFineCt_Val, uint16_t PsData, uint8_t Calibrate_Status);
uint8_t app_nvkey_factory_test_mode_read(void);
void app_nvkey_factory_test_mode_write(uint8_t test_mode);
uint8_t app_nvkey_eco_charging_profile_status_read(void);
void app_nvkey_eco_charging_profile_status_write(uint8_t profile_status);
void app_nvkey_ui_sounds_write(uint16_t uis);
void app_nvkey_action_button_configuration_write(uint8_t* abc);
void app_nvkey_action_touch_lock_write(uint8_t state);
void app_nvkey_anc_level_write(uint8_t level);
void app_nvkey_tra_level_write(uint8_t level);
app_customer_nvkey_t app_nvkey_customer_all(void);
uint8_t app_nvkey_shipping_mode_read(void);
void app_nvkey_shipping_mode_set(uint8_t shipping_mode);
void app_csutomer_nvkey_ota_update(void);
uint8_t app_nvkey_ble_color_id_read(void);
void app_nvkey_ble_color_id_set(uint8_t color_id);
uint8_t app_nvkey_ble_mini_ui_read(void);
void app_nvkey_ble_mini_ui_set(uint8_t mini_ui);
void app_nvkey_psensor_threshold_read(uint8_t* Crosstalk, uint16_t* thresholdHigh, uint16_t* thresholdLow);
void app_nvkey_psensor_threshold_write(uint16_t threshold, uint8_t type);
uint8_t app_nvkey_psensor_power_read(void);
void app_nvkey_psensor_power_set(uint8_t PsW);
void app_nvkey_hw_version_set(uint8_t* ver, uint8_t len);
void app_nvkey_hw_version_read(uint8_t* buf, uint8_t len);
void app_nvkey_sn_set(uint8_t* sn, uint8_t len);
void app_nvkey_sn_read(uint8_t* buf, uint8_t len);
void app_nvkey_eq_settings_set(uint8_t* eqs, uint8_t len);
void app_nvkey_eq_settings_read(uint8_t* eqs, uint8_t len);
void app_nvkey_wear_sensor_action_set(uint8_t* action, uint8_t len);
void app_nvkey_wear_sensor_action_read(uint8_t* action, uint8_t len);
void ota_update_cus_nvkey_set(void);
uint8_t app_nvkey_battery_heathy_percent2time_read(void);
void app_nvkey_battery_heathy_percent2time_set(uint8_t percent2time);
uint16_t app_nvkey_battery_heathy_read(void);
void app_nvkey_battery_heathy_write(uint8_t percent, uint8_t is_reset);
uint8_t app_nvkey_ble_touch_limit_disable_read(void);
void app_nvkey_ble_touch_limit_disable_set(uint8_t touch_limit_disable);
void app_customer_nvkey_backup(void);
void app_nvkey_eco_setting_set(void* setting);
void app_nvkey_eco_setting_read(void* setting);
void app_customer_custom_eq_reset(void);
void app_nvkey_factory_reset_flag_write(uint8_t flag);

uint8_t app_nvkey_anc_status_bk_read(void);
void app_nvkey_anc_status_bk_set(uint8_t status);

void app_nvkey_psensor_setting_read(uint8_t * data);

uint8_t app_nvkey_eco_soc_limit_status_read(void);
void app_nvkey_eco_soc_limit_status_write(uint8_t limit_enable);
void app_nvkey_hx300x_setting_save(uint8_t reg10, uint8_t reg12, uint8_t reg13, uint16_t THOD_H, uint16_t THOD_L, uint16_t read_ps);
uint8_t app_nvkey_hx300x_Gain_read(void);
void app_nvkey_hx300x_Thres_read(uint16_t* h_thrd, uint16_t* l_thrd);
void app_nvkey_hx300x_cur_read(uint16_t* reg_10, uint8_t* reg_13);
void app_nvkey_hx300x_read_cali_setting(void *pEvt);
uint8_t app_nvkey_get_hx300x_log_status(void);
void app_nvkey_set_hx300x_log_status(uint8_t log_state);
extern void app_nvkey_btname_write(uint8_t name);
extern uint8_t app_nvkey_btname_read(void);
void app_nvkey_action_factory_autopoweroff_write(uint8_t name);
uint8_t app_nvkey_action_factory_autopoweroff_read(void);
#endif //__APP_CUSTOMER_NVKEY_OPERATION_H__


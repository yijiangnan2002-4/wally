
/**
  * @file    app_customer_common.h
  *
  * @author  wayne.xiong
  *
  * @date    2022/3/29
  * 
  * @brief   customer common handle
**/

#ifndef __APP_CUSTOMER_COMMON_H__
#define __APP_CUSTOMER_COMMON_H__
#include <string.h>
#include <stdint.h>

#define CUS_LE_BIS_TEST 0

void app_customer_common_sync_aws_data(void* p_data);
void app_customer_common_setting_to_partner(void);
void app_customer_test_enable_dut(void);
void app_set_charger_case_version(uint8_t version_data);
void app_get_charger_case_version(uint8_t* p_version);
void app_set_eco_charing_profile_switch(uint8_t profile_status);
uint8_t app_get_eco_charing_profile_switch(void);
void app_set_a2dp_volume(uint8_t volume_level);
void app_system_factory_reset(uint8_t reset_mode);
void app_shipping_mode_exit(void);
extern void app_shipping_mode_enter(void);
extern uint8_t app_enter_shipping_mode_flag_read(void);
void app_enter_shipping_mode_flag_set(uint8_t shipping_mode);
void app_set_shipping_mode_state(uint8_t state);
uint8_t app_get_shipping_mode_state(void);
void app_spotify_tap_triggle(void);
void app_set_ble_device_color_state(uint8_t state);
void app_set_ble_device_mini_ui_state(uint8_t state);
void app_battery_heathy_send2peer(uint16_t times);
uint8_t app_smcharger_get_current_limit_status(void);
void app_smcharger_set_current_limit_status(uint8_t status);
void app_set_ble_write_SN(uint8_t *sn, uint8_t len);
void app_write_sn_compare(uint8_t *sn, uint8_t len);
void app_customer_common_tws_clean(void);
uint8_t app_write_sn_compare_result_read(void);
uint8_t app_touch_key_test_status_get(void);

void app_touch_key_test_status_set(uint8_t status);
void app_set_hw_version(uint8_t *ver, uint8_t len);
void app_force_disconnect_bt_connection_before_pairing(void);
void app_set_anc_status_bk(uint8_t status);
void app_common_set_eco_charging_soc(uint8_t status);
uint8_t app_common_get_eco_charging_soc(void);


#endif //__APP_CUSTOMER_COMMON_H__


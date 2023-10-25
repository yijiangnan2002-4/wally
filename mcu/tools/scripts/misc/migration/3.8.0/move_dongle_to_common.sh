mkdir ../bt_audio_common
mkdir ../bt_audio_common/src
mkdir ../bt_audio_common/src/app
mkdir ../bt_audio_common/inc
mkdir ../bt_audio_common/inc/app
mv -f inc/apps/app_battery ../bt_audio_common/inc/apps/
mv -f inc/apps/app_fota ../bt_audio_common/inc/apps/
mv -f inc/apps/app_line_in ../bt_audio_common/inc/apps/
mv -f inc/apps/app_power_save ../bt_audio_common/inc/apps/
mv -f inc/apps/app_state_report ../bt_audio_common/inc/apps/
mv -f inc/apps/audio_trans_mgr ../bt_audio_common/inc/apps/
mv -f inc/apps/config ../bt_audio_common/inc/apps/
mv -f inc/apps/led ../bt_audio_common/inc/apps/
mv -f inc/apps/utils ../bt_audio_common/inc/apps/
mv -f src/apps/app_battery ../bt_audio_common/src/apps/
mv -f src/apps/app_fota ../bt_audio_common/src/apps/
mv -f src/apps/app_line_in ../bt_audio_common/src/apps/
mv -f src/apps/app_power_save ../bt_audio_common/src/apps/
mv -f src/apps/app_state_report ../bt_audio_common/src/apps/
mv -f src/apps/audio_trans_mgr ../bt_audio_common/src/apps/
mv -f src/apps/config ../bt_audio_common/src/apps/
mv -f src/apps/led ../bt_audio_common/src/apps/
mv -f src/apps/utils ../bt_audio_common/src/apps/

sed -i '/APPS_INC = $(APP_PATH)\/inc\/apps/aAPPS_COMMON_SRC = $(APP_COMMON_PATH)\/src\/apps\r\nAPPS_COMMON_INC = $(APP_COMMON_PATH)\/inc\/apps' src/apps/module.mk

sed -i 's/$(APPS_SRC)\/app_fota/$(APPS_COMMON_SRC)\/app_fota/' src/apps/module.mk
sed -i 's/$(APPS_SRC)\/app_battery/$(APPS_COMMON_SRC)\/app_battery/' src/apps/module.mk
sed -i 's/$(APPS_SRC)\/app_power_save/$(APPS_COMMON_SRC)\/app_power_save/' src/apps/module.mk
sed -i 's/$(APPS_SRC)\/app_state_report/$(APPS_COMMON_SRC)\/app_state_report/' src/apps/module.mk
sed -i 's/$(APPS_SRC)\/config/$(APPS_COMMON_SRC)\/config/' src/apps/module.mk
sed -i 's/$(APPS_SRC)\/led/$(APPS_COMMON_SRC)\/led/' src/apps/module.mk
sed -i 's/$(APPS_SRC)\/utils/$(APPS_COMMON_SRC)\/utils/' src/apps/module.mk
sed -i 's/$(APPS_SRC)\/app_line_in/$(APPS_COMMON_SRC)\/app_line_in/' src/apps/module.mk
sed -i 's/$(APPS_SRC)\/audio_trans_mgr/$(APPS_COMMON_SRC)\/audio_trans_mgr/' src/apps/module.mk

sed -i 's/$(APPS_INC)\/app_fota/$(APPS_COMMON_INC)\/app_fota/' src/apps/module.mk
sed -i 's/$(APPS_INC)\/app_battery/$(APPS_COMMON_INC)\/app_battery/' src/apps/module.mk
sed -i 's/$(APPS_INC)\/app_power_save/$(APPS_COMMON_INC)\/app_power_save/' src/apps/module.mk
sed -i 's/$(APPS_INC)\/app_state_report/$(APPS_COMMON_INC)\/app_state_report/' src/apps/module.mk
sed -i 's/$(APPS_INC)\/config/$(APPS_COMMON_INC)\/config/' src/apps/module.mk
sed -i 's/$(APPS_INC)\/led/$(APPS_COMMON_INC)\/led/' src/apps/module.mk
sed -i 's/$(APPS_INC)\/utils/$(APPS_COMMON_INC)\/utils/' src/apps/module.mk
sed -i 's/$(APPS_INC)\/app_line_in/$(APPS_COMMON_INC)\/app_line_in/' src/apps/module.mk
sed -i 's/$(APPS_INC)\/audio_trans_mgr/$(APPS_COMMON_INC)\/audio_trans_mgr/' src/apps/module.mk
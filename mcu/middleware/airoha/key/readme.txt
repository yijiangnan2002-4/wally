Airo_key module usage guide

Brief:          Airo_key is a common upper layer for different types of keys, including captouch_key, eint_key, gsensor_key and powerkey.
                This module has a common interface and common event type.

Usage:          For specific information about the airo_key event type and API interfaces, please refer to middleware\airoha\key\common\inc\airo_key_event.h


Dependency:     AIRO_KEY_EVENT_ENABLE: This option configures whether airo_key is enabled. Defined in mcu\middleware\airoha\key\module.mk, the user can include this module.mk on Makefile to enable this feature.
                Sub options:
                    MTK_GSENSOR_KEY_ENABLE: This option configures whether airo_key can support gsensor_key. Defined in mcu\driver\board\component\bsp_gsensor_key\module.mk, the user can include this module.mk on Makefile to enable this feature.
                    MTK_EINT_KEY_ENABLE: This option configures whether airo_key supports eint_key. Defined in mcu\driver\board\component\bsp_eint_key\module.mk, the user can include this module.mk on Makefile to enable this feature.
                    HAL_CAPTOUCH_MODULE_ENABLED: This option configures whether airo_key supports captouch_key. It is defined in the hal_feature_config.h under the project inc folder.
                    AIRO_KEY_FEATRURE_POWERKEY: This option configures whether airo_key supports powerkey. It is defined in the airo_key_config.h under the project inc folder.
                    AIR_PSENSOR_KEY_ENABLE: This option configures whether airo_key supports powerkey. Defined in mcu\driver\board\component\bsp_psensor_key\module.mk, the user can include this module.mk on Makefile to enable this feature.
                    AIR_BSP_INEAR_ENABLE: This option configures whether airo_key supports powerkey. Defined in mcu\driver\board\component\bsp_inear\module.mk, the user can include this module.mk on Makefile to enable this feature.
Notice:         None

Relative doc:   None

Example project:None

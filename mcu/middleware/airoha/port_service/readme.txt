Port service module usage guide

Brief:          This module is the port service implementation. It bases on different serial device ports and offers user with unified interface to use.
Usage:          GCC:  Please add "AIR_USB_ENABLE = y" in feature.mk under the project GCC folder if USB support is needed.
                      Please add "APP_FILES += $(APP_PATH_SRC)/at_command_serial_port.c" and wrap them with "ifeq ($(MTK_PORT_SERVICE_ENABLE), y)" and "endif" in Makefile under the project GCC folder if AT command for port service is used.
                      Please add "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/port_service/module.mk" and wrap them with "ifeq ($(MTK_PORT_SERVICE_ENABLE), y)" and "endif" in Makefile under the project GCC folder.
                      Please add "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/usb/module.mk" and wrap them with "ifeq ($(AIR_USB_ENABLE), y)" and "endif" in Makefile under the project GCC folder.
Dependency:     Please define HAL_UART_MODULE_ENABLED in hal_feature_config.h under the project inc folder.
Notice:         None.
Relative doc:   None.
Example project:none.

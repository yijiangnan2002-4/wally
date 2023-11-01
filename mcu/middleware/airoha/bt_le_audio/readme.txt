LE Audio module usage guide
Brief:           This module is for middleware LE Audio implementation.
Usage:           GCC:  For LE Audio,
                       1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/module.mk
Dependency:      This module depends on Bluetooth LE Audio. Please set AIR_LE_AUDIO_ENABLE to be "y" in feature.mk and include Bluetooth module.mk in your project.
Notice:          N/A
Relative doc:    N/A
Example project: earbuds_ref_design
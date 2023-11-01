Audio Loopback Test module usage guide

Brief:          This module is the Audio Loopback Test implementation.

Usage:          GCC:  For Audio Loopback Test, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspalg/audio_loopback_test/module.mk
                      2) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspalg/audio_loopback_test/inc
                      3) Add FUNC_AUDIO_LOOPBACK_TEST into the feature table in dsp_sdk.c to apply
                         audio loopback test in the specified scenario,
                         like stream_feature_list_audio_loopback_test[].

Dependency:     None

Notice:         None

Relative doc:   None

Example project:None

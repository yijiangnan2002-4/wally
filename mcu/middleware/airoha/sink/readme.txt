BT sink module usage guide

Brief:          This module is the sink service which integrates HFP, A2DP, AVRCP and PBAPC profiles.
                It works as a Bluetooth headset and supports the headset features, such as, answering or rejecting incoming call,
                getting contact name of the incoming call, playing and pausing music, moving to previous song and next song, and
                connection reestablishing when power on or link lost.
Usage:          GCC: Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_sink/module.mk" in your GCC project makefile.
Dependency:     This module has the dependency with Bluetooth. Please also make sure to include Bluetooth.
Relative doc:   None.
Example project: Please find the earbuds_ref_design/headset_ref_design/speaker_ref_design project under the project folder.
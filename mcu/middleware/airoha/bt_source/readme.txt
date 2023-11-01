BT source module usage guide

Brief:          This module is the source service which integrates the HFP, A2DP, AVRCP profiles.
                It works as a Bluetooth dongle and supports the source features, such as,
                1. Transmitting call and music audio streams.
                2. Notifying call and music status change and accepting commands sent from sink, such as play and pause music, skip to the previous or next song.
                3. Triggering reconnection when powering on or when the link is lost.
Usage:          GCC: Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_source/module.mk" in your GCC project makefile.
Dependency:     This module has the dependency with Bluetooth. Please also make sure to include Bluetooth.
Relative doc:   None.
Example project: Please find the dongle_ref_design project under the project folder.
Bluetooth fast pair 3.2 module usage guide

Brief:           This module is used to provide google fast pair 3.2 feature. User can call API or send action provide by
                 this module to fast pair 3.1 feature.
Usage:           GCC:
                      1) Add the following module.mk for include path and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_fast_pair/module.mk
                         in your GCC project Makefile.
                      2) Set option AIR_BT_FAST_PAIR_ENABLE to y to eanble fast pair 3.1.
                      3) Set option AIR_BT_FAST_PAIR_ENABLE and AIR_BT_FAST_PAIR_SASS_ENABLE to y to enable fast pair 3.2 (fast pair 3.1 and Audio Switch).

Dependency:      None.
Notice:          None.
Relative doc:    None.
Example project: Please find the project earbuds_ref_design under the project folder.

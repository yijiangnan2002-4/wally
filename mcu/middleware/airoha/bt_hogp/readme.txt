bt hogp module usage guide

Brief:          This HID over GATT profile defines how a device with bluetooth low energy wireless communications can support HID services over the bluetooth low energy protocol stack using the GATT.
                This profile defines three roles: HID device, boot host, and report host. Use of the term HID host refers to both host roles: Boot Host, and Report Host. A report host is required to
                support a HID Parser and be able to handle arbitrary formats for data transfers (known as Reports) whereas a Boot Host is not required to support a
                HID parser as all data transfers (reports) for boot protocol mode are of predefined length and format.
Usage:          GCC: 1. Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_hogp/module.mk" in the GCC project Makefile
Dependency: None
Notice:         None.
Relative doc:   None.
Example project:Please find the project earbuds_ref_design or headset_ref_design under the project folder.
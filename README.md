# README #

This is the shared source code between Eastech and Audeara for the Hugo Earbuds Project

### About this Repository ###

* This repository is bases on the Aroha source code for the AB1585H IoT_SDK_for_BT_Audio_V3.10.0.AB158x_exe_V3.10.0.AB158x.7z

### The script files.

The Script files in the root directory are designed to automate operations

** build.sh **
This one is the default by Airoha.

### Build Command ###
** the build comman used for building Hugo earbuds
./build.sh ab1585h_evk earbuds_ref_design_hearing_aid

### Configuration to build this ###

* The Project does not have to be checked out inside the MSYS directory. The direcotry is mapped to a h:\ upon build
* Ensure you are not using windows mapped drive H already. (Or else it might not end well)

###  Windows Environment Variables###

* Set a windows environment variable MSYS_LOCATION to the Installation location of your MSYS in my case c:\Airoha\msys64
  This folder should have the file mingw64.ex and msys2.exe inside it.

### GPIO Configuration ###

* The GPIO tool uses a file in the "gpio settings from pinmux tool" directory and exports some .c and .h files these files are then put into the source code tree.
To use this tool run the executable program ept.exe which is the Easy Pinmux tool.

### NVM Configuration ###

* The GPIO tool uses a file in the "nv key settings from config tool" directory and exports an XML file. 
To use this tool run the Airoha ATK tool and select Config tool AB1585H

### Eclipse Build (Optional) ###

* If you use the Audeara branch the files can be edited and debugged in Eclipse.
* After opening the project in eclipse (With CDT Installed) you can change the build configuration and build the headset alone or the entire project

### Dependencies ###

* MSys. A linux subsystem for windows.
* Airoha ADK build tools.
* Cadence/ Xtensa tools. This is installed as part of the Airoha setup.







﻿
Airoha IoT Development Platform provides a comprehensive software solution for devices based
on the chips.

Chips of each product line are shown as below
    • Airoha IoT SDK for Smart MCU: AM255x
    • Airoha IoT SDK for BT Audio: AB155x/AB1565/AB1568/AB157x/AB158x/AB1627

1. Getting started
   The SDK package supports the GCC tool chain. Follow the instructions at <sdk_root>/doc/Airoha_IoT_SDK_for_<product_line>_Get_Started_Guide.pdf
   to build your first project and run it on the EVK or EVB. Release notes are also under the <sdk_root>/doc folder.

2. Folder structure
  \config                   ->  Includes make and compile configuration files for compiling a binary project.
  \doc                      ->  Includes SDK related documentation, such as developer and SDK API reference guides.
  \doc\<chip_name>\board    ->  EVK documents, including user guide, layout information, schematics, daughterboard information,
                            ->  BOM list. The EVK relative application notes are also in this folder, such as power measurement guide.
                            ->  If you cannot find these documents under the <sdk_root>/doc folder, you can find them on the DCC page of the MOL website.
  \driver                   ->  Includes common driver files, such as board drivers, peripheral and CMSIS-CORE interface drivers.
  \kernel                   ->  Includes the underlying RTOS and system services for exception handling and error logging.
  \middleware\airoha        ->  Airoha middleware. Read readme.txt in each module for details.                           
  \middleware\third_party   ->  Open source software, read readme.txt in each module for details.
  \prebuilt                 ->  Contains binary files, libraries, header files, makefiles and other pre-built files.
  \project\<board>          ->  Example projects of the current SDK. Read <sdk_root>/project/readme.txt for more details.
  \tools                    ->  Script, generation script, gcc compiler. If there isn't a gcc compiler under the /tools/gcc folder,
                            ->  extract the tool package to the root folder of the SDK with the following command.
                            ->  7za x SDK_VX.Y.Z_tool_chain.7z -o<sdk_root>
                            **  Please make sure you have /tools/gcc before you build the SDK under linux environment. **

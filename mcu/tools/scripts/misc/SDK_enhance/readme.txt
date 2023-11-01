[convert_name.sh]
* Background & Purpose
  For some major changes to the SDK version 3.0.0.AB1585_AB1588, we provide a script to help reduce the effort of 
  applying new NVkey name/ID value and replacing file content/folder name/file name from MTK to Airoha. 
  We recommend running the script before starting to merge code from SDK version 2.x.x to version 3.0.0.AB1585_AB1588.
  <Note> Please do not use this script for other purposes.

* Introduction
  convert_name.sh will run three script in config folder, and provide default setting in the config folder.
  For the detail description and setting of each script, please reference the description below.
  After run convert_name.sh in codebase, the codebase will be replaced:
  1. the file content, folder and file name from MTK to airoha.
  2. all the .c and .h file will be replaced with new nvkey name.
  3. all the nvkey.xml file will be replaced with new nvkey id value.

* Usage
  1. At SDK root execute below command: 
  cd mcu/tools/scripts/misc/SDK_enhance&&./convert_name.sh go

* Notice
  1. Make sure you do a backup of the codebase before running the script. 

* Troubleshooting
  1. If can't execute the script as command: "./convert_name.sh" , please execute "chmod 777 convert_name.sh".
  

*************************************************************************
[airoha_rename.sh]
* Introduction
  After run airoha_rename.sh in codebase, the codebase will be replaced the file content, folder and file name from MTK to airoha.The detail list of changing will be described in main_change_list.xlsx.

* Usage
  1. Place airoha_rename.sh into codebase root.
  2. ./airoha_rename.sh

* Notice
  1. Make sure you do a backup of the codebase before running the airoha_rename.sh. 


* Troubleshooting
  1. If can't execute the script as command: "./airoha_rename.sh" , please execute "chmod 777 airoha_rename.sh".
  
  
[rename.sh]
* Introduction
  After run rename.sh in codebase, all the .c and .h file will be replaced by following the rule:
  change name in column A to column B which define in NVkey_mapping_list.csv.

* Usage
  1. Place rename.sh into codebase root, and remind that please put NVkey_mapping_list.csv in <SDK root>/mcu.
  2. ./rename.sh mcu/NVkey_mapping_list.csv

* Troubleshooting
  1. sort Z->A in column A in NVkey_mapping_list.csv can avoid unexcepted change while using rename.sh

  EX: NVKEY_NAME_1¡BNVKEY_NAME_1_old_name.
  Assume that we want to change:
  NVKEY_NAME_1->NVID_NAME_2 
  & 
  NVKEY_NAME_1_xxx_old_name->NVID_NAME_3

  if don't sort Z->A in csv A column first, 
  then while run the script to change, 
  in this case, 
  the NVKEY_NAME_1_xxx_old_name in SDK will be change to NVID_NAME_2_xxx_old_name,
  and because the "NVID_NAME_2_xxx_old_name" is not define in csv A column,
  we can't get the result:   NVKEY_NAME_1_xxx_old_name->NVID_NAME_3.
  2. Execute "dos2unix NVkey_mapping_list.csv" to make sure the csv ending line is unix style.
  3. Check the value in NVkey_mapping_list.csv with vim, make sure the value in NVkey_mapping_list.csv had no space charater.
  4. If can't execute the rename.sh as command: "./rename.sh" , please execute "chmod 777 rename.sh".
  5. If you want to use other csv as config, please make sure the csv format follow the rule in this readme.
     Usage: ./rename.sh <your csv path>


[id_change.sh]
* Introduction
  After run id_change.sh in codebase, all the nvkey.xml file will be replaced by following the rule:
  change name in column A to column B which define in nvkey_id_list.csv.

* Usage
  1. Place id_change.sh into codebase root, and remind that please put nvkey_id_list.csv in <SDK root>/mcu.
  2. ./id_change.sh mcu/nvkey_id_list.csv

* Troubleshooting
  1. sort Z->A in column A in nvkey_id_list.csv can avoid unexcepted change while using id_change.sh

  EX: NVKEY_ID_1¡BNVKEY_ID_1_old_name.
  Assume that we want to change:
  NVKEY_ID_1->NVID_2 
  & 
  NVKEY_ID_1_old_name->NVID_3

  if don't sort Z->A in csv A column first, 
  then while run the script to change, 
  in this case, 
  the NVKEY_ID_1_old_name in SDK will be change to NVID_2_xxx_old_name,
  and because the "NVID_2_xxx_old_name" is not define in csv A column,
  we can't get the result:   NVKEY_NAME_1_xxx_old_name->NVID_NAME_3.
  2. Execute "dos2unix nvkey_id_list.csv" to make sure the csv ending line is unix style.
  3. Check the value in nvkey_id_list.csv with vim, make sure the value in nvkey_id_list.csv had no space charater.
  4. If can't execute the id_change.sh as command: "./id_change.sh" , please execute "chmod 777 id_change.sh".
  5. If you want to use other csv as config, please make sure the csv format follow the rule in this readme.
     Usage: ./id_change.sh <your csv path>
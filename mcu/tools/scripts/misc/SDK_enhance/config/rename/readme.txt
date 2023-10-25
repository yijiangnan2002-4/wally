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

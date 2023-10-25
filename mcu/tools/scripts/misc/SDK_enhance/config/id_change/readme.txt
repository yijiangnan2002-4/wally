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

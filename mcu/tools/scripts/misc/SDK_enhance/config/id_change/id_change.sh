#!/bin/bash
set -x
TMP_FILE="id_tmp_file_list"
GREP_TMP="id_grep_tmp"
CONFIG=${1}
function change_string {
IFS=","
find mcu/project -type f -name "nvkey.xml" > ${TMP_FILE}
find mcu/config/chip -type f -name "nvkey.xml" >> ${TMP_FILE}


while read Nvkey_id_Name Name_in_Next EMPTy
do             
        cat ${TMP_FILE}|xargs grep -rl "${Nvkey_id_Name}" > ${GREP_TMP}
	cat ${GREP_TMP}|xargs sed -i "s/${Nvkey_id_Name}/${Name_in_Next}/g"				
done < <(tail -n +2 $1)

rm -f ${TMP_FILE} ${GREP_TMP}
}

dos2unix ${CONFIG}
change_string ${CONFIG}


#usage: ./<script_name> <config csv path>




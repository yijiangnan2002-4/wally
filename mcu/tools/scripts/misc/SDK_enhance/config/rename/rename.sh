#!/bin/bash
set -x
TMP_FILE="tmp_file_list"
GREP_TMP="grep_tmp"
CONFIG=${1}
function change_string {
IFS=","
find .  -type f -name "*.h" > ${TMP_FILE}
find .  -type f -name "*.c" >> ${TMP_FILE}


while read Nvkey_ID_Name Name_in_Next_Chip EMPTY
do             
        cat ${TMP_FILE}|xargs grep -rl ${Nvkey_ID_Name} > ${GREP_TMP}
	cat ${GREP_TMP} |xargs sed -i "s/${Nvkey_ID_Name}/${Name_in_Next_Chip}/g"				
done < <(tail -n +2 $1)

rm -f ${TMP_FILE} ${GREP_TMP}
}

dos2unix ${CONFIG}
change_string ${CONFIG}


#usage: ./<script_name> <config csv path>




#!/bin/bash

if [ "$1" == "go" ]; then
    echo "Start convert name......"
	now_PWD=`pwd`
	echo $now_PWD
        SDKROOT="$now_PWD/../../../../../."
	cp $now_PWD/config/mtk_rename_to_airoha/airoha_rename.sh ${SDKROOT}&&cd ${SDKROOT}&&chmod 777 airoha_rename.sh&&./airoha_rename.sh
	cp $now_PWD/config/id_change/id_change.sh ${SDKROOT}&&cd ${SDKROOT}&&chmod 777 id_change.sh&&./id_change.sh mcu/tools/scripts/migration/config/id_change/nvkey_id_list.csv
	cp $now_PWD/config/rename/rename.sh ${SDKROOT}&&cd ${SDKROOT}&&chmod 777 rename.sh&&./rename.sh mcu/tools/scripts/migration/config/rename/NVkey_mapping_list.csv
	cd ${SDKROOT}&&rm -f rename.sh id_change.sh airoha_rename.sh
    echo "finished"
    exit
else
	echo "Wrong usage!"
	echo "Usage ./${0##*/} go"
fi

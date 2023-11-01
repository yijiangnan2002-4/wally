#!/bin/bash

map_dir=$1
out_dir=$2


# script_usage    $1
# script_usage    output
function script_usage(){
    echo '##################################################' >> $1
    echo 'Usage: ./general_fp.sh <map_dir> <out_dir>' >> $1
    echo 'Example: ./general_fp.sh ./out ./footprint' >> $1
    echo '         ( Parse all map files in the out directory.
           Put the results in the footprint directory. )' >> $1
    echo 'NOTE: All parameters do not include trailing path splitting symbol.' >> $1
    echo '##################################################' >> $1
    exit -1
}


# find_memory_type    $1
# find_memory_type    map_file
function find_memory_type(){
    for line in `cat $1 | grep limit | grep __ | grep -v free | grep -v limits | grep -v rom_pt`; do
        # echo $line
        reg_mem='__(.*)_limit'
        [[ "$line" =~ $reg_mem ]]
        for result in $BASH_REMATCH; do
            echo $result
        done
    done
}

# find_memory_type    $1          $2
# find_memory_type    map_file    symbol
function get_value(){
    # rm -f *.txt
    # cat $1 | grep $2 > 1.txt
    # cat $1 | grep -w $2 | sed 's/^ *//g' > 2.txt
    # cat $1 | grep $2 | sed 's/^ *//g' | cut -f1 -d' ' > 3.txt
    cat $1 | grep -w $2 | sed 's/^ *//g' | head -n 1 | cut -f1 -d' '
}

# rm_leading_zeros    $1
# rm_leading_zeros    string
function rm_leading_zeros(){
    # printf "%x\n" $((16#$1))
    # echo $1
    reg_hex='[^0x]+[0-9A-Fa-f]*'
    [[ "$1" =~ $reg_hex ]]
    if [ ${BASH_REMATCH[0]} ]; then
        echo '0x'${BASH_REMATCH[0]}
    else
        echo '0x0'
    fi
}


if [ $# != 2 ] || [ ! -d $map_dir ];then
    script_usage /dev/stdin
fi

if [ ! -d $out_dir ]; then
    mkdir -p $out_dir
fi

# csv file
# Item,Total,Used,Free
# IRAM,xxxx,xxxx,xxxx
# DRAM,xxxx,xxxx,xxxx

map_file_num=$(find $map_dir -name "*.map" -type f | wc -l)
if [ $map_file_num == 0 ]; then
    echo "=============================================================="
    echo "Map directory:" $map_dir
    echo "Ouput directory:" $out_dir
    echo "No map files were found and footprint information could not be obtained."
    echo "=============================================================="
    exit 0
fi
for map in $map_dir/*.map; do
    echo 'processing' `basename $map` 'file...'
    tmp_map=$out_dir/tmp.map
    rm -f $tmp_map
    touch $tmp_map
    cat $map | grep used | grep __ | cut -f1 -d'=' >> $tmp_map
    cat $map | grep free | grep __ | cut -f1 -d'=' >> $tmp_map
    cat $map | grep limit | grep __ | cut -f1 -d'=' >> $tmp_map
    res_csv=$out_dir/`basename $map`.csv
    rm -f $res_csv
    #echo 'Item,Total,Used,Free' >> $res_csv
    printf "%-16s, %-24s, %-24s, %-24s\n" Item Total Used Free >> $res_csv
    for mem in `find_memory_type $tmp_map`; do
        # echo $mem
        sym_limit=$mem
        sym_used=`echo $mem | sed 's/limit/used/g'`
        sym_free=`echo $mem | sed 's/limit/free/g'`
        # echo $sym_limit $sym_used $sym_free

        limit=`get_value $tmp_map $sym_limit`
        # echo 'limit:' $limit
        used=`get_value $tmp_map $sym_used`
        # echo 'used:' $used
        free=`get_value $tmp_map $sym_free`
        # echo 'free:' $free
        mem_type=`echo $mem | sed 's/_limit//g' | sed 's/__//g' | sed 's/rom_rtos/mcu_flash/g' | sed 's/rom_dsp0/dsp0_flash/g' | sed 's/sysram_dsp0/dsp0_sysram/g'`
        #echo "$mem_type,$limit,$used,$free" >> $res_csv
    printf "%-16s, %-24d, %-24d, %-24d\n" $mem_type $limit $used $free >> $res_csv
    done
    rm -f $tmp_map
done

echo "=============================================================="
echo "Map directory:" $map_dir
echo "Ouput directory:" $out_dir
echo "All the footprint information obtained from the map file has been generated."
echo "=============================================================="

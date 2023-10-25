#!/bin/bash

# ./rtos_size_overflow.sh project/ab157x/apps/earbuds_ref_design/config_bin/ab1571_evb/filesystem.bin  project/ab157x/apps/earbuds_ref_design/GCC/ab1577_flash_no_lm.ld

root="../../.."
filesystem_bin=${root}/$1
linkfile=${root}/$2


echo "filesystem_bin : $filesystem_bin"
echo "linkfile : $linkfile"

# linkfile
linkfile_size=`cat $linkfile |grep "ROM_ROFS(rx)"|cut -d"," -f2|cut -d" " -f4`
echo "ROM_ROFS(rx) size in linkfile: $linkfile_size"

# skip case: 0K  and nothing found
if [ -z $linkfile_size ]; then
    echo "SKIP: no ROM_ROFS in file $linkfile"
    exit 0
fi

if [ $linkfile_size == "0K" ]; then
    echo "SKIP: ROM_ROFS is 0K"
    exit 0
fi


linkfile_size=`echo $linkfile_size | tr -dc '0-9'`
linkfile_size_value=${linkfile_size}*1024
let linkfile_size_value=${linkfile_size}*1024
echo "ROM_ROFS(rx) value : $linkfile_size_value"

if [ ! -f $linkfile ]; then
    echo "Error: File $linkfile not exist!"
    exit 1
fi

# systembin
filesystem_bin_size=`ls -l $filesystem_bin |cut -d" " -f5`
echo "filesystem_bin_value : $filesystem_bin_size"

if [ ! -f $filesystem_bin ]; then
    echo "Error: File $filesystem_bin not exist!"
    exit 1
fi


# compare
if [ $linkfile_size_value -lt $filesystem_bin_size ]; then
    echo "Error: ROM_ROFS(rx) value is less than filesystem_bin_value"
    exit 1
else
    echo "ROM_ROFS(rx) value is greater than or equal to filesystem_bin_value."
    exit 0
fi

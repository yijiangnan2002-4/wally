#!/bin/bash

# This script will start the compilation of mcu/dsp according to the configuration file (mapping_proj.cfg).
# Build mcu would bring bl option by default.

###############################################################################
# Variables
OUT="${PWD}/out"
working_dir="${PWD}"
debug="0"
feature_mk=""
feature_mk_dsp0=""
feature_mk_dsp1=""
default_dsp_bin_switch="off"
compatibleTest="off"
variableCompare="off"
pwdLength=""
pwd_folder_length_limit=23
out_folder_length_limit=65
subst_disk=
mapping_disk_file="mapping_disk"
current_subst_info_file="current_subst_info"
co_build_command=$(echo "$0 $@")

# Reset
Color_Off='\033[0m'       # Text Reset

# Regular Colors
Black='\033[0;30m'        # Black
Red='\033[0;31m'          # Red
Green='\033[0;32m'        # Green
Yellow='\033[0;33m'       # Yellow
Blue='\033[0;34m'         # Blue
Purple='\033[0;35m'       # Purple
Cyan='\033[0;36m'         # Cyan
White='\033[0;37m'        # White

umount_subst_disk () {
    subst ${subst_disk}: //D
    echo "Disk ${subst_disk}: mapping clear"
}

subst_disk () {
    if [ -f ${mapping_disk_file} ]; then
        subst > $current_subst_info_file
        subst_disk_origin=$(head -1 ${mapping_disk_file} | tr -d '\n' | tr -d '\r')
        subst_disk=${subst_disk_origin^^}
        working_dir_disk_origin=$(echo ${working_dir:1:2} | tr -d '\n' | tr -d '\r' | tr -d '\/')
        working_dir_disk=${working_dir_disk_origin^^}
        subst_dir=$(tail -1 ${mapping_disk_file} | tr -d '\n' | tr -d '\r' )
        subst_dir_without_slash=$(tail -1 ${mapping_disk_file} | tr -d '\/' | tr -d '\n' | tr -d '\r' )
        echo "Mapping sdk folder to disk ${subst_disk}:"
        echo "\$subst_disk : ${subst_disk}"
        echo "\$subst_dir : ${subst_dir}"
        echo "\$subst_dir_without_slash : ${subst_dir_without_slash}"
        echo "\$working_dir_disk : ${working_dir_disk}"

        # codebase folder name changed , in the same partition or not
        if [ "${subst_disk}" != "${working_dir_disk}" ]; then
            # umount susbst when folder name changed
            if [[ "$working_dir" != "${subst_dir}" ]]; then
                echo "${subst_disk}" > "${mapping_disk_file}"
                echo "${working_dir}" >> "${mapping_disk_file}"
                umount_subst_disk
            fi
            # use the same subst as before
            subst ${subst_disk}: . 2>&1 > /dev/null
            working_dir=${subst_disk}:
        fi
        if [ -f ${current_subst_info_file} ]; then
            # check subst_disk used by another path or not
            while read line; do
                # reading each line
                #echo $line
                # split by space, [0]:disk  [1]: =>  [2]:  path
                stringarray=$line
                current_subst_disk_origin=$(echo $line | cut -d' ' -f1)
                current_subst_disk=${current_subst_disk_origin}
                current_dir=$(echo $line | cut -d' ' -f3 | tr -d ':' | tr -d '\n' | tr -d '\r')
                # disk used
                if [[ "${current_subst_disk:0:1}" == ${subst_disk^^} ]]; then
                    # mount path is different
                    if [[ "${current_dir,,}" != ${subst_dir_without_slash,,} ]]; then
                        echo "\$stringarray : ${stringarray}"
                        echo "\$current_subst_disk : ${current_subst_disk:0:1}"
                        echo "\$current_dir : ${current_dir}"
                        echo "\$subst_dir_without_slash : ${subst_dir_without_slash}"
                        working_dir="${PWD}"
                        subst_disk_dynamically
                        #echo "run function subst_disk_dynamically"
                        break
                    fi
                fi
            done < ${current_subst_info_file}
            rm $current_subst_info_file
        fi
        
    else
        subst_disk_dynamically
    fi
}

subst_disk_dynamically () {
    # dynamic
    for i in {H..Z}
    do
        subst $i: . 2>&1 > /dev/null
        if [[ $? -eq 0 ]]; then
            echo "$i" > "${mapping_disk_file}"
            echo "${working_dir}" >> "${mapping_disk_file}"
            echo "Mapping sdk folder to disk $i:"
            working_dir=$i:
            break
        fi
    done
}

platform=$(uname)
if [[ "${platform}" =~ "MINGW" || "${platform}" =~ "MSYS" ]]; then
    # not include forward slash
    pwdLength=$(echo ${PWD} | wc -m)
    if [[ "${working_dir}" =~ "/home" ]]; then
	    working_dir_abs=$(pwd -W)
	    working_dir_abs_disk=${working_dir_abs:0:1}
	    working_dir="/${working_dir_abs_disk,,}${working_dir_abs:2}"
	    echo working_dir in msys : ${working_dir}
    fi
    #  the last arugment : ${!#}
    if [ "${!#}" != "clean" ]; then
    subst_disk
    fi
elif [[ "${platform}" =~ "Darwin" ]]; then
    export PATH="/usr/local/bin:${PATH}"
fi

# check /bin/bash
if [ -f /bin/bash ]; then
    export SHELL="/bin/bash"
fi

#source mapping_proj.cfg

if [ -f mcu/tools/mapping_proj/mapping_proj.cfg ]; then
    source mcu/tools/mapping_proj/mapping_proj.cfg
fi

for d in $(cd mcu/tools/mapping_proj/; find . -name mapping_proj.cfg);
do
    echo "d : ${d:2}"
    if [ -f mcu/tools/mapping_proj/${d:2} ]; then
        source mcu/tools/mapping_proj/${d:2}
    fi
done

show_available_proj () {
    echo "==============================================================="
    echo "Available Build Projects:"
    echo "==============================================================="
    for var in ${!map_@}
    do
        board=$(echo ${var} | sed 's/__/:/g' | cut -d':' -f2)
        project=$(echo ${var} | sed 's/__/:/g' | cut -d':' -f3)
        echo "${board}"
        echo "  ${project}"
        eval proj_map_key=( \${!$var[@]} )
        eval proj_map_value=( \${$var[@]} )
        for key in ${proj_map_key[@]}
        do
            displayName=""
            case ${key} in
                0)
                 displayName="board_share"
                    ;;
                1)
                 displayName="mcu"
                    ;;
                2)
                 displayName="dsp0"
                    ;;
                3)
                 displayName="dsp1"
                    ;;
                4)
                 displayName="fm"
                    ;;
                5)
                 displayName="fd0"
                    ;;
                6)
                 displayName="fd1"
                    ;;
                7)
                 displayName="proj_bl"
                    ;;
                8)
                 displayName="f_bl"
                    ;;
            esac
            eval echo \"\ \ \ \ \ \ ${displayName}: \${${var}[\"${key}\"]}\"
        done
    done
}

clean_out () {
    rm -rf "${1}"
    echo "rm -rf ${1}"
}

check_folder_length () {
    #echo -e "\nout_folder_length_limit=$out_folder_length_limit"
    pwdLength=${1}
    out_folder_length=$(echo ${2}| wc -m)
    if [[ "${platform}" =~ "MINGW" || "${platform}" =~ "MSYS" ]]; then
        if [ "$(( pwdLength + out_folder_length - 1 ))" -gt "$(( pwd_folder_length_limit + out_folder_length_limit ))" ]; then

            echo -e "\n Error: over maximum path length"
            if [ "$(( pwdLength - 1 ))" -gt "${pwd_folder_length_limit}" ]; then
                echo -e "    1. Please install SDK in a path less than ${pwd_folder_length_limit} characters"
            fi

            if [ "${out_folder_length}" -gt "${out_folder_length_limit}" ]; then
                echo -e "    2. Please shorten the name of project or feature."
            fi

            exit 1
        else
            return 0
        fi
    fi
    return 0
}

# copy cm4 files
copy_cm4_files () {
    debug_dir=""
    download_dir=""
    if [  ${debug} -eq "1" ]; then
        echo "1 : ${1}"
        echo "2 : ${2}"
        echo "3 : ${3}"
        echo "4 : ${4}"
        echo "5 : ${5}"
    fi
    if [ "${4}" == "none" ]; then
        debug_dir=out/${1}/${3}/debug
        download_dir=out/${1}/${3}/download
    else
        debug_dir=out/${4}/${3}/debug
        download_dir=out/${4}/${3}/download
    fi
    if [ ! -e "${debug_dir}" ]; then
        mkdir -p "${debug_dir}"
    fi
    if [ ! -e "${download_dir}" ]; then
        mkdir -p "${download_dir}"
    fi
    # copy debug files if exist
    echo "cp -f mcu/out/${1}/${2}/${5}/${2}.elf  ${debug_dir}"
    echo "cp -f mcu/out/${1}/${2}/${5}/${2}.dis  ${debug_dir}"
    echo "cp -f mcu/out/${1}/${2}/${5}/${2}.map  ${debug_dir}"
    echo "cp -f mcu/out/${1}/${2}/${5}/cm4_log_str.bin  ${debug_dir}"
    echo "cp -f mcu/out/${1}/${2}/${5}/mcu_log_str.bin  ${debug_dir}"
    test -e "mcu/out/${1}/${2}/${5}/${2}.elf" && cp -f "mcu/out/${1}/${2}/${5}/${2}.elf"  "${debug_dir}"
    test -e "mcu/out/${1}/${2}/${5}/${2}.dis" && cp -f "mcu/out/${1}/${2}/${5}/${2}.dis"  "${debug_dir}"
    test -e "mcu/out/${1}/${2}/${5}/${2}.map" && cp -f "mcu/out/${1}/${2}/${5}/${2}.map"  "${debug_dir}"
    test -e "mcu/out/${1}/${2}/${5}/cm4_log_str.bin" && cp -f "mcu/out/${1}/${2}/${5}/cm4_log_str.bin"  "${debug_dir}"
    test -e "mcu/out/${1}/${2}/${5}/mcu_log_str.bin" && cp -f "mcu/out/${1}/${2}/${5}/mcu_log_str.bin"  "${debug_dir}"

    # copy download files if exist
    echo "cp -f mcu/out/$1/$2/$5/flash_download.cfg $download_dir"
    echo "cp -f mcu/out/$1/$2/$5/*bootloader.bin $download_dir"
    echo "cp -f mcu/out/$1/$2/$5/$2.bin $download_dir"
    echo "cp -f mcu/out/$1/$2/$5/partition_table.bin $download_dir"
    echo "cp -f mcu/out/$1/$2/$5/*_patch_hdr.bin $download_dir"
    echo "cp -f mcu/out/$1/$2/$5/nvdm.bin $download_dir"
    echo "cp -f mcu/out/$1/$2/$5/nvdm_ou.bin $download_dir"
    echo "cp -f mcu/out/$1/$2/$5/filesystem.bin $download_dir"
    echo "cp -f mcu/out/$1/$2/$5/secure_no_rtos_demo.bin $download_dir"
    test -e "mcu/out/$1/$2/$5/flash_download.cfg" && cp -f "mcu/out/$1/$2/$5/flash_download.cfg" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/"*bootloader.bin && cp -f "mcu/out/$1/$2/$5/"*bootloader.bin "${download_dir}"
    test -e "mcu/out/$1/$2/$5/$2.bin" && cp -f "mcu/out/$1/$2/$5/$2.bin" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/partition_table.bin" && cp -f "mcu/out/$1/$2/$5/partition_table.bin" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/"*_patch_hdr.bin && cp -f "mcu/out/$1/$2/$5/"*_patch_hdr.bin "${download_dir}"
    test -e "mcu/out/$1/$2/$5/nvdm.bin" && cp -f "mcu/out/$1/$2/$5/nvdm.bin" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/filesystem.bin" && cp -f "mcu/out/$1/$2/$5/filesystem.bin" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/Hey_Google.bin" && cp -f "mcu/out/$1/$2/$5/Hey_Google.bin" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/Hey_Siri.bin" && cp -f "mcu/out/$1/$2/$5/Hey_Siri.bin" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/nvdm_ou.bin" && cp -f "mcu/out/$1/$2/$5/nvdm_ou.bin" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/filesystem.bin" && cp -f "mcu/out/$1/$2/$5/"filesystem.bin "${download_dir}"
    test -e "mcu/out/$1/$2/$5/secure_no_rtos_demo.bin" && cp -f "mcu/out/$1/$2/$5/"secure_no_rtos_demo.bin "${download_dir}"
    test -e "mcu/out/$1/$2/$5/nvkey.xml" && cp -f "mcu/out/$1/$2/$5/nvkey.xml" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/nvkey_chip.xml" && cp -f "mcu/out/$1/$2/$5/nvkey_chip.xml" "${download_dir}"

    # copy ANC FW if exist
    echo "cp -f mcu/out/$1/$2/$5/ANC_Image.bin ${download_dir}"
    echo "cp -f mcu/out/$1/$2/$5/TurboCal_Image.bin ${download_dir}"
    echo "cp -f mcu/out/$1/$2/$5/ANC_Tune_Image.bin ${download_dir}"
    test -e "mcu/out/$1/$2/$5/ANC_Image.bin" && cp -f "mcu/out/$1/$2/$5/ANC_Image.bin" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/TurboCal_Image.bin" && cp -f "mcu/out/$1/$2/$5/TurboCal_Image.bin" "${download_dir}"
    test -e "mcu/out/$1/$2/$5/ANC_Tune_Image.bin" && cp -f "mcu/out/$1/$2/$5/ANC_Tune_Image.bin" "${download_dir}"

    # Copy the language model binary to the output folder.
    # For gva and ama are in different partition
    if [ -e "mcu/out/${1}/${2}/${5}/lm_gva.bin" ] ; then
        echo "cp -f mcu/out/${1}/${2}/${5}/lm_gva.bin ${download_dir}"
        cp -f "mcu/out/${1}/${2}/${5}/lm_gva.bin" "${download_dir}"
    fi
    if [ -e "mcu/out/${1}/${2}/${5}/lm_ama.bin" ] ; then
        echo "cp -f mcu/out/${1}/${2}/${5}/lm_ama.bin ${download_dir}"
        cp -f "mcu/out/${1}/${2}/${5}/lm_ama.bin" "${download_dir}"
    fi
    # For gva and ama are in the same partition
    if [ -e "mcu/out/${1}/${2}/${5}/lm.bin" ] ; then
        echo "cp -f mcu/out/${1}/${2}/${5}/lm.bin ${download_dir}"
        cp -f "mcu/out/${1}/${2}/${5}/lm.bin" "${download_dir}"
    fi
}

# copy dsp files
copy_dsp_files () {
    if [  ${debug} -eq "1" ]; then
        echo "1 : ${1}"
        echo "2 : ${2}"
        echo "3 : ${3}"
        echo "4 : ${4}"
        echo "5 : ${5}"
        echo "6 : ${6}"
    fi
    debug_dir=""
    download_dir=""
    if [ "${4}" == "none" ]; then
        debug_dir="out/${1}/${3}/debug"
        download_dir="out/${1}/${3}/download"
    else
        debug_dir="out/${4}/${3}/debug"
        download_dir="out/${4}/${3}/download"
    fi
    if [ ! -e "${debug_dir}" ]; then
        mkdir -p "${debug_dir}"
    fi
    if [ ! -e "${download_dir}" ]; then
        mkdir -p "${download_dir}"
    fi

    # copy debug files if exist
    echo "cp -f ${6}/${1}/${2}/${5}/${2}.out  ${debug_dir}"
    echo "cp -f ${6}/${1}/${2}/${5}/${2}.asm  ${debug_dir}"
    echo "cp -f ${6}/${1}/${2}/${5}/${2}.map  ${debug_dir}"
    echo "cp -f ${6}/${1}/${2}/${5}/audio_dump_id.csv  ${debug_dir}"
    echo "cp -f ${6}/${1}/${2}/${5}/dsp*_log_str.bin  ${debug_dir}"
    test -e "${6}/${1}/${2}/${5}/${2}.out" && cp -f "${6}/${1}/${2}/${5}/${2}.out"  "${debug_dir}"
    test -e "${6}/${1}/${2}/${5}/${2}.asm" && cp -f "${6}/${1}/${2}/${5}/${2}.asm"  "${debug_dir}"
    test -e "${6}/${1}/${2}/${5}/${2}.map" && cp -f "${6}/${1}/${2}/${5}/${2}.map"  "${debug_dir}"
    test -e "${6}/${1}/${2}/${5}/audio_dump_id.csv" && cp -f "${6}/${1}/${2}/${5}/audio_dump_id.csv"  "${debug_dir}"
    test -e "${6}/${1}/${2}/${5}/"dsp*_log_str.bin && cp -f "${6}/${1}/${2}/${5}/"dsp*_log_str.bin  "${debug_dir}"

    # copy download files if exist
    echo "cp -f ${6}/${1}/${2}/${5}/${2}.bin ${download_dir}"
    if [ -f "${6}/${1}/${2}/${5}/${2}.bin" ]; then
        cp -f "${6}/${1}/${2}/${5}/${2}.bin" "${download_dir}"
    else
        echo -e "${Red}File ${6}/${1}/${2}/${5}/${2}.bin not exist!${Color_Off}"
    fi

}

# error if variable have different value in mcu/dsp feature file
check_variable_value () {
    mcu_board=${1}
    mcu_project=${2}
    mcu_feature=${3}
    dsp_board=${4}
    dsp_project=${5}
    dsp_feature=${6}
    variableCompareSwitch=${7}
    status="0"

   #echo ${mcu_board}
   #echo ${mcu_project}
   #echo ${mcu_feature}
   #echo ${dsp_board}
   #echo ${dsp_project}
   #echo ${dsp_feature}

    echo "co-build command :  ./build.sh ${fake_board} ${project} "

    # find feature file under apps/templates
    if [ -d "mcu/project/${mcu_board}/apps/${mcu_project}/GCC/" ] ; then
        mcu_feaure_file=$(find "mcu/project/${mcu_board}/apps/${mcu_project}/GCC/" |grep ${mcu_feature})
    elif [ -d "mcu/project/${mcu_board}/templates/${mcu_project}/GCC/" ] ; then
        mcu_feaure_file=$(find "mcu/project/${mcu_board}/templates/${mcu_project}/GCC/" |grep ${mcu_feature})
    elif [ -d "mcu/project/${mcu_board}/test_loads/${mcu_project}/GCC/" ] ; then
        mcu_feaure_file=$(find "mcu/project/${mcu_board}/test_loads/${mcu_project}/GCC/" |grep ${mcu_feature})
    fi

    if [ -d "dsp/project/${dsp_board}/apps/${dsp_project}/XT-XCC/" ] ; then
        dsp_feaure_file=$(find "dsp/project/${dsp_board}/apps/${dsp_project}/XT-XCC/" |grep ${dsp_feature})
    elif [ -d "dsp/project/${dsp_board}/templates/${dsp_project}/XT-XCC/" ] ; then
        dsp_feaure_file=$(find "dsp/project/${dsp_board}/templates/${dsp_project}/XT-XCC/" |grep ${dsp_feature})
    elif [ -d "dsp/project/${dsp_board}/test_loads/${dsp_project}/XT-XCC/" ] ; then
        dsp_feaure_file=$(find "dsp/project/${dsp_board}/test_loads/${dsp_project}/XT-XCC/" |grep ${dsp_feature})
    fi

    echo "mcu feature mk : ${mcu_feaure_file}"
    echo "dsp feature mk : ${dsp_feaure_file}"
    echo ""

    if [ ! -f ${mcu_feaure_file} ] || [ -z ${mcu_feaure_file} ] ; then
        echo Feature file ${mcu_feature} not exist/defined in mcu side!
        echo "Feature option compare status : FAIL"
        echo "./build.sh $fake_board $project -variableCompare" >> .variable_check_fail_cases
        exit 1
    fi

    if [ ! -f ${dsp_feaure_file} ] || [ -z ${dsp_feaure_file} ] ; then
        echo Feature file ${dsp_feature} not exist/defined in dsp side!
        echo "Feature option compare status : FAIL"
        echo "./build.sh $fake_board $project -variableCompare" >> .variable_check_fail_cases
        exit 1
    fi

    # check variable
    make -f mcu/tools/scripts/build/Makefile_get_variable FEATURE=${mcu_feaure_file} |grep "^MTK_\|^AIR_" > .mcu_variable
    make -f mcu/tools/scripts/build/Makefile_get_variable FEATURE=${mcu_feaure_file} VAR_NAME=y |grep "^MTK_\|^AIR_" > .mcu_variable_name
    make -f mcu/tools/scripts/build/Makefile_get_variable FEATURE=${dsp_feaure_file} |grep "^MTK_\|^AIR_" > .dsp_variable
    make -f mcu/tools/scripts/build/Makefile_get_variable FEATURE=${dsp_feaure_file} VAR_NAME=y |grep "^MTK_\|^AIR_" > .dsp_variable_name
    cat .mcu_variable_name .dsp_variable_name > .total_variable

    rm -f .total_variable_count
    while read p; do
      count=$(grep -wc "$p" .total_variable)
      echo "$p : $count" >> .total_variable_count
      if [ ${count} = "2" ] ; then
          #echo "$p  : $count"
          # get mcu value
          mcu_value=$(cat .mcu_variable |grep $p |head -1 |cut -d '=' -f 2)
          # get dsp value
          dsp_value=$(cat .dsp_variable |grep $p |head -1 |cut -d '=' -f 2)
          # compare value
          #echo "mcu_value : $mcu_value"
          #echo "dsp_value : $dsp_value"
          if [ ${mcu_value} != ${dsp_value} ]; then
              status="1"
              #echo "co-build command :  ./build.sh $board $project"
              echo "Variable $p have different value in mcu/dsp side"
              echo "    $p = ${mcu_value}"
              echo "    $p = ${dsp_value}"
              echo ""
          fi
      fi
    done < .mcu_variable_name


    if [ ${status} = "1" ]; then
        echo "Feature option compare status : FAIL"
        echo "./build.sh $fake_board $project -variableCompare" >> .variable_check_fail_cases
        #exit 1;
    else
        rm .mcu_variable_name .dsp_variable_name .total_variable
        echo "Feature option compare status : PASS"
    fi

    if [ ${variableCompareSwitch} = "on" ]; then
        exit 0;
    fi
}

###############################################################################
# Parsing arguments
if [ "$#" -eq "0" ]; then
    show_usage
    exit 1
fi

declare -a argv=($0)
for i in $@
do
    case ${i} in
        list)
            if [ "${#argv[@]}" == "3" ]; then
                board=${argv[1]}
                project=${argv[2]}
                mapping_var="map__${board}__${project}"
                eval dsp1_proj=\${$mapping_var[3]}
                eval dsp0_proj=\${$mapping_var[2]}
                eval cm4_proj=\${$mapping_var[1]}
                eval board_share=\${$mapping_var[0]}
                if [ ! -z "${board_share}" ]; then
                    board=${board_share}
                fi
                cd ${working_dir}/mcu
                ./build.sh ${board} ${project} "list"
                cd ${working_dir}
                echo -e "\n\n"
                cd ${working_dir}/dsp
                if [ ! -z "${dsp0_proj}" ]; then
                    ./build.sh ${board} ${dsp0_proj} "list"
                fi
                if [ !  -z "${dsp1_proj}" ]; then
                    ./build.sh ${board} ${dsp1_proj} "list"
                fi
                cd ${working_dir}
                exit 0
            elif [ "${#argv[@]}" == "1" ]; then
                show_available_proj
                exit 0
            else
                #echo "args : ${#argv[@]}"
                echo "Error: wrong usage!!"
                exit 1
            fi
            ;;
        list_board_share)
            if [ "${#argv[@]}" == "3" ]; then
                board=${argv[1]}
                project=${argv[2]}
                mapping_var="map__${board}__${project}"
                eval board_share=\${$mapping_var[0]}
                echo "${board_share}"
                exit 0
            fi
            ;;
        -fm=*)
            feature_mk="${i#*=}"
            ;;
        -fd0=*)
            feature_mk_dsp0="${i#*=}"
            ;;
        -fd1=*)
            feature_mk_dsp1="${i#*=}"
            ;;
        -mcu)
            default_dsp_bin_switch="on"
            ;;
        -compTest)
            compatibleTest="on"
            ;;
        -variableCompare)
            variableCompare="on"
            ;;
        -o=*|--option=*)
            opt="${i#*=}"
            extra_opt+="-o=${opt} "
            shift
            ;;
        -odsp=*|--option_dsp=*)
            opt="${i#*=}"
            extra_opt_dsp+="-o=${opt} "
            shift
            ;;
        -omcu=*|--option_mcu=*)
            opt="${i#*=}"
            extra_opt_mcu+="-o=${opt} "
            shift
            ;;
        -*)
            echo "Error: unknown parameter \"$i\""
            show_usage
            exit 1
            ;;
        *)
            argv+=($i)
            ;;
    esac
done

# check configurations
board=${argv[1]}
project=${argv[2]}
fake_board="none"
output_dir=${OUT}/${board}/${project}
mapping_var="map__${board}__${project}"
eval board_share=\${$mapping_var[0]}
eval cm4_proj=\${$mapping_var[1]}
eval dsp0_proj=\${$mapping_var[2]}
eval dsp1_proj=\${$mapping_var[3]}

if [ ! -z "${board_share}" ]; then
    fake_board=${board}
    board=${board_share}
    echo -e "board from mapping_proj.cfg: ${board}\n"
fi

if [ -d "mcu/project/ab157x_ab158x" ]; then board="ab157x_ab158x"; fi

if [  ${debug} -eq "1" ]; then
    echo -e "feature_mk : ${feature_mk}\n"
    echo -e "feature_mk_dsp0 : ${feature_mk_dsp0}\n"
    echo -e "feature_mk_dsp1 : ${feature_mk_dsp1}\n"
fi

if [ -z "${feature_mk}${feature_mk_dsp0}${feature_mk_dsp1}" ]; then
    eval feature_mk=\${$mapping_var[4]}
    eval feature_mk_dsp0=\${$mapping_var[5]}
    eval feature_mk_dsp1=\${$mapping_var[6]}
    eval feature_mk_bootloader=\${$mapping_var[8]}
fi

if [ -z "$bootloader_proj" ]; then
    eval bootloader_proj="bootloader"
fi

if [  ${debug} -eq "1" ]; then
    echo -e "\n\n"
    echo -e "feature_mk from mapping_proj.cfg: ${feature_mk}\n";
    echo -e "feature_mk_dsp0 from mapping_proj.cfg: ${feature_mk_dsp0}\n";
    echo -e "feature_mk_dsp1 from mapping_proj.cfg: ${feature_mk_dsp1}\n";
fi

# To do actions according to arguments
if [ "${argv[3]}" == "clean" ]; then
    if [ "${#argv[@]}" != "4" ]; then
        show_usage
        exit 1
    fi
    if [ ! -z "${dsp1_proj}" ]; then
        clean_out ${working_dir}/dsp/out/${board}/${dsp1_proj}
    fi
    if [ ! -z "${dsp0_proj}" ]; then
        clean_out ${working_dir}/dsp/out/${board}/${dsp0_proj}
    fi
    if [ ! -z "${cm4_proj}" ]; then
        clean_out ${working_dir}/mcu/out/${board}/${cm4_proj}
    fi
    if [ "${fake_board}" == "none" ]; then
        clean_out ${OUT}/${board}/${project}
    else
        clean_out ${OUT}/${fake_board}/${project}
    fi

elif [ "${argv[2]}" == "clean" ]; then
    if [ "${#argv[@]}" != "3" ]; then
        show_usage
        exit 1
    fi
    clean_out ${working_dir}/dsp/out/${board}
    clean_out ${working_dir}/mcu/out/${board}
    if [ "${fake_board}" == "none" ]; then
        clean_out ${OUT}/${board}/${project}
    else
        clean_out ${OUT}/${fake_board}/${project}
    fi
elif [ "${argv[1]}" == "clean" ]; then
    if [ "${#argv[@]}" != "2" ]; then
        show_usage
        exit 1
    fi
    clean_out ${working_dir}/mcu/out
    clean_out ${working_dir}/dsp/out
    clean_out ${OUT}
else
    if [ "${#argv[@]}" != "3" ]; then
        show_usage
        exit 1
    fi
    eval mapping_var_key=\${!$mapping_var[@]}
    if [ -z "${mapping_var_key}" ]; then
        show_usage
        echo "Error: cannot find board=${board} project=${project} configurations in mapping_proj.cfg."
        exit 1
    fi

    #check feature option
    if [ "${variableCompare}" == "on" ]; then
        check_variable_value ${board_share} ${cm4_proj} ${feature_mk} ${board_share} ${dsp0_proj} ${feature_mk_dsp0} ${variableCompare}
    fi

    rm -rf "${output_dir}/log"
    mkdir -p "${output_dir}/log"
    rm -rf "${output_dir}/debug"
    mkdir -p "${output_dir}/debug"
    rm -rf "${output_dir}/download"
    mkdir -p "${output_dir}/download"
    echo "$co_build_command" > ${output_dir}/log/build_time.log
    echo "Start CO-Build: "`date` >> ${output_dir}/log/build_time.log


    # dsp1 build start
    dsp1_filename="${feature_mk_dsp1%.*}"
    if [ ! -z "${dsp1_proj}" ] && [ "${default_dsp_bin_switch}" == "off" ]; then

        echo "cd ${working_dir}/dsp"
        cd ${working_dir}/dsp
        echo "=============================================================="
        echo "Start DSP1 Build"
        echo "=============================================================="
        #echo "dsp1_filename : ${dsp1_filename}"
        #echo "Start DSP1 Build: "`date` >> $output_dir/log/build_time.log
        if [ ! -z "${feature_mk_dsp1}" ]; then
            ./build.sh ${board} ${dsp1_proj} -f=${feature_mk_dsp1} -o=OUT=out/${board}/${dsp1_proj}/${dsp1_filename} -cobuild ${extra_opt} ${extra_opt_dsp}
        else
            ./build.sh ${board} ${dsp1_proj} -f=feature.mk -o=OUT=out/${board}/${dsp1_proj}/feature -cobuild ${extra_opt} ${extra_opt_dsp}
        fi
        DSP1_BUILD_RESULT=$?
        #echo "End DSP1 Build: "`date` >> $output_dir/log/build_time.log
        echo "=============================================================="
        echo "End DSP1 Build"
        echo "=============================================================="
        # change back to working dir
        echo "cd ${working_dir}/"
        cd ${working_dir}/
        # copy dsp files
        if [ ! -z "${feature_mk_dsp1}" ]; then
            copy_dsp_files ${board} ${dsp1_proj} ${project} ${fake_board} ${dsp1_filename} dsp/out
        else
            copy_dsp_files ${board} ${dsp1_proj} ${project} ${fake_board} feature dsp/out
        fi
    elif [ ! -z "${dsp1_proj}" ] && [ "${default_dsp_bin_switch}" == "on" ]; then
        echo "=============================================================="
        echo "Copy prebuilt DSP1"
        echo "=============================================================="
        # change back to working dir
        echo "cd ${working_dir}/"
        cd ${working_dir}/
        # copy dsp files
        if [ ! -z "${feature_mk_dsp1}" ]; then
            copy_dsp_files ${board} ${dsp1_proj} ${project} ${fake_board} ${dsp1_filename} dsp/prebuilt/dsp1_bin
        else
            copy_dsp_files ${board} ${dsp1_proj} ${project} ${fake_board} feature dsp/prebuilt/dsp1_bin
        fi
        echo "=============================================================="
        echo "End copy prebuilt DSP1"
        echo -e "==============================================================\n"
    fi

    # dsp0 build start
    dsp0_filename="${feature_mk_dsp0%.*}"
    if [ ! -z "${dsp0_proj}" ] && [ "${default_dsp_bin_switch}" == "off" ]; then

        echo "cd ${working_dir}/dsp"
        cd ${working_dir}/dsp
        echo "=============================================================="
        echo "Start DSP0 Build"
        echo "=============================================================="
        #echo "Start DSP0 Build: "`date` >> $output_dir/log/build_time.log
        #echo "dsp0_filename : $dsp0_filename"
        if [ ! -z "${feature_mk_dsp0}" ]; then
            ./build.sh ${board} ${dsp0_proj} -f=${feature_mk_dsp0} -o=OUT=out/${board}/${dsp0_proj}/${dsp0_filename} -cobuild ${extra_opt} ${extra_opt_dsp}
        else
            ./build.sh ${board} ${dsp0_proj} -f=feature.mk -o=OUT=out/${board}/${dsp0_proj}/feature -cobuild ${extra_opt} ${extra_opt_dsp}
        fi
        DSP0_BUILD_RESULT=$?
        #echo "End DSP0 Build: "`date` >> $output_dir/log/build_time.log
        echo "=============================================================="
        echo "End DSP0 Build"
        echo "=============================================================="
        # change back to working dir
        echo "cd ${working_dir}/"
        cd ${working_dir}/
        # copy dsp files
        if [ ! -z "${feature_mk_dsp0}" ]; then
            copy_dsp_files ${board} ${dsp0_proj} ${project} ${fake_board} ${dsp0_filename} dsp/out
        else
            copy_dsp_files ${board} ${dsp0_proj} ${project} ${fake_board} feature dsp/out
        fi
    elif [ ! -z "${dsp0_proj}" ] && [ "${default_dsp_bin_switch}" == "on" ]; then
        echo "=============================================================="
        echo "Copy prebuilt DSP0"
        echo "=============================================================="
        # change back to working dir
        echo "cd ${working_dir}/"
        cd ${working_dir}/
        # copy dsp files
        if [ ! -z "${feature_mk_dsp0}" ]; then
            copy_dsp_files ${board} ${dsp0_proj} ${project} ${fake_board} ${dsp0_filename} dsp/prebuilt/dsp0_bin
        else
            copy_dsp_files ${board} ${dsp0_proj} ${project} ${fake_board} feature dsp/prebuilt/dsp0_bin
        fi
        echo "=============================================================="
        echo "End copy prebuilt DSP0"
        echo -e "==============================================================\n"
    fi

    # cm4 build start
    if [ ! -z "${cm4_proj}" ]; then
        echo "cd ${working_dir}/mcu"
        cd ${working_dir}/mcu
        echo "=============================================================="
        echo "Start CM4 Build"
        echo "=============================================================="
        #echo "Start CM4 Build: "`date` >> $output_dir/log/build_time.log
        cm4_filename="${feature_mk%.*}"
        #echo "cm4_filename : ${cm4_filename}"
        if [ ! -z "${feature_mk}" ]; then
                if [ "$compatibleTest" == "off" ]; then
                    ./build.sh ${board} ${cm4_proj} -f=${feature_mk} -o=OUT=out/${board}/${cm4_proj}/${cm4_filename} bl -cobuild ${extra_opt} ${extra_opt_mcu}
                else
                    ./build.sh ${board} ${cm4_proj} -f=${feature_mk} -o=OUT=out/${board}/${cm4_proj}/${cm4_filename} bl -compTest ${extra_opt} ${extra_opt_mcu}
                fi
        else
                if [ "$compatibleTest" == "off" ]; then
                    ./build.sh ${board} ${cm4_proj} -f=feature.mk -o=OUT=out/${board}/${cm4_proj}/feature bl -cobuild ${extra_opt} ${extra_opt_mcu}
                else
                    ./build.sh ${board} ${cm4_proj} -f=feature.mk -o=OUT=out/${board}/${cm4_proj}/feature bl -compTest ${extra_opt} ${extra_opt_mcu}
                fi
        fi
        CM4_BUILD_RESULT=$?
        #echo "End CM4 Build: "`date` >> ${output_dir}/log/build_time.log
        echo "=============================================================="
        echo "End CM4 Build"
        echo -e "==============================================================\n"
        # change back to working dir
        echo "cd ${working_dir}/"
        cd ${working_dir}/
        # copy cm4 files
        if [ ! -z "${feature_mk}" ]; then
            copy_cm4_files ${board} ${cm4_proj} ${project} ${fake_board} ${cm4_filename}
        else
            copy_cm4_files ${board} ${cm4_proj} ${project} ${fake_board} feature
        fi
    fi

    # update flash_download.cfg
    flash_cfg=${output_dir}/download/flash_download.cfg
    if [ -e "${flash_cfg}" ]; then
        if [ ! -z "${dsp0_proj}" ]; then
            sed -i "s|\bdsp0_freertos_create_thread.bin|${dsp0_proj}.bin|g" ${flash_cfg}
            sed -i -n '{/rom:/{x;n;{/dsp0/{x;s|^#||;p;x;tc};x;p;be}};{:c;/\bdsp0_/,+2s|^#||;}};p;bend;:e;x;p;:end' ${flash_cfg}
        fi
        if [ ! -z "${dsp1_proj}" ]; then
            sed -i "s|\bdsp1_no_rtos_initialize_system.bin|${dsp1_proj}.bin|g" ${flash_cfg}
            sed -i -n '{/rom:/{x;n;{/dsp1/{x;s|^#||;p;x;tc};x;p;be}};{:c;/\bdsp1_/,+2s|^#||;}};p;bend;:e;x;p;:end' ${flash_cfg}
        fi
    fi

    # return code
    declare -i BUILD_RESULT=0
    echo "End   CO-Build: "`date` >> ${output_dir}/log/build_time.log
    if [ ! -z "${DSP1_BUILD_RESULT}" ]; then
        if [ "${DSP1_BUILD_RESULT}" -eq "0" ]; then
            echo -e "DSP1 BUILD: ${Green}PASS${Color_Off}" >> ${output_dir}/log/build_time.log
        else
            echo -e "DSP1 BUILD: ${Red}FAIL${Color_Off}" >> ${output_dir}/log/build_time.log
            BUILD_RESULT+=1
        fi
    fi
    if [ ! -z "${DSP0_BUILD_RESULT}" ]; then
        if [ "${DSP0_BUILD_RESULT}" -eq "0" ]; then
            echo -e "DSP0 BUILD: ${Green}PASS${Color_Off}" >> ${output_dir}/log/build_time.log
        else
            echo -e "DSP0 BUILD: ${Red}FAIL${Color_Off}" >> ${output_dir}/log/build_time.log
            BUILD_RESULT+=1
        fi
    fi
    if [ ! -z "${CM4_BUILD_RESULT}" ]; then
        if [ "${CM4_BUILD_RESULT}" -eq "0" ]; then
            echo -e "CM4  BUILD: ${Green}PASS${Color_Off}" >> ${output_dir}/log/build_time.log
        else
            echo -e "CM4  BUILD: ${Red}FAIL${Color_Off}" >> ${output_dir}/log/build_time.log
            BUILD_RESULT+=1
        fi
    fi

    if [ "${BUILD_RESULT}" -eq "0" ]; then
        echo -e "TOTAL CO-BUILD: ${Green}PASS${Color_Off} (return code ${BUILD_RESULT})" >> ${output_dir}/log/build_time.log
    else
        echo -e "TOTAL CO-BUILD: ${Red}FAIL${Color_Off} (return code ${BUILD_RESULT})" >> ${output_dir}/log/build_time.log
    fi

    # Roughly count the footprint information of ROM and RAM.
    mcu/tools/scripts/build/general_fp.sh ${output_dir}/debug ${output_dir}/debug/footprint

    echo "=============================================================="
    echo "Summary CO-BUILD"
    echo "=============================================================="
    cat ${output_dir}/log/build_time.log
    mv -f ${output_dir}/*.log "${output_dir}/log/" 2> /dev/null

    exit ${BUILD_RESULT}
fi


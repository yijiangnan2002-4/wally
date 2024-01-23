#!/bin/bash

##############################################################################
# Variables

SOURCE_DIR=$1
BOARD=$2
MCU_FEATURE_MK_FILE=$3

# Define the key words that indicate the feature option is used both MCU and DSP side.
share_option_search_string_1="the MCU and DSP side"
share_option_search_string_2="both DSP and MCU"

# Found the shared option flag
found_shared_option_flag=0

# Shared option list that read from the DSP feature.mk
found_shared_feature_option_list=()
# MCU feature option list that parsed from the MCU feature.mk
mcu_feature_option_list=()

dsp_feature_mk_file=""

echo "start to check shared option in $MCU_FEATURE_MK_FILE"

# Parse MCU feature.mk to get all of the feature options
while IFS= read -r current_line || [[ -n "$current_line" ]]
do
    if [[ $current_line == *"#"* ]]; then
        continue
    elif [[ $current_line == *":="* ]]; then
        continue
    elif [[ $current_line == *"include"* ]]; then
        # Found the DSP feature.mk that use include at the begin of the line
        # Parse the string to get the feature.mk
        value=${current_line##*/}
        dsp_feature_mk_file="$SOURCE_DIR/../dsp/project/$BOARD/apps/dsp0_headset_ref_design/XT-XCC/$value"
        continue
    fi

    # Delete = and spaces
    value="${current_line%%=*}"
    value="${value// /}"
    # Append feature option
    mcu_feature_option_list+=("$value")
done < $MCU_FEATURE_MK_FILE

# Print MCU feature option list
# for share_option in "${mcu_feature_option_list[@]}"
# do
#     echo $share_option
# done

# Parse DSP feature.mk to get the list of the shared option
while IFS= read -r current_line || [[ -n "$current_line" ]]
do
    if [[ $current_line =~ $share_option_search_string_1 ]]; then
        found_shared_option_flag=1
    elif [[ $current_line =~ $share_option_search_string_2 ]]; then
        found_shared_option_flag=1
    fi
    # Skip the comments line
    if [[ $current_line == *"#"* ]]; then
        continue
    fi

    if [[ $found_shared_option_flag -eq 1 ]]; then
        # Delete = and spaces
        value="${current_line%%=*}"
        value="${value// /}"
        # Append the shared option list
        found_shared_feature_option_list+=("$value")
        found_shared_option_flag=0
    fi
done < $dsp_feature_mk_file

# Print the shared option list
# for share_option in "${found_shared_feature_option_list[@]}"
# do
#     echo $share_option
# done


##############################################################################
# Check the feature shared option in the mcu feature option list or not.
for share_option in "${found_shared_feature_option_list[@]}"
do
    for mcu_option in "${mcu_feature_option_list[@]}"
    do
        # if exist in MCU feature.mk need prompt make error to notify
        if [[ $mcu_option == $share_option ]]; then
            echo "Error: $share_option is a MCU-DSP-Shared feature option but defined in MCU. Remove it from the MCU feature.mk and define it in the corresponding DSP feature.mk."
            exit 1
        fi
    done
done

exit 0

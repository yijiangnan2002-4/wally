#!/usr/bin/python3
# -*- coding: UTF-8 -*-
#
# * Copyright Statement:
# *
# * (C) 2022  Airoha Technology Corp. All rights reserved.
# *
# * This software/firmware and related documentation ("Airoha Software") are
# * protected under relevant copyright laws. The information contained herein
# * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
# * Without the prior written permission of Airoha and/or its licensors,
# * any reproduction, modification, use or disclosure of Airoha Software,
# * and information contained herein, in whole or in part, shall be strictly prohibited.
# * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
# * if you have agreed to and been bound by the applicable license agreement with
# * Airoha ("License Agreement") and been granted explicit permission to do so within
# * the License Agreement ("Permitted User").  If you are not a Permitted User,
# * please cease any access or use of Airoha Software immediately.
# * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
# * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
# * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
# * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
# * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
# * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
# * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
# *
# * Airoha restricted information */

import os
import re
import csv
import sys
import copy
import getopt
from enum import Enum
from time import sleep
from pprint import pprint

airoha_license = '\n\
/* Copyright Statement:\n\
 *\n\
 * (C) 2022  Airoha Technology Corp. All rights reserved.\n\
 *\n\
 * This software/firmware and related documentation ("Airoha Software") are\n\
 * protected under relevant copyright laws. The information contained herein\n\
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.\n\
 * Without the prior written permission of Airoha and/or its licensors,\n\
 * any reproduction, modification, use or disclosure of Airoha Software,\n\
 * and information contained herein, in whole or in part, shall be strictly prohibited.\n\
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software\n\
 * if you have agreed to and been bound by the applicable license agreement with\n\
 * Airoha ("License Agreement") and been granted explicit permission to do so within\n\
 * the License Agreement ("Permitted User").  If you are not a Permitted User,\n\
 * please cease any access or use of Airoha Software immediately.\n\
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES\n\
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES\n\
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL\n\
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF\n\
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.\n\
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE\n\
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR\n\
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH\n\
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES\n\
 * THAT IT IS RECEIVER\'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES\n\
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA\n\
 * SOFTWARE RELEASES MADE TO RECEIVER\'S SPECIFICATION OR TO CONFORM TO A PARTICULAR\n\
 * STANDARD OR OPEN FORUM. RECEIVER\'S SOLE AND EXCLUSIVE REMEDY AND AIROHA\'S ENTIRE AND\n\
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,\n\
 * AT AIROHA\'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,\n\
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO\n\
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.\n\
 */'


def show_scripts_usage():
    '''
    Refer to the man page of the command line to display the help information of the script.
    '''

    print('Name\n\tdmm - A simple script that assists the user to generate standard-compliant memory management C code.\n')
    print('SYNOPSIS\n\tpython3 dmm.py -f input_file [-o output_dir] [-h]\n')
    print('DESCRIPTION')
    print("\tDMM is an acronym for DSP memory management, and its purpose is to avoid malloc failures due to memory fragmentation.")
    print("\tDMM script can generate standard C code according to a pre-planned format to ease the developer's maintenance effort.\n")
    print('OPTIONS')
    print('\t-f: A csv file that defines information such as Compnent, Sub Component, Region, etc.\n')
    print('\t-d: The feature option file used when the project is compiled.\n')
    print('\t-o: Indicates the directory where the files are generated.\n')
    print('EXAMPLE')
    print('\tEg1. python3 dmm.py -f ./PreDefine.csv')
    print('\tMeaning: Generate the corresponding header file and source file according to the PreDefine.csv file in the current directory.\n')
    print('\tEg2. python3 dmm.py -f ./pre_define.csv -o ./dmm_code')
    print('\tMeaning: Generate the corresponding header file and source file according to the pre_define.csv file in the dmm_code directory.\n')
    print('\tEg3. python3 dmm.py -f ./pre_define.csv -o ./dmm_code -d ./feature_opts_list.log')
    print('\tMeaning: Generate the header and source file according to the pre_define.csv and feature_opts_list file in the dmm_code directory.\n')


def get_args_from_cli(argv):
    '''
    A help function for obtaining command line parameter information.
    '''

    def show_all_input_parameter():
        for idx, item in enumerate(argv):
            print('  argv[%d]:' % idx, item)
        print('')

    out_args = {}

    try:
        opts, args = getopt.getopt(argv, "f:o:d:h")
    except getopt.GetoptError:
        print('Error input parameter:', file=sys.stderr)
        show_all_input_parameter()
        show_scripts_usage()
        sys.exit()
    for opt, arg in opts:
        if opt == '-f':
            if os.path.exists(arg):
                out_args['file'] = arg
            else:
                print('file "%s" not exist' % arg, file=sys.stderr)
                sys.exit()
        elif opt == '-h':
            out_args['help'] = 'help'
        elif opt == '-o':
            out_args['out_dir'] = arg
            if not os.path.exists(arg):
                print(out_args['out_dir'])
                os.makedirs(out_args['out_dir'])
        elif opt == '-d':
            out_args['feature'] = arg
            if not os.path.exists(arg):
                print(out_args['feature'])
                os.makedirs(out_args['feature'])
        else:
            pass
    return out_args


class DMM_region:
    def __init__(self) -> None:
        self.attr = str()
        self.name = str()
        self.size = str()

    def print(self):
        print('#'*80)
        for key, value in self.__dict__.items():
            print(key,':',value)
        print('#'*80)


class DMM_mapping:
    def __init__(self) -> None:
        self.rgn_number = str()                 # region number
        self.cpnt_name = str()                  # component name
        self.grp_number = str()                 # group number
        self.attr = str()                       # special pattern to assist script
        self.sub_cpnt_name = str()              # sub component name
        self.feature_type = str()               # feature type
        self.feature_expr = str()               # feature option expression
        self.hdr_inc_path = str()               # header included path

        self.rgn_name = list()                  # region name/variable name
        self.sub_cpnt_size = list()             # sub component size
        self.sub_cpnt_size_macro = list()       # sub component size macro used by compile check

        self.note = str()                       # note information

        self.skip_mark = False                  # a special flag to mark whether to skip this mapping

    def print(self):
        print('#'*80)
        for key, value in self.__dict__.items():
            print(key,':',value)
        print('#'*80)

    def get_sub_cpnt_size_check_pair(self):
        for idx, macro in enumerate(self.sub_cpnt_size_macro):
            if len(macro.strip()):
                yield (macro, self.sub_cpnt_size[idx])


class DMM_footprint:
    def __init__(self) -> None:
        self.cpnt_name = str()
        self.cpnt_size = list()
        self.rgn_name = list()

    def print(self):
        print('#'*80)
        for key, value in self.__dict__.items():
            print(key,':',value)
        print('#'*80)


class DMM_parser:
    def __init__(self, dmm_file = None, feature_option_file = None, out_dir = None) -> None:
        assert(os.path.exists(dmm_file))
        assert(os.path.exists(out_dir))
        self.__regions = []
        self.__component = []
        self.__sub_cpnt = []
        self.__mapping = []
        self.__sub_cpnt_number_in_one_grp = []
        self.__out_dir = out_dir
        self.__feature_list = feature_option_file
        self.__fp_rcd = []
        if feature_option_file is not None:
            assert(os.path.exists(feature_option_file))
            with open(feature_option_file, 'r', encoding = 'utf-8') as feature_list:
                self.__feature_list = feature_list.read().replace(' ', '')
            # print(self.__feature_list)

        with open(dmm_file, 'r', encoding = 'utf-8', errors = 'ignore') as src_info:
            class _info_type(Enum):
                unknown = 0
                region = 1
                component_type = 2
                mapping = 3
            info_type = _info_type.unknown
            reader = csv.reader(src_info)
            line_num = 1
            _tmp_mapping = None
            for line_ctx in reader:
                # print(line_num, info_type, line_ctx)
                line_num = line_num + 1
                if (len(line_ctx) >= 3) and \
                 (line_ctx[0] == 'Type' ) and \
                 (line_ctx[1] == 'Region Name') and \
                 (line_ctx[2] == 'Size'):
                    info_type = _info_type.region
                    continue
                elif (len(line_ctx) >= 1) and \
                 (line_ctx[0] == 'TotalComponentType_t' ):
                    info_type = _info_type.component_type
                    continue
                elif (len(line_ctx) >= 18) and \
                 (line_ctx[0] == 'Region No') and \
                 (line_ctx[1] == 'Component Name' ) and \
                 (line_ctx[2] == 'Group No' ) and \
                 (line_ctx[3] == 'Attr' ) and \
                 (line_ctx[4] == 'Sub-component Name' ) and \
                 (line_ctx[5] == 'Feature Type' ) and \
                 (line_ctx[6] == 'Feature Option' ):
                    info_type = _info_type.mapping
                    continue
                else:
                    # Take different actions based on info_type
                    not_empty_ctx = [ item for item in line_ctx if item is not '' ]
                    if (len(not_empty_ctx) == 0):
                        # Reset the scan type and continue scanning below
                        info_type = _info_type.unknown
                        if _tmp_mapping is not None:
                            self.__mapping.append(copy.deepcopy(_tmp_mapping))
                            _tmp_mapping = None
                        continue
                    elif (info_type == _info_type.region):
                        _tmp_rgn = DMM_region()
                        _tmp_rgn.attr = line_ctx[0]
                        _tmp_rgn.name = line_ctx[1]
                        _tmp_rgn.size = line_ctx[2]
                        self.__regions.append(copy.deepcopy(_tmp_rgn))

                    elif (info_type == _info_type.component_type):
                        _tmp_name = line_ctx[0]
                        self.__component.append(_tmp_name)
                    elif (info_type == _info_type.mapping):
                        _tmp_mapping = DMM_mapping()

                        _tmp_mapping.rgn_number = line_ctx[0]                 # region number
                        _tmp_mapping.cpnt_name = line_ctx[1]                  # component name
                        _tmp_mapping.grp_number = line_ctx[2]                 # group number
                        _tmp_mapping.attr = line_ctx[3]                       # special pattern to assist script
                        _tmp_mapping.sub_cpnt_name = line_ctx[4]              # sub component name
                        _tmp_mapping.feature_type = line_ctx[5]               # feature type
                        _tmp_mapping.feature_expr = line_ctx[6]               # feature option expression
                        _tmp_mapping.hdr_inc_path = line_ctx[7]               # header included path

                        # region name/variable name
                        _tmp_mapping.rgn_name = [ line_ctx[8], line_ctx[11], line_ctx[14] ]
                        # sub component size
                        _tmp_mapping.sub_cpnt_size = [ line_ctx[9], line_ctx[12], line_ctx[15] ]
                        # sub component size macro used by compile check
                        _tmp_mapping.sub_cpnt_size_macro = [ line_ctx[10], line_ctx[13], line_ctx[16] ]

                        _tmp_mapping.note = line_ctx[17]                       # note information

                        if self.__evaluation_expr(_tmp_mapping.feature_expr, self.__feature_list):
                            self.__mapping.append(copy.deepcopy(_tmp_mapping))
                        else:
                            _tmp_mapping.skip_mark = True
                    else:
                        print('%d:' %(line_num))
                        print('    ', line_ctx)
                        print('    length is: %d' %len(line_ctx))
                        assert(0)

        for item in self.__mapping:
            item.print()


    def __grp_size_align_and_padding(self, size, align = 64, padding = 84):
        # align : size align in csv. padding = header(20byte)+ align;
        size = (size + (align-1)) & (~(align-1))
        if size != 0:
            size += padding
        return size


    def __evaluation_expr(self, expr, feature_list):
        expr = expr.strip().replace(' ', '')
        if len(expr) == 0:
            return True
        result = False
        '''
        Replace the original expression according to the following principles,
        and use the eval built-in function to evaluate the expression value.
            1) separate expressions with the '&'/'|'/'('/')' symbols.
            2) '&&' => ' and '
            3) '||' => ' or '
            4) '!' => ' not '
        '''
        orig_expr = expr
        def split(in_str, sep_list):
            _ret = []
            return _ret

        sep_set = set('&|()')

        # keep the specified delimiter but no empty item
        expr_list = [ item for item in re.split('([&|()])', expr) if len(item)!=0 ]
        # print(expr_list)
        for idx, expr_item in enumerate(expr_list):
            if feature_list.find(expr_item) != -1:
                expr_list[idx] = 'True'
            else:
                if expr_item not in sep_set:
                    expr_list[idx] = 'False'
        # print(expr_list)
        expr = ''.join(expr_list).replace('&&', ' and ')
        expr = expr.replace('||', ' or ')
        expr = expr.replace('!', ' not ')
        # print('evaluate expression: %s => %s => %s' %(orig_expr, expr, result))
        result = eval(expr)
        '''
        print(eval('True and True'))
        print(eval('True or False'))
        print(eval('(not True) or False'))
        print(eval('(not (True or False))'))
        print('feature_list: %s' %feature_list)
        '''
        return result


    def gen_code(self):
        self.__gen_hdr_file()
        self.__gen_src_file()


    def __add_content(self, file_handler, content, need_blank = True):
        def add_blank_line_times(file_handler, times):
            while times != 0:
                # print('add empty line')
                file_handler.write('\n')
                times = times - 1

        def add_blank_line_regular(file_handler):
            add_blank_line_times(file_handler, 2)

        file_handler.write(content+'\n')
        if need_blank:
            add_blank_line_regular(file_handler)


    def __gen_hdr_file(self):
        file_path = self.__out_dir + os.sep + 'ComponentPreDefine.h'
        with open(file_path, 'wt', encoding = 'utf-8') as hdr_file:
            self.__add_content(hdr_file, airoha_license)
            self.__add_content(hdr_file, '#ifndef __COMPONENT_PRE_DEFINE_H__', False)
            self.__add_content(hdr_file, '#define __COMPONENT_PRE_DEFINE_H__')
            self.__add_content(hdr_file, '#include <stdint.h>')
            self.__add_content(hdr_file, '#ifdef __cplusplus\nextern "C" {\n#endif')
            self.__add_content(hdr_file, '/* The number of REGIONs filled in by the user and', False)
            self.__add_content(hdr_file, ' * the variable name corresponding to each region.', False)
            self.__add_content(hdr_file, ' */', False)
            self.__add_content(hdr_file, '#define DMM_PREDEFINED_REGION_NUM %dU' %(len(self.__regions)), False)

            # add region related infortmation
            self.__add_content(hdr_file, 'extern const uint32_t RegionAddr[];', False)
            for _tmp_reg in self.__regions:
                self.__add_content(hdr_file, 'extern uint32_t %s[];' %(_tmp_reg.name.strip()), False)
            self.__add_content(hdr_file, '')

            # add component enum definition
            self.__add_content(hdr_file, '/*'+'#'*86+'*/', False)
            self.__add_content(hdr_file, '/*'+'#'*29 +' ' + 'Component Enum Definition' + ' ' + '#'*30+'*/', False)
            self.__add_content(hdr_file, '/*'+'#'*86+'*/', False)
            self.__add_content(hdr_file, 'typedef enum{', False)
            self.__add_content(hdr_file, ' '*4+'Component_Start = 0x43504E54,    /* HEX: CPNT means component */', False)
            for _tmp_cpnt in self.__component:
                self.__add_content(hdr_file, ' '*4+'Component_'+_tmp_cpnt.strip()+',', False)
            self.__add_content(hdr_file, ' '*4+'Component_No_Used_Memory,', False)
            self.__add_content(hdr_file, ' '*4+'Component_Max,', False)
            self.__add_content(hdr_file, '} TotalComponentType_t;')

            # add sub component definition
            self.__add_content(hdr_file, '/*'+'#'*86+'*/', False)
            self.__add_content(hdr_file, '/*'+'#'*27 +' ' + 'Sub Component Enum Definition' + ' ' + '#'*28+'*/', False)
            self.__add_content(hdr_file, '/*'+'#'*86+'*/', False)
            self.__add_content(hdr_file, 'typedef enum{', False)
            self.__add_content(hdr_file, ' '*4+'SubComponent_Start = 0x53554243,    /* HEX: SUBC means sub-component */', False)
            for _tmp_mapping in self.__mapping:
                sub_cpnt_name = _tmp_mapping.sub_cpnt_name.strip()
                if len(sub_cpnt_name):
                    _tmp_sub_cpnt_name = 'SubComponent_'+sub_cpnt_name
                    self.__add_content(hdr_file, ' '*4+_tmp_sub_cpnt_name+',', False)
                    self.__sub_cpnt.append(_tmp_sub_cpnt_name)
            self.__add_content(hdr_file, ' '*4+'SubComponent_No_Used_Memory,', False)
            self.__add_content(hdr_file, ' '*4+'SubComponent_Max,', False)
            self.__add_content(hdr_file, '} SubComponentType_t;', True)

            self.__add_content(hdr_file, '/*'+'#'*86+'*/', False)
            self.__add_content(hdr_file, '/*'+'#'*24 +' ' + 'Sub Component Size Check Helper Macro' + ' ' + '#'*23+'*/', False)
            self.__add_content(hdr_file, '/*'+'#'*86+'*/', False)
            self.__add_content(hdr_file, '/* Take advantage of the fact that the number of elements in an array', False)
            self.__add_content(hdr_file, ' * is not negative to check whether two inputs are equal.', False)
            self.__add_content(hdr_file, ' */', False)
            self.__add_content(hdr_file, '#define SIZE_OF_MACRO_EQUAL_TO_NUMBER(MACRO, SIZE) \\', False)
            self.__add_content(hdr_file, '   typedef int check_size_of_##MACRO##_equal_to_##SIZE[(MACRO==SIZE)?(1):(-1)]', False)
            self.__add_content(hdr_file, '', False)

            # add group start/end address macro
            self.__add_content(hdr_file, '/*'+'#'*86+'*/', False)
            self.__add_content(hdr_file, '/*'+'#'*27 +' ' + 'Group Start/Length/End Address' + ' ' + '#'*27+'*/', False)
            self.__add_content(hdr_file, '/*'+'#'*86+'*/', False)
            # IRAM, DRAM, SYSRAM == 3
            for idx in range(3):
                grp_num = 0
                grp_size = 0
                rcd_grp_size = list()
                for _tmp_mapping in self.__mapping:
                    rgn_name = _tmp_mapping.rgn_name[idx]
                    # print('line 440: region name is %s' %rgn_name)
                    if _tmp_mapping.attr == 'Head':
                        macro = (_tmp_mapping.cpnt_name + '_' + _tmp_mapping.rgn_name[idx] + '_G000_start').upper()
                        value = _tmp_mapping.rgn_name[idx]
                        self.__add_content(hdr_file, '#define %s %s' %(macro, value), False)

                        macro = (_tmp_mapping.cpnt_name + '_' + _tmp_mapping.rgn_name[idx] + '_G000_len').upper()
                        value = _tmp_mapping.sub_cpnt_size[idx]
                        self.__add_content(hdr_file, '#define %s %s' %(macro, value), False)

                        macro = (_tmp_mapping.cpnt_name + '_' + _tmp_mapping.rgn_name[idx] + '_G000_end').upper()
                        value = _tmp_mapping.sub_cpnt_size[idx]
                        self.__add_content(hdr_file, '#define %s (uint32_t*)((uint8_t*)%s + %d)' %(
                            macro,
                            _tmp_mapping.rgn_name[idx],
                            int(_tmp_mapping.sub_cpnt_size[idx]))
                        )
                        rcd_grp_size.append(int(value))
                    elif _tmp_mapping.attr == 'start':
                        grp_num = grp_num + 1
                        macro = (_tmp_mapping.cpnt_name + '_' + _tmp_mapping.rgn_name[idx] + '_G%03d_start' %(grp_num)).upper()
                        value = (_tmp_mapping.cpnt_name + '_' + _tmp_mapping.rgn_name[idx] + '_G%03d_end' %(grp_num-1)).upper()
                        self.__add_content(hdr_file, '#define %s %s' %(macro, value), False)
                        if idx == 0:
                            self.__sub_cpnt_number_in_one_grp.append(0)
                        # print(len(self.__sub_cpnt_number_in_one_grp))
                    elif len(_tmp_mapping.attr.strip()) == 0:
                        # _tmp_mapping.print()
                        if len(_tmp_mapping.sub_cpnt_size[idx].strip()) != 0:
                            tmp_size = int(_tmp_mapping.sub_cpnt_size[idx])
                        else:
                            tmp_size = 0
                        # print(tmp_size)
                        if grp_size < tmp_size:
                            grp_size = tmp_size

                        if idx == 0:
                            self.__sub_cpnt_number_in_one_grp[-1] = self.__sub_cpnt_number_in_one_grp[-1] + 1
                    elif _tmp_mapping.attr == 'end':
                        grp_size = self.__grp_size_align_and_padding(grp_size)
                        macro = (_tmp_mapping.cpnt_name + '_' + _tmp_mapping.rgn_name[idx] + '_G%03d_len' %(grp_num)).upper()
                        self.__add_content(hdr_file, '#define %s %d' %(macro, grp_size), False)

                        macro = (_tmp_mapping.cpnt_name + '_' + _tmp_mapping.rgn_name[idx] + '_G%03d_end' %(grp_num)).upper()
                        start = (_tmp_mapping.cpnt_name + '_' + _tmp_mapping.rgn_name[idx] + '_G%03d_start' %(grp_num)).upper()
                        value = _tmp_mapping.sub_cpnt_size[idx]
                        self.__add_content(hdr_file, '#define %s (uint32_t*)((uint8_t*)%s + %d)' %(
                            macro,
                            start,
                            grp_size)
                        )
                        rcd_grp_size.append(grp_size)
                        grp_size = 0
                        if self.__sub_cpnt_number_in_one_grp[-1] == 0:
                            del self.__sub_cpnt_number_in_one_grp[-1]
                    elif _tmp_mapping.attr == 'Tail':
                        def _get_region_size_by_name():
                            size = 0xDEADBEEF
                            print('current region name is: %s' %rgn_name)
                            for _tmp_rgn in self.__regions:
                                if _tmp_rgn.name == rgn_name:
                                    size = int(_tmp_rgn.size)
                                    print('recorded region name is: %s' %_tmp_rgn.name)
                                    print('recorded region size is: %d' %size)
                                    break
                            assert(size != 0xDEADBEEF)
                            return size
                        def _set_region_size_by_name(new_size):
                            for _tmp_rgn in self.__regions:
                                # print(_tmp_rgn.name)
                                # print(rgn_name)
                                if _tmp_rgn.name == rgn_name:
                                    # print('sizeof(%s) need to be %d' %(rgn_name, new_size))
                                    _tmp_rgn.size = new_size
                                    break
                        rgn_size = _get_region_size_by_name()
                        '''
                        The script does not actively modify the region size according to the fill size,
                        but truthfully writes precompiled judgments to ensure that the data is filled in correctly.

                        if (sum(rcd_grp_size) > rgn_size):
                            error_flag_cnt = 25
                            print('#'*error_flag_cnt+'Warning'+'#'*error_flag_cnt, file = sys.stderr)
                            print('Size dismatch:', file = sys.stderr)
                            print(' '*4+'Record group size:', rcd_grp_size, file = sys.stderr)
                            print(' '*4+'All group size:', sum(rcd_grp_size), file = sys.stderr)
                            print(' '*4+'Predefined region size:', rgn_size, file = sys.stderr)
                            print('#'*error_flag_cnt+'Warning'+'#'*error_flag_cnt, file = sys.stderr)
                            _set_region_size_by_name(sum(rcd_grp_size))
                        '''

                        op_str = ' + \\\n'.join([(_tmp_mapping.cpnt_name + '_' + _tmp_mapping.rgn_name[idx] + '_G%03d_len' %(grp_idx)).upper() for grp_idx in range(grp_num+1)])
                        self.__add_content(hdr_file, '#if (%s > %d)' %(op_str, rgn_size), False)
                        self.__add_content(hdr_file, '    #error "CFG: Size of %s mismatch with %s !!!"'%(_tmp_mapping.cpnt_name, _tmp_mapping.rgn_name[idx]), False)
                        self.__add_content(hdr_file, '#endif')

                        grp_num = 0
                        rcd_grp_size = list()
                    else:
                        print('Unexpected entry:')
                        print(_tmp_mapping)
                        assert(0)

            self.__add_content(hdr_file, '#ifdef __cplusplus\n}\n#endif')
            self.__add_content(hdr_file, '#endif /* __COMPONENT_PRE_DEFINE_H__ */', False)


    def __gen_src_file(self):
        file_path = self.__out_dir + os.sep + 'ComponentPreDefine.c'
        with open(file_path, 'wt', encoding = 'utf-8') as src_file:
            self.__add_content(src_file, airoha_license)
            self.__add_content(src_file, '#include "memory_attribute.h"', False)
            self.__add_content(src_file, '#include "Component.h"', False)
            self.__add_content(src_file, '#include "ComponentPreDefine.h"')
            self.__add_content(src_file, '#ifdef __cplusplus\nextern "C" {\n#endif')

            # add sub component size check
            self.__add_content(src_file, '/*'+'#'*86+'*/', False)
            self.__add_content(src_file, '/*'+'#'*30 +' ' + 'Sub Component Size Check' + ' ' + '#'*30+'*/', False)
            self.__add_content(src_file, '/*'+'#'*86+'*/', False)
            for _tmp_mapping in self.__mapping:
                hdr_inc_path = _tmp_mapping.hdr_inc_path.strip()
                if len(hdr_inc_path):
                    self.__add_content(src_file, '#include "%s"' %hdr_inc_path, False)
            self.__add_content(src_file, '', False)
            for _tmp_mapping in self.__mapping:
                for macro, size in _tmp_mapping.get_sub_cpnt_size_check_pair():
                    self.__add_content(src_file, 'SIZE_OF_MACRO_EQUAL_TO_NUMBER(%s, %s);' %(macro, size), False)
            self.__add_content(src_file, '')

            # add region related variable
            self.__add_content(src_file, '/*'+'#'*86+'*/', False)
            self.__add_content(src_file, '/*'+'#'*30 +' ' + 'Region Related Variable' + ' ' + '#'*31+'*/', False)
            self.__add_content(src_file, '/*'+'#'*86+'*/', False)
            for _tmp_reg in self.__regions:
                self.__add_content(src_file, '%s uint32_t %s[%s/4+1];' %(
                    _tmp_reg.attr,
                    _tmp_reg.name.strip(),
                    _tmp_reg.size),
                False)
            self.__add_content(src_file, '')
            self.__add_content(src_file, 'const uint32_t RegionAddr[DMM_PREDEFINED_REGION_NUM] = {', False)
            for _tmp_reg in self.__regions:
                self.__add_content(src_file, '    (uint32_t)%s,' %(_tmp_reg.name.strip()), False)
            self.__add_content(src_file, '};')

            # add sub component definition
            self.__add_content(src_file, '/*'+'#'*86+'*/', False)
            self.__add_content(src_file, '/*'+'#'*26 +' ' + 'Mapping Relationship Definition' + ' ' + '#'*27+'*/', False)
            self.__add_content(src_file, '/*'+'#'*86+'*/', False)
            self.__add_content(src_file, 'const uint32_t g_sub_cpnt_to_tcb_idx[] = {', False)
            sub_cpnt_idx = 0
            for idx, value in enumerate(self.__sub_cpnt_number_in_one_grp):
                if(value == 0):
                    # self.__add_content(src_file, '    0xDEADBEFF,', False)
                    continue
                while value != 0:
                    self.__add_content(src_file, '    %5d,    /* %s */' %(idx, self.__sub_cpnt[sub_cpnt_idx]), False)
                    sub_cpnt_idx += 1
                    value -= 1
            self.__add_content(src_file, '};', False)
            self.__add_content(src_file, 'const DMM_TcbInternalData_t g_dmm_tcb[] = {', False)
            # pprint(self.__sub_cpnt_number_in_one_grp)
            start_flag = False
            for _tmp_mapping in self.__mapping:
                if (len(_tmp_mapping.attr.strip()) == 0) and (start_flag == True):
                    start_flag = False
                    self.__add_content(src_file, ' '*4+'{', False)
                    self.__add_content(src_file, ' '*8+'{ %s, %s },' %('Component_'+_tmp_mapping.cpnt_name, _tmp_mapping.grp_number), False)
                    for idx in range(len(_tmp_mapping.rgn_name)):
                        assert(len(_tmp_mapping.grp_number) != 0)
                        common_name = '%s_%s_G%03d_'%(
                                          _tmp_mapping.cpnt_name,
                                          _tmp_mapping.rgn_name[idx],
                                          int(_tmp_mapping.grp_number)
                                    )
                        buff_start = common_name+'start'
                        buff_end = common_name+'end'
                        # print(buff_start, buff_end)
                        self.__add_content(src_file, ' '*8+'{ %s, %s, %s },' %(
                                _tmp_mapping.rgn_name[idx],
                                buff_start.upper(),
                                buff_end.upper()
                            ),
                            False)
                    self.__add_content(src_file, ' '*4+'},', False)
                if _tmp_mapping.attr.upper == 'TAIL':
                    self.__add_content(src_file, '', False)
                if _tmp_mapping.attr.upper() == 'START':
                    start_flag = True
            self.__add_content(src_file, '};', False)
            self.__add_content(src_file, '')
            self.__add_content(src_file, '#ifdef __cplusplus\n}\n#endif')


    def gen_footprint(self):

        fp_cpnt_name = set()
        cpnt_name = str()

        _tmp_fp_entry = None

        def record_cpnt_size(cpnt_name, size):
            for idx, item in enumerate(self.__fp_rcd):
                if item.cpnt_name == cpnt_name:
                    self.__fp_rcd[idx].cpnt_size.append(size)

        # IRAM, DRAM, SYSRAM == 3
        for idx in range(3):
            grp_size = 0
            rcd_grp_size = list()
            for _tmp_mapping in self.__mapping:
                rgn_name = _tmp_mapping.rgn_name[idx]
                if _tmp_mapping.attr == 'Head':
                    value = _tmp_mapping.sub_cpnt_size[idx]
                    rcd_grp_size.append(int(value))

                    if _tmp_mapping.cpnt_name not in fp_cpnt_name:
                        _tmp_fp_entry = DMM_footprint()
                        _tmp_fp_entry.rgn_name = copy.deepcopy(_tmp_mapping.rgn_name)
                        _tmp_fp_entry.cpnt_name = _tmp_mapping.cpnt_name
                        self.__fp_rcd.append(copy.deepcopy(_tmp_fp_entry))

                    cpnt_name = _tmp_mapping.cpnt_name
                    fp_cpnt_name.add(_tmp_mapping.cpnt_name)
                elif _tmp_mapping.attr == 'start':
                    pass
                elif len(_tmp_mapping.attr.strip()) == 0:
                    if len(_tmp_mapping.sub_cpnt_size[idx].strip()) != 0:
                        tmp_size = int(_tmp_mapping.sub_cpnt_size[idx])
                    else:
                        tmp_size = 0
                    if grp_size < tmp_size:
                        grp_size = tmp_size
                elif _tmp_mapping.attr == 'end':
                    grp_size = self.__grp_size_align_and_padding(grp_size)
                    value = _tmp_mapping.sub_cpnt_size[idx]

                    rcd_grp_size.append(grp_size)
                    grp_size = 0
                elif _tmp_mapping.attr == 'Tail':
                    record_cpnt_size(cpnt_name, sum(rcd_grp_size))
                    rcd_grp_size = list()
                else:
                    print('Unexpected entry:')
                    _tmp_mapping.print()
                    assert(0)

        file_path = self.__out_dir + os.sep + 'dmm_footprint.csv'
        with open(file_path, 'wt') as fp_rcd:
            title = ['Region Name', 'Total Size', 'Used Size', 'Remain Size', 'Largest Component']
            self.__add_content(fp_rcd, ','.join(title), need_blank = False)
            def find_largest_cpnt(rgn_name):
                name, size = None, 0
                for item in self.__fp_rcd:
                    try:
                        idx = item.rgn_name.index(rgn_name)
                        size = size if item.cpnt_size[idx] < size else item.cpnt_size[idx]
                        name = item.cpnt_name
                    except:
                        pass
                return size, name

            for item in self.__regions:
                fp_rcd_entry = [str(), str(), str(), str(), str()]
                fp_rcd_entry[0] = item.name
                fp_rcd_entry[1] = str(int((int(item.size)/4+1)*4))
                fp_rcd_entry[2], fp_rcd_entry[4] = find_largest_cpnt(item.name)
                fp_rcd_entry[3] = str(int(fp_rcd_entry[1]) - int(fp_rcd_entry[2]))
                fp_rcd_entry[2] = str(fp_rcd_entry[2])

                self.__add_content(fp_rcd, ','.join(fp_rcd_entry), need_blank = False)

'''
Template cmd:
  ./dmm.py -f pre_define.csv -d feature_opts_list_checkoption.log -o ./gen_dmm
'''

if __name__ == '__main__':
    # The first parameter is the name of the script, which needs to be filtered out.
    args = get_args_from_cli(sys.argv[1:])
    if 'help' in args:
        show_scripts_usage()
        sys.exit(0)

    try:
        dmm_input_file = args['file']
        assert(os.path.exists(dmm_input_file))
    except:
        show_scripts_usage()
        sys.exit(-1)

    out_dir = '.' if 'out_dir' not in args else args['out_dir']
    feature = None if 'feature' not in args else args['feature']
    dmm_parser = DMM_parser(dmm_input_file, feature, out_dir)
    dmm_parser.gen_code()
    dmm_parser.gen_footprint()
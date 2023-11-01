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


def get_nvdm_parser_args(argv):
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


class DMM_scene:
    def __init__(self) -> None:
        self.cpnt_name = str()                  # component name
        self.rgn_name = str()                   # region name/variable name
        self.grp_id = str()                     # group id
        self.attr = str()                       # special pattern to assist script
        self.sub_cpnt_name = str()              # sub component name
        self.sub_cpnt_size = str()              # sub component size
        self.feature_type = str()               # feature type
        self.feature_expr = str()               # feature option expression
        self.note = str()                       # note information

    def print(self):
        print('{:<10} {:<20} {:<3} {:<8} {:<10} {:<8}'.format(
                self.cpnt_name,
                self.rgn_name,
                self.grp_id,
                self.attr,
                self.sub_cpnt_name,
                self.sub_cpnt_size
            )
        )


class DMM_mapping:
    def __init__(self) -> None:
        self.scenes = []
        self.name = str()

    def print(self):
        print('Mapping name: %s' %(self.name))
        if len(self.scenes):
            print('Scenes info:')
            for scene in self.scenes:
                scene.print()

    def traverse_all_sub_cpnt_name(self):
        for scene in self.scenes:
            if scene.sub_cpnt_name:
                yield scene.sub_cpnt_name


class DMM_record:
    def __init__(self) -> None:
        self.rgn_name = str()
        self.grp_number = int()
        self.cpnt_name = str()
        self.sub_cpnt_num = list()

    def print(self):
        print('{:<20} {:<20} {:<8}'.format(
            self.rgn_name,
            self.cpnt_name,
            self.grp_number
            ), self.sub_cpnt_num
        )


class DMM_parser:
    def __init__(self, dmm_file = None, feature_option_file = None, out_dir = None) -> None:
        assert(os.path.exists(dmm_file))
        assert(os.path.exists(out_dir))
        self.__regions = []
        self.__component = []
        self.__mapping = []
        self.__out_dir = out_dir
        self.__record = []
        self.__feature_list = feature_option_file
        if feature_option_file is not None:
            assert(os.path.exists(feature_option_file))
            with open(feature_option_file, 'r', encoding = 'utf-8') as feature_list:
                self.__feature_list = feature_list.read().replace(' ', '')
            # print(self.__feature_list)

        with open(dmm_file, 'r', encoding = 'utf-8') as src_info:
            class _info_type(Enum):
                unknown = 0
                region = 1
                component_type = 2
                mapping = 3
            info_type = _info_type.unknown
            reader = csv.reader(src_info)
            line_num = 0
            _tmp_mapping = None
            for line_ctx in reader:
                line_num = line_num + 1
                # print(line_ctx)
                if (len(line_ctx) >= 3) and \
                 (line_ctx[0] == 'Type' ) and \
                 (line_ctx[1] == 'Region name') and \
                 (line_ctx[2] == 'size'):
                    info_type = _info_type.region
                    continue
                elif (len(line_ctx) >= 1) and \
                 (line_ctx[0] == 'TotalComponentType_t' ):
                    info_type = _info_type.component_type
                    continue
                elif (len(line_ctx) >= 6) and \
                 (line_ctx[1] == 'Region name' ) and \
                 (line_ctx[2] == 'Group ID' ) and \
                 (line_ctx[3] == 'attr' ) and \
                 (line_ctx[4] == 'Sub component name' ) and \
                 (line_ctx[5] == 'size' ) and \
                 (line_ctx[6] == 'Feature Type' ) and \
                 (line_ctx[7] == 'Feature Option' ):
                    info_type = _info_type.mapping
                    _tmp_mapping = DMM_mapping()
                    _tmp_mapping.name = line_ctx[0]
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
                        _tmp_scene = DMM_scene()
                        _tmp_scene.cpnt_name = line_ctx[0]
                        _tmp_scene.rgn_name = line_ctx[1]
                        _tmp_scene.grp_id = line_ctx[2]
                        _tmp_scene.attr = line_ctx[3]
                        _tmp_scene.sub_cpnt_name = line_ctx[4]
                        _tmp_scene.sub_cpnt_size = line_ctx[5]
                        _tmp_scene.feature_type = line_ctx[6]
                        _tmp_scene.feature_expr = line_ctx[7]
                        _tmp_scene.note = line_ctx[8]

                        if self.__evaluation_expr(_tmp_scene.feature_expr, self.__feature_list):
                            _tmp_mapping.scenes.append(copy.deepcopy(_tmp_scene))
                    else:
                        print('%d:' %(line_num))
                        print(line_ctx)
                        assert(0)
            # SYSRAM may have no content, just reflected in the last line.
            if _tmp_mapping is not None:
                self.__mapping.append(copy.deepcopy(_tmp_mapping))
                _tmp_mapping = None
                info_type = _info_type.unknown

    def __evaluation_expr(self, expr, feature_list):
        expr = expr.strip().replace(' ', '')
        if len(expr) == 0:
            return True
        result = 'False'
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

    def print(self):
        for rg in self.__regions:
            print('{:<25} {:<20} {:<10}'.format(rg.attr, rg.name, rg.size))
        print(self.__component)
        for mapping in self.__mapping:
            mapping.print()

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
            self.__add_content(hdr_file, '/* The number of REGIONs filled in by the user and the variable name corresponding to each region. */', False)
            self.__add_content(hdr_file, '#define DMM_PREDEFINED_REGION_NUM %dU' %(len(self.__regions)), False)

            # add region related infortmation
            for _tmp_reg in self.__regions:
                self.__add_content(hdr_file, 'extern uint32_t %s[];' %(_tmp_reg.name.strip()), False)
            self.__add_content(hdr_file, '')

            # add component enum definition
            self.__add_content(hdr_file, 'typedef enum{', False)
            for _tmp_cpnt in self.__component:
                self.__add_content(hdr_file, ' '*4+_tmp_cpnt.strip()+',', False)
            self.__add_content(hdr_file, ' '*4+'Com_Max,', False)
            self.__add_content(hdr_file, '} TotalComponentType_t;')

            # add sub component definition
            for _tmp_mapping in self.__mapping:
                self.__add_content(hdr_file, 'typedef enum{', False)
                for sub_cpnt_name in _tmp_mapping.traverse_all_sub_cpnt_name():
                    self.__add_content(hdr_file, ' '*4+sub_cpnt_name.strip()+',', False)
                self.__add_content(hdr_file, ' '*4+_tmp_mapping.name[0:_tmp_mapping.name.find('Component_t')]+'_Max,', False)
                self.__add_content(hdr_file, '} %s;' %(_tmp_mapping.name))

            # add group start/end address macro
            for _tmp_mapping in self.__mapping:
                grp_num = 0
                grp_size = 0
                cpnt_name = str()
                sub_cpnt_num = list()
                rgn_name = str()
                rcd_grp_size = list()
                self.__add_content(hdr_file, '/*'+'#'*30 + '#'*(len(_tmp_mapping.name)+2) + '#'*30+'*/', False)
                self.__add_content(hdr_file, '/*'+'#'*30 +' ' + _tmp_mapping.name+ ' ' + '#'*30+'*/', False)
                self.__add_content(hdr_file, '/*'+'#'*30 + '#'*(len(_tmp_mapping.name)+2) + '#'*30+'*/', False)
                for _tmp_entry in _tmp_mapping.scenes:
                    if _tmp_entry.attr == 'Head':
                        cpnt_name = _tmp_entry.cpnt_name
                        rgn_name = _tmp_entry.rgn_name

                        macro = (_tmp_entry.cpnt_name + '_' + _tmp_entry.rgn_name + '_G000_start').upper()
                        value = _tmp_entry.rgn_name
                        self.__add_content(hdr_file, '#define %s %s' %(macro, value), False)

                        macro = (_tmp_entry.cpnt_name + '_' + _tmp_entry.rgn_name + '_G000_len').upper()
                        value = _tmp_entry.sub_cpnt_size
                        self.__add_content(hdr_file, '#define %s %s' %(macro, value), False)

                        macro = (_tmp_entry.cpnt_name + '_' + _tmp_entry.rgn_name + '_G000_end').upper()
                        value = _tmp_entry.sub_cpnt_size
                        self.__add_content(hdr_file, '#define %s (uint32_t*)((uint8_t*)%s + %d)' %(
                            macro,
                            _tmp_entry.rgn_name,
                            int(_tmp_entry.sub_cpnt_size))
                        )
                        rcd_grp_size.append(int(value))
                    elif _tmp_entry.attr == 'start':
                        grp_num = grp_num + 1
                        macro = (_tmp_entry.cpnt_name + '_' + _tmp_entry.rgn_name + '_G%03d_start' %(grp_num)).upper()
                        value = (_tmp_entry.cpnt_name + '_' + _tmp_entry.rgn_name + '_G%03d_end' %(grp_num-1)).upper()
                        self.__add_content(hdr_file, '#define %s %s' %(macro, value), False)
                        sub_cpnt_num.append(0)
                    elif len(_tmp_entry.attr.strip()) == 0:
                        # print(_tmp_entry.sub_cpnt_size)
                        tmp_size = int(_tmp_entry.sub_cpnt_size)
                        # print(tmp_size)
                        if grp_size < tmp_size:
                            grp_size = tmp_size

                        sub_cpnt_num[-1] = sub_cpnt_num[-1] + 1
                    elif _tmp_entry.attr == 'end':
                        macro = (_tmp_entry.cpnt_name + '_' + _tmp_entry.rgn_name + '_G%03d_len' %(grp_num)).upper()
                        self.__add_content(hdr_file, '#define %s %d' %(macro, grp_size), False)

                        macro = (_tmp_entry.cpnt_name + '_' + _tmp_entry.rgn_name + '_G%03d_end' %(grp_num)).upper()
                        start = (_tmp_entry.cpnt_name + '_' + _tmp_entry.rgn_name + '_G%03d_start' %(grp_num)).upper()
                        value = _tmp_entry.sub_cpnt_size
                        self.__add_content(hdr_file, '#define %s (uint32_t*)((uint8_t*)%s + %d)' %(
                            macro,
                            start,
                            grp_size)
                        )
                        rcd_grp_size.append(grp_size)
                        grp_size = 0
                    elif _tmp_entry.attr == 'Tail':
                        def _get_region_size_by_name():
                            size = 0xDEADBEEF
                            for _tmp_rgn in self.__regions:
                                # print(_tmp_rgn.name)
                                # print(rgn_name)
                                if _tmp_rgn.name == rgn_name:
                                    size = int(_tmp_rgn.size)
                                    break
                            assert(size != 0xDEADBEEF)
                            return size
                        rgn_size = _get_region_size_by_name()
                        if (sum(rcd_grp_size) > rgn_size):
                            error_flag_cnt = 25
                            print('#'*error_flag_cnt+'ERROR'+'#'*error_flag_cnt)
                            print('Size dismatch:')
                            print(' '*4+'Record group size:', rcd_grp_size)
                            print(' '*4+'All group size:', sum(rcd_grp_size))
                            print(' '*4+'Predefined region size:', rgn_size)
                            print('#'*error_flag_cnt+'ERROR'+'#'*error_flag_cnt)
                        self.__add_content(hdr_file, '#if (%d > %d)' %(sum(rcd_grp_size), rgn_size), False)
                        self.__add_content(hdr_file, '    #error "CFG: The combined size of all groups exceeds the predetermined memory pool size."', False)
                        self.__add_content(hdr_file, '    #error "CFG: Sum of all group is %d bytes."' %(sum(rcd_grp_size)), False)
                        self.__add_content(hdr_file, '    #error "CFG: Predefined memory pool is %d bytes."' %(rgn_size), False)
                        self.__add_content(hdr_file, '#endif')

                        _tmp_record = DMM_record()
                        _tmp_record.rgn_name = rgn_name
                        _tmp_record.grp_number = grp_num
                        _tmp_record.cpnt_name = cpnt_name
                        _tmp_record.sub_cpnt_num = sub_cpnt_num
                        self.__record.append(copy.deepcopy(_tmp_record))

                        grp_num = 0
                        rcd_grp_size = list()
                        sub_cpnt_num = list()
                    else:
                        print('Unexpected entry:')
                        print(_tmp_entry)
                        assert(0)
                if len(_tmp_mapping.scenes) == 0:
                    self.__add_content(hdr_file, '', False)
            self.__add_content(hdr_file, '#ifdef __cplusplus\n}\n#endif')
            self.__add_content(hdr_file, '#endif /* __COMPONENT_PRE_DEFINE_H__ */', False)

    def __gen_src_file(self):
        file_path = self.__out_dir + os.sep + 'ComponentPreDefine.c'
        with open(file_path, 'wt', encoding = 'utf-8') as src_file:
            self.__add_content(src_file, airoha_license)
            self.__add_content(src_file, '#include "memory_attribute.h"', False)
            self.__add_content(src_file, '#include "Component.h"', False)
            self.__add_content(src_file, '#include "ComponentPreDefine.h"')

            # add region related variable
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
            for _tmp_mapping in self.__mapping:
                ram_type = _tmp_mapping.name[0:_tmp_mapping.name.find('Component_t')].lower()
                self.__add_content(src_file, 'const DMM_TcbInternalData_t g_dmm_%s_tcb[] = {' %(ram_type), False)
                self.__add_content(src_file, '    /* region start address, group start address, group end address, Component enum, Group number */', False)
                for _tmp_rcd_idx, _tmp_rcd in enumerate(self.__record, 1):
                    # _tmp_rcd.print()
                    need_add_blank_line = False
                    for idx in range(_tmp_rcd.grp_number):
                        if _tmp_rcd.rgn_name.lower().find(ram_type) != -1:
                            for sub_cpnt_idx in range(_tmp_rcd.sub_cpnt_num[idx]):
                                self.__add_content(src_file, ' '*4+'{ %s, %s, %s, %s, %d },' %(
                                        _tmp_rcd.rgn_name,
                                        (_tmp_rcd.cpnt_name+'_'+_tmp_rcd.rgn_name+'_G%03d'%(idx+1) + '_start').upper(),
                                        (_tmp_rcd.cpnt_name+'_'+_tmp_rcd.rgn_name+'_G%03d'%(idx+1) + '_end').upper(),
                                        _tmp_rcd.cpnt_name,
                                        idx+1
                                    ),
                                    False
                                )
                                need_add_blank_line = True
                    if need_add_blank_line:
                        self.__add_content(src_file, '', False)
                self.__add_content(src_file, '};', False)
                self.__add_content(src_file, '')


if __name__ == '__main__':
    # The first parameter is the name of the script, which needs to be filtered out.
    args = get_nvdm_parser_args(sys.argv[1:])
    if 'help' in args:
        show_scripts_usage()
        sys.exit()

    try:
        dmm_input_file = args['file']
        assert(os.path.exists(dmm_input_file))
    except:
        show_scripts_usage()
        sys.exit()

    out_dir = '.' if 'out_dir' not in args else args['out_dir']
    feature = None if 'feature' not in args else args['feature']
    dmm_parser = DMM_parser(dmm_input_file, feature, out_dir)
    # dmm_parser.print()
    dmm_parser.gen_code()


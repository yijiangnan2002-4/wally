/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

/**
 * @brief This file used to skip the build warning
 * How to use : please add #include "skip_warning.h" into the C/C++ files that do
 * not wish to generate the build waring in the target files.
 */

#ifndef __SKIP_WARNING_H__
#define __SKIP_WARNING_H__

_Pragma("GCC diagnostic ignored \"-Wimplicit-function-declaration\"");
_Pragma("GCC diagnostic ignored \"-Wunused-variable\"");
_Pragma("GCC diagnostic ignored \"-Waddress\"");
_Pragma("GCC diagnostic ignored \"-Wbool-compare\"");
_Pragma("GCC diagnostic ignored \"-Wbool-operation\"");
_Pragma("GCC diagnostic ignored \"-Wchar-subscripts\"");
_Pragma("GCC diagnostic ignored \"-Wcomment\"");
_Pragma("GCC diagnostic ignored \"-Wduplicate-decl-specifier\"");
_Pragma("GCC diagnostic ignored \"-Wformat\"");
_Pragma("GCC diagnostic ignored \"-Wint-in-bool-context\"");
_Pragma("GCC diagnostic ignored \"-Wimplicit\"");
_Pragma("GCC diagnostic ignored \"-Wimplicit-int\"");
_Pragma("GCC diagnostic ignored \"-Wimplicit-function-declaration\"");
_Pragma("GCC diagnostic ignored \"-Wlogical-not-parentheses\"");
_Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"");
_Pragma("GCC diagnostic ignored \"-Wmemset-elt-size\"");
_Pragma("GCC diagnostic ignored \"-Wmemset-transposed-args\"");
_Pragma("GCC diagnostic ignored \"-Wmisleading-indentation\"");
_Pragma("GCC diagnostic ignored \"-Wmissing-attributes\"");
_Pragma("GCC diagnostic ignored \"-Wmissing-braces\"");
_Pragma("GCC diagnostic ignored \"-Wmultistatement-macros\"");
_Pragma("GCC diagnostic ignored \"-Wnonnull\"");
_Pragma("GCC diagnostic ignored \"-Wnonnull-compare\"");
_Pragma("GCC diagnostic ignored \"-Wopenmp-simd\"");
_Pragma("GCC diagnostic ignored \"-Wparentheses\"");
_Pragma("GCC diagnostic ignored \"-Wpointer-sign\"");
_Pragma("GCC diagnostic ignored \"-Wrestrict\"");
_Pragma("GCC diagnostic ignored \"-Wreturn-type\"");
_Pragma("GCC diagnostic ignored \"-Wsequence-point\"");
_Pragma("GCC diagnostic ignored \"-Wsizeof-pointer-div\"");
_Pragma("GCC diagnostic ignored \"-Wsizeof-pointer-memaccess\"");
_Pragma("GCC diagnostic ignored \"-Wstrict-aliasing\"");
_Pragma("GCC diagnostic ignored \"-Wstrict-overflow=1\"");
_Pragma("GCC diagnostic ignored \"-Wswitch\"");
_Pragma("GCC diagnostic ignored \"-Wtautological-compare\"");
_Pragma("GCC diagnostic ignored \"-Wtrigraphs\"");
_Pragma("GCC diagnostic ignored \"-Wuninitialized\"");
_Pragma("GCC diagnostic ignored \"-Wunused-function\"");
_Pragma("GCC diagnostic ignored \"-Wunused-label\"");
_Pragma("GCC diagnostic ignored \"-Wunused-value\"");
_Pragma("GCC diagnostic ignored \"-Wunused-variable\"");
_Pragma("GCC diagnostic ignored \"-Wvolatile-register-var\"");
_Pragma("GCC diagnostic ignored \"-Wint-conversion\"");

#endif /* __SKIP_WARNING_H__ */

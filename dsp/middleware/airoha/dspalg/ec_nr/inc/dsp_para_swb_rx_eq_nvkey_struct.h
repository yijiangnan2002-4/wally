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
#ifndef _DSP_PARA_SWB_RX_EQ_H_
#define _DSP_PARA_SWB_RX_EQ_H_

#include "types.h"

/**
 * @brief DSP Parameter For Super Wideband Speech Receiving Equlizer
 * @KeyID 0xE16C
 */

typedef struct stru_dsp_swb_rx_eq_para_s {
    U8  ENABLE;                         /**< @Value 0x01 @Desc 1 */
    U8  REVISION;                       /**< @Value 0x01 @Desc 1 */
    U16 SWB_RXEQ_COF_0;                 /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_1;                 /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_2;                 /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_3;                 /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_4;                 /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_5;                 /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_6;                 /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_7;                 /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_8;                 /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_9;                 /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_10;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_11;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_12;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_13;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_14;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_15;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_16;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_17;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_18;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_19;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_20;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_21;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_22;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_23;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_24;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_25;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_26;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_27;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_28;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_29;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_30;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_31;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_32;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_33;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_34;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_35;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_36;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_37;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_38;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_39;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_40;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_41;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_42;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_43;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_44;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_45;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_46;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_47;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_48;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_49;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_50;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_51;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_52;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_53;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_54;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_55;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_56;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_57;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_58;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_59;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_60;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_61;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_62;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_63;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_64;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_65;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_66;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_67;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_68;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_69;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_70;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_71;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_72;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_73;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_74;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_75;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_76;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_77;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_78;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_79;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_80;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_81;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_82;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_83;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_84;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_85;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_86;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_87;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_88;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_89;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_90;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_91;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_92;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_93;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_94;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_95;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_96;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_97;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_98;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_99;                /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_100;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_101;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_102;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_103;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_104;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_105;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_106;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_107;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_108;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_109;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_110;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_111;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_112;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_113;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_114;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_115;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_116;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_117;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_118;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_119;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_120;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_121;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_122;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_123;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_124;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_125;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_126;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_127;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_128;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_129;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_130;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_131;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_132;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_133;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_134;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_135;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_136;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_137;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_138;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_139;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_140;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_141;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_142;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_143;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_144;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_145;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_146;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_147;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_148;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_149;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_150;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_151;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_152;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_153;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_154;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_155;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_156;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_157;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_158;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_159;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_160;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_161;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_162;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_163;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_164;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_165;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_166;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_167;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_168;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_169;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_170;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_171;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_172;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_173;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_174;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_175;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_176;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_177;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_178;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_179;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_180;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_181;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_182;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_183;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_184;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_185;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_186;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_187;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_188;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_189;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_190;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_191;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_192;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_193;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_194;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_195;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_196;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_197;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_198;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_199;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_200;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_201;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_202;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_203;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_204;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_205;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_206;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_207;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_208;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_209;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_210;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_211;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_212;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_213;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_214;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_215;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_216;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_217;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_218;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_219;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_220;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_221;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_222;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_223;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_224;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_225;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_226;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_227;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_228;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_229;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_230;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_231;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_232;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_233;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_234;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_235;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_236;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_237;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_238;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_239;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_240;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_241;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_242;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_243;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_244;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_245;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_246;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_247;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_248;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_249;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_250;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_251;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_252;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_253;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_254;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_255;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_256;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_257;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_258;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_259;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_260;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_261;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_262;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_263;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_264;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_265;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_266;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_267;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_268;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_269;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_270;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_271;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_272;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_273;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_274;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_275;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_276;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_277;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_278;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_279;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_280;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_281;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_282;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_283;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_284;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_285;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_286;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_287;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_288;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_289;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_290;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_291;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_292;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_293;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_294;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_295;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_296;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_297;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_298;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_299;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_300;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_301;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_302;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_303;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_304;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_305;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_306;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_307;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_308;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_309;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_310;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_311;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_312;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_313;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_314;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_315;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_316;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_317;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_318;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_319;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_320;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_321;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_322;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_323;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_324;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_325;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_326;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_327;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_328;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_329;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_330;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_331;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_332;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_333;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_334;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_335;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_336;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_337;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_338;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_339;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_340;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_341;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_342;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_343;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_344;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_345;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_346;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_347;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_348;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_349;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_350;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_351;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_352;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_353;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_354;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_355;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_356;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_357;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_358;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_359;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_360;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_361;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_362;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_363;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_364;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_365;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_366;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_367;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_368;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_369;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_370;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_371;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_372;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_373;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_374;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_375;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_376;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_377;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_378;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_379;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_380;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_381;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_382;               /**< @Value 0x1000 @Desc 1 */
    U16 SWB_RXEQ_COF_383;               /**< @Value 0x1000 @Desc 1 */

} PACKED DSP_PARA_SWB_RX_EQ_STRU;

#endif /* _DSP_PARA_SWB_RX_EQ_H_ */

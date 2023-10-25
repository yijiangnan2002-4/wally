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
#ifndef _DSP_PARA_WB_TX_EQ_H_
#define _DSP_PARA_WB_TX_EQ_H_

#include "types.h"

/**
 * @brief Parameter for DSP WB TX EQ
 * @KeyID 0xE162
 */

typedef struct stru_dsp_wb_tx_eq_para_s {
    U8  ENABLE;                         /**< @Value 0x01 @Desc 1 */
    U8  REVISION;                       /**< @Value 0x01 @Desc 1 */
    U16 WB_TXEQ_COF_0;                  /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_1;                  /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_2;                  /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_3;                  /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_4;                  /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_5;                  /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_6;                  /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_7;                  /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_8;                  /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_9;                  /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_10;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_11;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_12;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_13;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_14;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_15;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_16;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_17;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_18;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_19;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_20;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_21;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_22;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_23;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_24;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_25;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_26;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_27;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_28;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_29;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_30;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_31;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_32;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_33;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_34;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_35;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_36;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_37;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_38;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_39;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_40;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_41;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_42;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_43;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_44;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_45;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_46;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_47;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_48;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_49;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_50;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_51;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_52;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_53;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_54;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_55;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_56;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_57;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_58;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_59;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_60;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_61;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_62;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_63;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_64;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_65;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_66;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_67;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_68;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_69;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_70;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_71;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_72;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_73;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_74;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_75;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_76;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_77;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_78;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_79;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_80;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_81;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_82;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_83;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_84;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_85;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_86;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_87;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_88;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_89;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_90;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_91;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_92;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_93;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_94;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_95;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_96;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_97;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_98;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_99;                 /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_100;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_101;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_102;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_103;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_104;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_105;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_106;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_107;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_108;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_109;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_110;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_111;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_112;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_113;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_114;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_115;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_116;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_117;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_118;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_119;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_120;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_121;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_122;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_123;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_124;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_125;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_126;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_127;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_128;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_129;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_130;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_131;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_132;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_133;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_134;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_135;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_136;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_137;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_138;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_139;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_140;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_141;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_142;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_143;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_144;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_145;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_146;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_147;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_148;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_149;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_150;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_151;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_152;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_153;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_154;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_155;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_156;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_157;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_158;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_159;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_160;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_161;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_162;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_163;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_164;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_165;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_166;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_167;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_168;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_169;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_170;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_171;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_172;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_173;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_174;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_175;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_176;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_177;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_178;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_179;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_180;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_181;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_182;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_183;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_184;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_185;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_186;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_187;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_188;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_189;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_190;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_191;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_192;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_193;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_194;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_195;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_196;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_197;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_198;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_199;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_200;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_201;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_202;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_203;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_204;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_205;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_206;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_207;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_208;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_209;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_210;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_211;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_212;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_213;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_214;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_215;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_216;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_217;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_218;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_219;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_220;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_221;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_222;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_223;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_224;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_225;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_226;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_227;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_228;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_229;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_230;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_231;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_232;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_233;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_234;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_235;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_236;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_237;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_238;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_239;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_240;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_241;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_242;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_243;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_244;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_245;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_246;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_247;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_248;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_249;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_250;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_251;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_252;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_253;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_254;                /**< @Value 0x1000 @Desc 1 */
    U16 WB_TXEQ_COF_255;                /**< @Value 0x1000 @Desc 1 */
} PACKED DSP_PARA_WB_TX_EQ_STRU;


#endif /* _DSP_PARA_WB_TX_EQ_H_ */

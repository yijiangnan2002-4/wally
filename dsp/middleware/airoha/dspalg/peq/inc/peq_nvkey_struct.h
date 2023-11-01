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

#ifndef _PEQ_NVKEY_STRUCT_H_
#define _PEQ_NVKEY_STRUCT_H_

#define FW_MAX_ELEMENT      (4)
#define FW_MAX_BANDS        (10)
#define MAX_BANDS           (15)

typedef struct {
    S16 RUN_N;
    S32 bq1X[MAX_BANDS];
    S32 bq2X[MAX_BANDS];
    S32 bq3X[MAX_BANDS];
    S32 aq2X[MAX_BANDS];
    S32 aq3X[MAX_BANDS];

    S32 output_gain ;

    //L
    S32 Xstate1_l[MAX_BANDS];
    S32 Xstate2_l[MAX_BANDS] ;
    S32 Ystate1_l[MAX_BANDS] ;
    S32 Ystate2_l[MAX_BANDS] ;

    //R
    S32 Xstate1_r[MAX_BANDS] ;
    S32 Xstate2_r[MAX_BANDS] ;
    S32 Ystate1_r[MAX_BANDS] ;
    S32 Ystate2_r[MAX_BANDS] ;

    //Zero index
    S32 ZeroIndex_L;
    S32 ZeroIndex_R;
} PEQ_ST;

typedef struct dsp_peq_param_s {
    U16 elementID;
    U16 numOfParameter;
    S16 peq_inter_param[5 * FW_MAX_BANDS * 2 + 2 + 1];
} PACKED DSP_PEQ_PARAM_t;

typedef struct dsp_peq_nvkey_s {
    U16 numOfElement;
    U16 peqAlgorithmVer;
    DSP_PEQ_PARAM_t peq_element_param[FW_MAX_ELEMENT];
} PACKED DSP_PEQ_NVKEY_t;

#endif /* _PEQ_NVKEY_STRUCT_H_ */

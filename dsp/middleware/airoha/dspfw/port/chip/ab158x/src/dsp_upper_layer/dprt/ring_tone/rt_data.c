/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#include "rt_data.h"

////////////////////////////////////////////////////////////////////////////////
// Enumerations ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum {
    NOTE_LA3 = 0x02,
    NOTE_LA3_HIGH,
    NOTE_SI3,
    NOTE_DO4 = 0x06,
    NOTE_DO4_HIGH,
    NOTE_RE4,
    NOTE_RE4_HIGH,
    NOTE_ME4,
    NOTE_FA4 = 0x0C,
    NOTE_FA4_HIGH,
    NOTE_SO4,
    NOTE_SO4_HIGH,
    NOTE_LA4,
    NOTE_LA4_HIGH,
    NOTE_SI4,
    NOTE_SI4_HIGH,
    NOTE_DO5 = 0x14,
    NOTE_DO5_HIGH,
    NOTE_RE5,
    NOTE_RE5_HIGH,
    NOTE_ME5,
    NOTE_FA5 = 0x1A,
    NOTE_FA5_HIGH,
    NOTE_SO5,
    NOTE_SO5_HIGH,
    NOTE_LA5,
    NOTE_LA5_HIGH,
    NOTE_SI5,
    NOTE_SI5_HIGH,
    NOTE_DO6 = 0x22,
    NOTE_DO6_HIGH,
    NOTE_RE6,
    NOTE_RE6_HIGH,
    NOTE_ME6,
    NOTE_FA6 = 0x28,
    NOTE_FA6_HIGH,
    NOTE_SO6,
    NOTE_SO6_HIGH,
    NOTE_LA6,
    NOTE_LA6_HIGH,
    NOTE_SI6,
    NOTE_SI6_HIGH,
    NOTE_DO7 = 0x30,
    NOTE_DO7_HIGH,
    NOTE_RE7,
    NOTE_RE7_HIGH,
    NOTE_ME7,
    NOTE_FA7 = 0x36,
    NOTE_FA7_HIGH,
    NOTE_SO7,
    NOTE_SO7_HIGH,
    NOTE_LA7,
    NOTE_LA7_HIGH,
    NOTE_SI7,
    NOTE_SI7_HIGH,

    NOTE_DO8 = 0x3E,  // note frequency which is higher than this is supported only when DSP sample rate is set on 16k
    NOTE_DO8_HIGH,

    NOTE_REST = 0xC0,
};

#define METRONOME_1_16_100MS (5)
#define METRONOME_1_8_200MS (10)
#define METRONOME_3_16_300MS (15)
#define METRONOME_1_4_400MS (20)
#define METRONOME_3_8_600MS (30)
#define METRONOME_1_2_800MS (40)
#define METRONOME_3_4_1200MS (60)
#define METRONOME_1_1600MS (80)
#define METRONOME_2_200MS (160)
#define VOL_BEGIN   0x4F
#define VOL_END     0x4F

////////////////////////////////////////////////////////////////////////////////
// Definitions /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// External Variables //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Function Declarations ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
// Ringtine Data
///////////////////////////////////////////////////////////////////////
#define RTDAT_SHORT_1_NOTE_1        NOTE_ME4, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END,
#define RTDAT_SHORT_1_NOTE_2        NOTE_LA4, METRONOME_1_2_800MS, VOL_BEGIN, VOL_END,
#define RTDAT_SHORT_1_NOTE_3        NOTE_ME5, METRONOME_1_2_800MS, VOL_BEGIN, VOL_END,
#define RTDAT_SHORT_1_NOTE_4        NOTE_SI5, METRONOME_1_2_800MS, VOL_BEGIN, VOL_END,
#define RTDAT_SHORT_2_NOTE          NOTE_ME5, METRONOME_1_2_800MS, VOL_BEGIN, VOL_END, NOTE_REST, METRONOME_3_16_300MS, VOL_BEGIN, VOL_END, NOTE_ME5, METRONOME_1_2_800MS, VOL_BEGIN, VOL_END,
#define RTDAT_MEDIAN_1_NOTE         NOTE_SO6, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END,
#define RTDAT_LONG_1_NOTE_HIGH      NOTE_RE5, METRONOME_3_8_600MS, VOL_BEGIN, VOL_END,
#define RTDAT_LONG_1_NOTE_LOW       NOTE_DO4, METRONOME_3_8_600MS, VOL_BEGIN, VOL_END,
#define RTDAT_VERYLONG_1_NOTE_HIGH  NOTE_RE5, METRONOME_1_2_800MS, VOL_BEGIN, VOL_END,
#define RTDAT_VERYLONG_1_NOTE_LOW   NOTE_DO4, METRONOME_1_2_800MS, VOL_BEGIN, VOL_END,
#define RTDAT_RISING_2_NOTE         NOTE_FA4, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_ME5, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END,
#define RTDAT_RISING_4_NOTE         NOTE_FA4, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_LA4, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_DO5, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_ME5, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END,
#define RTDAT_FALLING_2_NOTE        NOTE_ME5, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_FA4, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END,
#define RTDAT_FALLING_4_NOTE        NOTE_ME5, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_DO5, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_LA4, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_FA4, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END,
#define RTDAT_LONG_RING_A           NOTE_SI5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_LA5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_SI5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_SI5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_RE5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_LA5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_RE5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_REST, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END,
#define RTDAT_LONG_RING_B           NOTE_ME5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_REST, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_ME5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_REST, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_ME5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_REST, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_DO5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_REST, METRONOME_3_4_1200MS, VOL_BEGIN, VOL_END, NOTE_REST, METRONOME_1_1600MS, VOL_BEGIN, VOL_END,
#define RTDAT_LONG_RING_C           NOTE_RE5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_SI5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_RE6, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_SO6, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_LA5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_FA6, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_SO6, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_REST, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_RE5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_REST, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_RE5, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END, NOTE_REST, METRONOME_1_8_200MS, VOL_BEGIN, VOL_END,
#define RTDAT_USER_DEFINED_A        NOTE_FA4, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_LA4, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_DO5, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END, NOTE_ME5, METRONOME_1_16_100MS, VOL_BEGIN, VOL_END,


#define RTDAT_ALIGN_16(name)            ((((name) + (16-1)) / 16) * 16)
#define RTDAT_NAME(name)                _rtdat_##name
#define RTDAT_SIZE(name)                (sizeof( RTDAT_NAME(name)))
#define RTDAT_ALIGN_SIZE(name)          (RTDAT_ALIGN_16(RTDAT_SIZE(name)))
#define RTDAT_OFFSET(name)              RTOFF_##name
#define RTDAT_OFFSET_NEXT(name)         ( RTDAT_OFFSET(name) + RTDAT_ALIGN_SIZE(name) )
#define RTDAT_RECORD(name)              { RTDAT_OFFSET(name), RTDAT_SIZE(name) }


U32 ALIGN(4) CODE gDriver_ringtone_data_init = {
#if (FEA_SUPP_DSP_RINGTONE)
    TOTAL_BEEP_PATTERNS,
#else
    0,
#endif
};



#if (FEA_SUPP_DSP_RINGTONE)
/*
Ringtone Record Begins, declare forward let linker locate record table before ringtone data
*/
extern RINGTONE_RECORD_TYPE ALIGN(4) CODE _ringtoneRecord[TOTAL_BEEP_PATTERNS];

/*
Ringtone Data Begins
*/
#undef RTONEDEF
#define RTONEDEF(name)  U8 ALIGN(16) CODE RTDAT_NAME(name) [] = { RTDAT_##name };
RINGTONE_LIST

#define RTDAT_OFFSET_START  RTDAT_ALIGN_16(RTDAT_ALIGN_16(4) + TOTAL_BEEP_PATTERNS * sizeof(RINGTONE_RECORD_TYPE))

enum {
    RTDAT_OFFSET(SHORT_1_NOTE_1)        = RTDAT_OFFSET_START,
    RTDAT_OFFSET(SHORT_1_NOTE_2)        = RTDAT_OFFSET_NEXT(SHORT_1_NOTE_1),
    RTDAT_OFFSET(SHORT_1_NOTE_3)        = RTDAT_OFFSET_NEXT(SHORT_1_NOTE_2),
    RTDAT_OFFSET(SHORT_1_NOTE_4)        = RTDAT_OFFSET_NEXT(SHORT_1_NOTE_3),
    RTDAT_OFFSET(SHORT_2_NOTE)          = RTDAT_OFFSET_NEXT(SHORT_1_NOTE_4),
    RTDAT_OFFSET(MEDIAN_1_NOTE)         = RTDAT_OFFSET_NEXT(SHORT_2_NOTE),
    RTDAT_OFFSET(LONG_1_NOTE_HIGH)      = RTDAT_OFFSET_NEXT(MEDIAN_1_NOTE),
    RTDAT_OFFSET(LONG_1_NOTE_LOW)       = RTDAT_OFFSET_NEXT(LONG_1_NOTE_HIGH),
    RTDAT_OFFSET(VERYLONG_1_NOTE_HIGH)  = RTDAT_OFFSET_NEXT(LONG_1_NOTE_LOW),
    RTDAT_OFFSET(VERYLONG_1_NOTE_LOW)   = RTDAT_OFFSET_NEXT(VERYLONG_1_NOTE_HIGH),
    RTDAT_OFFSET(RISING_2_NOTE)         = RTDAT_OFFSET_NEXT(VERYLONG_1_NOTE_LOW),
    RTDAT_OFFSET(RISING_4_NOTE)         = RTDAT_OFFSET_NEXT(RISING_2_NOTE),
    RTDAT_OFFSET(FALLING_2_NOTE)        = RTDAT_OFFSET_NEXT(RISING_4_NOTE),
    RTDAT_OFFSET(FALLING_4_NOTE)        = RTDAT_OFFSET_NEXT(FALLING_2_NOTE),
    RTDAT_OFFSET(LONG_RING_A)           = RTDAT_OFFSET_NEXT(FALLING_4_NOTE),
    RTDAT_OFFSET(LONG_RING_B)           = RTDAT_OFFSET_NEXT(LONG_RING_A),
    RTDAT_OFFSET(LONG_RING_C)           = RTDAT_OFFSET_NEXT(LONG_RING_B),
    RTDAT_OFFSET(USER_DEFINED_A)        = RTDAT_OFFSET_NEXT(LONG_RING_C),
    RTDAT_OFFSET_END                    = RTDAT_OFFSET_NEXT(USER_DEFINED_A),
};

#define RTDAT_ACTUAL_SIZE   (RTDAT_OFFSET_END - RTDAT_OFFSET_START)

RINGTONE_RECORD_TYPE ALIGN(4) CODE _ringtoneRecord[TOTAL_BEEP_PATTERNS] = {
#undef RTONEDEF
#define RTONEDEF(name)  RTDAT_RECORD(name),
    RINGTONE_LIST
};

//U8 CODE _ringtone_reserved_data[TOTAL_BUILTIN_RINGTONE_LENGTH - RTDAT_ACTUAL_SIZE] = {0};

#endif //FEA_SUPP_DSP_RINGTONE



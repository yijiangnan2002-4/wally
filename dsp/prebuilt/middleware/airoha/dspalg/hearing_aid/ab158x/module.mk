# Copyright Statement:
#
# (C) 2021  Airoha Technology Corp. All rights reserved.
#
# This software/firmware and related documentation ("Airoha Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
# Without the prior written permission of Airoha and/or its licensors,
# any reproduction, modification, use or disclosure of Airoha Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
# You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
# if you have agreed to and been bound by the applicable license agreement with
# Airoha ("License Agreement") and been granted explicit permission to do so within
# the License Agreement ("Permitted User").  If you are not a Permitted User,
# please cease any access or use of Airoha Software immediately.
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
# ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
# AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
#

###################################################
HA_PREBUILT_PATH = prebuilt/middleware/airoha/dspalg/hearing_aid/$(IC_CONFIG)


LIBS += $(strip $(LIBDIR2))/hearing_aid/ab158x/libhearing_aid_if.a
ifeq ($(AIR_HEARTHROUGH_HA_USE_PIC), y)
    PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_afc.o
    PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_afc_fs.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_inr.o
    PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_fft.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_nr.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_ola.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_beamforming.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_drc.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_wnr.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_math.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_ndm.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_calib.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_tln.o
	PIC   += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/pisplit_awha_ha_biquad.o
    LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_afc.a
    LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_afc_fs.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_inr.a
    LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_fft.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_nr.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_ola.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_beamforming.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_drc.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_aea.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_wnr.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_math.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_ha_biquad.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_calib.a
	LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_tln.a
    LIBS  += $(strip $(LIBDIR2))/hearing_aid/$(IC_CONFIG)/pisplit/libawha_mfa.a
endif

###################################################
# include path

INC += $(HA_PREBUILT_PATH)/inc/


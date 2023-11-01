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

ifeq ($(AIR_VOICE_NR_ENABLE), y)
	ECNR_FUNC_PATH = $(MIDDLEWARE_PROPRIETARY)/dspalg/ec_nr

	C_SRC += $(ECNR_FUNC_PATH)/src/voice_nr_interface.c
	C_SRC += $(ECNR_FUNC_PATH)/src/alg_driver/voice_nr_driver.c
	C_SRC += $(ECNR_FUNC_PATH)/src/alg_driver/voice_nr_control.c
	ifeq ($(AIR_BTA_IC_PREMIUM_G2), y)
	C_SRC += $(ECNR_FUNC_PATH)/src/alg_driver/voice_nr_nvkey.c
	endif
	C_SRC += $(ECNR_FUNC_PATH)/portable/voice_nr_portable.c
	ifeq ($(AIR_HFP_DNN_PATH_ENABLE), y)
		C_SRC += $(ECNR_FUNC_PATH)/src/dnn_nr_interface.c
	endif

	ifneq ($(AIR_VOICE_NR_USE_PIC_ENABLE), y)
		ifeq ($(AIR_ECNR_1MIC_INEAR_ENABLE), y)
		else ifeq ($(AIR_ECNR_2MIC_INEAR_ENABLE), y)
		else ifeq ($(AIR_ECNR_1_OR_2_MIC_ENABLE), y)
		else ifeq ($(AIR_3RD_PARTY_NR_ENABLE), y)
		endif
	else
		ifeq ($(AIR_ECNR_1MIC_INEAR_ENABLE), y)
			ifeq ($(AIR_BTA_IC_PREMIUM_G2), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ecnr_inear_v2_swb_prcs.o
				LIBS += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/libecnr_v2_swb_init.a
			else ifeq ($(AIR_BTA_IC_PREMIUM_G3), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ecnr_inear_v2_swb.o
			else ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ecnr_inear_v2_swb_prcs.o
				LIBS += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/libecnr_v2_swb_init.a
			endif
		else ifeq ($(AIR_ECNR_2MIC_INEAR_ENABLE), y)
			ifeq ($(AIR_BTA_IC_PREMIUM_G2), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ecnr_inear_v2_swb_prcs.o
				LIBS += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/libecnr_v2_swb_init.a
			else ifeq ($(AIR_BTA_IC_PREMIUM_G3), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ecnr_inear_v2_swb.o
			else ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ecnr_inear_v2_swb_prcs.o
				LIBS += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/libecnr_v2_swb_init.a
			endif
		else ifeq ($(AIR_ECNR_1_OR_2_MIC_ENABLE), y)
			ifeq ($(AIR_BTA_IC_PREMIUM_G2), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ecnr_inear_v2_swb_prcs.o
				LIBS += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/libecnr_v2_swb_init.a
			else ifeq ($(AIR_BTA_IC_PREMIUM_G3), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ecnr_inear_v2_swb.o
			else ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ecnr_inear_v2_swb_prcs.o
				LIBS += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/libecnr_v2_swb_init.a
			endif
		else ifeq ($(AIR_3RD_PARTY_NR_ENABLE), y)
			ifeq ($(AIR_BTA_IC_PREMIUM_G2), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ec_rxnr_swb_prcs.o
				LIBS += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/libec_rxnr_swb_init.a
			else ifeq ($(AIR_BTA_IC_PREMIUM_G3), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ec_rxnr_swb_prcs.o
				LIBS += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/libec_rxnr_swb_init.a
			else ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ec_rxnr_swb_prcs.o
				LIBS += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/libec_rxnr_swb_init.a
				ifeq ($(AIR_VOICE_BAND_CONFIG_TYPE), FB)
					PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ec_fb.o
				endif
			endif
			ifeq ($(AIR_BTA_IC_PREMIUM_G2), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_fir_eq.o
			else ifeq ($(AIR_BTA_IC_PREMIUM_G3), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_fir_eq.o
			endif
			ifeq ($(AIR_AI_NR_PREMIUM_ENABLE), y)
				ifeq ($(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE), y)
					INC += prebuilt/middleware/third_party/dspalg/igo_nr_short_boom_oo/$(IC_CONFIG)/inc
					PIC += $(strip $(LIBDIR3))/igo_nr_short_boom_oo/$(IC_CONFIG)/pisplit/pisplit_igo_txnr_short_boom_oo.o
				else ifeq ($(AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE), y)
					INC += prebuilt/middleware/third_party/dspalg/igo_nr_pro_broadside/$(IC_CONFIG)/inc
					PIC += $(strip $(LIBDIR3))/igo_nr_pro_broadside/$(IC_CONFIG)/pisplit/pisplit_igo_plus_1_txnr_pro_broadside.o
				else ifeq ($(AIR_BTA_IC_PREMIUM_G2), y)
					INC += prebuilt/middleware/third_party/dspalg/igo_nr/$(IC_CONFIG)/inc
					PIC += $(strip $(LIBDIR3))/igo_nr/$(IC_CONFIG)/pisplit/pisplit_igo_txnr.o
				else ifeq ($(AIR_BTA_IC_PREMIUM_G3), y)
					INC += prebuilt/middleware/third_party/dspalg/igo_nr/$(IC_CONFIG)/inc
					PIC += $(strip $(LIBDIR3))/igo_nr/$(IC_CONFIG)/pisplit/pisplit_igo_plus_1_txnr.o
				else ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3), y)
					ifeq ($(XTENSA_CORE), AIR_STEREO_HIGH_G3_MINI)
						INC += prebuilt/middleware/third_party/dspalg/igo_nr/$(IC_CONFIG)/inc
						PIC += $(strip $(LIBDIR3))/igo_nr/$(IC_CONFIG)/pisplit/pisplit_igo_txnr_stereo_high_g3_72_270k.o
					else ifeq ($(XTENSA_CORE), AIR_STEREO_HIGH_G3_MINI_A)
						ifeq ($(AIR_AI_NR_PRIMARY_80K_ENABLE), y)
							INC += prebuilt/middleware/third_party/dspalg/igo_nr/$(IC_CONFIG)/inc
							PIC += $(strip $(LIBDIR3))/igo_nr/$(IC_CONFIG)/pisplit/pisplit_igo_txnr_stereo_high_g3_77_80k.o
						else
							INC += prebuilt/middleware/third_party/dspalg/igo_nr/$(IC_CONFIG)/inc
							PIC += $(strip $(LIBDIR3))/igo_nr/$(IC_CONFIG)/pisplit/pisplit_igo_txnr_stereo_high_g3_77_270k.o
						endif
					endif
				endif
			endif
			ifeq ($(AIR_AI_NR_PREMIUM_INEAR_ENABLE), y)
				ifeq ($(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE), y)
					INC += prebuilt/middleware/third_party/dspalg/igo_nr_pro_distractor/$(IC_CONFIG)/inc
					PIC += $(strip $(LIBDIR3))/igo_nr_pro_distractor/$(IC_CONFIG)/pisplit/pisplit_igo_plus_1_txnr_pro_distractor.o
				else ifeq ($(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE), y)
					INC += prebuilt/middleware/third_party/dspalg/igo_nr_pro_tws_oo/$(IC_CONFIG)/inc
					PIC += $(strip $(LIBDIR3))/igo_nr_pro_tws_oo/$(IC_CONFIG)/pisplit/pisplit_igo_plus_1_txnr_pro_tws_oo.o
				else ifeq ($(XTENSA_CORE), AIR_STEREO_HIGH_G3_MINI)
					INC += prebuilt/middleware/third_party/dspalg/igo_nr/$(IC_CONFIG)/inc
					PIC += $(strip $(LIBDIR3))/igo_nr/$(IC_CONFIG)/pisplit/pisplit_igo_txnr_stereo_high_g3_72_270k.o
				else ifeq ($(XTENSA_CORE), AIR_STEREO_HIGH_G3_MINI_A)
					ifeq ($(AIR_AI_NR_PRIMARY_80K_ENABLE), y)
						INC += prebuilt/middleware/third_party/dspalg/igo_nr/$(IC_CONFIG)/inc
						PIC += $(strip $(LIBDIR3))/igo_nr/$(IC_CONFIG)/pisplit/pisplit_igo_txnr_stereo_high_g3_77_80k.o
					else
						INC += prebuilt/middleware/third_party/dspalg/igo_nr/$(IC_CONFIG)/inc
						PIC += $(strip $(LIBDIR3))/igo_nr/$(IC_CONFIG)/pisplit/pisplit_igo_txnr_stereo_high_g3_77_270k.o
					endif
				else
					INC += prebuilt/middleware/third_party/dspalg/igo_nr/$(IC_CONFIG)/inc
					PIC += $(strip $(LIBDIR3))/igo_nr/$(IC_CONFIG)/pisplit/pisplit_igo_plus_1_txnr.o
				endif
			endif
		endif
		ifeq ($(AIR_ECNR_PREV_PART_USE_PIC_ENABLE), y)
			ifneq ($(AIR_ECNR_SEPARATE_MODE_ENABLE), y)
				ifeq ($(AIR_BTA_IC_PREMIUM_G2), y)
					PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ec120.o
				else ifeq ($(AIR_BTA_IC_PREMIUM_G3), y)
					PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_ec120_swb.o
				endif
			endif
		endif
		ifeq ($(AIR_ECNR_POST_PART_USE_PIC_ENABLE), y)
			ifneq ($(AIR_ECNR_SEPARATE_MODE_ENABLE), y)
				PIC += $(strip $(LIBDIR2))/ec_nr/$(IC_CONFIG)/pisplit/pisplit_post_ec.o
			endif
		endif
	endif

	INC += $(ECNR_FUNC_PATH)/inc
	INC += $(ECNR_FUNC_PATH)/portable
endif

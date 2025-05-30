# Copyright Statement:
#
# (C) 2005-2016  MediaTek Inc. All rights reserved.
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
# Without the prior written permission of MediaTek and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
# You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
# if you have agreed to and been bound by the applicable license agreement with
# MediaTek ("License Agreement") and been granted explicit permission to do so within
# the License Agreement ("Permitted User").  If you are not a Permitted User,
# please cease any access or use of MediaTek Software immediately.
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
# ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#

MBEDTLS_SRC = middleware/third_party/mbedtls

###############################################################################
# feature option dependency
###############################################################################
ifeq ($(AIR_MBEDTLS_HW_CRYPTO),y)
CFLAGS += -DCONFIG_MBEDTLS_HW_CRYPTO
CFLAGS += -DAIR_MBEDTLS_HW_USE_NC_MEMORY
#CFLAGS += -DMBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
endif


#
# Get the list of C files
#

mbedtls_PATH = $(shell pwd)

ifneq ($(MTK_BOOTLOADER_USE_MBEDTLS),y)
C_FILES += \
$(MBEDTLS_SRC)/port/porting_layer.c
endif

C_FILES += \
$(MBEDTLS_SRC)/library/aes.c \
$(MBEDTLS_SRC)/library/aesni.c \
$(MBEDTLS_SRC)/library/asn1parse.c \
$(MBEDTLS_SRC)/library/asn1write.c \
$(MBEDTLS_SRC)/library/base64.c \
$(MBEDTLS_SRC)/library/bignum.c \
$(MBEDTLS_SRC)/library/camellia.c \
$(MBEDTLS_SRC)/library/ccm.c \
$(MBEDTLS_SRC)/library/chacha20.c \
$(MBEDTLS_SRC)/library/cipher.c \
$(MBEDTLS_SRC)/library/cipher_wrap.c \
$(MBEDTLS_SRC)/library/ctr_drbg.c \
$(MBEDTLS_SRC)/library/debug.c \
$(MBEDTLS_SRC)/library/des.c \
$(MBEDTLS_SRC)/library/dhm.c \
$(MBEDTLS_SRC)/library/ecdh.c \
$(MBEDTLS_SRC)/library/ecdsa.c \
$(MBEDTLS_SRC)/library/ecp.c \
$(MBEDTLS_SRC)/library/ecp_curves.c \
$(MBEDTLS_SRC)/library/entropy.c \
$(MBEDTLS_SRC)/library/entropy_poll.c \
$(MBEDTLS_SRC)/library/entropy_hardware_poll.c \
$(MBEDTLS_SRC)/library/error.c \
$(MBEDTLS_SRC)/library/gcm.c \
$(MBEDTLS_SRC)/library/hmac_drbg.c \
$(MBEDTLS_SRC)/library/md5.c \
$(MBEDTLS_SRC)/library/md.c \
$(MBEDTLS_SRC)/library/memory_buffer_alloc.c \
$(MBEDTLS_SRC)/library/oid.c \
$(MBEDTLS_SRC)/library/padlock.c \
$(MBEDTLS_SRC)/library/pem.c \
$(MBEDTLS_SRC)/library/pk.c \
$(MBEDTLS_SRC)/library/pkcs12.c \
$(MBEDTLS_SRC)/library/pkcs5.c \
$(MBEDTLS_SRC)/library/pkparse.c \
$(MBEDTLS_SRC)/library/pk_wrap.c \
$(MBEDTLS_SRC)/library/pkwrite.c \
$(MBEDTLS_SRC)/library/platform.c \
$(MBEDTLS_SRC)/library/platform_util.c \
$(MBEDTLS_SRC)/library/ripemd160.c \
$(MBEDTLS_SRC)/library/rsa.c \
$(MBEDTLS_SRC)/library/rsa_alt_helpers.c \
$(MBEDTLS_SRC)/library/sha1.c \
$(MBEDTLS_SRC)/library/sha256.c \
$(MBEDTLS_SRC)/library/sha512.c \
$(MBEDTLS_SRC)/library/ssl_cache.c \
$(MBEDTLS_SRC)/library/ssl_ciphersuites.c \
$(MBEDTLS_SRC)/library/ssl_cli.c \
$(MBEDTLS_SRC)/library/ssl_srv.c \
$(MBEDTLS_SRC)/library/ssl_tls.c \
$(MBEDTLS_SRC)/library/threading.c \
$(MBEDTLS_SRC)/library/threading_alt.c \
$(MBEDTLS_SRC)/library/timing.c \
$(MBEDTLS_SRC)/library/version.c \
$(MBEDTLS_SRC)/library/version_features.c \
$(MBEDTLS_SRC)/library/x509.c \
$(MBEDTLS_SRC)/library/x509_create.c \
$(MBEDTLS_SRC)/library/x509_crl.c \
$(MBEDTLS_SRC)/library/x509_crt.c \
$(MBEDTLS_SRC)/library/x509_csr.c \
$(MBEDTLS_SRC)/library/x509write_crt.c \
$(MBEDTLS_SRC)/library/x509write_csr.c \
$(MBEDTLS_SRC)/library/ssl_cookie.c \
$(MBEDTLS_SRC)/library/ssl_ticket.c \
$(MBEDTLS_SRC)/library/cmac.c \
$(MBEDTLS_SRC)/library/ecjpake.c \
$(MBEDTLS_SRC)/library/net_sockets.c \
$(MBEDTLS_SRC)/library/aes_alt.c \
$(MBEDTLS_SRC)/library/des_alt.c \
$(MBEDTLS_SRC)/library/md5_alt.c \
$(MBEDTLS_SRC)/library/sha1_alt.c \
$(MBEDTLS_SRC)/library/sha256_alt.c \
$(MBEDTLS_SRC)/library/sha512_alt.c \
$(MBEDTLS_SRC)/library/aria.c \
$(MBEDTLS_SRC)/library/chachapoly.c \
$(MBEDTLS_SRC)/library/poly1305.c \
$(MBEDTLS_SRC)/library/hkdf.c \
$(MBEDTLS_SRC)/library/nist_kw.c \
$(MBEDTLS_SRC)/library/ssl_msg.c \
$(MBEDTLS_SRC)/library/ssl_tls13_keys.c \
$(MBEDTLS_SRC)/library/error.c \
$(MBEDTLS_SRC)/library/constant_time.c \
$(MBEDTLS_SRC)/library/ssl_debug_helpers_generated.c \
$(MBEDTLS_SRC)/library/ssl_tls13_client.c \
$(MBEDTLS_SRC)/library/ssl_tls13_generic.c \
$(MBEDTLS_SRC)/library/ssl_tls13_server.c \
$(MBEDTLS_SRC)/library/version_features.c \
$(MBEDTLS_SRC)/library/psa_crypto_aead.c \
$(MBEDTLS_SRC)/library/psa_crypto_cipher.c \
$(MBEDTLS_SRC)/library/psa_crypto_client.c \
$(MBEDTLS_SRC)/library/psa_crypto_driver_wrappers.c \
$(MBEDTLS_SRC)/library/psa_crypto_ecp.c \
$(MBEDTLS_SRC)/library/psa_crypto_hash.c \
$(MBEDTLS_SRC)/library/psa_crypto_mac.c \
$(MBEDTLS_SRC)/library/psa_crypto_rsa.c \
$(MBEDTLS_SRC)/library/psa_crypto_se.c \
$(MBEDTLS_SRC)/library/psa_crypto_slot_management.c \
$(MBEDTLS_SRC)/library/psa_crypto_storage.c \
$(MBEDTLS_SRC)/library/psa_crypto.c \
$(MBEDTLS_SRC)/library/psa_its_file.c \
$(MBEDTLS_SRC)/airoha/crc32.c \
$(MBEDTLS_SRC)/airoha/x963_kdf.c

#################################################################################
#include path
CFLAGS  += $(FPUFLAGS)
ifeq ($(AIR_DEBUG_VERSION_ENABLE),y)
COM_CFLAGS += -Og
else
COM_CFLAGS += -Os
endif
CFLAGS  += -fno-common
CFLAGS  += -fomit-frame-pointer
CFLAGS	+= -I$(SOURCE_DIR)/middleware/third_party/lwip/src/include
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/lwip/src/include/lwip
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/lwip/ports/include
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/include
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/configs
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/airoha
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/library
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/portable/GCC/ARM_CM4F
CFLAGS  += -I$(SOURCE_DIR)/kernel/service/inc

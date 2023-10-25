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

#ifndef __HAL_CRYPT_INTERNAL_H__
#define __HAL_CRYPT_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "air_chip.h"
//#include "nvic.h"
#include "memory_attribute.h"

#ifdef BIT
#undef BIT
#endif

#ifdef BITS
#undef BITS
#endif

#define BIT(n)                  ((uint32_t) 1 << (n))

/*
 * bits range: for example BITS(16,23) = 0xFF0000
 *   ==>  (BIT(m)-1)   = 0x0000FFFF     ~(BIT(m)-1)   => 0xFFFF0000
 *   ==>  (BIT(n+1)-1) = 0x00FFFFFF
 */
#define BITS(m,n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))

#define CRYPTO_ENGINE_CTRL_BASE (volatile uint32_t *)(CRYPTO_BASE + 0x4)
#define ENGINE_CTRL_START_OFFSET (4)

#define CRYPTO_ENGINE_STA_BASE (volatile uint32_t *)(CRYPTO_BASE + 0x8)
#define CRYPTO_ENGINE_STA_INT_CLR (16)

#define CRYPT_LOCK           1
#define CRYPT_UNLOCK         0
#define AESOTF_LOCK          1
#define AESOTF_UNLOCK        0
#define AESOTF_ESC_LOCK      1
#define AESOTF_ESC_UNLOCK    0

// airoha CRYPTO
typedef struct _IOT_CRYPTO_CTRL_TypeDef {
    volatile uint32_t ENGINE_CTRL;  //0x0004
    volatile uint32_t ENGINE_STA;   //0x0008
    volatile uint32_t TOTAL_LEN;    //0x000C
    volatile uint32_t SOUR_ADR;     //0x0010
    volatile uint32_t RESV0;        //0x0014
    volatile uint32_t RESV1;        //0x0018
    volatile uint32_t RESV2;        //0x001C
    volatile uint32_t DEST_ADR;     //0x0020
} IOT_CRYPTO_CTRL_TypeDef, *P_IOT_CRYPTO_CTRL_TypeDef;

// ENGINE_CTRL;  //0x0004
#define ENGINE_CTRL_HW_KEY_BANK_OFFSET  (10)
#define ENGINE_CTRL_HW_KEY_BANK_MASK    BITS(10, 11)
#define ENGINE_CTRL_KEY_BANK_OFFSET     (8)
#define ENGINE_CTRL_KEY_BANK_MASK       BITS(8, 9)
#define ENGINE_CTRL_START_OFFSET        (4)
#define ENGINE_CTRL_KEY_MODE_AES_OFFSET (3)
#define ENGINE_CTRL_KEY_MODE_OFFSET     (2)
#define ENGINE_CTRL_ES_OFFSET           (0)
#define ENGINE_CTRL_ES_MASK             BITS(0, 1)

// AESOTF_CTL; 0x0000
#define AESOTF_CTL_ALWAYS_ON                (24)
#define AESOTF_CTL_KEY_SEL1                 (17)
#define AESOTF_CTL_KEY_SEL0                 (16)
#define AESOTF_CTL_USE_SW_KEY               (8)
#define AESOTF_CTL_ENABLE                   (0)
#define AESOTF_STATUS_RDY                   (0)

#define IOT_CRYPTO_AESOTF_OFFSET            (0x0000)
#define IOT_CRYPTO_AESOTF_CTL               (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0000)
#define IOT_CRYPTO_AESOTF_NONCE0            (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0010)
#define IOT_CRYPTO_AESOTF_NONCE1            (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0014)
#define IOT_CRYPTO_AESOTF_NONCE2            (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0018)
#define IOT_CRYPTO_AESOTF_NONCE3            (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x001C)
#define IOT_CRYPTO_AESOTF_SWKEY0            (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0020)
#define IOT_CRYPTO_AESOTF_SWKEY1            (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0024)
#define IOT_CRYPTO_AESOTF_SWKEY2            (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0028)
#define IOT_CRYPTO_AESOTF_SWKEY3            (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x002C)

#define IOT_CRYPTO_AESOTF_REGION0_BEGIN     (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0030)
#define IOT_CRYPTO_AESOTF_REGION0_END       (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0034)
#define IOT_CRYPTO_AESOTF_REGION1_BEGIN     (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0038)
#define IOT_CRYPTO_AESOTF_REGION1_END       (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x003C)
#define IOT_CRYPTO_AESOTF_REGION2_BEGIN     (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0040)
#define IOT_CRYPTO_AESOTF_REGION2_END       (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0044)
#define IOT_CRYPTO_AESOTF_REGION3_BEGIN     (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0048)
#define IOT_CRYPTO_AESOTF_REGION3_END       (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x004C)
#define IOT_CRYPTO_AESOTF_REGION4_BEGIN     (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0050)
#define IOT_CRYPTO_AESOTF_REGION4_END       (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0054)
#define IOT_CRYPTO_AESOTF_REGION5_BEGIN     (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0058)
#define IOT_CRYPTO_AESOTF_REGION5_END       (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x005C)
#define IOT_CRYPTO_AESOTF_REGION6_BEGIN     (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0060)
#define IOT_CRYPTO_AESOTF_REGION6_END       (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0064)
#define IOT_CRYPTO_AESOTF_REGION7_BEGIN     (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x0068)
#define IOT_CRYPTO_AESOTF_REGION7_END       (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x006C)

#define IOT_CRYPTO_AESOTF_STATUS            (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x1000)
#define IOT_CRYPTO_AESOTF_DOUT0             (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x1004)
#define IOT_CRYPTO_AESOTF_DOUT1             (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x1008)
#define IOT_CRYPTO_AESOTF_DOUT2             (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x100C)
#define IOT_CRYPTO_AESOTF_DOUT3             (AESOTF_BASE + IOT_CRYPTO_AESOTF_OFFSET + 0x1010)


// AESOTF_ESC_CTL; 0x0000
#define AESOTF_ESC_CTL_ALWAYS_ON                (24)
#define AESOTF_ESC_CTL_KEY_SEL1                 (17)
#define AESOTF_ESC_CTL_KEY_SEL0                 (16)
#define AESOTF_ESC_CTL_USE_SW_KEY               (8)
#define AESOTF_ESC_CTL_ENABLE                   (0)
#define AESOTF_ESC_STATUS_RDY                   (0)

#define IOT_CRYPTO_AESOTF_ESC_OFFSET            (0x0000)
#define IOT_CRYPTO_AESOTF_ESC_CTL               (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x0000)
#define IOT_CRYPTO_AESOTF_ESC_NONCE0            (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x0010)
#define IOT_CRYPTO_AESOTF_ESC_NONCE1            (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x0014)
#define IOT_CRYPTO_AESOTF_ESC_NONCE2            (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x0018)
#define IOT_CRYPTO_AESOTF_ESC_NONCE3            (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x001C)
#define IOT_CRYPTO_AESOTF_ESC_SWKEY0            (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x0020)
#define IOT_CRYPTO_AESOTF_ESC_SWKEY1            (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x0024)
#define IOT_CRYPTO_AESOTF_ESC_SWKEY2            (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x0028)
#define IOT_CRYPTO_AESOTF_ESC_SWKEY3            (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x002C)

#define IOT_CRYPTO_AESOTF_ESC_STATUS            (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x1000)
#define IOT_CRYPTO_AESOTF_ESC_DOUT0             (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x1004)
#define IOT_CRYPTO_AESOTF_ESC_DOUT1             (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x1008)
#define IOT_CRYPTO_AESOTF_ESC_DOUT2             (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x100C)
#define IOT_CRYPTO_AESOTF_ESC_DOUT3             (AESOTF_ESC_BASE + IOT_CRYPTO_AESOTF_ESC_OFFSET + 0x1010)


// ENGINE_STA;  //0x0008
#define CRYPTO_ENGINE_STA_INT_CLR       (16)

//AES
#define IOT_CRYPTO_AES_OFFSET           (0x1000)
#define IOT_CRYPTO_AES_KEY1_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0040)
#define IOT_CRYPTO_AES_KEY2_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0050)
#define IOT_CRYPTO_AES_KEY3_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0060)
#define IOT_CRYPTO_AES_KEY4_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0070)
#define IOT_CRYPTO_AES_KEY5_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0080)
#define IOT_CRYPTO_AES_KEY6_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0090)
#define IOT_CRYPTO_AES_KEY7_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x00A0)
#define IOT_CRYPTO_AES_KEY8_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x00B0)
#define IOT_CRYPTO_AES_EOD1_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x00C0)
#define IOT_CRYPTO_AES_EOD2_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x00D0)
#define IOT_CRYPTO_AES_EOD3_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x00E0)
#define IOT_CRYPTO_AES_EOD4_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x00F0)

#define IOT_CRYPTO_AES_SWOTP1           (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0300)
#define IOT_CRYPTO_AES_SWOTP2           (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0310)
#define IOT_CRYPTO_AES_SWOTP3           (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0320)
#define IOT_CRYPTO_AES_SWOTP4           (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0330)
#define IOT_CRYPTO_AES_SWOTP5           (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0340)
#define IOT_CRYPTO_AES_SWOTP6           (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0350)
#define IOT_CRYPTO_AES_SWOTP7           (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0360)
#define IOT_CRYPTO_AES_SWOTP8           (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0370)

#define IOT_CRYPTO_AES_SWOTP1A          (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0400)
#define IOT_CRYPTO_AES_SWOTP2A          (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0410)
#define IOT_CRYPTO_AES_SWOTP3A          (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0420)
#define IOT_CRYPTO_AES_SWOTP4A          (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0430)
#define IOT_CRYPTO_AES_SWOTP5A          (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0440)
#define IOT_CRYPTO_AES_SWOTP6A          (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0450)
#define IOT_CRYPTO_AES_SWOTP7A          (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0460)
#define IOT_CRYPTO_AES_SWOTP8A          (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0470)

//AES_MODE
#define IOT_CRYPTO_AES_MODE_ADDR        (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0200)
#define AES_MODE_CKDF_SWOTP_SEL_OFFSET  (11)
#define AES_MODE_CKDF_EN_OFFSET         (10)
#define AES_MODE_CMAC_EN_OFFSET         (9)
#define AES_MODE_XOREOD_OFFSET          (8)
#define AES_MODE_UK_OFFSET              (6)
#define AES_MODE_XORDAT_OFFSET          (5)
#define AES_MODE_XORO_OFFSET            (4)
#define AES_MODE_XORI_OFFSET            (3)
#define AES_MODE_ENC_OFFSET             (2)
#define AES_MODE_KEY_LEN_OFFSET         (0)
#define AES_MODE_KEY_LEN_MASK           BITS(0, 1)

//AES_KEY_MODE
#define IOT_CRYPTO_AES_KEY_MODE_ADDR    (CRYPTO_BASE + IOT_CRYPTO_AES_OFFSET + 0x0210)
#define AES_KEY_MODE_CKDF_LOCK_1        (12)
#define AES_KEY_MODE_CKDF_LOCK_0        (11)
#define AES_KEY_MODE_SW_OTP_LOCK_1      (10)
#define AES_KEY_MODE_SW_OTP_LOCK_0      (9)
#define AES_KEY_MODE_CMAC_SUBKEY_SEL    (8)

//DES
#define IOT_CRYPTO_DES_OFFSET           (0x2000)
#define IOT_CRYPTO_DES_KEY1_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DES_OFFSET + 0x0040)
#define IOT_CRYPTO_DES_KEY2_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DES_OFFSET + 0x0050)
#define IOT_CRYPTO_DES_KEY3_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DES_OFFSET + 0x0060)
#define IOT_CRYPTO_DES_KEY4_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DES_OFFSET + 0x0070)
#define IOT_CRYPTO_DES_KEY5_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DES_OFFSET + 0x0080)
#define IOT_CRYPTO_DES_KEY6_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DES_OFFSET + 0x0090)
#define IOT_CRYPTO_DES_IV1_ADDR         (CRYPTO_BASE + IOT_CRYPTO_DES_OFFSET + 0x00C0)
#define IOT_CRYPTO_DES_IV2_ADDR         (CRYPTO_BASE + IOT_CRYPTO_DES_OFFSET + 0x00D0)
#define IOT_CRYPTO_DES_MODE_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DES_OFFSET + 0x0200)
#define DES_MODE_CBC_OFFSET             (3)
#define DES_MODE_ENC_OFFSET             (2)
#define DES_MODE_KEY_LEN_OFFSET         (0)
#define DES_MODE_KEY_LEN_MASK           BITS(0, 1)

//SHA256
#define IOT_CRYPTO_SHA256_OFFSET        (0x3000)
#define IOT_CRYPTO_SHA256_IV1           (CRYPTO_BASE + IOT_CRYPTO_SHA256_OFFSET + 0x0000)
#define IOT_CRYPTO_SHA256_IV2           (CRYPTO_BASE + IOT_CRYPTO_SHA256_OFFSET + 0x0010)
#define IOT_CRYPTO_SHA256_IV3           (CRYPTO_BASE + IOT_CRYPTO_SHA256_OFFSET + 0x0020)
#define IOT_CRYPTO_SHA256_IV4           (CRYPTO_BASE + IOT_CRYPTO_SHA256_OFFSET + 0x0030)
#define IOT_CRYPTO_SHA256_IV5           (CRYPTO_BASE + IOT_CRYPTO_SHA256_OFFSET + 0x0040)
#define IOT_CRYPTO_SHA256_IV6           (CRYPTO_BASE + IOT_CRYPTO_SHA256_OFFSET + 0x0050)
#define IOT_CRYPTO_SHA256_IV7           (CRYPTO_BASE + IOT_CRYPTO_SHA256_OFFSET + 0x0060)
#define IOT_CRYPTO_SHA256_IV8           (CRYPTO_BASE + IOT_CRYPTO_SHA256_OFFSET + 0x0070)
#define IOT_CRYPTO_SHA256_MODE_ADDR     (CRYPTO_BASE + IOT_CRYPTO_SHA256_OFFSET + 0x0200)
#define IOT_CRYPTO_SHA256_MODE_MASK     0x00000010
#define IOT_CRYPTO_SHA256_MODE_SHFT     4

//SHA512
#define IOT_CRYPTO_SHA512_OFFSET        (0x4000)
#define IOT_CRYPTO_SHA512_IV1           (CRYPTO_BASE + IOT_CRYPTO_SHA512_OFFSET + 0x0000)
#define IOT_CRYPTO_SHA512_IV2           (CRYPTO_BASE + IOT_CRYPTO_SHA512_OFFSET + 0x0010)
#define IOT_CRYPTO_SHA512_IV3           (CRYPTO_BASE + IOT_CRYPTO_SHA512_OFFSET + 0x0020)
#define IOT_CRYPTO_SHA512_IV4           (CRYPTO_BASE + IOT_CRYPTO_SHA512_OFFSET + 0x0030)
#define IOT_CRYPTO_SHA512_IV5           (CRYPTO_BASE + IOT_CRYPTO_SHA512_OFFSET + 0x0040)
#define IOT_CRYPTO_SHA512_IV6           (CRYPTO_BASE + IOT_CRYPTO_SHA512_OFFSET + 0x0050)
#define IOT_CRYPTO_SHA512_IV7           (CRYPTO_BASE + IOT_CRYPTO_SHA512_OFFSET + 0x0060)
#define IOT_CRYPTO_SHA512_IV8           (CRYPTO_BASE + IOT_CRYPTO_SHA512_OFFSET + 0x0070)
#define IOT_CRYPTO_SHA512_MODE_ADDR     (CRYPTO_BASE + IOT_CRYPTO_SHA512_OFFSET + 0x0200)
#define IOT_CRYPTO_SHA512_MODE_MASK     0x00000010
#define IOT_CRYPTO_SHA512_MODE_SHFT     4

#define IOT_CRYPTO_DMA1_OFFSET          (0x8000)
#define IOT_CRYPTO_DMA1_SRC_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DMA1_OFFSET + 0x0000)
#define IOT_CRYPTO_DMA1_WPPT_ADDR       (CRYPTO_BASE + IOT_CRYPTO_DMA1_OFFSET + 0x0008)
#define IOT_CRYPTO_DMA1_WPTO_ADDR       (CRYPTO_BASE + IOT_CRYPTO_DMA1_OFFSET + 0x000C)
#define IOT_CRYPTO_DMA1_CON_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DMA1_OFFSET + 0x0014)
#define IOT_CRYPTO_DMA1_RLCT_ADDR       (CRYPTO_BASE + IOT_CRYPTO_DMA1_OFFSET + 0x0024)

#define CRYPTO_DMA1_CON_WPSD_OFFSET     (16)
#define CRYPTO_DMA1_CON_WPEN_OFFSET     (17)

#define IOT_CRYPTO_DMA2_OFFSET          (0x9000)
#define IOT_CRYPTO_DMA2_DST_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DMA2_OFFSET + 0x0000)
#define IOT_CRYPTO_DMA2_WPPT_ADDR       (CRYPTO_BASE + IOT_CRYPTO_DMA2_OFFSET + 0x0008)
#define IOT_CRYPTO_DMA2_WPTO_ADDR       (CRYPTO_BASE + IOT_CRYPTO_DMA2_OFFSET + 0x000C)
#define IOT_CRYPTO_DMA2_CON_ADDR        (CRYPTO_BASE + IOT_CRYPTO_DMA2_OFFSET + 0x0014)
#define IOT_CRYPTO_DMA2_RLCT_ADDR       (CRYPTO_BASE + IOT_CRYPTO_DMA2_OFFSET + 0x0024)

#define CRYPTO_DMA2_CON_WPSD_OFFSET     (16)
#define CRYPTO_DMA2_CON_WPEN_OFFSET     (17)

#define DRV_Reg32(addr)                 (*(volatile uint32_t *)(addr))
#define HAL_REG_32(reg)                 (*((volatile unsigned int *)(reg)))


#define CRYPT_SWAP32(x) \
    ((uint32_t) (\
	       (((uint32_t) (x) & (uint32_t) 0x000000ffUL) << 24) | \
	       (((uint32_t) (x) & (uint32_t) 0x0000ff00UL) << 8) | \
	       (((uint32_t) (x) & (uint32_t) 0x00ff0000UL) >> 8) | \
	       (((uint32_t) (x) & (uint32_t) 0xff000000UL) >> 24)))

#define CRYPT_SWAP64(x) \
    ((uint64_t)( \
    (uint64_t)(((uint64_t)(x) & (uint64_t) 0x00000000000000ffULL) << 56) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t) 0x000000000000ff00ULL) << 40) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t) 0x0000000000ff0000ULL) << 24) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t) 0x00000000ff000000ULL) <<  8) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t) 0x000000ff00000000ULL) >>  8) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t) 0x0000ff0000000000ULL) >> 24) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t) 0x00ff000000000000ULL) >> 40) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t) 0xff00000000000000ULL) >> 56) ))

#define cpu2be64(x) CRYPT_SWAP64((x))
#define cpu2be32(x) CRYPT_SWAP32((x))


/* global crypt lock variable */
extern int8_t g_crypt_lock;

extern void restore_interrupt_mask(uint32_t mask);
extern uint32_t save_and_set_interrupt_mask(void);

/* internal function for getting lock, -100 means the crypto engine is not available */
int32_t hal_crypt_lock_take(void);

/* internal function for releasing lock */
void hal_crypt_lock_give(void);

/* internal function for getting lock, -100 means the aesotf is not available */
int32_t hal_aesotf_lock_take(void);

/* internal function for releasing lock */
void hal_aesotf_lock_give(void);

/* internal function for getting lock, -100 means the aesotf_esc  is not available */
int32_t hal_aesotf_esc_lock_take(void);

/* internal function for releasing lock */
void hal_aesotf_esc_lock_give(void);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_CRYPT_INTERNAL_H__ */


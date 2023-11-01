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
 * File: x963_kdf.c
 *
 * Description: This file defines the interface of the ANSI X9.63 KDF algorithm.
 *
 */

#include "common.h"

#if defined(MBEDTLS_ANSI_X963_KDF_C)

#include "x963_kdf.h"

#include <string.h>
#include "mbedtls/platform_util.h"
#include "mbedtls/error.h"


static void mbedtls_x963_kdf_covert_endian(uint32_t data, uint8_t *data_be)
{
    data_be[0] = *((uint8_t *)&data + 3);
    data_be[1] = *((uint8_t *)&data + 2);
    data_be[2] = *((uint8_t *)&data + 1);
    data_be[3] = *((uint8_t *)&data);
}

int mbedtls_x963_kdf(const mbedtls_md_info_t *md,
                     const unsigned char *ikm, size_t ikm_len,
                     const unsigned char *info, size_t info_len,
                     unsigned char *okm, size_t okm_len)
{
    if (md == NULL || ikm == NULL || ikm_len == 0 || okm == NULL || okm_len == 0) {
        return -1;
    }

    int ret = -1;
    uint32_t count = 1;
    size_t n = 0;
    size_t i = 0;
    size_t copy_len = 0;
    size_t copy_index = 0;

    uint8_t hash[MBEDTLS_MD_MAX_SIZE] = {0};
    size_t hash_len = mbedtls_md_get_size(md);

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    if ((ret = mbedtls_md_setup(&ctx, md, 0)) != 0) {
        goto exit;
    }

    n = okm_len / hash_len;
    if (okm_len % hash_len != 0) {
        n++;
    }

    for (i = 0; i < n; i++) {
        ret = mbedtls_md_starts(&ctx);
        if (ret != 0) {
            goto exit;
        }

        ret = mbedtls_md_update(&ctx, ikm, ikm_len);
        if (ret != 0) {
            goto exit;
        }

        uint8_t count_array[4] = {0};
        mbedtls_x963_kdf_covert_endian(count, count_array);
        ret = mbedtls_md_update(&ctx, count_array, sizeof(count));
        if (ret != 0) {
            goto exit;
        }

        if (info != NULL && info_len > 0) {
            ret = mbedtls_md_update(&ctx, info, info_len);
            if (ret != 0) {
                goto exit;
            }
        }

        ret = mbedtls_md_finish(&ctx, hash);
        if (ret != 0) {
            goto exit;
        }

        count++;

        copy_len = (i != n - 1) ? hash_len : okm_len - copy_index;
        memcpy(okm + copy_index, hash, copy_len);
        copy_index += hash_len;
    }

exit:
    mbedtls_md_free(&ctx);

    return ret;
}

///* SHA-1, COUNT = 0
// * shared secret length: 192
// * SharedInfo length: 0
// * key data length: 128
// */
//static const uint8_t Z[] = {
//    0x1c, 0x7d, 0x7b, 0x5f, 0x05, 0x97, 0xb0, 0x3d,
//    0x06, 0xa0, 0x18, 0x46, 0x6e, 0xd1, 0xa9, 0x3e,
//    0x30, 0xed, 0x4b, 0x04, 0xdc, 0x64, 0xcc, 0xdd
//};
//
//static const uint8_t verify[] = {
//    0xbf, 0x71, 0xdf, 0xfd, 0x8f, 0x4d, 0x99, 0x22,
//    0x39, 0x36, 0xbe, 0xb4, 0x6f, 0xee, 0x8c, 0xcc
//};
//
///* SHA-256, COUNT = 3
// * shared secret length: 192
// * SharedInfo length: 0
// * key data length: 128
// */
//static const uint8_t Z2[] = {
//    0xd3, 0x8b, 0xdb, 0xe5, 0xc4, 0xfc, 0x16, 0x4c,
//    0xdd, 0x96, 0x7f, 0x63, 0xc0, 0x4f, 0xe0, 0x7b,
//    0x60, 0xcd, 0xe8, 0x81, 0xc2, 0x46, 0x43, 0x8c
//};
//
//static const uint8_t verify2[] = {
//    0x5e, 0x67, 0x4d, 0xb9, 0x71, 0xba, 0xc2, 0x0a,
//    0x80, 0xba, 0xd0, 0xd4, 0x51, 0x4d, 0xc4, 0x84
//};
//
///* SHA-512, COUNT = 0
// * shared secret length: 192
// * SharedInfo length: 0
// * key data length: 128
// */
//static const uint8_t Z3[] = {
//    0x87, 0xfc, 0x0d, 0x8c, 0x44, 0x77, 0x48, 0x5b,
//    0xb5, 0x74, 0xf5, 0xfc, 0xea, 0x26, 0x4b, 0x30,
//    0x88, 0x5d, 0xc8, 0xd9, 0x0a, 0xd8, 0x27, 0x82
//};
//
//static const uint8_t verify3[] = {
//    0x94, 0x76, 0x65, 0xfb, 0xb9, 0x15, 0x21, 0x53,
//    0xef, 0x46, 0x02, 0x38,
//};
//
///* SHA-512, COUNT = 0
// * shared secret length: 521
// * SharedInfo length: 128
// * key data length: 1024
// */
//static const uint8_t Z4[] = {
//    0x00, 0xaa, 0x5b, 0xb7, 0x9b, 0x33, 0xe3, 0x89,
//    0xfa, 0x58, 0xce, 0xad, 0xc0, 0x47, 0x19, 0x7f,
//    0x14, 0xe7, 0x37, 0x12, 0xf4, 0x52, 0xca, 0xa9,
//    0xfc, 0x4c, 0x9a, 0xdb, 0x36, 0x93, 0x48, 0xb8,
//    0x15, 0x07, 0x39, 0x2f, 0x1a, 0x86, 0xdd, 0xfd,
//    0xb7, 0xc4, 0xff, 0x82, 0x31, 0xc4, 0xbd, 0x0f,
//    0x44, 0xe4, 0x4a, 0x1b, 0x55, 0xb1, 0x40, 0x47,
//    0x47, 0xa9, 0xe2, 0xe7, 0x53, 0xf5, 0x5e, 0xf0,
//    0x5a, 0x2d
//};
//
//static const uint8_t info4[] = {
//    0xe3, 0xb5, 0xb4, 0xc1, 0xb0, 0xd5, 0xcf, 0x1d,
//    0x2b, 0x3a, 0x2f, 0x99, 0x37, 0x89, 0x5d, 0x31
//};
//
//static const uint8_t verify4[] = {
//    0x44, 0x63, 0xf8, 0x69, 0xf3, 0xcc, 0x18, 0x76,
//    0x9b, 0x52, 0x26, 0x4b, 0x01, 0x12, 0xb5, 0x85,
//    0x8f, 0x7a, 0xd3, 0x2a, 0x5a, 0x2d, 0x96, 0xd8,
//    0xcf, 0xfa, 0xbf, 0x7f, 0xa7, 0x33, 0x63, 0x3d,
//    0x6e, 0x4d, 0xd2, 0xa5, 0x99, 0xac, 0xce, 0xb3,
//    0xea, 0x54, 0xa6, 0x21, 0x7c, 0xe0, 0xb5, 0x0e,
//    0xef, 0x4f, 0x6b, 0x40, 0xa5, 0xc3, 0x02, 0x50,
//    0xa5, 0xa8, 0xee, 0xee, 0x20, 0x80, 0x02, 0x26,
//    0x70, 0x89, 0xdb, 0xf3, 0x51, 0xf3, 0xf5, 0x02,
//    0x2a, 0xa9, 0x63, 0x8b, 0xf1, 0xee, 0x41, 0x9d,
//    0xea, 0x9c, 0x4f, 0xf7, 0x45, 0xa2, 0x5a, 0xc2,
//    0x7b, 0xda, 0x33, 0xca, 0x08, 0xbd, 0x56, 0xdd,
//    0x1a, 0x59, 0xb4, 0x10, 0x6c, 0xf2, 0xdb, 0xbc,
//    0x0a, 0xb2, 0xaa, 0x8e, 0x2e, 0xfa, 0x7b, 0x17,
//    0x90, 0x2d, 0x34, 0x27, 0x69, 0x51, 0xce, 0xcc,
//    0xab, 0x87, 0xf9, 0x66, 0x1c, 0x3e, 0x88, 0x16
//};
//
//void mbedtls_x963_kdf_self_test(void)
//{
//    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
//    uint8_t output1[16] = {0};
//    int ret = mbedtls_x963_kdf(md, Z, sizeof(Z), NULL, 0, output1, 16);
//    printf("x963_kdf, SHA1--16 ret=%d\r\n", ret);
//    if (memcmp(verify, output1, 16) == 0) {
//        printf("x963_kdf, successfully\r\n");
//    }
//
//    md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
//    uint8_t output2[16] = {0};
//    ret = mbedtls_x963_kdf(md, Z2, sizeof(Z2), NULL, 0, output2, 16);
//    printf("x963_kdf, SHA256--16 ret=%d\r\n", ret);
//    if (memcmp(verify2, output2, 16) == 0) {
//        printf("x963_kdf, successfully\r\n");
//    }
//
//    md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
//    uint8_t output3[12] = {0};
//    ret = mbedtls_x963_kdf(md, Z3, sizeof(Z3), NULL, 0, output3, 12);
//    printf("x963_kdf, SHA512--12 ret=%d\r\n", ret);
//    if (memcmp(verify3, output3, 12) == 0) {
//        printf("x963_kdf, successfully\r\n");
//    }
//
//    md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
//    uint8_t output4[128] = {0};
//    ret = mbedtls_x963_kdf(md, Z4, sizeof(Z4), info4, sizeof(info4), output4, 128);
//    printf("x963_kdf, SHA512--128 ret=%d\r\n", ret);
//    if (memcmp(verify4, output4, 128) == 0) {
//        printf("x963_kdf, successfully\r\n");
//    }
//}

#endif /* MBEDTLS_ANSI_X963_KDF_C */


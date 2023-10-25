/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#include "common.h"

#include "mbedtls/base64.h"
#include "mbedtls/bignum.h"
#include "mbedtls/ecp.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include "stdbool.h"
#include "stdint.h"

//#define MBEDTLS_ENTROPY_C
//#define MBEDTLS_CTR_DRBG_C

void mbedtls_ecp_test()
{
    // P = a * A + b * B, a = 1, A = r * G, B = G
    int ret = 0;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    const char *pers = "ecp_test";
    mbedtls_ecp_group grp;
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP224R1);
    mbedtls_mpi a;
    mbedtls_mpi_init(&a);
    mbedtls_mpi b;
    mbedtls_mpi_init(&b);
    mbedtls_ecp_point P;
    mbedtls_ecp_point_init(&P);
    mbedtls_ecp_point A;
    mbedtls_ecp_point_init(&A);
    mbedtls_ecp_point B;
    mbedtls_ecp_point_init(&B);

    // init DRBG
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                (const unsigned char *)pers, strlen(pers));
    printf("ECP_TEST, ctr_drbg ret=%d\r\n", ret);

    // set a = 1
    uint8_t buf[1] = {0};
    buf[0] = 1;
    ret = mbedtls_mpi_read_binary(&a, buf, sizeof(buf));
    size_t a_size = mbedtls_mpi_size(&a);
    printf("ECP_TEST, set a=1 ret=%d a_size=%d\r\n", ret, a_size);

    // A = r * G, gen r then ec_mul
    mbedtls_mpi r;
    mbedtls_mpi_init(&r);
    ret = mbedtls_ecp_gen_privkey(&grp, &r, mbedtls_ctr_drbg_random, &ctr_drbg);
    size_t r_size = mbedtls_mpi_size(&r);
    int check_priv_ret = mbedtls_ecp_check_privkey(&grp, &r);
    printf("ECP_TEST, gen r ret=%d r_size=%d\r\n check_priv=%d", ret, r_size, check_priv_ret);

    ret = mbedtls_ecp_mul(&grp, &A, &r, &grp.G,
                          mbedtls_ctr_drbg_random, &ctr_drbg);
    int check_pub_ret = mbedtls_ecp_check_pubkey(&grp, &A);
    mbedtls_mpi_free(&r);
    printf("ECP_TEST, A=r*G ret=%d check_pub_ret=%d\r\n", ret, check_pub_ret);

    // Check A1 = 1 * A equal to A
    mbedtls_ecp_point A1;
    mbedtls_ecp_point_init(&A1);
    ret = mbedtls_ecp_mul(&grp, &A1, &a, &A,
                          mbedtls_ctr_drbg_random, &ctr_drbg);
    check_pub_ret = mbedtls_ecp_check_pubkey(&grp, &A1);
    printf("ECP_TEST, A1=1*A ret=%d check_pub_ret=%d", ret, check_pub_ret);
    ret = mbedtls_ecp_point_cmp(&A1, &A);
    printf("ECP_TEST, A1==A? ret=%d", ret);

    // Gen b
    ret = mbedtls_ecp_gen_privkey(&grp, &b, mbedtls_ctr_drbg_random, &ctr_drbg);
    size_t b_size = mbedtls_mpi_size(&b);
    check_priv_ret = mbedtls_ecp_check_privkey(&grp, &b);
    printf("ECP_TEST, gen b ret=%d b_size=%d\r\n check_priv=%d", ret, r_size, check_priv_ret);

    // B copy from G
    ret = mbedtls_ecp_copy(&B, &grp.G);
    check_pub_ret = mbedtls_ecp_check_pubkey(&grp, &B);
    int equal_G_ret = mbedtls_ecp_point_cmp(&B, &grp.G);
    printf("ECP_TEST, B copy from G ret=%d check_pub_ret=%d equal G=%d", ret, check_pub_ret, equal_G_ret);

    // Compute P = a * A + b * B, a = 1, A = r * G, B = G
    ret = mbedtls_ecp_muladd(&grp, &P, &a, &A, &b, &B);
    check_pub_ret = mbedtls_ecp_check_pubkey(&grp, &P);
    printf("ECP_TEST, P=a*A + b*B ret=%d check_pub_ret=%d", ret, check_pub_ret);

    // B1 = b * B(G)
    mbedtls_ecp_point B1;
    mbedtls_ecp_point_init(&B1);
    ret = mbedtls_ecp_mul(&grp, &B1, &b, &B,
                          mbedtls_ctr_drbg_random, &ctr_drbg);
    check_pub_ret = mbedtls_ecp_check_pubkey(&grp, &B1);
    printf("ECP_TEST, B1=b*B ret=%d check_pub_ret=%d", ret, check_pub_ret);

    // Note: ecp_add_mixed cannot compute P = A + B, if you want to P = A + B
    // you should use mbedtls_ecp_muladd to do P = 1 * A + 1 * B, because A1 = 1 * A is equal to A
//    // Compute P1 = A1 + B1 via ecp_add_mixed
//    mbedtls_ecp_point P1;
//    mbedtls_ecp_point_init(&P1);
//    extern int ecp_add_mixed( const mbedtls_ecp_group *grp, mbedtls_ecp_point *R, const mbedtls_ecp_point *P, const mbedtls_ecp_point *Q );
//    ret = ecp_add_mixed(&grp, &P1, &A1, &B1);
//    check_pub_ret = mbedtls_ecp_check_pubkey(&grp, &P1);
//    printf("ECP_TEST, P1=A1+B1 ret=%d check_pub_ret=%d", ret, check_pub_ret);
//    // Check P1 == P
//    ret = mbedtls_ecp_point_cmp(&P1, &P);
//    printf("ECP_TEST, P1==P ret=%d", ret);
//    mbedtls_ecp_point_free(&P1);

    mbedtls_ecp_point_free(&A1);
    mbedtls_ecp_point_free(&B1);

    mbedtls_ecp_group_free(&grp);
    mbedtls_mpi_free(&a);
    mbedtls_mpi_free(&b);
    mbedtls_ecp_point_free(&P);
    mbedtls_ecp_point_free(&A);
    mbedtls_ecp_point_free(&B);
}


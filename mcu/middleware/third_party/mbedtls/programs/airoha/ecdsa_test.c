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
#include "mbedtls/ecdsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/sha512.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include "stdbool.h"
#include "stdint.h"


//#define MBEDTLS_ECDSA_C
//#define MBEDTLS_PK_C
//#define MBEDTLS_PK_PARSE_C
//#define MBEDTLS_PEM_PARSE_C
//#define MBEDTLS_ASN1_PARSE_C
//#define MBEDTLS_SHA512_C
//#define MBEDTLS_BIGNUM_C
//#define MBEDTLS_OID_C
//#define MBEDTLS_ECP_NIST_OPTIM

//#define MBEDTLS_ENTROPY_C
//#define MBEDTLS_CTR_DRBG_C

const static char ecdsa_public_key[] =
        "-----BEGIN PUBLIC KEY-----\r\n"
        "MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEu8szNU5yeDn8ZFEXmQ+0FyyqzFp4FDcw\r\n"
        "lfeh0zmGfq31fqEg4Q90L7FinQ+IekHCpBzK5esLmbzmJv99t+IkB6GsOLtZ9B5A\r\n"
        "D56vpNCDXyKLC7ZHg8/Lzd2oxnJ2dCZ5\r\n"
        "-----END PUBLIC KEY-----\r\n";

const static char ecdsa_private_key[] =
        "-----BEGIN EC PRIVATE KEY-----\r\n"
        "MIGkAgEBBDBYzwmEYP3+Aj2KNMqF9pV9j3kzsq+H2QgmF+O24cTxnek22OtbBWN6\r\n"
        "tAIDrhrUnbqgBwYFK4EEACKhZANiAAS7yzM1TnJ4OfxkUReZD7QXLKrMWngUNzCV\r\n"
        "96HTOYZ+rfV+oSDhD3QvsWKdD4h6QcKkHMrl6wuZvOYm/3234iQHoaw4u1n0HkAP\r\n"
        "nq+k0INfIosLtkeDz8vN3ajGcnZ0Jnk=\r\n"
        "-----END EC PRIVATE KEY-----\r\n";

static char plaintext[] = "ECDSA secp384r1 signature testing plain text";

#define ECDSA_SHA512_LEN        64

static int mbedtls_ecdsa_pseudo_random(void *ctx, unsigned char *out, size_t len)
{
    srand((unsigned)time(NULL));
    out[0] = (uint8_t)rand();
    for (size_t i = 1; i < len; i++) {
        out[i] = (uint8_t)(out[0] + i);
    }
    return 0;
}

void mbedtls_ecdsa_test()
{
    int ret = 0;
    mbedtls_pk_context priv_pk;
    mbedtls_pk_init(&priv_pk);
    mbedtls_pk_context pub_pk;
    mbedtls_pk_init(&pub_pk);

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    const char *pers = "ecdsa_test";

    mbedtls_ecp_group grp;
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP384R1);
    mbedtls_mpi p;
    mbedtls_mpi_init(&p);
    mbedtls_ecp_point P;
    mbedtls_ecp_point_init(&P);
    mbedtls_mpi r;
    mbedtls_mpi_init(&r);
    mbedtls_mpi s;
    mbedtls_mpi_init(&s);

    // init DRBG
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                (const unsigned char *)pers, strlen(pers));
    printf("ECDSA, ctr_drbg ret=%d\r\n", ret);

    // Parse private/public key
    ret = mbedtls_pk_parse_key(&priv_pk, (const unsigned char *)ecdsa_private_key, strlen(ecdsa_private_key) + 1,
                               NULL, 0, mbedtls_ctr_drbg_random, &ctr_drbg);
    printf("ECDSA priv_pk parse, ret=%d\r\n", ret);

    ret = mbedtls_pk_parse_public_key(&pub_pk, (const unsigned char *)ecdsa_public_key, strlen(ecdsa_public_key) + 1);
    printf("ECDSA pub_pk parse, ret=%d\r\n", ret);

    // Check key via PK API
    mbedtls_pk_type_t type1 = mbedtls_pk_get_type(&priv_pk);
    mbedtls_pk_type_t type2 = mbedtls_pk_get_type(&pub_pk);
    printf("ECDSA check pk, pk_type=%d %d\r\n", type1, type2);
    size_t priv_size = mbedtls_pk_get_bitlen(&priv_pk);
    size_t pub_size = mbedtls_pk_get_bitlen(&pub_pk);
    printf("ECDSA check pk, priv_size=%d pub_size=%d\r\n", priv_size, pub_size);          // 384
    //ret = mbedtls_pk_check_pair(&pub_pk, &priv_pk, mbedtls_ctr_drbg_random, &ctr_drbg); // Note: cost 22 sec
    //printf("ECDSA check pk, check_pair ret=%d\r\n", ret);

    // Check key via ECP API
    extern mbedtls_ecp_keypair *mbedtls_pk_ec( const mbedtls_pk_context pk );
    mbedtls_ecp_keypair *priv_ec = mbedtls_pk_ec(priv_pk);
    int check_privkey = mbedtls_ecp_check_privkey(&priv_ec->grp, &priv_ec->d);
    printf("ECDSA check ecp, check grp_id=%d check_privkey=%d\r\n", priv_ec->grp.id, check_privkey);
    mbedtls_ecp_keypair *pub_ec = mbedtls_pk_ec(pub_pk);
    int check_pubkey = mbedtls_ecp_check_pubkey(&pub_ec->grp, &pub_ec->Q);
    printf("ECDSA check ecp, check grp_id=%d check_pubkey=%d\r\n", pub_ec->grp.id, check_pubkey);
    if (priv_size != 384 || pub_ec->grp.id != MBEDTLS_ECP_DP_SECP384R1 || check_privkey || check_pubkey) {
        return;
    }
    mbedtls_mpi_copy(&p, &priv_ec->d);
    mbedtls_ecp_copy(&P, &pub_ec->Q);

    // SHA512 plaintext
    uint8_t hash[ECDSA_SHA512_LEN] = {0};
    ret = mbedtls_sha512(plaintext, strlen(plaintext), hash, false);
    printf("ECDSA sha512, ret=%d\r\n", ret);

#if 0  // Use signature R/S raw data
    // ECDSA sign, signature r/s total len = (384 / 8) * 2
    ret = mbedtls_ecdsa_sign(&grp, &r, &s, &p, hash, ECDSA_SHA512_LEN, mbedtls_ecdsa_pseudo_random, NULL);
    size_t r_size =  mbedtls_mpi_size(&r);
    size_t s_size = mbedtls_mpi_size(&s);
    printf("ECDSA sign, ret=%d r_size=%d s_size=%d\r\n", ret, r_size, s_size);

    // ECDSA verify
    ret = mbedtls_ecdsa_verify(&grp, hash, ECDSA_SHA512_LEN, &P, &r, &s);
    printf("ECDSA verify, ret=%d\r\n", ret);

#else  // Use signature ASN1 Der format
    // ECDSA sign
    mbedtls_ecdsa_context priv_ctx;
    mbedtls_ecdsa_init(&priv_ctx);
    ret = mbedtls_ecdsa_from_keypair(&priv_ctx, priv_ec);
    if (ret != 0) {
        printf("ECDSA sign, ecdsa_from_keypair ret=%d\r\n", ret);
    }
    uint8_t sig[512] = {0};
    size_t slen = 0;
    ret = mbedtls_ecdsa_write_signature(&priv_ctx, MBEDTLS_MD_SHA512, hash, ECDSA_SHA512_LEN,
                                        sig, 512, &slen,
                                        mbedtls_ecdsa_pseudo_random, NULL);
    printf("ECDSA sign, ret=%d len=%d\r\n", ret, slen);
//    for (int i = 0; i < (512 / 8); i++) {
//        const uint8_t *buf = sig + (i * 8);
//        printf("ECDSA sign, sig=%02X:%02X:%02X:%02X %02X:%02X:%02X:%02X\r\n",
//               buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
//        // 30 66, 02 41 00 (48) 02 41 00 (48)
//    }

    // ECDSA read and verify
    mbedtls_ecdsa_context pub_ctx;
    mbedtls_ecdsa_init(&pub_ctx);
    ret = mbedtls_ecdsa_from_keypair(&pub_ctx, pub_ec);
    if (ret != 0) {
        printf("ECDSA verify, ecdsa_from_keypair ret=%d\r\n", ret);
    }
    ret = mbedtls_ecdsa_read_signature(&pub_ctx, hash, ECDSA_SHA512_LEN, sig, slen);
    printf("ECDSA verify, ret=%d \r\n", ret);

    mbedtls_ecdsa_free(&priv_ctx);
    mbedtls_ecdsa_free(&pub_ctx);
#endif

    mbedtls_pk_free(&priv_pk);
    mbedtls_pk_free(&pub_pk);

    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);

    mbedtls_ecp_group_free(&grp);
    mbedtls_mpi_free(&p);
    mbedtls_ecp_point_free(&P);
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
}


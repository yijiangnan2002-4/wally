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
#include "mbedtls/rsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/sha512.h"

#include "stdbool.h"
#include "stdint.h"

//#define MBEDTLS_RSA_C
//#define MBEDTLS_PKCS1_V15
//#define MBEDTLS_PKCS1_V21
//#define MBEDTLS_PK_C
//#define MBEDTLS_PK_PARSE_C
//#define MBEDTLS_PEM_PARSE_C
//#define MBEDTLS_ASN1_PARSE_C
//#define MBEDTLS_SHA512_C
//#define MBEDTLS_BIGNUM_C
//#define MBEDTLS_OID_C

#define RSA_TEST_BUF            1024

static char rsa_plaintext[RSA_TEST_BUF] = "RSA 4096 signature testing plain text";

const static char rsa_public_key[] =
        "-----BEGIN PUBLIC KEY-----\r\n"
        "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAnQOD9vIsAdm6WJQ5LhJv\r\n"
        "q8EsBEn0QUWZH7gFVxOimVUXJmeB8Pf2C8OVNX1CzLjShxRwn9/x99XWdX0LdUNQ\r\n"
        "hsEF54CJ5Z7wVHsI9Od0rV5Ah8cp1nzszdiejqvlozJJ6Sg2RaxaguHAkcdEs2ea\r\n"
        "bhvZrhfJSjR88YV0amjZqYOFMEYZ9cIjewzKdijAXijleRaI9580EEC22BjJ6AA4\r\n"
        "+Kop+KQptkO4/rZqlEVCIjFdzFkQ8mhbkebRNFmz7+hhk/raIghU2w3vCmn4yyTg\r\n"
        "98vWgHAR0lzk2nxI3Ani5ZrpSunDEvd74Xw1+h30AfHymqX9s3IxqJGiUkP+2W7e\r\n"
        "Q+YZ6iBC6eJM/OqripVrJAPTkgszE63QZBVhkIaFV4S0gPSXvIZlLt+s1dvEYWMg\r\n"
        "lq4TmkEQ8WmfGaBHSmI6kIXY+XHTnGwBuXjrv6FTxQA5cHQXXV1w58S5mTiEVbLY\r\n"
        "9ITmK3f06FC2MMLJ5JYeHLHm5ot0YT5Ki3KlDA96IV2YH8hQICXCnbqbhR5FRbiu\r\n"
        "0ToHyMqcrwpdNWxbLQDYcTcUsAy8mlCTh97xgZo+IdCRxehyomK5x/jEXr/5m2As\r\n"
        "wn/dP1JtijxFubY6JntM8sEN24HAN9bMFWZkAf3O3m70cQF9RYDBR2kEyW8pANQd\r\n"
        "taikBJp7lAWl2YSFfoyVZ7ECAwEAAQ==\r\n"
        "-----END PUBLIC KEY-----\r\n";

const static char rsa_private_key[] =
        "-----BEGIN PRIVATE KEY-----\r\n"
        "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\r\n"
        "xTJFzIM+VNWui4GHmz1FaUBFTfr3dA==\r\n"
        "-----END PRIVATE KEY-----\r\n";

static int mbedtls_rsa_test_rng( void *ctx, unsigned char *out, size_t len )
{
    static uint32_t state = 42;

    (void) ctx;

    for( size_t i = 0; i < len; i++ )
    {
        state = state * 1664525u + 1013904223u;
        out[i] = (unsigned char) state;
    }

    return( 0 );
}

void mbedtls_rsa_signature_test()
{
    int ret = 0;
    size_t i;
    mbedtls_pk_context priv_pk;
    mbedtls_pk_init(&priv_pk);
    mbedtls_pk_context pub_pk;
    mbedtls_pk_init(&pub_pk);

    ret = mbedtls_pk_parse_key(&priv_pk, (const unsigned char *)rsa_private_key, strlen(rsa_private_key) + 1,
                               NULL, 0, mbedtls_rsa_test_rng, NULL);
    if (ret != 0) {
        printf("RSA ~~~~~~~~~ priv_pk parse, ret=%d\r\n", ret);
    }

    ret = mbedtls_pk_parse_public_key(&pub_pk, (const unsigned char *)rsa_public_key, strlen(rsa_public_key) + 1);
    if (ret != 0) {
        printf("RSA ~~~~~~~~~ pub_pk parse, ret=%d\r\n", ret);
    }

    mbedtls_pk_type_t type1 = mbedtls_pk_get_type(&priv_pk);
    mbedtls_pk_type_t type2 = mbedtls_pk_get_type(&pub_pk);
    if (type1 != MBEDTLS_PK_RSA || type2 != MBEDTLS_PK_RSA) {
        printf("RSA ~~~~~~~~~ pub_pk parse, type1=%d type2=\r\n", type1, type2);
    }

    ret = mbedtls_pk_check_pair(&pub_pk, &priv_pk, mbedtls_rsa_test_rng, NULL);
    if (ret != 0) {
        printf("RSA ~~~~~~~~~ check_pair, ret=%d\r\n", ret);
    }

    // Note: PK RSA encrypt use mbedtls_rsa_public, PK RSA decrypt use mbedtls_rsa_private
    // Note: RSA 4096 min output 512 bytes
    // Note: Output still is 512 when input 600, if use the output to decrypt, will occur MBEDTLS_ERR_RSA_INVALID_PADDING
    uint8_t ct_output[RSA_TEST_BUF] = {0};
    size_t ct_output_len = 0;
    //memset(rsa_plaintext, 65, 600);
    ret = mbedtls_pk_encrypt(&pub_pk, rsa_plaintext, strlen(rsa_plaintext),
                             ct_output, &ct_output_len, RSA_TEST_BUF,
                             mbedtls_rsa_test_rng, NULL);
    if (ret != 0) {
        printf("RSA ~~~~~~~~~ encrypt, ret=%d ct_output_len=%d\r\n", ret, ct_output_len);
    }

    printf("RSA ~~~~~~~~~ encrypt, ilen=%d ct_output_len=%d\r\n", strlen(rsa_plaintext), ct_output_len);

    uint8_t pt_output[RSA_TEST_BUF] = {0};
    size_t pt_output_len = 0;
    ret = mbedtls_pk_decrypt(&priv_pk,
                             ct_output, ct_output_len,
                             pt_output, &pt_output_len, RSA_TEST_BUF,
                             mbedtls_rsa_test_rng, NULL);
    if (ret != 0 || pt_output_len != ct_output_len || pt_output_len != strlen(rsa_plaintext) || memcmp(pt_output, rsa_plaintext, strlen(rsa_plaintext)) != 0) {
        printf("RSA ~~~~~~~~~ decrypt, ret=%d pt_output_len=%d\r\n", ret, pt_output_len);
    }
    printf("RSA ~~~~~~~~~ decrypt, pt_output=%s\r\n", pt_output);


    uint8_t hash[64] = {0};
    ret = mbedtls_sha512(rsa_plaintext, strlen(rsa_plaintext), hash, false);
    if (ret != 0) {
        printf("RSA ~~~~~~~~~ hash, ret=%d\r\n", ret);
    }

    // Note: PK (RSA) sign use mbedtls_rsa_private (only PKCS#1 v1.5)
    // Note: PK (RSA) verify use mbedtls_rsa_public (support MBEDTLS_PK_RSASSA_PSS via mbedtls_pk_verify_ext)
    memset(ct_output, 0, RSA_TEST_BUF);
    ct_output_len = 0;
    memset(pt_output, 0, RSA_TEST_BUF);
    pt_output_len = 0;
    ret = mbedtls_pk_sign(&priv_pk, MBEDTLS_MD_SHA512, hash, 64,
                          ct_output, RSA_TEST_BUF, &ct_output_len,
                          mbedtls_rsa_test_rng, NULL);
    printf("RSA ~~~~~~~~~ PK sign, ret=%d\r\n", ret);

    ret = mbedtls_pk_verify(&pub_pk, MBEDTLS_MD_SHA512,
                            hash, 64,
                            ct_output, ct_output_len);
    printf("RSA ~~~~~~~~~ PK verify, ret=%d\r\n", ret);

    // Note: PK(RSA) covert to RSA context
    extern mbedtls_rsa_context *mbedtls_pk_rsa( const mbedtls_pk_context pk );
    mbedtls_rsa_context *priv_rsa = mbedtls_pk_rsa(priv_pk);
    mbedtls_rsa_context *pub_rsa = mbedtls_pk_rsa(pub_pk);
    // MBEDTLS_RSA_PKCS_V15, MBEDTLS_RSA_PKCS_V21 for RSAES-OAEP / RSASSA-PSS
    int ret1 = mbedtls_rsa_set_padding(priv_rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA512);
    int ret2 = mbedtls_rsa_set_padding(pub_rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA512);
    int ret3 = mbedtls_rsa_check_pubkey(pub_rsa);
    int ret4 = mbedtls_rsa_check_privkey(priv_rsa);
    int ret5 = mbedtls_rsa_check_pub_priv(pub_rsa, priv_rsa);
    printf("RSA ~~~~~~~~~ RSA context, %d %d %d %d %d\r\n", ret1, ret2, ret3, ret4, ret5);

    memset(ct_output, 0, RSA_TEST_BUF);
    ct_output_len = 0;
    memset(pt_output, 0, RSA_TEST_BUF);
    pt_output_len = 0;
    ret = mbedtls_rsa_rsassa_pss_sign(priv_rsa, mbedtls_rsa_test_rng, NULL,
                                      MBEDTLS_MD_SHA512, 64, hash, ct_output);
    printf("RSA ~~~~~~~~~ rsa_rsassa_pss sign, ret=%d\r\n", ret);

    ret = mbedtls_rsa_rsassa_pss_verify(pub_rsa, MBEDTLS_MD_SHA512,
                                        64, hash, ct_output);
    printf("RSA ~~~~~~~~~ rsa_rsassa_pss verify, ret=%d\r\n", ret);

    mbedtls_pk_free(&priv_pk);
    mbedtls_pk_free(&pub_pk);
}


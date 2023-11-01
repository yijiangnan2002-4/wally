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

#include "psa/crypto.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//MBEDTLS_CIPHER_C

#define PSA_TEST_INPUT_LEN              100

static psa_status_t cipher_operation( psa_cipher_operation_t *operation,
                                      const uint8_t * input,
                                      size_t input_size,
                                      size_t part_size,
                                      uint8_t * output,
                                      size_t output_size,
                                      size_t *output_len )
{
    psa_status_t status;
    size_t bytes_to_write = 0, bytes_written = 0, len = 0;

    *output_len = 0;
    while( bytes_written != input_size )
    {
        bytes_to_write = ( input_size - bytes_written > part_size ?
                           part_size :
                           input_size - bytes_written );

        status = psa_cipher_update( operation, input + bytes_written,
                                    bytes_to_write, output + *output_len,
                                    output_size - *output_len, &len );
        if (status != PSA_SUCCESS) {
            printf("[PSA] mbedtls_psa_test, operation->cipher_update status=%d\r\n", status);
        }

        bytes_written += bytes_to_write;
        *output_len += len;
    }

    status = psa_cipher_finish( operation, output + *output_len,
                                output_size - *output_len, &len );
    if (status != PSA_SUCCESS) {
        printf("[PSA] mbedtls_psa_test, operation->cipher_finish status=%d\r\n", status);
    }
    *output_len += len;

exit:
    return( status );
}

static psa_status_t cipher_encrypt( psa_key_id_t key,
                                    psa_algorithm_t alg,
                                    uint8_t * iv,
                                    size_t iv_size,
                                    const uint8_t * input,
                                    size_t input_size,
                                    size_t part_size,
                                    uint8_t * output,
                                    size_t output_size,
                                    size_t *output_len )
{
    psa_status_t status;
    psa_cipher_operation_t operation = PSA_CIPHER_OPERATION_INIT;
    size_t iv_len = 0;

    memset( &operation, 0, sizeof( operation ) );
    status = psa_cipher_encrypt_setup( &operation, key, alg );
    if (status != PSA_SUCCESS) {
        printf("[PSA] mbedtls_psa_test, encrypt_setup status=%d\r\n", status);
    }

    status = psa_cipher_generate_iv( &operation, iv, iv_size, &iv_len );
    if (status != PSA_SUCCESS) {
        printf("[PSA] mbedtls_psa_test, generate_iv status=%d\r\n", status);
    }

    status = cipher_operation( &operation, input, input_size, part_size,
                               output, output_size, output_len );
    if (status != PSA_SUCCESS) {
        printf("[PSA] mbedtls_psa_test, operation status=%d\r\n", status);
    }

exit:
    psa_cipher_abort( &operation );
    return( status );
}

static psa_status_t cipher_decrypt( psa_key_id_t key,
                                    psa_algorithm_t alg,
                                    const uint8_t * iv,
                                    size_t iv_size,
                                    const uint8_t * input,
                                    size_t input_size,
                                    size_t part_size,
                                    uint8_t * output,
                                    size_t output_size,
                                    size_t *output_len )
{
    psa_status_t status;
    psa_cipher_operation_t operation = PSA_CIPHER_OPERATION_INIT;

    memset( &operation, 0, sizeof( operation ) );
    status = psa_cipher_decrypt_setup( &operation, key, alg );

    status = psa_cipher_set_iv( &operation, iv, iv_size );

    status = cipher_operation( &operation, input, input_size, part_size,
                               output, output_size, output_len );

exit:
    psa_cipher_abort( &operation );
    return( status );
}

void mbedtls_psa_test()
{
    psa_status_t psa_status = psa_crypto_init();
    printf("[PSA] mbedtls_psa_test, init status=%d\r\n", psa_status);

    uint8_t input[PSA_TEST_INPUT_LEN] = {0};
    psa_status = psa_generate_random(input, PSA_TEST_INPUT_LEN);
    printf("[PSA] mbedtls_psa_test, random status=%d %02X:%02X %02X:%02X:%02X\r\n",
           psa_status, input[0], input[1], input[97], input[98], input[99]);



    uint8_t hash[32] = {0};
    size_t hash_size = 0;
    psa_hash_operation_t hash_op = PSA_HASH_OPERATION_INIT;
    psa_status = psa_hash_setup(&hash_op, PSA_ALG_SHA_256);
    if (psa_status != PSA_SUCCESS) {
        printf("[PSA] mbedtls_psa_test, hash_setup status=%d\r\n", psa_status);
    }
    psa_status = psa_hash_update(&hash_op, input, PSA_TEST_INPUT_LEN);
    if (psa_status != PSA_SUCCESS) {
        printf("[PSA] mbedtls_psa_test, hash_update status=%d\r\n", psa_status);
    }
    psa_status = psa_hash_finish(&hash_op, hash, 32, &hash_size);
    printf("[PSA] mbedtls_psa_test, hash_finish status=%d hash_size=%d\r\n", psa_status, hash_size);
    if (psa_status != PSA_SUCCESS) {
        psa_hash_abort(&hash_op);
    }



    enum {
        block_size = PSA_BLOCK_CIPHER_BLOCK_LENGTH( PSA_KEY_TYPE_AES ),
        key_bits = 256,
        part_size = block_size,
    };
    const psa_algorithm_t alg = PSA_ALG_CBC_NO_PADDING;

    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t key = 0;
    size_t output_len = 0;
    uint8_t iv[block_size] = {0};
    uint8_t input1[block_size] = {0};
    uint8_t encrypt[block_size] = {0};
    uint8_t decrypt[block_size] = {0};

    psa_set_key_usage_flags( &attributes,
                             PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT );
    psa_set_key_algorithm( &attributes, alg );
    psa_set_key_type( &attributes, PSA_KEY_TYPE_AES );
    psa_set_key_bits( &attributes, key_bits );

    psa_status = psa_import_key(&attributes, hash, hash_size, &key);
    if (psa_status != PSA_SUCCESS) {
        printf("[PSA] mbedtls_psa_test, import_key status=%d\r\n", psa_status);
    }

    psa_status = cipher_encrypt(key, alg, iv, sizeof( iv ), input1, sizeof( input1 ), part_size,
                                encrypt, sizeof( encrypt ), &output_len );
    if (psa_status != PSA_SUCCESS) {
        printf("[PSA] mbedtls_psa_test, cipher_encrypt status=%d\r\n", psa_status);
    }

    psa_status = cipher_decrypt(key, alg, iv, sizeof( iv ), encrypt, output_len, part_size,
                                decrypt, sizeof( decrypt ), &output_len );
    if (psa_status != PSA_SUCCESS) {
        printf("[PSA] mbedtls_psa_test, cipher_decrypt status=%d\r\n", psa_status);
    }

    size_t ret = memcmp( input1, decrypt, block_size );
    printf("[PSA] mbedtls_psa_test, cipher=%d\r\n", ret);

    psa_destroy_key( key );
}


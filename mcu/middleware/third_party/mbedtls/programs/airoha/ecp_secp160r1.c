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
//#include "FreeRTOS.h"
//#include "task.h"

#include "stdbool.h"
#include "stdint.h"

// Note: Need more task stack size
static int mbedtls_ecp_self_test_rng( void *ctx, unsigned char *out, size_t len )
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

void mbedtls_ecp_secp160r1_test()
{
    int ret = 0;

    // expected result
    uint8_t result[] = {0xB3, 0xA6, 0x8A, 0x81, 0xA0, 0xAF, 0x56, 0xD0,
                        0xBD, 0x37, 0xD1, 0x02, 0x68, 0x45, 0x95, 0xF3,
                        0xDC, 0xDE, 0xF4, 0xA6};

    // init r'
    mbedtls_mpi r1;
    mbedtls_mpi_init(&r1);
    uint8_t r_array[32] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0A, 0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x01, 0x02,
    };
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_binary(&r1, r_array, 32) );

    // r = r' mod n
    mbedtls_ecp_group grp;
    mbedtls_ecp_group_init(&grp);
    MBEDTLS_MPI_CHK( mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP160R1) );
    mbedtls_mpi r;
    mbedtls_mpi_init(&r);
    ret = mbedtls_mpi_mod_mpi(&r, &r1, &grp.N);
    if (ret == 0) {
        uint8_t r_out_array[32] = {0};
        size_t len = mbedtls_mpi_size(&r);
        mbedtls_mpi_write_binary(&r, r_out_array, len);
        unsigned char dst[100] = {0};
        size_t olen = 0;
        ret = mbedtls_base64_encode(dst, 100, &olen, r_out_array, len);
        printf("secp160r1_test r=%s\r\n", dst);
    } else {
        printf("secp160r1_test r=r1 mode n: error ret=%d\r\n", ret);
    }

    // R = r * G
    mbedtls_ecp_point R;
    mbedtls_ecp_point_init(&R);
    ret = mbedtls_ecp_mul(&grp, &R, &r, &grp.G, mbedtls_ecp_self_test_rng, NULL);
    if (ret == 0) {
        uint8_t r_out_array[32] = {0};
        size_t len = mbedtls_mpi_size(&R.X);
        printf("secp160r1_test R.x_len=%d\r\n", len);
        mbedtls_mpi_write_binary(&R.X, r_out_array, len);
        unsigned char dst[100] = {0};
        size_t olen = 0;
        ret = mbedtls_base64_encode(dst, 100, &olen, r_out_array, len);

        if (memcmp(r_out_array, result, 20) == 0) {
            printf("secp160r1_test successfully\r\n");
        } else {
            printf("secp160r1_test fail\r\n");
            printf("secp160r1_test R.x=%s\r\n", dst);
            uint8_t *out = r_out_array;
            printf("secp160r1_test R.x=%02X:%02X:%02X:%02X %02X:%02X:%02X:%02X\r\n", out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]);
            printf("secp160r1_test R.x=%02X:%02X:%02X:%02X %02X:%02X:%02X:%02X\r\n", out[8], out[9], out[10], out[11], out[12], out[13], out[14], out[15]);
            printf("secp160r1_test R.x=%02X:%02X:%02X:%02X\r\n", out[16], out[17], out[18], out[19]);
        }
    } else {
        printf("secp160r1_test R = r * G: error ret=%d\r\n", ret);
    }

    uint32_t water = uxTaskGetStackHighWaterMark(NULL);
    printf("secp160r1_test water=%d\r\n", water);

cleanup:
    if (ret < 0) {
        printf("secp160r1_test r: error ret=%d\r\n", ret);
    }

   mbedtls_mpi_free(&r1);
   mbedtls_mpi_free(&r);
   mbedtls_ecp_group_free(&grp);
   mbedtls_ecp_point_free(&R);
}


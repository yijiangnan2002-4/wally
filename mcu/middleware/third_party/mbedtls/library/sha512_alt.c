/*
 *  FIPS-180-2 compliant SHA-384/512 implementation
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
/*
 *  The SHA-512 Secure Hash Standard was published by NIST in 2002.
 *
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 */

#include "common.h"

#if defined(MBEDTLS_SHA512_C)

#include "mbedtls/sha512.h"

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_printf printf
#define mbedtls_calloc    calloc
#define mbedtls_free       free
#endif /* MBEDTLS_PLATFORM_C */

#if defined(MBEDTLS_SHA512_ALT)

#include <string.h>

#ifndef __EXT_BOOTLOADER__
#include "FreeRTOSConfig.h"
#else
#include "bl_common.h"
#define configASSERT(x) { if (!(x)) {bl_print(LOG_CRIT,"assert on mbedtls SHA512 ALT\n\r"); while (1); } }
#endif // #ifndef __EXT_BOOTLOADER__

#define SHA512_VALIDATE_RET(cond)                           \
    MBEDTLS_INTERNAL_VALIDATE_RET( cond, MBEDTLS_ERR_SHA512_BAD_INPUT_DATA )
#define SHA512_VALIDATE(cond)  MBEDTLS_INTERNAL_VALIDATE( cond )

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
#include "top.h"
#endif

/* Implementation that should never be optimized out by the compiler */
static void mbedtls_zeroize(void *v, size_t n)
{
    volatile unsigned char *p = v;
    while (n--) {
        *p++ = 0;
    }
}

void mbedtls_sha512_init(mbedtls_sha512_context *ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha512_context));
}

void mbedtls_sha512_free(mbedtls_sha512_context *ctx)
{
    if (ctx == NULL) {
        return;
    }

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha512_context));
}

void mbedtls_sha512_clone(mbedtls_sha512_context *dst,
                          const mbedtls_sha512_context *src)
{
    *dst = *src;
}

/*
 * SHA-512 context setup
 */
int mbedtls_sha512_starts(mbedtls_sha512_context *ctx, int is384)
{
    hal_sha_status_t status = HAL_SHA_STATUS_ERROR;

    ctx->is384 = is384;

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo96M();
#endif

    int count = 0;
    if (1 == is384) {
        do {
            status = hal_sha384_init(&(ctx->ctx.sha384_ctx));
            if (status == -100) {
                mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
                count++;
            }
        } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    } else {
        do {
            status = hal_sha512_init(&(ctx->ctx.sha512_ctx));
            if (status == -100) {
                mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
                count++;
            }
        } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    }
    configASSERT(count <= MBEDTLS_HW_ALT_RETRY_TIME);

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo192M();
#endif

    return HAL_SHA_STATUS_OK == status ? 0 : (int)status;
}

/*
 * SHA-512 process buffer
 */
int mbedtls_sha512_update(mbedtls_sha512_context *ctx,
                          const unsigned char *input,
                          size_t ilen)
{
    hal_sha_status_t status = HAL_SHA_STATUS_ERROR;

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo96M();
#endif

    int count = 0;
    if (1 == ctx->is384) {
        do {
            status = hal_sha384_append(&(ctx->ctx.sha384_ctx), (uint8_t *)input, (uint32_t)ilen);
            if (status == -100) {
                mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
                count++;
            }
        } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    } else {
        do {
            status = hal_sha512_append(&(ctx->ctx.sha512_ctx), (uint8_t *)input, (uint32_t)ilen);
            if (status == -100) {
                mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
                count++;
            }
        } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    }
    configASSERT(count <= MBEDTLS_HW_ALT_RETRY_TIME);

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo192M();
#endif
    return HAL_SHA_STATUS_OK == status ? 0 : (int)status;
}

/*
 * SHA-512 final digest
 */
int mbedtls_sha512_finish(mbedtls_sha512_context *ctx,
                          unsigned char output[64])
{
    hal_sha_status_t status = HAL_SHA_STATUS_ERROR;

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo96M();
#endif

    int count = 0;
    if (1 == ctx->is384) {
        do {
            status = hal_sha384_end(&(ctx->ctx.sha384_ctx), (uint8_t *)output);
            if (status == -100) {
                mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
                count++;
            }
        } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    } else {
        do {
            status = hal_sha512_end(&(ctx->ctx.sha512_ctx), (uint8_t *)output);
            if (status == -100) {
                mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
                count++;
            }
        } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    }
    configASSERT(count <= MBEDTLS_HW_ALT_RETRY_TIME);

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo192M();
#endif

    return HAL_SHA_STATUS_OK == status ? 0 : (int)status;
}

#endif /* MBEDTLS_SHA512_ALT */

#endif /* MBEDTLS_SHA512_C */


/*
 *  FIPS-180-2 compliant SHA-256 implementation
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
 *  The SHA-256 Secure Hash Standard was published by NIST in 2002.
 *
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 */

#include "common.h"

#if defined(MBEDTLS_SHA256_C)

#include "mbedtls/sha256.h"

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_printf printf
#define mbedtls_calloc    calloc
#define mbedtls_free       free
#endif /* MBEDTLS_PLATFORM_C */

#if defined(MBEDTLS_SHA256_ALT)

#include <string.h>

#ifndef __EXT_BOOTLOADER__
#include "FreeRTOSConfig.h"
#else
#include "bl_common.h"
#define configASSERT(x) { if (!(x)) {bl_print(LOG_CRIT,"assert on mbedtls SHA256 ALT\n\r"); while (1); } }
#endif // #ifndef __EXT_BOOTLOADER__

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

void mbedtls_sha256_init(mbedtls_sha256_context *ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_free(mbedtls_sha256_context *ctx)
{
    if (ctx == NULL) {
        return;
    }

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_clone(mbedtls_sha256_context *dst,
                          const mbedtls_sha256_context *src)
{
    *dst = *src;
}

/*
 * SHA-256 context setup
 */
int mbedtls_sha256_starts(mbedtls_sha256_context *ctx, int is224)

{
    hal_sha_status_t status = HAL_SHA_STATUS_ERROR;

    ctx->is224 = is224;

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo96M();
#endif

    int count = 0;
    if (1 == is224) {
        do {
            status = hal_sha224_init(&(ctx->ctx.sha224_ctx));
            if (status == -100) {
                mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
                count++;
            }
        } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    } else {
        do {
            status = hal_sha256_init(&(ctx->ctx.sha256_ctx));
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
 * SHA-256 process buffer
 */
int mbedtls_sha256_update(mbedtls_sha256_context *ctx,
                          const unsigned char *input,
                          size_t ilen)
{
    hal_sha_status_t status = HAL_SHA_STATUS_ERROR;

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo96M();
#endif

    int count = 0;
    if (1 == ctx->is224) {
        do {
            status = hal_sha224_append(&(ctx->ctx.sha224_ctx), (uint8_t *)input, ilen);
            if (status == -100) {
                mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
                count++;
            }
        } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    } else {
        do {
            status = hal_sha256_append(&(ctx->ctx.sha256_ctx), (uint8_t *)input, ilen);
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
 * SHA-256 final digest
 */
int mbedtls_sha256_finish(mbedtls_sha256_context *ctx,
                          unsigned char output[32])

{
    hal_sha_status_t status = HAL_SHA_STATUS_ERROR;

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo96M();
#endif

    int count = 0;
    if (1 == ctx->is224) {
        do {
            status = hal_sha224_end(&(ctx->ctx.sha224_ctx), (uint8_t *)output);
            if (status == -100) {
                mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
                count++;
            }
        } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    } else {
        do {
            status = hal_sha256_end(&(ctx->ctx.sha256_ctx), (uint8_t *)output);
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

#endif /* MBEDTLS_SHA256_ALT */

#endif /* MBEDTLS_SHA256_C */

/*
 *  FIPS-180-1 compliant SHA-1 implementation
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
 *  The SHA-1 standard was published by NIST in 1993.
 *
 *  http://www.itl.nist.gov/fipspubs/fip180-1.htm
 */

#include "common.h"

#if defined(MBEDTLS_SHA1_C)

#include "mbedtls/sha1.h"

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif /* MBEDTLS_PLATFORM_C */

#if defined(MBEDTLS_SHA1_ALT)

#include <string.h>

#ifndef __EXT_BOOTLOADER__
#include "FreeRTOSConfig.h"
#else
#include "bl_common.h"
#define configASSERT(x) { if (!(x)) {bl_print(LOG_CRIT,"assert on mbedtls SHA1 ALT\n\r"); while (1); }}
#endif // #ifndef __EXT_BOOTLOADER__

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
#include "top.h"
#endif

/* Implementation that should never be optimized out by the compiler */
static void mbedtls_zeroize(void *v, size_t n)
{
    volatile unsigned char *p = (unsigned char *)v;
    while (n--) {
        *p++ = 0;
    }
}

void mbedtls_sha1_init(mbedtls_sha1_context *ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_free(mbedtls_sha1_context *ctx)
{
    if (ctx == NULL) {
        return;
    }

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_clone(mbedtls_sha1_context *dst,
                        const mbedtls_sha1_context *src)
{
    *dst = *src;
}

/*
 * SHA-1 context setup
 */
int mbedtls_sha1_starts(mbedtls_sha1_context *ctx)
{
    hal_sha_status_t status = HAL_SHA_STATUS_ERROR;

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo96M();
#endif

    int count = 0;
    do {
        status = hal_sha1_init(ctx);
        if (status == -100) {
            mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
            count++;
        }
    } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    configASSERT(count <= MBEDTLS_HW_ALT_RETRY_TIME);

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo192M();
#endif
    return HAL_SHA_STATUS_OK == status ? 0 : (int)status;
}

/*
 * SHA-1 process buffer
 */
int mbedtls_sha1_update(mbedtls_sha1_context *ctx,
                        const unsigned char *input,
                        size_t ilen)
{
    hal_sha_status_t status = HAL_SHA_STATUS_ERROR;

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo96M();
#endif

    int count = 0;
    do {
        status = hal_sha1_append(ctx, (uint8_t *)input, (uint32_t)ilen);
        if (status == -100) {
            mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
            count++;
        }
    } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    configASSERT(count <= MBEDTLS_HW_ALT_RETRY_TIME);

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo192M();
#endif
    return HAL_SHA_STATUS_OK == status ? 0 : (int)status;
}

/*
 * SHA-1 final digest
 */
int mbedtls_sha1_finish(mbedtls_sha1_context *ctx,
                        unsigned char output[20])
{
    hal_sha_status_t status = HAL_SHA_STATUS_ERROR;

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo96M();
#endif

    int count = 0;
    do {
        status = hal_sha1_end(ctx, (uint8_t *)output);
        if (status == -100) {
            mbedtls_platform_delay(MBEDTLS_HW_ALT_DELAY_TIME);
            count++;
        }
    } while (status == -100 && count <= MBEDTLS_HW_ALT_RETRY_TIME);
    configASSERT(count <= MBEDTLS_HW_ALT_RETRY_TIME);

#ifdef MBEDTLS_HW_ALGORITHM_CHANGE_CPU_CLOCK
    cmnCpuClkSwitchTo192M();
#endif

    return HAL_SHA_STATUS_OK == status ? 0 : (int)status;
}

#endif /* MBEDTLS_SHA1_ALT */

#endif /* MBEDTLS_SHA1_C */


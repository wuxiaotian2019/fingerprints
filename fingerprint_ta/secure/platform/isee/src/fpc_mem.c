/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "ut_sys_util.h"
#include "ut_sys_type.h"
#include "ut_pf_time.h"

#include "fpc_mem.h"
#include "fpc_log.h"
#ifdef FPC_CONFIG_PRIV_HEAP_DEBUG
#include "fpc_debug.h"
#endif


void fpc_tee_debug(const char* tag, const char* format, ...)
{
    (void)tag;
#ifdef FPC_CONFIG_DEBUG
    char log_str[1024];
    va_list args;
    va_start(args, format);
    vsprintf(log_str, format, args);
    ut_sys_log("%s \n", log_str);
    va_end(args);
#else
    (void)format;
#endif
}

void fpc_tee_error(const char* tag, const char* format, ...)
{
    (void)tag;
#ifdef FPC_CONFIG_DEBUG
    char log_str[1024];
    va_list args;
    va_start(args, format);
    vsprintf(log_str, format, args);
    ut_sys_log("error : %s \n", log_str);
    va_end(args);
#else
    (void)format;
#endif
}

void fpc_get_timestamp(uint64_t *time)
{
    ut_uint32_t high, low;
    ut_pf_time_get_microseconds(&high, &low);

    *time = high; *time<<=32; *time |= low;
}

int fpc_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    return vsnprintf(str, size, format, ap);
}


/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */


#include "fpc_log.h"
#include "fpc_crypto.h"
#include "fpc_types.h"
#include "ut_pf_time.h"
#include "ut_pf_cp.h"
#include "utils.h"


#define SHA256_SIZE 32

uint64_t start_time;

/*
WARNING: may not work on some hardware platforms!
*/
uint64_t fpc_get_uptime(void)
{
    uint64_t timestamp;
    uint32_t time_result;
    uint32_t high;
    uint32_t low;
    time_result = ut_pf_time_get_microseconds(&high, &low);
    if(time_result){
        LOGE("get exact timestamp failed!");
        return -1;
    }
    timestamp = high;
    timestamp = timestamp << 32;
    timestamp = timestamp | low;
    return timestamp / 1000; //convert us to ms.
}

int32_t fpc_secure_random(uint8_t* data, uint32_t data_size)
{
    return ut_pf_cp_rd_random(NULL, data, data_size);
}


int fpc_hmac_sha256(const uint8_t* data, uint32_t size_data, const uint8_t* key,
        uint32_t size_key, uint8_t* hmac) {
    //get shared key every time IMPORTANT!
    (void)key;
    (void)size_key;
    int ret = 0;
    ut_pf_cp_context_t *ctx = NULL;
    ut_size_t maclen = SHA256_SIZE;

    /* open operation */
    ret = ut_pf_cp_open(&ctx, UT_PF_CP_CLS_MC, UT_PF_CP_ACT_MC_HMAC_SHA256);
    if ( ret < 0 ) {
        LOGE("call ut_pf_cp_open failed !!! ret = %d", ret);
        return ret;
    }

    /* start mac operaton */
    ret = ut_pf_cp_mc_starts(ctx, key, size_key, NULL, 0);
    if ( ret < 0 ) {
        goto out;
    }

    /* update mac operaton */
    ret = ut_pf_cp_mc_update(ctx, data,  size_data);
    if ( ret < 0 ) {
        goto out;
    }

    /* finish mac operaton */
    ret = ut_pf_cp_mc_finish(ctx, hmac, &maclen);
    if ( ret < 0 ) {
        goto out;
    }

    /* close operation */
    ret = ut_pf_cp_close(ctx);
    if ( ret < 0 ) {
        LOGE("call ut_pf_cp_close failed !!! ret = %d", ret);
        return ret;
    }

    return ret;

out:
    ut_pf_cp_close(ctx);
    LOGE("call ut_pf_cp_mc_update failed !!! ret = %d", ret);
    return ret;
}

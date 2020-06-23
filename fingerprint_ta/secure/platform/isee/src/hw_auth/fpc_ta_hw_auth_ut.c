/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdint.h>

#include "ut_pf_km.h"

#include "fpc_log.h"
#include "fpc_ta_hw_auth.h"
#include "fpc_types.h"

/**
 * Derive the per-boot HMAC key.
 *
 * Note that Keymaster must derive the same key.
 *
 * @param[out] hmac_key per-boot HMAC key
 *
 */

int fpc_ta_hw_auth_unwrap_key(uint8_t* encrypted_key,
                              uint32_t size_encrypted_key,
                              uint8_t* key, uint32_t* size_key)
{
    int status;
    (void)encrypted_key;
    (void)size_encrypted_key;
    status = ut_pf_km_get_hmac_key(key, (int *)size_key);
    if (status != 0) {
        LOGE("fp get hmac key has failed with code: %d", status);
        return -FPC_ERROR_CRYPTO;
    } else {
        LOGD("fp get hmac key success");
    }
    return 0;
}

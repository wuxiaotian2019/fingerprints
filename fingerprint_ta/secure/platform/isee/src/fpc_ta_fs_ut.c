/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "fpc_log.h"
#include "fpc_fs.h"
#include "fpc_types.h"

#include "ut_pf_ts.h"

const char* DB_PATH_PRE_FIX = "../../../";

#define MAX_PATH_LENGTH 128

static int transform_path(const char* path_in, char* path_out) {
    if(!path_in|| !path_out){
        return -FPC_ERROR_INPUT;
    }
    if (strlen(DB_PATH_PRE_FIX) + strlen(path_in) > MAX_PATH_LENGTH) {
        LOGE("DB path is too long");
        return -FPC_ERROR_INPUT;
    }
    memset(path_out, 0, MAX_PATH_LENGTH);
    memcpy(path_out, DB_PATH_PRE_FIX, strlen(DB_PATH_PRE_FIX));
    memcpy(path_out + strlen(DB_PATH_PRE_FIX), path_in, strlen(path_in));
    return 0;
}

int fpc_fs_remove(const char* path) {
    char tran_path[MAX_PATH_LENGTH];
    int retval = 0;
    retval = transform_path(path, tran_path);
    if (0 != retval) {
        LOGE(" %s Bad parameter: %s", __func__, path);
        return retval;
    }
    retval = ut_pf_ts_unlink(tran_path);
    if (0 != retval) {
        LOGE("%s failed for %s with error %d", __func__, path, retval);
        return -FPC_ERROR_IO;
    }
    return 0;
}

int fpc_fs_read(const char* path, uint8_t *buf, uint32_t size) {
    int read_bytes = 0;
    int retval = 0;
    int fd = -1;

    if (NULL == buf) {
        LOGE("%s: Read buffer is NULL", __func__);
        return -FPC_ERROR_INPUT;
    }
    char tran_path[MAX_PATH_LENGTH];
    retval = transform_path(path, tran_path);
    if (0 != retval) {
        LOGE(" %s Bad parameter", __func__);
        return retval;
    }
    fd = ut_pf_ts_open(tran_path, UT_PF_TS_O_RDONLY);
    if (fd < 0) {
        LOGE("%s open failed for %s with error %d", __func__, tran_path, fd);
        return -FPC_ERROR_IO;
    }
    retval = ut_pf_ts_read(fd, (char *) buf, (int) size);
    if (0 > retval) {
        LOGE("%s read failed for %s with error %d", __func__, tran_path, retval);
        goto out;
    } else if ((int)size != retval) {
        LOGE("%s: requested size for reading %s does not match read bytes %d %d",
            __func__, tran_path, size, retval);
        goto out;
    }

    read_bytes = retval;
    retval = ut_pf_ts_close(fd);
    if (0 != retval) {
        LOGE("%s: close failed for %s with error %d", __func__, tran_path, retval);
        return -FPC_ERROR_IO;
    }
    return read_bytes;

out:
    ut_pf_ts_close(fd);
    return -FPC_ERROR_IO;
}

int fpc_fs_write(const char* path, uint8_t *buf, uint32_t size) {
    int written_bytes = 0;
    int retval = 0;
    int fd = -1;

    if (NULL == buf) {
        LOGE("%s: Write buffer is NULL", __func__);
        return -FPC_ERROR_INPUT;
    }
    char tran_path[MAX_PATH_LENGTH];
    retval = transform_path(path, tran_path);
    if (0 != retval) {
        LOGE(" %s Bad parameter", __func__);
        return retval;
    }
    fd = ut_pf_ts_open(tran_path, UT_PF_TS_O_RDWR | UT_PF_TS_O_CREAT | UT_PF_TS_O_TRUNC);
    if (fd < 0) {
        LOGE("%s: open failed for %s with error %d", __func__, tran_path, fd);
        return -FPC_ERROR_IO;
    }

    retval = ut_pf_ts_write(fd, (const char *) buf, (int) size);
    if (0 > retval) {
        LOGE("%s: write failed for %s with error %d", __func__, tran_path, retval);
        goto out;
    } else if ((int)size != retval) {
        LOGE("%s: requested size for writing %s does not match written bytes %d %d",
            __func__, tran_path, size, retval);
        goto out;
    }
    written_bytes = retval;
    retval = ut_pf_ts_close(fd);
    if (0 != retval) {
        LOGE("%s: close failed for %s with error %d", __func__, tran_path, retval);
        return -FPC_ERROR_IO;
    }
    LOGD("%s: Wrote %d bytes", __func__, written_bytes);
    return written_bytes;

out:
    ut_pf_ts_close(fd);
    return -FPC_ERROR_IO;
}

int fpc_fs_get_filesize(const char* path, uint32_t *size) {
    int retval = 0;
    int fd = -1;
    char tran_path[MAX_PATH_LENGTH];
    retval = transform_path(path, tran_path);
    if (0 != retval) {
        LOGE(" %s Bad parameter", __func__);
        return retval;
    }
    fd = ut_pf_ts_open(tran_path, UT_PF_TS_O_RDONLY);

    if (fd < 0) {
        LOGE("%s: open failed for %s with error %d", __func__, tran_path, errno);
        return -FPC_ERROR_IO;
    }
    retval = ut_pf_ts_size(fd);
    if (retval < 0) {
        LOGE("%s: getSize failed for %s with error %d", __func__, tran_path, retval);
        goto out;
    }
    *size = retval;
    retval = ut_pf_ts_close(fd);
    if (0 != retval) {
        LOGE("%s: close failed for %s with error %d", __func__, tran_path, retval);
        return -FPC_ERROR_IO;
    }
    return retval;

out:
    ut_pf_ts_close(fd);
    return -FPC_ERROR_IO;
}


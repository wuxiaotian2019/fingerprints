/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <stdio.h>
#include <unistd.h>

#include <signal.h>
#include <sys/types.h>
#include "../../../../../../mediatek/proprietary/trustzone/trustkernel/source/external/AndroidServices/libteec/include/tee_client_api.h"

#include "fpc_error_str.h"
#include "fpc_tac.h"
#include "fpc_sysfs.h"
#include "fpc_log.h"
#include "fpc_types.h"


#define CMD_TRANSFER_VALUE_IN		0x100
#define CMD_TRANSFER_VALUE_OUT		0x101
#define CMD_TRANSFER_VALUE_INOUT	0x102
#define CMD_TRANSFER_MEM_IN			0x103
#define CMD_TRANSFER_MEM_OUT		0x104
#define CMD_TRANSFER_MEM_INOUT		0x105

//#define MEGABYTE (1024*1024)
//#define FPC_TA_TIMEOUT 10000
#define TK_MEMORY_ALLOCATE_LIMIT_BYTES (512*1024)

//#define UT_FP_ENTRY_IOCTL_DEVICE "/dev/teei_fp"
//#define UT_FP_ENTRY_IOCTL_MAGIC_NO _IO(0x775A777E,0x2)

#define FPC_TAC_SHARED_BUFFER_SIZE 1024 * 512

//static int ut_fp_entry_fd = -1;
//static void* ut_fp_entry_buffer;

static TEEC_Session session;
static TEEC_Context context;

#if 0
typedef struct {
  uint32_t reserved_field_1;
  uint32_t reserved_field_2;
  int response;
  uint32_t len;
  void* cmd_data;
} ut_mem_map;

typedef struct {
  ut_mem_map header;
} __attribute__((packed)) fpc_ut_msg;
#endif

struct fpc_tac {
  //fpc_ut_msg msg;
  int sysfs_fd;
};

#if 0
typedef struct {
    fpc_tac_shared_mem_t shared_mem;
    ut_mem_map mem_map;
    fpc_tac_t* tac;
} ut_mapped_memory_t;
#else
typedef struct {
    fpc_tac_shared_mem_t shared_mem;
    uint32_t len;
} tk_mapped_memory_t;
#endif

/**
 * @brief fpc_tac_open open a connection to the ta.
 * @return the tac or NULL on failure.
 */
fpc_tac_t* fpc_tac_open()
{
    int ret = 0;
    char path[PATH_MAX];
    fpc_tac_t* tac = NULL;
#ifdef FPC_TA_HW_AUTHENTICATION
    uint8_t* encapsulated_key = NULL;
    uint32_t size_encapsulated_key;
#endif

    TEEC_Result r;

    TEEC_UUID uuid = { 0xfda3c56b, 0xc277, 0x4b18, \
                       { 0xd0, 0x25, 0xdb, 0x6a, 0xad, 0xa9, 0xe0, 0x55 } };

    //Initialize a new TEE context
    r = TEEC_InitializeContext(
               NULL,
               &context);
    if (r != TEEC_SUCCESS) {
        LOGE("%s TEEC_InitializeContext failed with return value:%x\n", __func__, r);
        return NULL;
    }

    r = TEEC_OpenSession(
                &context,
                &session,
                &uuid,
                TEEC_LOGIN_PUBLIC,
                NULL,
                NULL,
                NULL);

    if (r != TEEC_SUCCESS) {
        //fprintf(stderr, "TEEC_OpenSession failed with return value:%x\n", r);
        LOGE("%s TEEC_OpenSession failed with return value:%x\n", __func__, r);
        TEEC_FinalizeContext(&context);
        return NULL;
    }

#if 0
    if (ut_fp_entry_fd == -1) {
        ut_fp_entry_fd = open(UT_FP_ENTRY_IOCTL_DEVICE, O_RDWR);
    }
    if (ut_fp_entry_fd == -1) {
        LOGE("%s open teei_fp_fd failed", __func__);
        return NULL;
    }

    LOGI("%s ut fp entry open success ut_fp_entry_fd = %d", __func__, ut_fp_entry_fd);

    ut_fp_entry_buffer = malloc(FPC_TAC_SHARED_BUFFER_SIZE);
    if (!ut_fp_entry_buffer) {
        LOGE("%s No memory for fpc tac buffer", __func__);
        ret = -FPC_ERROR_ALLOC;
        goto out;
    }
#endif

    tac = (fpc_tac_t*) malloc(sizeof(fpc_tac_t));
    if (!tac) {
        ret = -FPC_ERROR_ALLOC;
        LOGD("<--%s FPC_ERROR_ALLOC ret=%d", __func__, ret);
        goto out;
    }

    memset(tac, 0, sizeof(fpc_tac_t));
    tac->sysfs_fd = -1;

    if (!fpc_sysfs_path_by_attr(FPC_REE_DEVICE_ALIAS_FILE, FPC_REE_DEVICE_NAME, FPC_REE_DEVICE_PATH,
                                path, PATH_MAX)) {
        ret = -FPC_ERROR_IO;
        
        LOGD("<--%s FPC_ERROR_IO ret=%d", __func__, ret);
        goto out;
    }

    tac->sysfs_fd = open(path, O_RDONLY);
    if (tac->sysfs_fd == -1) {
        LOGD("%s open %s failed %i", __func__, path, errno);
        ret = -FPC_ERROR_IO;
        goto out;
    }

out:
    if(0 > ret) {
        if (tac && tac->sysfs_fd != -1) {
            close(tac->sysfs_fd);
        }
        if (tac) {
            free(tac);
            tac = NULL;
        }

        TEEC_CloseSession(&session);
        TEEC_FinalizeContext(&context);
#if 0
        if (ut_fp_entry_fd != -1) {
            close(ut_fp_entry_fd);
            ut_fp_entry_fd = -1;
        }
        if (!ut_fp_entry_buffer) {
            free(ut_fp_entry_buffer);
            ut_fp_entry_buffer = NULL;
        }
#endif
    }

    LOGD("<--%s debug 201801091927xx returns %d", __func__, ret);
    return tac;

}

/**
 * @brief fpc_tac_release close the connection to the ta.
 */
void fpc_tac_release(fpc_tac_t* tac)
{
    //int ret = 0;

    LOGD("-->%s Closing the session", __func__);

    if (tac && tac->sysfs_fd != -1) {
        close(tac->sysfs_fd);
    }

    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);

#if 0
    if (ut_fp_entry_fd != -1) {
        close(ut_fp_entry_fd);
        ut_fp_entry_fd = -1;
    }
    if(!ut_fp_entry_buffer) {
        free(ut_fp_entry_buffer);
        ut_fp_entry_buffer = NULL;
    }
#endif

    free(tac);
    tac = NULL;
#if 0
error:
    LOGD("<--%s Result: %d", __func__, ret);
#endif
}


fpc_tac_shared_mem_t* fpc_tac_alloc_shared(fpc_tac_t *tac, uint32_t size)
{
#if 0
    ut_mapped_memory_t* utmm = NULL;
    if (MEGABYTE < size) {
        LOGE("%s Error: size does not fit into isee map block size", __func__);
        goto error;
    }
    utmm = (ut_mapped_memory_t *)malloc(sizeof(ut_mapped_memory_t));
    if (!utmm) {
        LOGE("%s Error: Failed to alloc ut_mapped_memory_t for shared memory", __func__);
        goto error;
    }
    memset(utmm, 0, sizeof(ut_mapped_memory_t));
    utmm->shared_mem.addr = malloc(size);
    if (!utmm->shared_mem.addr) {
        LOGE("%s Error: Failed to alloc normal world buffer (malloc)", __func__);
        goto error;
    }
    utmm->mem_map.len = size;
    utmm->mem_map.cmd_data = utmm->shared_mem.addr;
    utmm->tac = tac;
    return &utmm->shared_mem;

error:
    if (utmm) {
        free(utmm);
    }
    return NULL;
#else
    (void)(tac);
    tk_mapped_memory_t* tkmm = NULL;

    if (TK_MEMORY_ALLOCATE_LIMIT_BYTES < (size + 4 /*sizeof(int), for response*/)) {
        //LOGE("%s Error: size %u (%u + %lu) does not fit into trustkernel map block size %d", __func__, (size + 4 /*sizeof(int)*/), size, sizeof(int), TK_MEMORY_ALLOCATE_LIMIT_BYTES);
        goto error;
    }

    tkmm = (tk_mapped_memory_t *)malloc(sizeof(tk_mapped_memory_t));
    if (!tkmm) {
        LOGE("%s Error: Failed to alloc tk_mapped_memory_t for shared memory", __func__);
        goto error;
    }
    memset(tkmm, 0, sizeof(tk_mapped_memory_t));
    tkmm->shared_mem.addr = malloc(size + 4 /*sizeof(int), for response*/);
    if (!tkmm->shared_mem.addr) {
        LOGE("%s Error: Failed to alloc normal world buffer (malloc)", __func__);
        goto error;
    }

    tkmm->len = size + 4 /*sizeof(int), for response*/;

    return &tkmm->shared_mem;

error:
    if (tkmm) {
        free(tkmm);
    }

    return NULL;
#endif
}

void fpc_tac_free_shared(fpc_tac_shared_mem_t* shared_mem)
{
    if (shared_mem) {
        if (shared_mem->addr) {
            free(shared_mem->addr);
        }
        free(shared_mem);
    }
}

/**
 * transfer control to the ta, the entire content of the shared buffer shall
 * be treated as both input/output.
 */
int fpc_tac_transfer(fpc_tac_t* tac, fpc_tac_shared_mem_t* shared_buffer)
{
#if 0
    int header_size = 0;
    ut_mapped_memory_t* utmm = (ut_mapped_memory_t *)shared_buffer;
    tac->msg.header = utmm->mem_map;
    int status = fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "1");

    if (status) {
        LOGE("%s clk_enable enable failed, error no %s", __func__, strerror(errno));
        goto out;
    }
    header_size =  sizeof(ut_mem_map) - sizeof(void *);
    if (ut_fp_entry_fd && ut_fp_entry_buffer) {
        if (tac->msg.header.len + header_size > FPC_TAC_SHARED_BUFFER_SIZE) {
            status = -1;
            LOGE("%s fp msg is too big to transfer %d ", __func__, tac->msg.header.len);
            goto out;
        }
        memcpy(ut_fp_entry_buffer ,&tac->msg.header ,header_size);
        memcpy((uint8_t*)ut_fp_entry_buffer + header_size, tac->msg.header.cmd_data, tac->msg.header.len);
        status = ioctl(ut_fp_entry_fd, UT_FP_ENTRY_IOCTL_MAGIC_NO, ut_fp_entry_buffer);
        if (status != 0) {
            LOGE("[%s][%d] status = %d , errno:\t\t%s ", __func__, __LINE__, status, strerror(errno));
            goto out;
        }
        memcpy(&tac->msg.header, ut_fp_entry_buffer,header_size);
        memcpy(tac->msg.header.cmd_data, (uint8_t*)ut_fp_entry_buffer + header_size, tac->msg.header.len);
    }
    else {
        status = -1;
        LOGE("%s fp entry is not ready!", __func__);
    }
#else
    TEEC_Result r;
    TEEC_Operation op;
    uint32_t ro;
    tk_mapped_memory_t* tkmm = (tk_mapped_memory_t *)shared_buffer;
    int response = 0;
    int status =0; 
    int8_t *pResponseStart = NULL;
    (void)(tac);
/*
    int status = fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "1");

    if (status) {
        LOGE("%s clk_enable enable failed, error no %s", __func__, strerror(errno));
        goto out;
    }
*/
    op.paramTypes = TEEC_PARAM_TYPES(
    TEEC_MEMREF_TEMP_INOUT,
    TEEC_NONE,
    TEEC_NONE,
    TEEC_NONE);

    op.params[0].tmpref.size = tkmm->len;
    op.params[0].tmpref.buffer = tkmm->shared_mem.addr; //tkmm->shared_mem
    
    op.started = 1;

    //Invoke a Command within the specified Session
    r = TEEC_InvokeCommand(
			&session,
			CMD_TRANSFER_MEM_INOUT,
			&op,
			&ro);

    if (r != TEEC_SUCCESS) {
        LOGE("%s failed with return value: %x return_origin: %d\n",
             __func__, r, ro);
        status = -1;
        goto out;
    }

    pResponseStart = ((int8_t *)(tkmm->shared_mem.addr)) + (tkmm->len - 4 /*sizeof(int), for response*/);
    response = *pResponseStart + (((int)(*(pResponseStart + 1))) << 8) + (((int)(*(pResponseStart + 2))) << 16) + (((int)(*(pResponseStart + 3))) << 24);
    //LOGE("%s response %d (0x%02x, 0x%02x, 0x%02x, 0x%02x), sizeof(int) %lu", __func__, response, *(pResponseStart + 3), *(pResponseStart + 2), *(pResponseStart + 1), *pResponseStart, (sizeof(int))); //tkmm->shared_mem

#endif
out:
    //fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "0");

    if (status) {
        return -FPC_ERROR_COMM;
    }

#if 0
    if (0 > tac->msg.header.response) {
        LOGE("%s TA returned error response %s", __func__, fpc_error_str(tac->msg.header.response));
        return tac->msg.header.response;
    }

    memset(&tac->msg.header, 0, sizeof(tac->msg.header));
#else
    if (response < 0)
    {
        LOGE("%s TA returned error response %s", __func__, fpc_error_str(response));
        return response;
    }
    
#endif

    return 0;
}


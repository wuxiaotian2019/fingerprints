/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fpc_ta_targets.h"
#include "fpc_ta_interface.h"
#include "fpc_tee.h"
#include "fpc_tac.h"
#include "fpc_tee_internal.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_bio_internal.h"

#if 0
static int get_file_size(const char* path, uint32_t* size)
{
    struct stat file_info;

    memset(&file_info, 0, sizeof(file_info));
    *size = 0;

    if (stat(path, &file_info)) {
        switch(errno) {
        case ENOENT:
            return 0;
        default:
            LOGE("%s stat failed with error %s", __func__, strerror(errno));
            return -FPC_ERROR_IO;
        }
    }

    *size = file_info.st_size;
	
	return 0;
}
#else
static int get_db_file_size(fpc_tee_bio_t* bio, const char* path, uint32_t* size)
{
	LOGD("%s", __func__);
    int status = 0;
	fpc_tee_t* tee = &bio->tee;
    uint32_t size_string = strlen(path) + 1;
	uint32_t size_shared_mem = sizeof(fpc_ta_byte_array_msg_t) + size_string;
    char temp_path[PATH_MAX];

	*size = 0;
	
	if (snprintf(temp_path, PATH_MAX, "%s"/*"%s.bak"*/, path) >= PATH_MAX) {
        LOGE("%s input:path too long", __func__);
        return -FPC_ERROR_INPUT;
    }
	
	fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, size_shared_mem);
	
    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }
	
    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.target    = TARGET_FPC_TA_FS;
    command->header.command   = FPC_TA_BIO_GET_DB_SIZE_CMD;
    command->get_db_size.size = size_string;
	memcpy(command->get_db_size.array, temp_path, size_string);

    status = fpc_tac_transfer(tee->tac, shared_buffer);

	if (status)
	{
		*size = 0;
		
		if(status == -FPC_ERROR_NOENTITY)
		{
			status = 0;
			LOGE("%s status FPC_ERROR_NOENTITY", __func__);
		}
		else
		{
            LOGE("%s failed status %d", __func__, status);
		}
	}
	else
	{
		*size = command->get_db_size.size;
		LOGE("%s file size %d", __func__, command->get_db_size.size);
	}
	
	//*size = 0;
	//status = -FPC_ERROR_IO;
	fpc_tac_free_shared(shared_buffer);
    return status;
}
#endif

static int db_command(fpc_tee_bio_t* bio, int32_t op, const char* path)
{
    fpc_tee_t* tee = &bio->tee;
    uint32_t size_string = strlen(path) + 1;
    uint32_t size = sizeof(fpc_ta_byte_array_msg_t) + size_string;

    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, size);

    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    int status;
    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.command = op;
    command->header.target = TARGET_FPC_TA_FS;
    command->store_db.size = size_string;
    memcpy(command->store_db.array, path, size_string);

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    status = command->store_db.response;

out:
    fpc_tac_free_shared(shared_buffer);

    return status;
}

int fpc_tee_store_template_db(fpc_tee_bio_t* bio, const char* path)
{
    LOGD("%s", __func__);
    char temp_path[PATH_MAX];
    int status;

    if (snprintf(temp_path, PATH_MAX, "%s"/*"%s.bak"*/, path) >= PATH_MAX) {
        LOGE("%s input:path too long", __func__);
        return -FPC_ERROR_INPUT;
    }

    status = db_command(bio, FPC_TA_BIO_STORE_DB_CMD, temp_path);
    if (status) {
        goto out;
    }

#if 0
    if (rename(temp_path, path)) {
        LOGE("%s stat failed with error %s", __func__, strerror(errno));
        status = -FPC_ERROR_IO;
    }
#endif

out:
#if 0    
    remove(temp_path);
#endif
    return status;
}

int fpc_tee_load_template_db(fpc_tee_bio_t* bio, const char* path)
{
    LOGD("%s", __func__);
    uint32_t size;
	
#if 0
    int status = get_file_size(path, &size);
#else
	int status = get_db_file_size(bio, path, &size);
#endif

    if (status) {
        return status;
    }

    if (size == 0)
    {
        return fpc_tee_load_empty_db(bio);
    }

    status = db_command(bio, FPC_TA_BIO_LOAD_DB_CMD, path);
    if(-FPC_ERROR_IO == status)
    {
        //-FPC_ERROR_IO is returned on file read and crypto errors.
        //In both cases we assume that the db is non-recoverable.
        LOGE("%s, failed to load db, corrupt?, load empty db.", __func__);
        status = fpc_tee_load_empty_db(bio);
    }

    return status;
}

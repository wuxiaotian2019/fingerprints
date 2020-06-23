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

#include "ut_bta.h"
#include "ut_pf_fp.h"

#include "fpc_log.h"
#include "fpc_ta_interface.h"
#include "fpc_db.h"
#include "fpc_ta_router.h"
#include "fpc_spi.h"

static int fpc_ta_initialized = 0;
typedef struct
{
  uint32_t reserved_field_1;
  uint32_t reserved_field_2;
  uint32_t response;
  uint32_t len;
}  ut_ta_mem_map;

int main(void) {
    int ret = 0;

    LOGI("enter into main ");
    ret = ut_pf_fp_main();
    return ret;
}
#ifdef READ_CHIP_ID_FIRST
static void read_device_id(void){
    unsigned char txTemp[10] = { 0 },rxTemp[10] = { 0 };
    txTemp[0] = 0xFC;
    fpc_spi_writeread(txTemp, 1, rxTemp, 2);   // 0xFC

    LOGI("FPC chip id: [%x][%x][%x][%x][%x][%x][%x][%x][%x][%x]", rxTemp[0],
            rxTemp[1], rxTemp[2],
            rxTemp[3], rxTemp[4],
            rxTemp[5], rxTemp[6],
            rxTemp[7], rxTemp[8],
            rxTemp[9]);
}
#endif
//#define READ_CHIP_ID_FIRST
void init_main() {
#ifdef READ_CHIP_ID_FIRST
    fpc_spi_init(0,0);
    while (1) {
        read_device_id();
    }
#endif
}
/******************************************************************************/
TEE_Result ut_fp_invokeCommand(void *param, void * data, uint32_t param_length) {
    int ret = 0;

    if(!fpc_ta_initialized) {
        ret = fpc_ta_router_init();
    }
    if (!ret) {
        fpc_ta_initialized = 1;
    } else {
        LOGE("%s ta router init failed %d", __func__, ret);
        return -1;
    }
    ret = fpc_ta_route_command(data, param_length);
    ((ut_ta_mem_map *) param)->response = ret;

    return 0;
}


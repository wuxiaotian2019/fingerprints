/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "spi.h"
#include "ut_pf_spi.h"

#include "fpc_spi.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_result.h"
#include "fpc_mem.h"

static struct mt_chip_conf spi_chip_config;

#define PARAMETER_NOT_USED(A) (void)(A)
#define SPI_ALIGN_SIZE( X ) ( X % 4 ? ( X + 4 ) - ( X & 0x3 ) : X )

//#define _PERFORMANCE_MEASUREMENT_
void fpc_spi_chipcfg(void);
struct mt_chip_conf * my_chip_conf;

static void fpc_spi_config(struct mt_chip_conf *chip_config)
{
    chip_config->setuptime = 15;
    chip_config->holdtime = 15;
    chip_config->high_time = 12;
    chip_config->low_time = 12;
    chip_config->cs_idletime = 20;
    chip_config->ulthgh_thrsh = 0;

    chip_config->cpol = SPI_CPOL_0;
    chip_config->cpha = SPI_CPHA_0;

    chip_config->rx_mlsb = SPI_MSB;
    chip_config->tx_mlsb = SPI_MSB;

    chip_config->tx_endian = SPI_LENDIAN;
    chip_config->rx_endian = SPI_LENDIAN ;

    chip_config->com_mod = FIFO_TRANSFER; //DMA_TRANSFER;//FIFO_TRANSFER;
    chip_config->pause = PAUSE_MODE_DISABLE;
    chip_config->finish_intr = FINISH_INTR_DIS;
    chip_config->deassert = DEASSERT_DISABLE;
    chip_config->ulthigh = ULTRA_HIGH_DISABLE;
    chip_config->tckdly = TICK_DLY0;
}


// ------------------------------------------------------------------------------------------------
int fpc_spi_init(uint32_t freq_low_khz, uint32_t freq_high_khz)
{
    PARAMETER_NOT_USED(freq_low_khz);
    PARAMETER_NOT_USED(freq_high_khz);
    LOGD("fpc_spi_init \n");
    fpc_spi_config(&spi_chip_config);
    return FPC_RESULT_OK;
}

// ------------------------------------------------------------------------------------------------
int fpc_spi_destroy(void)
{
    LOGD("%s\n", __func__);
    return FPC_RESULT_OK;
}

// ------------------------------------------------------------------------------------------------
int fpc_spi_select_low_freq(void)
{
    return FPC_RESULT_OK;
}

// ------------------------------------------------------------------------------------------------
int fpc_spi_select_high_freq(void)
{
    return FPC_RESULT_OK;
}

// ------------------------------------------------------------------------------------------------
int fpc_spi_read(uint8_t *buff, size_t num_bytes)
{
    PARAMETER_NOT_USED(buff);
    PARAMETER_NOT_USED(num_bytes);
    LOGD("%s\n", __func__);
    return FPC_RESULT_OK;
}

// ------------------------------------------------------------------------------------------------
int fpc_spi_write(uint8_t *buff, size_t num_bytes)
{
    int ret = FPC_RESULT_ERROR_UNKNOWN;
    void *tx_buff = NULL;
#ifdef _PERFORMANCE_MEASUREMENT_
    uint64_t ts_start = 0;
    uint64_t ts_stop = 0;

    fpc_get_timestamp(&ts_start);
#endif
    if (num_bytes >= 32) {
        spi_chip_config.com_mod = DMA_TRANSFER;
    }
    else {
        spi_chip_config.com_mod = FIFO_TRANSFER;
    }
    tx_buff = malloc(SPI_ALIGN_SIZE(num_bytes));
    if (!tx_buff) {
        return FPC_RESULT_ERROR_MEMORY;
    }
    memcpy(tx_buff, buff, num_bytes);

    if (ut_pf_spi_send_and_receive(tx_buff, tx_buff, num_bytes, 0 , &spi_chip_config, 1)) {
        ret = FPC_RESULT_ERROR_SPI;
    } else {
        ret = FPC_RESULT_OK;
    }

    free(tx_buff);
#ifdef _PERFORMANCE_MEASUREMENT_
    fpc_get_timestamp(&ts_stop);
    if ((ts_stop - ts_start) > 500) {
        LOGD("KPI fpc_spi_write(), size = %d, ret= %d. time: %d\n", num_bytes, ret, (ts_stop - ts_start));
    }
#endif
    return ret;
}

// ------------------------------------------------------------------------------------------------
int fpc_spi_writeread(uint8_t *tx, size_t tx_bytes, uint8_t *rx, size_t rx_bytes)
{
    int ret = FPC_RESULT_ERROR_UNKNOWN;
    uint32_t total_bytes = tx_bytes + rx_bytes;
    void *tx_buff = NULL;
    void *rx_buff = NULL;
#ifdef _PERFORMANCE_MEASUREMENT_
    uint64_t ts_start = 0;
    uint64_t ts_stop = 0;

    fpc_get_timestamp(&ts_start);
#endif
    if (total_bytes >= 32) {
        spi_chip_config.com_mod = DMA_TRANSFER;
    }
    else {
        spi_chip_config.com_mod = FIFO_TRANSFER;
    }
    tx_buff = malloc(SPI_ALIGN_SIZE(total_bytes));
    if (!tx_buff) {
        return FPC_RESULT_ERROR_MEMORY;
    }
    memset(tx_buff, 0, SPI_ALIGN_SIZE(total_bytes));

    rx_buff = malloc(SPI_ALIGN_SIZE(total_bytes));
    if (!rx_buff) {
        free(tx_buff);
        return FPC_RESULT_ERROR_MEMORY;
    }
    memset(rx_buff, 0, SPI_ALIGN_SIZE(total_bytes));

    memcpy(tx_buff, tx, tx_bytes);

    if (ut_pf_spi_send_and_receive(tx_buff, rx_buff, tx_bytes, rx_bytes , &spi_chip_config, 1)) {
        ret = FPC_RESULT_ERROR_SPI;
    } else {
        ret = FPC_RESULT_OK;
    }

    memcpy(rx, rx_buff, total_bytes);
    free(tx_buff);
    free(rx_buff);

#ifdef _PERFORMANCE_MEASUREMENT_
    fpc_get_timestamp(&ts_stop);
    if ((ts_stop - ts_start) > 500) {
        LOGD("KPI fpc_spi_writeread(), size = %d, ret= %d. time: %d\n", total_bytes, ret, (ts_stop - ts_start));
    }
#endif

    return ret;
}

// ------------------------------------------------------------------------------------------------
int fpc_spi_cs_low(void)
{
    return FPC_RESULT_OK;
}

// ------------------------------------------------------------------------------------------------
int fpc_spi_cs_high(void)
{
    return FPC_RESULT_OK;
}


#ifndef __SPI__INCLUDE__
#define __SPI__INCLUDE__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define    SPI_CPHA    0x01            /* clock phase */
#define    SPI_CPOL    0x02            /* clock polarity */
#define    SPI_MODE_0    (0|0)            /* (original MicroWire) */
#define    SPI_MODE_1    (0|SPI_CPHA)
#define    SPI_MODE_2    (SPI_CPOL|0)
#define    SPI_MODE_3    (SPI_CPOL|SPI_CPHA)

enum spi_sample_sel {
    POSEDGE,
    NEGEDGE
};

enum spi_cs_pol {
    ACTIVE_LOW,
    ACTIVE_HIGH
};

enum spi_cpol {
    SPI_CPOL_0,
    SPI_CPOL_1
};

enum spi_cpha {
    SPI_CPHA_0,
    SPI_CPHA_1
};

enum spi_mlsb {
    SPI_LSB,
    SPI_MSB
};

enum spi_endian {
    SPI_LENDIAN,
    SPI_BENDIAN
};

enum spi_transfer_mode {
    FIFO_TRANSFER,
    DMA_TRANSFER,
    OTHER1,
    OTHER2,
};

enum spi_pause_mode {
    PAUSE_MODE_DISABLE,
    PAUSE_MODE_ENABLE
};
enum spi_finish_intr {
    FINISH_INTR_DIS,
    FINISH_INTR_EN,
};

enum spi_deassert_mode {
    DEASSERT_DISABLE,
    DEASSERT_ENABLE
};

enum spi_ulthigh {
    ULTRA_HIGH_DISABLE,
    ULTRA_HIGH_ENABLE
};

enum spi_tckdly {
    TICK_DLY0,
    TICK_DLY1,
    TICK_DLY2,
    TICK_DLY3
};

enum spi_irq_flag {
    IRQ_IDLE,
    IRQ_BUSY
};

struct mt_chip_conf {
    unsigned int  setuptime;
    unsigned int holdtime;
    unsigned int high_time;
    unsigned int low_time;
    unsigned int cs_idletime;
    unsigned int ulthgh_thrsh;
    enum spi_sample_sel sample_sel;
    enum spi_cs_pol cs_pol;
    enum spi_cpol cpol;
    enum spi_cpha cpha;
    enum spi_mlsb tx_mlsb;
    enum spi_mlsb rx_mlsb;
    enum spi_endian tx_endian;
    enum spi_endian rx_endian;
    enum spi_transfer_mode com_mod;
    enum spi_pause_mode pause;
    enum spi_finish_intr finish_intr;
    enum spi_deassert_mode deassert;
    enum spi_ulthigh ulthigh;
    enum spi_tckdly tckdly;
};

struct arm_spi_ops {
    int (*probe)(const char *configstr);
    int (*spi_recv)(unsigned char* pBuf, int timeout_ms,unsigned int *recved_len);
    int (*spi_send)(unsigned char *pBuf, unsigned char len);
    int (*spi_open)(int mode, int rate);
    int (*spi_close)(void);
    int (*spi_reset)(int mode, int rate);
    //int (*spiTransfer)(const void *tx_buf, void*rx_buf, unsigned int tx_len, unsigned int rx_len,
    //                struct mt_chip_conf *chip_conf, unsigned int flag);
};

unsigned int drSpiSend(const void *tx_buf,
                                                void*rx_buf,
                                                unsigned int tx_len,
                                                unsigned int rx_len,
                                                struct mt_chip_conf *chip_conf,
                                                unsigned int flag);

unsigned int drSpiSend_classic(const void *tx_buf,
                                                void*rx_buf,
                                                unsigned int total_len,
                                                struct mt_chip_conf *chip_conf,
                                                unsigned int flag);

void spi_module_init(void) ;

struct arm_spi_ops *arm_spi_probe(const char *configstr);

void arm_spi_register_driver(struct arm_spi_ops *);

/* Callable once per file (should be enough?) */
#define arm_spi_register(ops)                                        \
    void __attribute__((constructor)) __register_ops(void)    \
    { arm_spi_register_driver(ops); }


#endif /* ! __SPI__INCLUDE__ */

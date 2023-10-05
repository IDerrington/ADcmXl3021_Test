#ifndef ADCMXL3021_H
#define ADCMXL3021_H

#include <inttypes.h>
#include "driver/spi_master.h"

#define PAGE_ID2        0x00
#define TEMP_OUT        0x02
#define SUPPLY_OUT      0x04
#define FFT_AVG1        0x06
#define FFT_AVG2        0x08
#define BUF_PNTR        0x0A
#define REC_PNTR        0x0C
#define X_BUF           0x0E
#define Y_BUF           0x10
#define Z_BUF_RSS_BUF   0x12
#define X_ANULL         0x14
#define Y_ANULL         0x16
#define Z_ANULL         0x18
#define REC_CTRL        0x1A
#define RT_CTRL         0x1C
#define REC_PRD         0x1E
#define ALM_F_LOW       0x20
#define ALM_F_HIGH      0x22
#define ALM_X_MAG1      0x24
#define ALM_Y_MAG1      0x26
#define ALM_Z_MAG1      0x28
#define ALM_X_MAG2      0x2A
#define ALM_Y_MAG2      0x2C
#define ALM_Z_MAG2      0x2E
#define ALM_PNTR        0x30
#define ALM_S_MAG       0x32
#define ALM_CTRL        0x34
#define Reserved        0x36
#define FILT_CTRL       0x38
#define AVG_CNT         0x3A
#define DIAG_STAT       0x3C
#define GLOB_CMD        0x3E
#define ALM_X_STAT      0x40
#define ALM_Y_STAT      0x42
#define ALM_Z_STAT      0x44
#define ALM_X_PEAK      0x46
#define ALM_Y_PEAK      0x48
#define ALM_Z_PEAK      0x4A
#define TIME_STAMP_L    0x4B
#define TIME_STAMP_H    0x4D
#define Reserved_1      0x50
#define DAY_REV         0x52
#define YEAR_MON        0x54
#define PROD_ID         0x56
#define SERIAL_NUM      0x58
#define USER_SCRATCH    0x5A
#define REC_FLASH_CNT   0x5C
#define MISC_CTRL       0x64

/**
 * @breif Initiate comms with device read back info
 * @param spi_device_handle_t
 */
esp_err_t InitAdcmXl(spi_device_handle_t spi);

/**
 * @breif
*/
esp_err_t ReadAdcmXlReg(spi_device_handle_t spi, uint8_t address, uint16_t *data);

/**
 * @breif
*/
esp_err_t WriteAdcmXlReg(spi_device_handle_t spi, uint8_t address, uint16_t *data);

typedef union
{
    struct
    {
        uint8_t cntr;
        uint8_t header;
        uint16_t xVal[32];
        uint16_t yVal[32];
        uint16_t zVal[32];
        uint16_t temp;
        uint16_t sts;
        uint16_t crc16;
    };
    uint8_t data[200];
}ADCMXL3021_t;

#endif
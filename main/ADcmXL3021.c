#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ADcmXL3021.h"
#include "driver/gpio.h"
#include <rom/ets_sys.h>

#define ADcmXL3021_WRITE 0x80
#define T_STALL 16

typedef union
{
    struct
    {
        uint8_t address;
        uint8_t data;
    };
    uint8_t raw[2];
}adcmxl3021_cmd_t;

/****************************Public functions*********************************/
esp_err_t InitAdcmXl(spi_device_handle_t spi)
{
    
    esp_err_t ret;
    uint16_t data = 0;

    
    ret = WriteAdcmXlReg(spi, PAGE_ID2, &data);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    ret = ReadAdcmXlReg(spi, DIAG_STAT, &data);

    if (ESP_OK == ret)
    {
        printf("Prod Id = 0x%x \n \r", data);
        ret = ReadAdcmXlReg(spi, PROD_ID, &data);
    }

    if (ESP_OK == ret)
    {
        printf("Prod Id = 0x%x \n \r", data); 
        ret = ReadAdcmXlReg(spi, YEAR_MON, &data);
    }

    if (ESP_OK == ret)
    {
        printf("Year / Mon =  0x%x \n \r", data);
        ret = ReadAdcmXlReg(spi, DAY_REV, &data);
    }
    
    if (ESP_OK == ret)
    {
        printf("Day / Rev =  0x%x \n \r", data);
        ret = ReadAdcmXlReg(spi, SERIAL_NUM, &data);
    }
    
    if  (ESP_OK == ret)
    {            
        printf("Serial Num = 0x%x \n \r", data);
    }
    
    return ret;
}

esp_err_t ReadAdcmXlReg(spi_device_handle_t spi, uint8_t address, uint16_t *data)
{
    esp_err_t ret = ESP_OK;
    spi_transaction_t msg;

    spi_device_acquire_bus(spi, portMAX_DELAY);

    memset(&msg, 0, sizeof(msg));       //Zero out the transaction
    msg.flags = SPI_TRANS_USE_TXDATA;
    msg.tx_data[0] = address;           // !R/W
    msg.tx_data[1] = 0;
    msg.length = 16;

    ret = spi_device_polling_transmit(spi, &msg);

    vTaskDelay(pdMS_TO_TICKS(100));

    if (ESP_OK == ret)
    {
        memset(&msg, 0, sizeof(msg));       //Zero out the transaction
        msg.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
        msg.length = 2 * 8;
        msg.rxlength = 2 * 8;

        ret = spi_device_polling_transmit(spi, &msg);
    }

    if (ESP_OK == ret)
    {
        *data = (uint16_t)msg.rx_data[1] + ((uint16_t)(msg.rx_data[0]) << 8);
    }

    

    // Release bus
    spi_device_release_bus(spi);
    return ret;
}

esp_err_t WriteAdcmXlReg(spi_device_handle_t spi, uint8_t address, uint16_t *data)
{
    esp_err_t ret;
    spi_transaction_t msg;

    spi_device_acquire_bus(spi, portMAX_DELAY);

    // Send lower address data
    memset(&msg, 0, sizeof(msg));       //Zero out the transaction
    msg.flags = SPI_TRANS_USE_TXDATA;
    msg.tx_data[0] = address | ADcmXL3021_WRITE;
    msg.tx_data[1] =  (uint8_t) *data;
    msg.length = 8*2;

    ret = spi_device_polling_transmit(spi, &msg);

    ets_delay_us(T_STALL);

    // Send upper address and data
    memset(&msg, 0, sizeof(msg));       //Zero out the transaction
    msg.flags = SPI_TRANS_USE_TXDATA;
    msg.tx_data[0] = (address+1) | ADcmXL3021_WRITE;
    msg.tx_data[1] =  (uint8_t) (*data >> 8);
    msg.length = 8*2;

    ret = spi_device_polling_transmit(spi, &msg);

    spi_device_release_bus(spi);
    
    return ret; 
}
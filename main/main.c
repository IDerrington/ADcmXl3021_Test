/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "ADcmXL3021.h"
#include "esp_log.h"

static const char *TAG = "M";
#define RECORD_LEN 10

#ifdef CONFIG_IDF_TARGET_ESP32
#define ADCM_HOST    HSPI_HOST

#define PIN_NUM_MISO    25
#define PIN_NUM_MOSI    23
#define PIN_NUM_CLK     19
#define PIN_NUM_CS      22
#define PIN_ADCMXL_BUSY  2
#define PIN_ADCMXL_RST   0


#endif

ADCMXL3021_t buf[RECORD_LEN];

esp_err_t InitGPIO(void)
{
    gpio_config_t io_conf;
    esp_err_t err;

    // Configure ADcl Busy line
    io_conf.intr_type       = GPIO_INTR_DISABLE;      // disable interrupt
    io_conf.mode            = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask    = 1ULL << PIN_ADCMXL_BUSY;
    err = gpio_config(&io_conf);

    if (ESP_OK == err)
    {
        //Configure the Adcl Reset line
        io_conf.intr_type       = GPIO_INTR_DISABLE;      // disable interrupt
        io_conf.mode            = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask    = 1ULL << PIN_ADCMXL_RST;
        err = gpio_config(&io_conf);
    }

    return err;
}

esp_err_t InitSPI(spi_device_handle_t *spi)
{
    esp_err_t err;
    
    spi_bus_config_t buscfg=
    {
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=sizeof(ADCMXL3021_t), 
    };

    spi_device_interface_config_t devcfg={
        .clock_speed_hz=14*1000*1000,      // Clock out at 12.5 MHz
        .mode=3,                             // SPI mode 3
        .spics_io_num=PIN_NUM_CS,            // CS pin
        .queue_size=7,                       // We want to be able to queue 7 transactions at a time
        .cs_ena_pretrans = 1,
        .cs_ena_posttrans = 1,
    };

    // Initialize the SPI bus
    err = spi_bus_initialize(ADCM_HOST, &buscfg, SPI_DMA_CH_AUTO);
   
    if (ESP_OK == err)
    {
        // Attach the ADcm to the SPI bus
        err = spi_bus_add_device(ADCM_HOST, &devcfg, spi);
    }

    return err;
}

#define DELAY pdMS_TO_TICKS(0)

void app_main(void)
{
    esp_err_t err;
    spi_device_handle_t spi;
    uint16_t data;

    /*
     *  Configure hardware
     */
    ESP_ERROR_CHECK( InitSPI(&spi) );
    ESP_ERROR_CHECK( InitGPIO() );

    /*
     * Reset ADcm and place in to known state
     */
    ESP_LOGI(TAG, "Assert reset ADcmXL3021");
    gpio_set_level(PIN_ADCMXL_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI(TAG, "De-assert reset ADcmXL3021");
    gpio_set_level(PIN_ADCMXL_RST, 1);

    /*
     * Wait for busy line to de-assert
     */
    do
    {
        vTaskDelay(pdMS_TO_TICKS(200));
        ESP_LOGI(TAG, "Waiting for busy to de-assert...");
    }
    while(gpio_get_level(PIN_ADCMXL_BUSY) == 0);

    /*
     * Basic Initialise ADcmXL3021
     */
    ESP_LOGI(TAG, "Init Device");
    ESP_ERROR_CHECK (InitAdcmXl(spi));
 
    /**
     * Configure the device for RTS mode
     * 
     * 1) Write to misc control
     * 2) Wrtite to rec_ctrl. Puts device into RTS mode)
     * 3) Write to GLOB_CMD. Starts RTS
     */
    data =  0;
    err = WriteAdcmXlReg(spi, MISC_CTRL, &data);

    if (ESP_OK == err)
    {
        ReadAdcmXlReg(spi,  MISC_CTRL, &data);  
        ESP_LOGI(TAG, "MISC_CTRL = 0x%x",data);

        data = 0x8003;
        err = WriteAdcmXlReg(spi,  REC_CTRL, &data);
    }

    vTaskDelay(DELAY);

    if (ESP_OK == err)
    {
        ReadAdcmXlReg(spi,  REC_CTRL, &data);  
        ESP_LOGI(TAG, "REC_CTRL = 0x%x",data);

        data = 0x0800;
        err = WriteAdcmXlReg(spi,  GLOB_CMD, &data);
    }

    vTaskDelay(pdMS_TO_TICKS(5));

    for(uint16_t idx = 0; idx < RECORD_LEN; idx++)
    {
        spi_transaction_t msg;
        
        /*
         * Crude implementation... waiting for /BUSY to de-assert
         */
        do
        {
        }
        while(gpio_get_level(PIN_ADCMXL_BUSY) == 0);

        memset(&msg, 0, sizeof(msg));            
        memset(&buf[idx], 0, sizeof(buf));       
        
        msg.length = sizeof(ADCMXL3021_t) * 8;

        msg.tx_buffer = buf[idx].data;
        msg.rx_buffer = buf[idx].data;

        err = spi_device_polling_transmit(spi, &msg);
    }

    /*
     * Display the data on stdio
     */
    for(uint16_t idx = 0; idx < RECORD_LEN; idx++)
    {
        ESP_LOGI(TAG,"RECORD #: %d", idx);
        ESP_LOGI(TAG,"hdr = 0x%X , cntr = 0x%X", buf[idx].header, buf[idx].cntr);
        ESP_LOGI(TAG,"X Values");
        for(uint16_t idy = 0; idy < 32; idy++)
        {
            printf("0x%X ",  buf[idx].xVal[idy]);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        printf("\n\r");

        ESP_LOGI(TAG,"Y Values");
        for(uint16_t idy = 0; idy < 32; idy++)
        {
            printf("0x%X ",  buf[idx].yVal[idy]);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        printf("\n\r");

        ESP_LOGI(TAG,"Z Values");
        for(uint16_t idy = 0 ; idy < 32; idy++)
        {
            printf("0x%X ",  buf[idx].zVal[idy]);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        printf("\n\r");

        ESP_LOGI(TAG,"t = 0x%X , sts = 0x%X, crc = 0x%X", buf[idx].temp, buf[idx].sts,  buf[idx].crc16);
        printf("\n\n\r");
    }

    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
 
}

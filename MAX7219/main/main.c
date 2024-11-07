#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_idf_version.h>
#include <max7219.h>

#ifndef APP_CPU_NUM
    #define APP_CPU_NUM PRO_CPU_NUM
#endif

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 0, 0)
    #define HOST    HSPI_HOST
#else
    #define HOST    SPI2_HOST
#endif

#define CASCADE_SIZE 1
#define MOSI_PIN 23
#define CS_PIN 5
#define CLK_PIN 18

const uint64_t IMAGES[] = {
  0x0000000000000001,
  0x0000000000000002,
  0x0000000000000004,
  0x0000000000000008,
  0x0000000000000010,
  0x0000000000000020,
  0x0000000000000040,
  0x0000000000000080,
  0x0000000000008000,
  0x0000000000004000,
  0x0000000000002000,
  0x0000000000001000,
  0x0000000000000800,
  0x0000000000000400,
  0x0000000000000200,
  0x0000000000000100,
  0x0000000000010000,
  0x0000000000020000,
  0x0000000000040000,
  0x0000000000080000,
  0x0000000000100000,
  0x0000000000200000,
  0x0000000000400000,
  0x0000000000800000,
  0x0000000080000000,
  0x0000000040000000,
  0x0000000020000000,
  0x0000000010000000,
  0x0000000008000000,
  0x0000000004000000,
  0x0000000002000000,
  0x0000000001000000,
  0x0000000100000000,
  0x0000000200000000,
  0x0000000400000000,
  0x0000000800000000,
  0x0000001000000000,
  0x0000002000000000,
  0x0000004000000000,
  0x0000008000000000,
  0x0000800000000000,
  0x0000400000000000,
  0x0000200000000000,
  0x0000100000000000,
  0x0000080000000000,
  0x0000040000000000,
  0x0000020000000000,
  0x0000010000000000,
  0x0001000000000000,
  0x0002000000000000,
  0x0004000000000000,
  0x0008000000000000,
  0x0010000000000000,
  0x0020000000000000,
  0x0040000000000000,
  0x0080000000000000,
  0x8000000000000000,
  0x4000000000000000,
  0x2000000000000000,
  0x1000000000000000,
  0x0800000000000000,
  0x0400000000000000,
  0x0200000000000000,
  0x0100000000000000,
  0xffffffffffffffff,
  0xffffffe7e7ffffff,
  0xffffc3c3c3c3ffff,
  0xff818181818181ff
};
const int IMAGES_LEN = sizeof(IMAGES)/8;

void task(void *pvParameter)
{

    spi_bus_config_t cfg = {
       .mosi_io_num = MOSI_PIN,
       .miso_io_num = -1,
       .sclk_io_num = CLK_PIN,
       .quadwp_io_num = -1,
       .quadhd_io_num = -1,
       .max_transfer_sz = 0,
       .flags = 0
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HOST, &cfg, 1));


    max7219_t dev = {
       .cascade_size = CASCADE_SIZE,
       .digits = 0,
       .mirrored = true
    };
    ESP_ERROR_CHECK(max7219_init_desc(&dev, HOST, MAX7219_MAX_CLOCK_SPEED_HZ, CS_PIN));
    ESP_ERROR_CHECK(max7219_init(&dev));
   while (1)
    {
        for (uint8_t i=0; i < IMAGES_LEN; i++)
        //max7219_draw_image_8x8(&dev,0,(uint8_t *)symbols + i*8 + offs);
        {
            max7219_draw_image_8x8(&dev,0,(uint8_t *)IMAGES+ i*8);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        char myString[] = "Hello World";
        
        max7219_draw_string_8x8(&dev,myString);
    }
}

void app_main()
{
    xTaskCreatePinnedToCore(task, "task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL, APP_CPU_NUM);
}
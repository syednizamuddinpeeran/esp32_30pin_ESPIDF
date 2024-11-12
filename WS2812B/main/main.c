#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_random.h"
#include "WS2812B.h"

#define LED_STRIP_GPIO 2  // GPIO pin connected to the data line of the LED strip
#define LED_STRIP_LENGTH 64  // Total number of LEDs in the strip

static const char *TAG = "LED_STRIP";
void color_wipe(led_strip_handle_t led_strip, uint8_t red, uint8_t green, uint8_t blue, int delay_ms) {
    for (int i = 0; i < LED_STRIP_LENGTH; i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}
void app_main(void) {
    // Configuration for the LED strip
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,
        .max_leds = LED_STRIP_LENGTH,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {
            .invert_out = false,
        },
    };

    // RMT configuration for the LED strip
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,  // 10 MHz
        .mem_block_symbols = 64,
        .flags = {
            .with_dma = false,
        },
    };

    // Create the LED strip object
    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    // Set the color of each LED in the strip
    for (int i = 0; i < LED_STRIP_LENGTH; i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 255, 0, 0));  // Set to red color
    }

    // Refresh the strip to show the colors
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));

    
    // Main loop
    color_wipe(led_strip, 0,0,0,10);
    uint64_t images[2];
    images[0] = 0x0000007f3e1c0800;
    images[1] = 0x2222227f3e1c0800;
    uint8_t rgbArray[2][64][3];
    int scroll = 0;
    int r = 255;
    int g = 255;
    int b = 255;
    uint64ToRGBArray(images, rgbArray, r, g, b);
    while (true) {

        int selectorImage =0;
        for (int i = 0; i < 128; i++) {
            int pos = (i + scroll) % 64;
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, pos, rgbArray[selectorImage][i][0], rgbArray[selectorImage][i][1], rgbArray[selectorImage][i][2]));
            if(i%64==0)
            {
                selectorImage =1;
            }
        }

        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(100));

        scroll = (scroll +1) % 8;
    }

    // Clean up
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
    ESP_ERROR_CHECK(led_strip_del(led_strip));
}
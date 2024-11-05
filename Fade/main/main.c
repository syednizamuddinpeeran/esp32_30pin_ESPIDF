#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define LED_PIN 2
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY 5000 // Frequency in Hertz. Set frequency at 5 kHz
#define DUTY_CYCE_ms 2000

void app_main(void)
{
    // Configure the timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Configure the channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_FADE_END,
        .gpio_num = LED_PIN,
        .duty = 0, // Set duty to 0%
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);

    // Install fade function
    ledc_fade_func_install(0);

    while (1) {
        // Fade in
        ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 8191, DUTY_CYCE_ms); // 8191 is 100% duty cycle for 13-bit resolution
        ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_NO_WAIT);
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        // Fade out
        ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 0, DUTY_CYCE_ms);
        ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_NO_WAIT);
        vTaskDelay(DUTY_CYCE_ms / portTICK_PERIOD_MS);
    }
}

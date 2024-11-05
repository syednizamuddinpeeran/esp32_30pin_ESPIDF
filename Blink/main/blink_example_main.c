#include "freertos/FreeRTOS.h"
// to use vTaskDelay,portTICK_PERIOD_MS
#include "driver/gpio.h"
//to use gpio_set_level,gpio_reset_pin,gpio_set_direction,GPIO_MODE_OUTPUT

static uint8_t s_led_state = 0;
static const uint8_t LED_GPIO = 2;
static const int DELAY = (int)1000;

static void flip_led_state(void)
{
    s_led_state = !s_led_state;
    gpio_set_level(LED_GPIO, s_led_state);
}

static void configure_led(void)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    configure_led();
    while (1) {
        flip_led_state();
        vTaskDelay(DELAY / portTICK_PERIOD_MS);
    }
}

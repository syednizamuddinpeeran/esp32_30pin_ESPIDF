#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "HelloWorldExample";


void app_main(void)
{
    ESP_LOGI(TAG, "Hello %s!", "World");
}
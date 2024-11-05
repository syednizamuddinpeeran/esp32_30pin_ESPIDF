#include <stdio.h>
#include "esp_log.h"

static const char *TAG = "LoggingExample";


void app_main(void)
{
    ESP_LOGI(TAG, "This is an info log");
    ESP_LOGW(TAG, "This is a warning log");
    ESP_LOGE(TAG, "This is an error log");
}

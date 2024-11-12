#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static const char *TAG = "Keypad";

static int rows;      
static int cols;      
static int *rowIo;    
static int *colIo;    
static int *btnVals;   

/* --------------------------------------------------------*/

void keypad_setup(int rowCount, int columnCount, int *rowPinCons, int *columnPinCons, int *buttonValues)
{
    esp_err_t err = ESP_OK;
    rows = rowCount;
    cols = columnCount;
    rowIo = rowPinCons;
    colIo = columnPinCons;
    btnVals = buttonValues;

    /**< set row as output */
    for (int row = 0; row < rows; row++)
    {
        err = gpio_set_direction(rowIo[row], GPIO_MODE_OUTPUT);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to set the row pin %d as Key pad scan output. error %s", rowIo[row], esp_err_to_name(err));
        }
        ESP_LOGI(TAG, "set the row pin %d as Key pad scan output.", rowIo[row]);
    }

    /**< set column as input */
    for (int col = 0; col < cols; col++)
    {
        err = gpio_set_direction(colIo[col], GPIO_MODE_INPUT);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to set the column pin %d as Key pad scan input. error %s", colIo[col], esp_err_to_name(err));
        }
        err = gpio_pulldown_en(colIo[col]);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to set the column pin %d as Key pad scan input pull down. error %s", colIo[col], esp_err_to_name(err));
        }
        err = gpio_set_pull_mode(colIo[col], GPIO_PULLDOWN_ONLY);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to set the column pin %d as Key pad scan input pull down enable. error %s", colIo[col], esp_err_to_name(err));
        }
        ESP_LOGI(TAG, "set the column pin %d as Key pad scan input.", colIo[col]);
    }
    return;
}

char scanForSingleKeyOnce(char KeyToReturnWhenNOKeyPressed)
{
    for (int i = 0; i < rows; i++)
    {
        for (int k = 0; k < rows; k++)
        {
            gpio_set_level(rowIo[k], 0);
        }
        gpio_set_level(rowIo[i], 1);
        for (int j = 0; j < cols; j++)
        {
            if (gpio_get_level(colIo[j]))
            {
                // Debounce delay
                vTaskDelay(50 / portTICK_PERIOD_MS);
                if (gpio_get_level(colIo[j])) // Check again to confirm the key press
                {
                    ESP_LOGI(TAG, "Key in row %d and col %d Pressed: %c", i, j, btnVals[(i * cols) + j]);
                    for (int k = 0; k < rows; k++)
                    {
                        gpio_set_level(rowIo[k], 0);
                    }
                    vTaskDelay(200 / portTICK_PERIOD_MS);
                    return btnVals[(i * cols) + j];
                }
            }
        }
        gpio_set_level(rowIo[i], 0);
    }
    return KeyToReturnWhenNOKeyPressed;
}

char scanForSingleKeyWithTimeOut(char KeyToReturnWhenNOKeyPressed,TickType_t timeout_ticks) {
    TickType_t start_tick = xTaskGetTickCount();
    char keyPressed = KeyToReturnWhenNOKeyPressed;
    ESP_LOGI(TAG, "Waiting for key press will timeout in %d",  (int)timeout_ticks);
    while ((xTaskGetTickCount() - start_tick) < timeout_ticks) {
        keyPressed = scanForSingleKeyOnce(KeyToReturnWhenNOKeyPressed);
        if (keyPressed != KeyToReturnWhenNOKeyPressed)
          return keyPressed;
        vTaskDelay(pdMS_TO_TICKS(10)); // Check every 10ms
    }
    ESP_LOGI(TAG, "Key press not detected!");
    return keyPressed;
}
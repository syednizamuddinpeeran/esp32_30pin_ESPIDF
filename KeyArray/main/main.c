#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "keyarray.h"

static const char *TAG = "Main";

void app_main(void)
{
    int rows1[4] = {4,27, 26, 25}; 
    int cols1[4] = { 33,32,18,19}; 
    char values[16] = { '1', '2', '3', '/', 
                      '4', '5', '6', '*',
                      '7', '8', '9', '-',
                      '.', '0', '^', '+' };
    keypad_setup(4, 4, rows1, cols1, values);
    while (1)
    {
        TickType_t ticks = pdMS_TO_TICKS(10000);
        char keyPressed = scanForSingleKeyWithTimeOut('|',ticks);
        if (keyPressed != '|')
            printf("%c \n", (char)keyPressed);
    }
}

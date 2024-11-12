#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "keyarray.h"
#include "max7219.h"
#include "freertos/queue.h"
#include "esp_random.h"

static const char *TAG = "Main";
#define HOST    SPI2_HOST
#define APP_CPU_NUM PRO_CPU_NUM

#define CASCADE_SIZE 1
#define MOSI_PIN 23
#define CS_PIN 5
#define CLK_PIN 21
QueueHandle_t queue;
QueueHandle_t queue1;
max7219_t dev = {
    .cascade_size = CASCADE_SIZE,
    .digits = 0,
    .mirrored = true
};

spi_bus_config_t cfg = {
    .mosi_io_num = MOSI_PIN,
    .miso_io_num = -1,
    .sclk_io_num = CLK_PIN,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 0,
    .flags = 0
};
const int *rows1[4] = {4,27, 26, 25}; 
const int *cols1[4] = { 33,32,18,19}; 
const int *values[16] = {'1', '2', '3', '/', '4', '5', '6', '*', '7', '8', '9', '-', '.', '0', '^', '+'};
int time=15000;
void setUP() {
    ESP_ERROR_CHECK(spi_bus_initialize(HOST, &cfg, 1));
    ESP_ERROR_CHECK(max7219_init_desc(&dev, HOST, MAX7219_MAX_CLOCK_SPEED_HZ, CS_PIN));
    ESP_ERROR_CHECK(max7219_init(&dev));
    keypad_setup(4, 4, rows1, cols1, values);
}
char digit_to_char(int digit) {
    return digit + '0';
}

void Task1(void *pvParameters) {
    int counter = 0 ;
    int level = 0;
    while (1) {
        char message = '?';
        char myString[] = "1+1?";
        int num1 = esp_random() % 5;
        int num2 = esp_random() % 5;
        int operator = esp_random() % 2;
        int res;
        if (operator !=0)
        {
            myString[1] = '-';
            if(num1 < num2){
                int temp = num2;
                num2 = num1;
                num1 = temp;
            }
            res = num1-num2;
        }
        else
        {
            myString[1] = '+';
            res = num1+num2;
        } 
        myString[0] =  num1 + '0';
        myString[2] =  num2 + '0';
        uint64_t okay_image = 0x4020180c06060c00;
        uint64_t okay_timeout = 0x1e2125a5eda1211e;
        if (xQueueSend(queue, &message, portMAX_DELAY) != pdPASS) {
            printf("Failed to send to queue\n");
        }
        while(xQueueReceive(queue1, &message, 0) != pdPASS)
        {
            max7219_draw_string_8x8(&dev, myString);
            vTaskDelay(pdMS_TO_TICKS(600));
        }
        printf("task 1 Received: %c\n", message);
        
        if (res == message - '0'){
            counter++;
            max7219_draw_image_8x8(&dev,0,&okay_image);
        }
        else
        {
            counter=0;
            if ('?'==message)
                max7219_draw_image_8x8(&dev,0,&okay_timeout);
            else
                max7219_draw_char_8x8(&dev, 0,'*');
        }
            
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (counter ==5)
        {
            counter =0;
            max7219_draw_char_8x8(&dev, 0,level + '0');
            time = time - (1000 + (50*level));
            level++;
            vTaskDelay(pdMS_TO_TICKS(400));
            max7219_draw_string_8x8(&dev, "<<<");
            max7219_draw_char_8x8(&dev, 0,level + '0');
            vTaskDelay(pdMS_TO_TICKS(400));
            max7219_draw_string_8x8(&dev, "<<<");
        }
    }
}

void Task2(void *pvParameters) {
    char message;
    while (1) {
        if (xQueueReceive(queue, &message, portMAX_DELAY) == pdPASS) {
            printf("task 2 Received: %c\n", message);
            if (message == '?') {
                TickType_t ticks = pdMS_TO_TICKS(time);
                char keyPressed = scanForSingleKeyWithTimeOut(message, ticks);
                message = keyPressed;
                if (xQueueSend(queue1, &message, portMAX_DELAY) != pdPASS) {
                    printf("Failed to send to queue\n");
                }
                printf("task 2 Sent: %c\n", message);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void app_main() {
    queue = xQueueCreate(10, sizeof(char *));
    if (queue == NULL) {
        printf("Failed to create queue\n");
        return;
    }
    queue1 = xQueueCreate(10, sizeof(char *));
    if (queue1 == NULL) {
        printf("Failed to create queue\n");
        return;
    }
    setUP();
    xTaskCreate(Task1, "Task1", 2048, NULL, 1, NULL);
    xTaskCreate(Task2, "Task2", 2048, NULL, 1, NULL);

}

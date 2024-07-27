
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freeRTOS/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "FreeRtos";

QueueHandle_t xQueue = NULL;  //创建队列句柄

typedef struct {
    int a;
    char b[10];
} queue_data_t;  //定义一个结构体，包含一个int和一个char数组


//创建2个任务来测试队列
//---------------------------------TASK-A-------------------------------------
void vTask1(void *pvParameters)
{
    int i = 0;
    queue_data_t data;
    
    while (1) {
        ESP_LOGI(TAG, "Task1 send %d\n", i);
        xQueueSend(xQueue, &data, 0);  //发送数据到队列
        data.a = i++;
        data.b[i] = 'a' + i ;
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

//--------------------------------TASK-B--------------------------------------

void vTask2(void *pvParameters)
{
    int i = 0;
    queue_data_t buffer;

    while (1) {
        xQueueReceive(xQueue, &buffer, portMAX_DELAY);  //接收数据
        ESP_LOGI(TAG, "Task2 receive %d, %c\n", buffer.a,buffer.b[i++]);
    
    }
}
//----------------------------------------------------------------------------

void app_main(void)
{
    xQueue = xQueueCreate(5, sizeof(queue_data_t));  //创建一个队列，长度为5，数据类型为结构体

    xTaskCreatePinnedToCore(vTask1, "Task1", 2048, NULL, 10, NULL, 0);  //创建任务1
    xTaskCreatePinnedToCore(vTask2, "Task2", 2048, NULL, 10, NULL, 0);  //创建任务2

    while (1) {
        ESP_LOGI(TAG, "This is a freertos example\n");
        

        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}

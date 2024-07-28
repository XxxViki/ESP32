
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freeRTOS/queue.h"
#include "freertos/semphr.h"

#include "esp_task_wdt.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "FreeRtos";


//---------------------------------QUEUE--------------------------------------
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


//*****************************************Sepmaphore************* */
SemaphoreHandle_t xSemaphore = NULL;  //创建信号量句柄
SemaphoreHandle_t xMutex = NULL;  //创建互斥信号量句柄
SemaphoreHandle_t xCountingSemaphore = NULL;  //创建计数信号量句柄

//*****************************************Sepmaphore************* */

void vTask3(void *pvParameters)
{
    while (1) {
        for(int i = 0; i < 10; i++)
        {
            xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);  //获取信号量
            ESP_LOGI(TAG, "Task3 get the semaphore %d\n", i);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        }
         esp_task_wdt_reset();
    }
}


void vTask4(void *pvParameters)
{
    while (1) {
        for (size_t i = 0; i < 3; i++)
        {          
            // xSemaphoreGive(xSemaphore);  //释放信号量
           xSemaphoreGive( xCountingSemaphore);
            ESP_LOGI(TAG, "Task4 give the semaphore %d\n", i);

        }

        vTaskDelay(3000 / portTICK_PERIOD_MS);
          esp_task_wdt_reset();
    }
   
}

//++++++++++++++++++++++++++++++++EventGroup+++++++++++++++++++++++++++++ */
EventGroupHandle_t xEventGroup = NULL;  //创建事件组句柄
EventBits_t uxBits;  //事件组的bit位

#define BIT_0 (1 << 0)
#define BIT_1 (1 << 1)


//++++++++++++++++++++++++++++++++EventGroup+++++++++++++++++++++++++++++ */

void vTask5(void *pvParameters)
{
    while (1) {
        xEventGroupSetBits(xEventGroup, BIT_0);  //设置事件组的bit0
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        xEventGroupSetBits(xEventGroup, BIT_1);  //设置事件组的bit0
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void vTask6(void *pvParameters)
{
    while (1) {
        uxBits = xEventGroupWaitBits(xEventGroup, BIT_0|BIT_1, pdTRUE, pdFALSE, portMAX_DELAY);  //等待事件组的bit0
        if ((uxBits & (BIT_0|BIT_1)) == (BIT_0|BIT_1)) {
            ESP_LOGI(TAG, "Task6 get the bit0 and bit1\n");
        } else if ((uxBits & BIT_0) == BIT_0) {
            ESP_LOGI(TAG, "Task6 get the bit1\n");
        } else if ((uxBits & BIT_1) == BIT_1) {

            ESP_LOGI(TAG, "Task6 get the bit0\n");
        }
    }
}
//++++++++++++++++++++++++++++++++EventGroup+++++++++++++++++++++++++++++ */
void app_main(void)
{

    //TODO 需要了解为什么添加vTaskStartScheduler();会异常重启
    // esp_task_wdt_config_t wdt_config = {
    //     .timeout_ms = 5000,
    //     .idle_core_mask = 0,
    //     .trigger_panic = true,
    // };
    // //   esp_task_wdt_init(&wdt_config);
    // //  esp_task_wdt_delete(NULL);
    // esp_err_t err = esp_task_wdt_reconfigure(&wdt_config);

    //队列
    xQueue = xQueueCreate(5, sizeof(queue_data_t));  //创建一个队列，长度为5，数据类型为结构体

    // xTaskCreatePinnedToCore(vTask1, "Task1", 2048, NULL, 10, NULL, 0);  //队列
    // xTaskCreatePinnedToCore(vTask2, "Task2", 2048, NULL, 10, NULL, 0);  //队列

    //信号量
    xMutex = xSemaphoreCreateMutex();  //创建一个互斥信号量
    xSemaphore = xSemaphoreCreateBinary();  //创建一个二值信号量
    xCountingSemaphore = xSemaphoreCreateCounting(10, 7);  //创建一个计数信号量，最大计数为10，初始计数为0

    // xTaskCreatePinnedToCore(vTask3, "Task3", 2048, NULL, 10, NULL, 0);  //信号量
    // xTaskCreatePinnedToCore(vTask4, "Task4", 2048, NULL, 10, NULL, 0);  //信号量

    //事件组
    xEventGroup = xEventGroupCreate();  //创建一个事件组
    xTaskCreatePinnedToCore(vTask5, "Task5", 2048, NULL, 10, NULL, 0);  //事件组
    xTaskCreatePinnedToCore(vTask6, "Task6", 2048, NULL, 10, NULL, 0);  //事件组


    //TODO 需要了解为什么添加vTaskStartScheduler();会异常重启
    // vTaskStartScheduler();
}

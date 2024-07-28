#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define LED    GPIO_NUM_25

#define DUTY_FULl  BIT0
#define DUTY_EMPTY  BIT1

EventGroupHandle_t led_event_group; //事件标志组句柄

bool IRAM_ATTR ledc_finish_cb(const ledc_cb_param_t *ledc_cb, void *arg)  //程序放在内存中执行
{
    BaseType_t task_woken = pdFALSE;

    if(ledc_cb->duty)   //当前占空比
    {
        
        xEventGroupSetBitsFromISR(led_event_group, DUTY_FULl,&task_woken); //设置事件标志位(led_event_group, DUTY_EMPTY);
    }
    else 
    {
        xEventGroupSetBitsFromISR(led_event_group, DUTY_EMPTY,&task_woken); //设置事件标志位(led_event_group, DUTY_EMPTY);
    }
    
    //vTaskDelay(pdMS_TO_TICKS(1000));  //TODO 中断回调中添加延时会导致重启

    return task_woken;
        
}

void led_task(void *pvParameter)
{
    
    while(1)
    {

#if 0
        gpio_set_level(LED, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        gpio_set_level(LED, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
#else

    EventBits_t bits = xEventGroupWaitBits(led_event_group, DUTY_FULl | DUTY_EMPTY, pdTRUE, pdFALSE, pdMS_TO_TICKS(1000)); //等待事件标志位

    if(bits & DUTY_FULl)   //当前占空比
    {
        ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0, 0, 1000); //设置LED的亮度为1000，持续时间为1000ms
        ledc_fade_start(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT); //开始淡入
    }
   if(bits & DUTY_EMPTY)
    {
        ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0, 8191, 1000); //设置LED的亮度为1000，持续时间为1000ms
        ledc_fade_start(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT); //开始淡入
    }

    ledc_cbs_t cbs = {
        .fade_cb = ledc_finish_cb
    };

    ledc_cb_register(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0,&cbs, NULL);//注册回调函数

#endif
    }
}

void app_main(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL<<LED),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };

    gpio_config(&io_conf);

    //初始化定时器
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, //占空比分辨率
        .freq_hz = 5000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&ledc_timer);
    
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = LED,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE
    };

    ledc_channel_config(&ledc_channel);

    ledc_fade_func_install(0); //TODO 安装淡入淡出功能  

    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0, 0, 1000); //设置LED的亮度为1000，持续时间为1000ms
    ledc_fade_start(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT); //开始淡入

    ledc_cbs_t cbs = {
        .fade_cb = ledc_finish_cb
    };

    ledc_cb_register(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0,&cbs, NULL);//注册回调函数

    led_event_group = xEventGroupCreate(); //创建事件标志组

    xTaskCreatePinnedToCore(&led_task, "led_task", 2048, NULL, 5, &led_task, 1);

}

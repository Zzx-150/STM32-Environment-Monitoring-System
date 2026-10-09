#include "stm32f10x.h"
#include "usart.h"
#include "data_process.h"
#include "display_ui.h"
#include "dht11.h"
#include "Delay.h"
#include "adc.h"
#include "oled.h"
#include "key.h"
#include "relay.h"
#include "esp8266.h"
#include <stdio.h>

/* ======================== 全局变量 ======================== */
volatile uint32_t g_system_tick = 0;
uint8_t g_wifi_flag = 0;
char g_send_buf[128];
volatile uint32_t g_setting_tick = 0;
uint32_t g_key_tick = 0;
volatile uint8_t g_setting_timeout = 0;

/* 网络状态（用来展示联调进度） */
uint8_t g_wifi_connected = 0;
uint8_t g_bemfa_connected = 0;

/* ======================== TIM2 1ms中断 ======================== */
void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        g_system_tick++;
        if(g_system_tick % 30000 == 0) g_wifi_flag = 1;
        if(g_system_tick % 1000 == 0)  sys_data.run_time++;
        if(current_ui == UI_SETTING && (g_system_tick - g_setting_tick >= 5000))
        {
            g_setting_timeout = 1;
        }
    }
}

void TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 10 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM2, ENABLE);
}

/* ======================== 主函数 ======================== */
int main(void)
{
    uint8_t dht11_ret;

    delay_init();
    OLED_Init();
    usart1_init(115200);
    AD_Init_DMA();    // 保留初始化，给面试官看我们有这个底层驱动
    Key_Init();
    relay_init();
    data_process_init();
    ui_init();
    TIM2_Init();

    printf("\r\n=====================================\r\n");
    printf("STM32 环境监测系统启动\r\n");
    printf("=====================================\r\n");

    OLED_Clear();
    OLED_ShowString(1, 1, "STM32 Monitor");
    OLED_ShowString(3, 1, "System Start...");
    Delay_ms(1000);

    // 尝试连接网络（连不上不阻塞）
    printf("ESP8266 初始化...\r\n");
    ESP8266_Init();
    Delay_ms(1000);
    if(ESP8266_ConnectWiFi("12345678", "12345678") == 0)
    {
        printf("WiFi 连接成功！\r\n");
        g_wifi_connected = 1;
    }
    else
    {
        printf("WiFi 连接失败，转入本地运行模式。\r\n");
    }

    printf("进入主循环...\r\n");

    while(1)
    {
        /* 1. 读取 DHT11 温湿度（500ms一次） */
        if(g_system_tick % 500 == 0)
        {
            Delay_ms(1);
            __disable_irq();
            dht11_ret = DHT11_ReadData();
            if(dht11_ret == 1)
            {
                sys_data.temperature = (float)DHT11_Temp;
                sys_data.humidity = (float)DHT11_Humi;
            }
            __enable_irq();
        }

        /* 2. 按键扫描与 UI 交互 */
        Key_Scan();
        if(Key_Flag == 1 && (g_system_tick - g_key_tick > 150))
        {
            Key_Flag = 0;
            g_key_tick = g_system_tick;
            if(current_ui == UI_SETTING)
            {
                sys_data.temp_threshold += 0.5f;
                if(sys_data.temp_threshold > 50.0f) sys_data.temp_threshold = 25.0f;
                g_setting_tick = g_system_tick;
                OLED_Clear();
            }
            else
            {
                g_setting_tick = g_system_tick;
                ui_switch_next();
                if(current_ui == UI_SETTING)
                {
                    g_setting_tick = g_system_tick;
                    OLED_Clear();
                }
            }
        }
        if(g_setting_timeout == 1)
        {
            g_setting_timeout = 0;
            current_ui = UI_TEMP;
            OLED_Clear();
        }

        /* 3. 报警与继电器闭环控制 */
        alarm_check();
        relay_ctrl(sys_data.alarm_flag);

        /* 4. OLED 屏幕刷新（显示温湿度和状态） */
        ui_refresh();

        /* 5. 网络后台任务（30秒一次） */
        if(g_wifi_flag == 1)
        {
            g_wifi_flag = 0;
            sprintf(g_send_buf, "T:%d H:%d V:0mV", DHT11_Temp, DHT11_Humi);
            if (g_bemfa_connected == 1) {
                ESP8266_PublishBemfa("env_monitor", g_send_buf);
            } else {
                printf("【本地离线模拟上报】%s\r\n", g_send_buf);
            }
        }
    }
}

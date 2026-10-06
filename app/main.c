#include "stm32f10x.h"
#include "usart.h"
#include "data_process.h"
#include "display_ui.h"
#include "dht11.h"
#include "key.h"
#include "adc.h"
#include "Delay.h"
#include "relay.h"

static volatile uint32_t sys_tick = 0;

uint32_t get_tick(void)
{
    return sys_tick;
}

void tim2_init(void)
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

void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        sys_tick++;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

int main(void)
{
    uint32_t time_dht = 0;
    uint32_t time_adc = 0;
    uint32_t time_key = 0;
    uint32_t time_ui = 0;
    uint32_t time_uart = 0;
    uint32_t time_sec = 0;
    uint32_t time_last_key = 0;
    
    SystemInit();
    delay_init();
    
    usart1_init(9600);
    tim2_init();
    data_process_init();
    ui_init();
    DHT11_Init();
    AD_Init_DMA();
    Key_Init();
    relay_init();
    
    printf("All Init Done\r\n");
    
    while(1)
    {
        if(get_tick() - time_dht >= 1000)
        {
            time_dht = get_tick();
            if(DHT11_ReadData() == 1)
            {
                sys_data.temperature = (float)DHT11_Temp;
                sys_data.humidity = (float)DHT11_Humi;
            }
        }
        
        if(get_tick() - time_adc >= 100)
        {
            float volt_f;
            time_adc = get_tick();
            volt_f = ADC_GetAverageVoltage_DMA(8);
            sys_data.adc_volt = (uint16_t)(volt_f * 1000);
            sys_data.adc_volt = filter_adc(sys_data.adc_volt);
        }
        
        if(get_tick() - time_key >= 50)
        {
            time_key = get_tick();
            if(Key_Flag == 1)
            {
                Key_Flag = 0;
                time_last_key = get_tick();
                
                if(current_ui == UI_SETTING)
                {
                    sys_data.temp_threshold += 0.5f;
                    if(sys_data.temp_threshold > 50.0f)
                        sys_data.temp_threshold = 20.0f;
                }
                else
                {
                    ui_switch_next();
                }
            }
        }
        
        if(current_ui == UI_SETTING && (get_tick() - time_last_key > 3000))
        {
            ui_switch_next();
        }
        
        if(get_tick() - time_ui >= 200)
        {
            time_ui = get_tick();
            ui_refresh();
        }
        
        if(get_tick() - time_uart >= 2000)
        {
            time_uart = get_tick();
            printf("Temp:%.1fC Humi:%.1f%% Volt:%dmV Alarm:%d TH:%.1f\r\n",
                   sys_data.temperature, sys_data.humidity,
                   sys_data.adc_volt, sys_data.alarm_flag,
                   sys_data.temp_threshold);
        }
        
        if(get_tick() - time_sec >= 1000)
        {
            time_sec = get_tick();
            sys_data.run_time++;
            alarm_check();
            relay_ctrl(sys_data.alarm_flag);
        }
    }
}

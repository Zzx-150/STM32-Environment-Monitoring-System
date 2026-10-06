#include "display_ui.h"
#include "data_process.h"
#include "oled.h"

uint8_t current_ui = UI_TEMP;

void ui_init(void)
{
    OLED_Init();
    OLED_Clear();
    current_ui = UI_TEMP;
}

void ui_switch_next(void)
{
    current_ui++;
    if(current_ui > UI_SETTING) current_ui = UI_TEMP;
    OLED_Clear();
}

void ui_refresh(void)
{
    switch(current_ui)
    {
        case UI_TEMP:
            OLED_ShowString(1, 1, "Temp:");
            OLED_ShowNum(1, 6, (uint32_t)sys_data.temperature, 2);
            OLED_ShowString(1, 8, ".");
            OLED_ShowNum(1, 9, (uint32_t)(sys_data.temperature*10)%10, 1);
            OLED_ShowString(1, 11, "C");
            
            OLED_ShowString(2, 1, "Humi:");
            OLED_ShowNum(2, 6, (uint32_t)sys_data.humidity, 2);
            OLED_ShowString(2, 9, "%");
            break;
            
        case UI_VOLT:
            OLED_ShowString(1, 1, "ADC Volt:");
            OLED_ShowNum(1, 10, sys_data.adc_volt, 4);
            OLED_ShowString(1, 15, "mV");
            break;
            
        case UI_STATUS:
            OLED_ShowString(1, 1, "Status:");
            if(sys_data.alarm_flag)
                OLED_ShowString(1, 8, "ALARM");
            else
                OLED_ShowString(1, 8, "NORMAL");
            
            OLED_ShowString(2, 1, "Run:");
            OLED_ShowNum(2, 5, sys_data.run_time, 5);
            OLED_ShowString(2, 11, "s");
            break;
            
        case UI_SETTING:
            OLED_ShowString(1, 1, "Set Temp:");
            OLED_ShowNum(1, 10, (uint32_t)sys_data.temp_threshold, 2);
            OLED_ShowString(1, 12, ".");
            OLED_ShowNum(1, 13, (uint32_t)(sys_data.temp_threshold*10)%10, 1);
            
            OLED_ShowString(2, 1, "Set Volt:");
            OLED_ShowNum(2, 10, sys_data.volt_threshold, 4);
            OLED_ShowString(2, 15, "mV");
            break;
            
        default: break;
    }
}

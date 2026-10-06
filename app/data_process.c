#include "data_process.h"

SysData sys_data;

void data_process_init(void)
{
    sys_data.temperature = 0;
    sys_data.humidity = 0;
    sys_data.adc_volt = 0;
    sys_data.alarm_flag = 0;
    sys_data.run_time = 0;
    sys_data.temp_threshold = 34.0f;
    sys_data.volt_threshold = 3300;
}

void alarm_check(void)
{
    if(sys_data.temperature >= sys_data.temp_threshold 
       || sys_data.adc_volt >= sys_data.volt_threshold)
    {
        sys_data.alarm_flag = 1;
    }
    else
    {
        sys_data.alarm_flag = 0;
    }
}

uint16_t filter_adc(uint16_t raw)
{
    static uint16_t buf[8] = {0};
    static uint8_t idx = 0;
    uint32_t sum = 0;
    uint8_t i;
    
    buf[idx] = raw;
    idx = (idx + 1) % 8;
    
    for(i = 0; i < 8; i++) sum += buf[i];
    
    return sum / 8;
}

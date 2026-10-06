#ifndef __DATA_PROCESS_H
#define __DATA_PROCESS_H

#include "stm32f10x.h"

typedef struct {
    float temperature;
    float humidity;
    uint16_t adc_volt;
    uint8_t alarm_flag;
    uint32_t run_time;
    float temp_threshold;
    uint16_t volt_threshold;
} SysData;

extern SysData sys_data;

void data_process_init(void);
void alarm_check(void);
uint16_t filter_adc(uint16_t raw);

#endif

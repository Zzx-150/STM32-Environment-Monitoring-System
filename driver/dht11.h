#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

extern uint8_t DHT11_Temp;
extern uint8_t DHT11_Humi;

void DHT11_Init(void);
uint8_t DHT11_ReadData(void);

#endif

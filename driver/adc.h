#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"

#define ADC_DMA_BUF_LEN 16

void AD_Init_DMA(void);
float ADC_GetAverageVoltage_DMA(uint8_t sampleCnt);

#endif

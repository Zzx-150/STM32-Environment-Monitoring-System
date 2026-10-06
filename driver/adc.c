#include "stm32f10x.h"

#define ADC_DMA_BUF_LEN 16
static uint16_t adcDmaBuf[ADC_DMA_BUF_LEN];

void AD_Init_DMA(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    DMA_InitTypeDef DMA_InitStruct;
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStruct.DMA_PeripheralBaseAddr  = (uint32_t)&ADC1->DR;
    DMA_InitStruct.DMA_MemoryBaseAddr     = (uint32_t)adcDmaBuf;
    DMA_InitStruct.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStruct.DMA_BufferSize         = ADC_DMA_BUF_LEN;
    DMA_InitStruct.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    DMA_InitStruct.DMA_Mode               = DMA_Mode_Circular;
    DMA_InitStruct.DMA_Priority           = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStruct);
    DMA_Cmd(DMA1_Channel1, ENABLE);
    
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
    ADC_DMACmd(ADC1, ENABLE);
    
    ADC_Cmd(ADC1, ENABLE);
    
    uint32_t timeout = 0xFFFF;
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET)
    {
        if(--timeout == 0) return;
    }
    
    timeout = 0xFFFF;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET)
    {
        if(--timeout == 0) return;
    }
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

float ADC_GetAverageVoltage_DMA(uint8_t sampleCnt)
{
    uint32_t sum = 0U;
    uint8_t i;
    
    if(sampleCnt == 0 || sampleCnt > ADC_DMA_BUF_LEN)
    {
        sampleCnt = ADC_DMA_BUF_LEN;
    }
    
    DMA_Cmd(DMA1_Channel1, DISABLE);
    
    for(i = 0; i < sampleCnt; i++)
    {
        sum += adcDmaBuf[i];
    }
    
    DMA_Cmd(DMA1_Channel1, ENABLE);
    
    return (float)sum / sampleCnt / 4095.0f * 3.3f;
}

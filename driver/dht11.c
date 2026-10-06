#include "stm32f10x.h"
#include "dht11.h"
#include "Delay.h"

uint8_t DHT11_Temp = 0;
uint8_t DHT11_Humi = 0;

static void DHT11_PIN_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void DHT11_PIN_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static uint8_t DHT11_Reset(void)
{
    uint16_t timeout = 0;
    
    DHT11_PIN_OUT();
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
    Delay_ms(30);
    GPIO_SetBits(GPIOA, GPIO_Pin_0);
    Delay_us(40);
    
    DHT11_PIN_IN();
    
    timeout = 0;
    while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
    {
        timeout++;
        Delay_us(1);
        if(timeout > 200) return 0;
    }
    
    timeout = 0;
    while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
    {
        timeout++;
        Delay_us(1);
        if(timeout > 200) return 0;
    }
    
    timeout = 0;
    while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
    {
        timeout++;
        Delay_us(1);
        if(timeout > 200) return 0;
    }
    
    return 1;
}

static uint8_t DHT11_ReadBit(void)
{
    uint8_t bit = 0;
    uint16_t timeout = 0;
    
    while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
    {
        timeout++;
        Delay_us(1);
        if(timeout > 100) return 0;
    }
    
    Delay_us(30);
    
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
    {
        bit = 1;
    }
    
    timeout = 0;
    while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
    {
        timeout++;
        Delay_us(1);
        if(timeout > 100) break;
    }
    
    return bit;
}

static uint8_t DHT11_ReadByte(void)
{
    uint8_t byte = 0;
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        byte <<= 1;
        byte |= DHT11_ReadBit();
    }
    return byte;
}

uint8_t DHT11_ReadData(void)
{
    uint8_t humi_int, humi_dec, temp_int, temp_dec, check_sum;
    
    if (DHT11_Reset() == 0) return 0;
    
    humi_int = DHT11_ReadByte();
    humi_dec = DHT11_ReadByte();
    temp_int = DHT11_ReadByte();
    temp_dec = DHT11_ReadByte();
    check_sum = DHT11_ReadByte();
    
    if (humi_int + humi_dec + temp_int + temp_dec == check_sum)
    {
        DHT11_Humi = humi_int;
        DHT11_Temp = temp_int;
        return 1;
    }
    return 0;
}

void DHT11_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    DHT11_PIN_OUT();
    GPIO_SetBits(GPIOA, GPIO_Pin_0);
}

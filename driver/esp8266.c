#include "esp8266.h"
#include "Delay.h"
#include <string.h>
#include <stdio.h>

char esp8266_rx_buf[256];
uint16_t esp8266_rx_len = 0;

static void USART2_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_Cmd(USART2, ENABLE);
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        char ch = USART_ReceiveData(USART2);
        if(esp8266_rx_len < 255)
        {
            esp8266_rx_buf[esp8266_rx_len++] = ch;
        }
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

static void ESP8266_SendString(char *str)
{
    while(*str)
    {
        USART_SendData(USART2, *str++);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
}

static uint8_t ESP8266_SendCmd(char *cmd, char *ack, uint16_t timeout)
{
    uint16_t i;
    esp8266_rx_len = 0;
    memset(esp8266_rx_buf, 0, sizeof(esp8266_rx_buf));
    
    ESP8266_SendString(cmd);
    ESP8266_SendString("\r\n");
    
    for(i = 0; i < timeout; i++)
    {
        Delay_ms(10);
        if(strstr(esp8266_rx_buf, ack) != NULL)
        {
            return 1;
        }
    }
    return 0;
}

void ESP8266_Init(void)
{
    USART2_Init(115200);
    Delay_ms(2000);
    
    ESP8266_SendCmd("AT", "OK", 1000);
    ESP8266_SendCmd("AT+CWMODE=1", "OK", 1000);
}

uint8_t ESP8266_ConnectWiFi(char *ssid, char *pwd)
{
    char cmd[64];
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
    return ESP8266_SendCmd(cmd, "WIFI GOT IP", 15000);
}

uint8_t ESP8266_ConnectTCPServer(char *ip, char *port)
{
    char cmd[64];
    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s", ip, port);
    return ESP8266_SendCmd(cmd, "CONNECT", 5000);
}

void ESP8266_SendData(char *data)
{
    char cmd[64];
    sprintf(cmd, "AT+CIPSEND=%d", strlen(data));
    ESP8266_SendCmd(cmd, "OK", 1000);
    Delay_ms(100);
    ESP8266_SendString(data);
    ESP8266_SendString("\r\n");
}

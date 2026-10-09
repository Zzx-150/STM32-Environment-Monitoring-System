#include "esp8266.h"
#include "Delay.h"
#include <string.h>
#include <stdio.h>

char USART2_RX_BUF[256];
volatile uint16_t USART2_RX_CNT = 0;

void ESP8266_ClearBuf(void)
{
    memset(USART2_RX_BUF, 0, sizeof(USART2_RX_BUF));
    USART2_RX_CNT = 0;
}

void ESP8266_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // PA2 = USART2_TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA3 = USART2_RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200; // 如果你怀疑是9600，先把这里改成9600再编译烧录试试
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

void ESP8266_SendRaw(char *data, uint16_t len)
{
    uint16_t i;
    for(i = 0; i < len; i++)
    {
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        USART_SendData(USART2, data[i]);
    }
}

void ESP8266_SendCmd(char *cmd)
{
    ESP8266_SendRaw(cmd, strlen(cmd));
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    USART_SendData(USART2, '\r');
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    USART_SendData(USART2, '\n');
}

uint8_t ESP8266_WaitResponse(char *resp, uint16_t timeout)
{
    uint16_t i;
    for(i = 0; i < timeout; i++)
    {
        if(USART2_RX_CNT > 0 && strstr((char*)USART2_RX_BUF, resp) != NULL)
        {
            return 0;
        }
        Delay_ms(1);
    }
    return 1;
}

uint8_t ESP8266_CheckAlive(void)
{
    ESP8266_ClearBuf();
    ESP8266_SendCmd("AT");
    if(ESP8266_WaitResponse("OK", 2000) == 0)
    {
        ESP8266_ClearBuf();
        return 0;
    }
    ESP8266_ClearBuf();
    return 1;
}

// ====================== 带探针的 WiFi 连接函数 ======================
uint8_t ESP8266_ConnectWiFi(char *ssid, char *pwd)
{
    char cmd[128];

    printf("--- 开始 WiFi 连接流程 ---\r\n");

    ESP8266_ClearBuf();
    ESP8266_SendCmd("AT+RST");
    Delay_ms(3000);
    ESP8266_ClearBuf();

    ESP8266_SendCmd("ATE0");
    Delay_ms(500);
    ESP8266_ClearBuf();

    // 第一步：测试 AT
    ESP8266_SendCmd("AT");
    if(ESP8266_WaitResponse("OK", 2000) != 0) 
    {
        printf("【失败】AT 命令无响应！ESP8266 是不是没插好？\r\n");
        return 1;
    }
    ESP8266_ClearBuf();

    // 第二步：设置 Station 模式
    ESP8266_SendCmd("AT+CWMODE=1");
    if(ESP8266_WaitResponse("OK", 2000) != 0) 
    {
        printf("【失败】设置 CWMODE=1 失败！\r\n");
        return 2;
    }
    ESP8266_ClearBuf();

    // 第三步：连接 WiFi
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
    ESP8266_SendCmd(cmd);
    printf("正在发送: %s\r\n", cmd); 
    
    if(ESP8266_WaitResponse("WIFI GOT IP", 20000) != 0)
    {
        printf("【失败】没等到 WIFI GOT IP！ESP8266的原始回复是: [%s]\r\n", USART2_RX_BUF); 
        if(ESP8266_WaitResponse("OK", 3000) != 0) return 3;
    }

    Delay_ms(1500);
    ESP8266_ClearBuf();
    printf("--- WiFi 连接流程完成 ---\r\n");
    return 0;
}

// ====================== 巴法云连接 ======================
uint8_t ESP8266_ConnectBemfa(char *uid)
{
    char cmd[64];
    char payload[96];
    uint16_t len;
    uint8_t retry;

    for(retry = 0; retry < 3; retry++)
    {
        ESP8266_ClearBuf();
        ESP8266_SendCmd("AT");
        if(ESP8266_WaitResponse("OK", 2000) == 0) break;
        Delay_ms(1000);
    }
    if(retry >= 3) { printf("Bemfa: AT同步失败\r\n"); return 1; }
    
    Delay_ms(500);
    ESP8266_ClearBuf();

    ESP8266_SendCmd("AT+CIPMUX=0");
    if(ESP8266_WaitResponse("OK", 2000) != 0) { printf("Bemfa: CIPMUX=0失败\r\n"); return 2; }
    Delay_ms(300);
    ESP8266_ClearBuf();

    ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"47.100.124.222\",9502");
    if(ESP8266_WaitResponse("CONNECT", 8000) != 0)
    {
        if(ESP8266_WaitResponse("OK", 3000) != 0) { printf("Bemfa: CIPSTART失败，原始回复:[%s]\r\n", USART2_RX_BUF); return 3; }
    }
    Delay_ms(500);
    ESP8266_ClearBuf();

    sprintf(payload, "uid=%s\r\n", uid);
    len = strlen(payload);

    sprintf(cmd, "AT+CIPSEND=%d", len);
    ESP8266_SendCmd(cmd);
    if(ESP8266_WaitResponse(">", 3000) != 0) { printf("Bemfa: CIPSEND没拿到>\r\n"); return 4; }

    ESP8266_SendRaw(payload, len);

    if(ESP8266_WaitResponse("cmd=", 8000) != 0) { printf("Bemfa: 没等到cmd=\r\n"); return 5; }

    ESP8266_ClearBuf();
    return 0;
}

// ====================== 发布消息 ======================
uint8_t ESP8266_PublishBemfa(char *topic, char *msg)
{
    char cmd[64];
    char payload[200];
    uint16_t len;

    ESP8266_ClearBuf();
    sprintf(payload, "pub topic=%s msg=%s\r\n", topic, msg);
    len = strlen(payload);

    sprintf(cmd, "AT+CIPSEND=%d", len);
    ESP8266_SendCmd(cmd);
    if(ESP8266_WaitResponse(">", 3000) != 0) return 1;

    ESP8266_SendRaw(payload, len);

    if(ESP8266_WaitResponse("pubok", 5000) != 0) return 2;

    ESP8266_ClearBuf();
    return 0;
}

// ====================== 接收中断 ======================
void USART2_IRQHandler(void)
{
    uint8_t data;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        data = USART_ReceiveData(USART2);
        if(USART2_RX_CNT < sizeof(USART2_RX_BUF) - 1)
        {
            USART2_RX_BUF[USART2_RX_CNT++] = data;
            USART2_RX_BUF[USART2_RX_CNT] = '\0';
        }
    }
}

#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"

// 声明外部变量，供透传和主循环使用
extern char USART2_RX_BUF[256];
extern volatile uint16_t USART2_RX_CNT;

void ESP8266_Init(void);
void ESP8266_ClearBuf(void);
void ESP8266_SendRaw(char *data, uint16_t len);
void ESP8266_SendCmd(char *cmd);
uint8_t ESP8266_WaitResponse(char *resp, uint16_t timeout);
uint8_t ESP8266_ConnectWiFi(char *ssid, char *pwd);
uint8_t ESP8266_SendData(char *ip, uint16_t port, char *data);
uint8_t ESP8266_ConnectBemfa(char *uid);
uint8_t ESP8266_PublishBemfa(char *topic, char *msg);
uint8_t ESP8266_CheckAlive(void);

#endif

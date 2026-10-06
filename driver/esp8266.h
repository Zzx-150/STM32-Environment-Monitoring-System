#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"

void ESP8266_Init(void);
uint8_t ESP8266_ConnectWiFi(char *ssid, char *pwd);
uint8_t ESP8266_ConnectTCPServer(char *ip, char *port);
void ESP8266_SendData(char *data);

#endif

#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

extern uint8_t Key_Flag;

void Key_Init(void);
void Key_Scan(void);  // 主循环里调用检测按键

#endif

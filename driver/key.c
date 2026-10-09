#include "stm32f10x.h"
#include "key.h"
#include "Delay.h"

uint8_t Key_Flag = 0;
static uint8_t key_last_state = 1;  // 上次按键状态（默认释放=1）

void Key_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// 主循环里调用这个函数检测按键
void Key_Scan(void)
{
    uint8_t now_state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10);
    
    // 检测下降沿：上次是1(释放)，现在是0(按下)
    if(key_last_state == 1 && now_state == 0)
    {
        Delay_ms(20);  // 消抖
        // 再读一次，确认还是按下
        if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0)
        {
            Key_Flag = 1;
        }
    }
    
    key_last_state = now_state;
}

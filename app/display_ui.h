#ifndef __DISPLAY_UI_H
#define __DISPLAY_UI_H

#include "stm32f10x.h"

typedef enum {
    UI_TEMP = 0,
    UI_VOLT,
    UI_STATUS,
    UI_SETTING
} UI_Type;

extern uint8_t current_ui;

void ui_init(void);
void ui_refresh(void);
void ui_switch_next(void);

#endif

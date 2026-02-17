#ifndef __LCD12864_H__
#define __LCD12864_H__

#include "stm32f10x.h"                  // Device header



// º¯ÊýÉùÃ÷
void LCD_Init(void);
void Write_Command(uint8_t cmd);
void Write_Data(uint8_t dat);
void Set_Graphic_Mode(void);
void Clear_Screen(void);
void Fill_GDRAM(void);
void Clear_GDRAM(void);
void Display_Chinese(uint8_t x, uint8_t y, uint8_t const *dat);

#endif

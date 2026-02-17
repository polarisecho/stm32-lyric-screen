#include "stm32f10x.h"                  // Device header

void led_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6 | GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6 | GPIO_Pin_0);
}

void led1_on(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);
}

void led1_off(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_6);
}

void led2_on(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
}

void led2_off(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
}

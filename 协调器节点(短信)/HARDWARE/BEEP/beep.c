#include "beep.h"
#include "stm32f4xx.h"
#include "sys.h"

void BEEP_Init(void){
	//PF8驱动蜂鸣器，推挽输出下拉，置低电压初始化
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_8;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_DOWN;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOF,&GPIO_InitStruct);
	GPIO_ResetBits(GPIOF,GPIO_Pin_8);
}



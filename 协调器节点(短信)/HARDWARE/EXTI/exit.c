#include "stm32f4xx.h"
#include "delay.h"
#include "key.h"
#include "exit.h"
#include "stm32f4xx_exti.h"

 void MYEXIT_Init(){
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);//SYSCFG时钟使能
	KEY_Init();//GPIO中的初始化
	
	//设置IO与中断线的映射关系
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);//PA0接到中断线0
	
	//初始化线上中断
	EXTI_InitStruct.EXTI_Line=EXTI_Line0;//PA0中断上升沿有效，对应WK_UP
	EXTI_InitStruct.EXTI_LineCmd=ENABLE;
	EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStruct);
	
	//配置中断分组WK_UP
	NVIC_InitStruct.NVIC_IRQChannel=EXTI0_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	

}

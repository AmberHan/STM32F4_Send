#include "key.h"
#include "stm32f4xx.h"
#include "delay.h"
void KEY_Init(){
	
	GPIO_InitTypeDef GPIO_InitStructer; 
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOA,ENABLE);
	//KEY0,KEY1,KEY2初始化，PE4，PE3，PE2推挽输入，上拉，100MHz
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_2;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	//WK_UP初始化，PA0推挽输入，下拉，100MHz
	//初始化GPIO的，PA0对应KEY_UP，推挽下拉，PA口检测输入电压，100MHZ
	GPIO_InitStructer.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructer.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructer.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructer.GPIO_PuPd=GPIO_PuPd_DOWN;
	GPIO_InitStructer.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructer);
}








 #include "led.h"
 #include "stm32f4xx.h"
 
 void LED_Init(){	
	//GPIO PF9，PF10的初始化，输出推挽，上拉，100MHz
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOF,&GPIO_InitStruct);
	GPIO_SetBits(GPIOF,GPIO_Pin_9);//LED0置高电压使其不亮
	GPIO_SetBits(GPIOF,GPIO_Pin_10);//LED1置高电压使其不亮
 }
 

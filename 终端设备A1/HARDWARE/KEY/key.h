#ifndef __KEY_H
#define __KEY_H
#include "sys.h"

void KEY_Init(void);
u16 Key_Scan(u8 mode);
#define KEY0 	 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)
#define KEY1 	 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)
#define KEY2	 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)
#define WK_UP	 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)  	
#define KEY_UP_BEEP 4

#endif



//按键KEY_UP记录（参数sum）按键次数
#include "stm32f4xx.h"                 
#include "led.h"
#include "beep.h"
#include "exit.h"
#include "key.h"
#include "delay.h"
#include "lcd.h"
#include "usart.h"
#include "adc.h"
#include "spi.h"
#include "24l01.h"
#include "w25qxx.h" 
#define KEY Key_Scan(0)
//报警灯闪烁设置
static u16 sum=0;//sum记录按键次数


void BEEP_Warn(void){
	LED0=0;//绿灯灭，红灯亮，蜂鸣器响
	LED1=1;
	BEEP;
	delay_ms(1000);
	UNBEEP;//红灯灭，蜂鸣器不响
	LED0=1;
}

//中断服务函数，EXTI0
void EXTI0_IRQHandler(void){
	delay_ms(10);//防抖
	if(EXTI_GetITStatus(EXTI_Line0)!=RESET){//判断中断标记是否发生
	sum++;
	LCD_ShowNum(30+48,130,sum,2,16);	//显示警报次数			
	BEEP_Warn();
	}
	EXTI_ClearITPendingBit(EXTI_Line0);//清除中断0上面的标志位
}

/*按键扫描设置，mode=1支持连按，mode=0，不支持连按，本实验main底下设置的不支持连按。
u16 Key_Scan(u8 mode){
		static int KEY_UP=1; //不支持连按，按键松开标志
		if(mode) KEY_UP=1;  //支持连按，假设前一次始终松开
		if(KEY_UP && (KEY0==0||WK_UP==1)){
			delay_ms(10);//防抖
			KEY_UP=0;//标记按键已经按下
			if(KEY0==0) return 1;
			if(KEY1==0) return 2;
			if(KEY2==0) return 3;
			if(WK_UP==1) return 4;
			else if(WK_UP==1)return 2;
		}
		else if(KEY0==1&&WK_UP==0)
			KEY_UP=1;
			return 0;//没有任何操作
} */ 



int main(void){
//	volatile u8  key; //保存按键
//	u16 i=0;
	u16 VOL[33];//存储电压电流
	float temp;			    
	u16 adcx,adcx1;	//电压采集变量
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	uart_init(115200);
	Adc_Init();         //初始化ADC
	LED_Init();
	BEEP_Init();
	delay_init(168);
	MYEXIT_Init();
	W25QXX_Init();
	NRF24L01_Init();    		//初始化NRF24L01 
	KEY_Init();
	LCD_Init();					//LCD初始化   
	POINT_COLOR=RED;//设置字体为红色 
	LCD_ShowString(30,50,200,16,16,"STM32F4 Device A");	
	LCD_ShowString(30,70,200,16,16,"ADC/DAC Alarm TEST");	
	LCD_ShowString(30,90,200,16,16,"2019/11/16");
	
	while(NRF24L01_Check())		//NRF24L01初始化
	{
		LCD_ShowString(30,110,200,16,16,"NRF24L01 Error");
		delay_ms(200);
		LCD_Fill(30,110,239,130+16,WHITE);
 		delay_ms(200);
	}
	LCD_ShowString(30,110,200,16,16,"NRF24L01 OK"); 
	NRF24L01_TX_Mode();
		
	POINT_COLOR=BLUE;//设置字体为蓝色 
	LCD_ShowString(30,130,200,16,16,"Alarm:0  ");
	LCD_ShowString(30,170,200,16,16,"ADC1_CH5_VOL:000.0V");	//先在固定位置显示小数点      
	LCD_ShowString(30,190,200,16,16,"ADC1_CH6_VOL:000.0V");	
	LCD_ShowString(30,210,200,16,16,"ADC1_CH7_VOL:000.0V");	
	LCD_ShowString(30,230,200,16,16,"ADC1_CH4_AOL: 0.000A");	//先在固定位置显示小数点      
	LCD_ShowString(30,250,200,16,16,"ADC1_CH14_AOL: 0.000A");	
	LCD_ShowString(30,270,200,16,16,"ADC1_CH15_AOL: 0.000A");	
	while(1)
	{ 			
	 	delay_ms(10);
		LED1=!LED1;//绿灯亮    	   
		
/*警报报警
		key=Key_Scan(0);
		if(key==KEY_UP_BEEP) {
			sum++;
			BEEP_Warn();}
			LCD_ShowNum(30+50,190,sum,2,16);	//显示警报次数	
		*/			
		
//ADC电压1采集,PA5
		adcx=Get_Adc_Average(ADC_Channel_5,20);//获取通道5的转换值，20次取平均
		temp=(float)adcx*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
		temp*=1000;															//取整
		adcx1=(u16)temp;                       //赋值整数部分给adcx变量，因为adcx为u16整形
	
	//	if(adcx1>3100||adcx1<2000){sum++;BEEP_Warn();}//电压超过310V或者低于200V
//		LCD_ShowNum(30+48,130,sum,3,16);	//显示警报次数
		
		adcx=adcx1/10;					//现在是312，即小数点前的数
		LCD_ShowxNum(134,170,adcx,3,16,0);    //显示电压值的整数部分，311.1的话，这里就是显示311
		VOL[0]=adcx/10;			//例如312.4； 现在得到的是31，最高位		
		VOL[1]=adcx%10;			//例如312.4； 现在得到的是31，最高位				
		adcx=adcx1%10;                           //现在是4，即小数点后的数
		LCD_ShowxNum(166,170,adcx,1,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
		VOL[2]=adcx;
		
	
//ADC电压2采集,PA6
		adcx=Get_Adc_Average(ADC_Channel_6,20);//获取通道6的转换值，20次取平均	
		temp=(float)adcx*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
		temp*=1000;
		adcx1=(u16)temp;                            //赋值整数部分给adcx变量，因为adcx为u16整形

//		if(adcx1>3100||adcx1<2000){sum++;BEEP_Warn();}//电压超过310V或者低于200V
//	LCD_ShowNum(30+48,130,sum,3,16);	//显示警报次数
			
		adcx=adcx1/10;					//现在是312，即小数点前的数	
		LCD_ShowxNum(134,190,adcx,3,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3
			VOL[3]=adcx/10;			//例如312.4； 现在得到的是31，最高位		
		VOL[4]=adcx%10;			//例如312.4； 现在得到的是31，最高位	
		adcx=adcx1%10;      
		LCD_ShowxNum(166,190,adcx,1,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
		VOL[5]=adcx;		
		
//ADC电压3采集,PA7
		adcx=Get_Adc_Average(ADC_Channel_7,20);//获取通道7的转换值，20次取平均
		temp=(float)adcx*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
		temp*=1000;
		adcx1=(u16)temp;                            //赋值整数部分给adcx变量，因为adcx为u16整形
	
	//	if(adcx1>3100||adcx1<2000){sum++;BEEP_Warn();}//电压超过310V或者低于200V
	//LCD_ShowNum(30+48,130,sum,3,16);	//显示警报次数
 	
		adcx=adcx1/10;					//现在是312，即小数点前的数	
		LCD_ShowxNum(134,210,adcx,3,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3
			VOL[6]=adcx/10;			//例如312.4； 现在得到的是31，最高位		
		VOL[7]=adcx%10;			//例如312.4； 现在得到的是31，最高位	
		adcx=adcx1%10;        
		LCD_ShowxNum(166,210,adcx,1,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
		VOL[8]=adcx;							//最后一位
		
//ADC电流1采集,PA4
		adcx=Get_Adc_Average(ADC_Channel_4,20);//获取通道4的转换值，20次取平均
		temp=(float)adcx*((3.3*5)/4096);          //获取计算后的带小数的实际电压值，比如3.1111		temp*=100;
		adcx1=(u16)(temp*1000);                           //小数部分乘以1000，例如：1.111就转换为1111，相当于保留三位小数。	
	 
	// if(adcx1>2000||adcx1<1000){sum++;BEEP_Warn();}//电流超过2A或者低于1A
	//LCD_ShowNum(30+48,130,sum,3,16);	//显示警报次数
		adcx=(u16)temp;
		LCD_ShowxNum(134,230,adcx,2,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3		temp-=adcx;                           
		VOL[9]=adcx; 
	  adcx=adcx1%1000;		
		LCD_ShowxNum(158,230,adcx,3,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
		VOL[10]=adcx/10;	
	 VOL[11]=adcx%10;		
		
//ADC电流2采集,PC4
		adcx=Get_Adc_Average(ADC_Channel_14,20);//获取通道14的转换值，20次取平均
		temp=(float)adcx*((3.3*5)/4096);          //获取计算后的带小数的实际电压值，比如3.1111		temp*=100;
		adcx1=(u16)(temp*1000);                           //小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数。		VOL[6]=adcx;
	
	// if(adcx1>2000||adcx1<1000){sum++;BEEP_Warn();}//电流超过2A或者低于1A
//	LCD_ShowNum(30+48,130,sum,3,16);	//显示警报次数
		adcx=adcx1/1000;
		LCD_ShowxNum(144,250,adcx,2,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3		temp-=adcx;                           //把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111	
		VOL[12]=adcx;
	  adcx=adcx1%1000;		
		LCD_ShowxNum(166,250,adcx,3,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
			VOL[13]=adcx/10;	
	 VOL[14]=adcx%10;		
		
//ADC电流3采集,PC5
		adcx=Get_Adc_Average(ADC_Channel_15,20);//获取通道15的转换值，20次取平均
		temp=(float)adcx*((3.3*5)/4096);          //获取计算后的带小数的实际电压值，比如3.1111		temp*=100;
		adcx1=(u16)(temp*1000);                           //小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数。		VOL[6]=adcx;
	
	//	 if(adcx1>2000||adcx1<1000){sum++;BEEP_Warn();}//电流超过2A或者低于1A
//	LCD_ShowNum(30+48,130,sum,3,16);	//显示警报次数
		adcx=adcx1/1000;
		LCD_ShowxNum(144,270,adcx,2,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3		temp-=adcx;                           //把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111	
		VOL[15]=adcx;
	  adcx=adcx1%1000;		
		LCD_ShowxNum(166,270,adcx,3,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
		VOL[16]=adcx/10;	
	 VOL[17]=adcx%10;	
		 
		VOL[18]=sum;
		VOL[19]='a';

	//	 	for(i=0;i<20;i++)
		//	LCD_ShowxNum(30,310+16*i,VOL[i],5,16,0);		 
//NRF24L01发送数据		
		if(NRF24L01_TxPacket1(VOL)==TX_OK)	
		{
			POINT_COLOR=RED;//设置字体为红色 
			LCD_ShowString(30,290,239,32,16,"Sended sucess");
//			for(i=0;i<14;i++)
//			LCD_ShowxNum(30,310+16*i,VOL[i],5,16,0);	
		}	else
		{										   	
 			LCD_Fill(30,290,lcddev.width,290+16*3,WHITE);//清空显示	
			POINT_COLOR=RED;//设置字体为红色 			
			LCD_ShowString(30,290,lcddev.width-1,32,16,"Send Failed "); 
		}
		
	}
}


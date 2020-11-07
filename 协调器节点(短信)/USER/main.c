//按键KEY_UP记录（参数sum）按键次数
#include "stm32f4xx.h"                 
#include "led.h"
#include "beep.h"
#include "exit.h"
#include "key.h"
#include "delay.h"
#include "lcd.h"
#include "dht11.h"
#include "usart.h"
#include "adc.h"
#include "spi.h"
#include "24l01.h"
#include "usart3.h"
#include "sim800c.h" 
#include "w25qxx.h" 
#include "ff.h"  
#include "fontupd.h"
#include "text.h"	
#include "exfuns.h" 
#include "malloc.h" 
#include "sdio_sdcard.h" 
#include "usmart.h"	

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

//中断服务函数，EXTI0
void EXTI0_IRQHandler(void){
	delay_ms(10);//防抖
	if(EXTI_GetITStatus(EXTI_Line0)!=RESET){//判断中断标记是否发生
		sum++;
		LCD_ShowNum(30+48,190,sum,2,16);	//显示警报次数	
		BEEP_Warn();
	}
	EXTI_ClearITPendingBit(EXTI_Line0);//清除中断0上面的标志位
}



int main(void){
//	volatile u8  key; //保存按键
	u16 i=0,j=0;//显示传输数据
static	float VOL[6];//存储电压电流	
static	float VOL1[6];//存储电压电流
static	float VOL2[6];//存储电压电流
static	u8 OOL[5];
u8 timenum=2;
	u8 tmp_buf[20];	
	float temp;			    
	u8 temperature;  	    
	u8 humidity;
	u8 time1,time2;
	u8 mode=0;				//0,TCP连接;1,UDP连接
	
	u16 adcx,adcx1;	//电压采集变量
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);
	uart_init(115200);
	Adc_Init();         //初始化ADC
	LED_Init();
	BEEP_Init();
	MYEXIT_Init();

	KEY_Init();
	LCD_Init();		//LCD初始化 
	W25QXX_Init();				                   //初始化W25Q128	
	usart3_init(115200);		                   //初始化串口3	
	usmart_dev.init(168);		//初始化USMART
	my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM);		//初始化CCM内存池 
	exfuns_init();				//为fatfs相关变量申请内存  
  	f_mount(fs[0],"0:",1); 		//挂载SD卡 
 	f_mount(fs[1],"1:",1); 		//挂载FLASH.
	while(font_init()) 			//检查字库
	{    
		LCD_Clear(WHITE);		   	//清屏
 		POINT_COLOR=RED;			//设置字体为红色	   	   	  
		LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");
		while(SD_Init())			//检测SD卡
		{
			LCD_ShowString(30,70,200,16,16,"SD Card Failed!");
			delay_ms(200);
			LCD_Fill(30,70,200+30,70+16,WHITE);
			delay_ms(200);		    
		}
	}	
		
	POINT_COLOR=RED;//设置字体为红色 
	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"DHT11 ADC Alarm TEST");	
	LCD_ShowString(30,90,200,16,16,"2019/11/16");
	NRF24L01_Init();    		//初始化NRF24L01 
	while(NRF24L01_Check())		//NRF24L01初始化
	{
		LCD_ShowString(30,110,200,16,16,"NRF24L01 Error");
		delay_ms(200);
		LCD_Fill(30,110,239,130+16,WHITE);
 		delay_ms(200);
	}
	LCD_ShowString(30,110,200,16,16,"NRF24L01 OK"); 
	NRF24L01_RX_Mode();	
 	while(DHT11_Init())	//DHT11初始化	
	{
		LCD_ShowString(30,130,200,16,16,"DHT11 Error");
		delay_ms(200);
		LCD_Fill(30,130,239,130+16,WHITE);
 		delay_ms(200);
	}								   
	LCD_ShowString(30,130,200,16,16,"DHT11 OK");

	
	while(gprs_init())		//GPRS模块,检测是否应答AT指令 
	{
		u8 state;
		state=gprs_init();
		Show_Str(30,490,200,16,"GPRS错误:",16,0); 
		LCD_ShowNum(30+72,490,state,2,16);	
		delay_ms(200);
		LCD_Fill(30+72,490,239,490+16,WHITE);//清空显示
		delay_ms(200);
		if(timenum--==0)
			break;
	} 
	LCD_Fill(30,570,239,490+16,WHITE);//清空显示
	LCD_ShowString(30,570,200,16,16,"GPRS OK");
	connect(mode);
	
	POINT_COLOR=BLUE;//设置字体为蓝色
 	LCD_ShowString(30,150,200,16,16,"Temp:  C");	 
 	LCD_ShowString(30,170,200,16,16,"Humi:  %");
	LCD_ShowString(30,190,200,16,16,"Alarm:0  ");
//	LCD_ShowString(30,210,200,16,16,"ADC1_CH5_VAL:");	      
	LCD_ShowString(30,230,200,16,16,"ADC1_CH5_VOL:000.0V");	//先在固定位置显示小数点      
	LCD_ShowString(30,250,200,16,16,"ADC1_CH6_VOL:000.0V");	
	LCD_ShowString(30,270,200,16,16,"ADC1_CH7_VOL:000.0V");	
	LCD_ShowString(30,310,200,16,16,"ADC1_CH4_AOL: 0.000A");	//先在固定位置显示小数点      
	LCD_ShowString(30,330,200,16,16,"ADC1_CH14_AOL: 0.000A");	
	LCD_ShowString(30,350,200,16,16,"ADC1_CH15_AOL: 0.000A");	
			
				
		LCD_ShowString(30,410,280,16,16,"ADC2_VOL:000.0V 000.0V 000.0V");	
		LCD_ShowString(30,430,280,16,16,"ADC2_AOL: 0.000A  0.000A  0.000A");
		LCD_ShowString(30,450,200,16,16,"Alarm2:0");
		LCD_ShowString(30,490,280,16,16,"ADC3_VOL:000.0V 000.0V 000.0V");	
		LCD_ShowString(30,510,280,16,16,"ADC3_AOL: 0.000A  0.000A  0.000A");
		LCD_ShowString(30,530,200,16,16,"Alarm3:0");	
	POINT_COLOR=RED;//设置字体为红色 
		
	
	while(1)
	{ 
//测量读取温湿度值			
		DHT11_Read_Data(&temperature,&humidity);		//读取温湿度值					    
		LCD_ShowNum(30+40,150,temperature,2,16);		//显示温度	   		   
		LCD_ShowNum(30+40,170,humidity,2,16);			//显示湿度	 			
	 	delay_ms(10);
		LED1=!LED1;//绿灯亮  
		OOL[0]=temperature;  	    
	  OOL[1]=humidity;
		
/*警报报警
		key=Key_Scan(0);
		if(key==KEY_UP_BEEP) {
			sum++;
			BEEP_Warn();}
			LCD_ShowNum(30+50,190,sum,2,16);	//显示警报次数	
		*/			
//ADC电压1采集,PA5
		adcx=Get_Adc_Average(ADC_Channel_5,20);//获取通道5的转换值，20次取平均
//		LCD_ShowxNum(134,210,adcx,4,16,0);    //显示ADCC采样后的原始值
		temp=(float)adcx*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
		VOL[0]=temp*100;
		temp*=1000;
		adcx1=(u16)temp;                            //赋值整数部分给adcx变量，因为adcx为u16整形
		adcx=adcx1/10;
		LCD_ShowxNum(134,230,adcx,3,16,0);       //显示电压值的整数部分，3.1111的话，这里就是显示3
		adcx=adcx1%10;                          //小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数。
		LCD_ShowxNum(166,230,adcx,1,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
		
	
//ADC电压2采集,PA6
		adcx=Get_Adc_Average(ADC_Channel_6,20);//获取通道6的转换值，20次取平均	
		temp=(float)adcx*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
		VOL[1]=temp*100;
    temp*=1000;
		adcx1=(u16)temp;                            //赋值整数部分给adcx变量，因为adcx为u16整形
		adcx=adcx1/10;
		LCD_ShowxNum(134,250,adcx,3,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3
		adcx=adcx1%10;      
		LCD_ShowxNum(166,250,adcx,1,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
		
		
//ADC电压3采集,PA7
		adcx=Get_Adc_Average(ADC_Channel_7,20);//获取通道7的转换值，20次取平均
		temp=(float)adcx*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
		VOL[2]=temp*100;	
	temp*=1000;
		adcx1=(u16)temp;                            //赋值整数部分给adcx变量，因为adcx为u16整形
		adcx=adcx1/10;
		LCD_ShowxNum(134,270,adcx,3,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3
		adcx=adcx1%10;        
		LCD_ShowxNum(166,270,adcx,1,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	

//ADC电流1采集,PA4
		adcx=Get_Adc_Average(ADC_Channel_4,20);//获取通道4的转换值，20次取平均
		temp=(float)adcx*((3.3*5)/4096);          //获取计算后的带小数的实际电压值，比如3.1111		temp*=100;
		VOL[3]=temp;	
		adcx1=(u16)(temp*1000);                           //小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数。		VOL[6]=adcx;
	  adcx=adcx1/1000;
		LCD_ShowxNum(134,310,adcx,2,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3		temp-=adcx;                           //把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111			VOL[6]=adcx;
	  adcx=adcx1%1000;		
		LCD_ShowxNum(158,310,adcx,3,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
			
		
//ADC电流2采集,PC4
		adcx=Get_Adc_Average(ADC_Channel_14,20);//获取通道14的转换值，20次取平均
		temp=(float)adcx*((3.3*5)/4096);         //获取计算后的带小数的实际电压值，比如3.1111		temp*=100;
		VOL[4]=temp;	
		adcx1=(u16)(temp*1000);                           //小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数。		VOL[6]=adcx;
	  adcx=adcx1/1000;
		LCD_ShowxNum(144,330,adcx,2,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3		temp-=adcx;                           //把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111	
	  adcx=adcx1%1000;		
		LCD_ShowxNum(166,330,adcx,3,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
		
//ADC电流3采集,PC5
		adcx=Get_Adc_Average(ADC_Channel_15,20);//获取通道15的转换值，20次取平均
		temp=(float)adcx*((3.3*5)/4096);          //获取计算后的带小数的实际电压值，比如3.1111		temp*=100;
		VOL[5]=temp;	
		adcx1=(u16)(temp*1000);                           //小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数。		VOL[6]=adcx;
	  adcx=adcx1/1000;
		LCD_ShowxNum(144,350,adcx,2,16,0);    //显示电压值的整数部分，3.1111的话，这里就是显示3		temp-=adcx;                           //把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111	
	  adcx=adcx1%1000;		
		LCD_ShowxNum(166,350,adcx,3,16,0X80); //显示小数部分（前面转换为了整形显示），这里显示的就是1	
		OOL[2]=sum;		

//NRF24L01接收数据	
		if(NRF24L01_RxPacket(tmp_buf)==0)//一旦接收到信息,则显示出来.
			{
				if(tmp_buf[19]==97){
					time2=0;
/*					if(time1++==4){	
					time1--;	
					LCD_Fill(30,390,lcddev.width,390+16,WHITE);//清空显示	
					POINT_COLOR=RED;//设置字体为红色 			
					LCD_ShowString(30,390,lcddev.width-1,32,16,"Received Failed"); 
			//		for(j=0;j<3;j++){
				//	VOL2[j]=999.9;}
		  //	for(j=3;j<6;j++){
				//	VOL2[j]=9.999;}
						}*/
				OOL[3]=tmp_buf[18];
				for(i=0;i<3;i++)
				VOL1[i]=tmp_buf[3*i]*10+tmp_buf[3*i+1]+(float)tmp_buf[3*i+2]/10.0f;
				for(i=3;i<6;i++)
				VOL1[i]=tmp_buf[3*i]+tmp_buf[3*i+1]/100.0f+(float)tmp_buf[3*i+2]/1000.0f;
for(j=0;j<3;j++){
				adcx=tmp_buf[3*j]*10+tmp_buf[3*j+1];
				LCD_ShowxNum(102+j*56,410,adcx,3,16,0); 		
				LCD_ShowxNum(134+j*56,410,tmp_buf[3*j+2],1,16,0X80);	
}	
for(j=3;j<6;j++){
				adcx=tmp_buf[3*j+1]*10+tmp_buf[3*j+2];
	 			LCD_ShowxNum(102+(j-3)*64,430,tmp_buf[3*j],2,16,0); 		
				LCD_ShowxNum(126+(j-3)*64,430,adcx,3,16,0);		
}	
		LCD_ShowNum(30+56,450,tmp_buf[18],3,16);	//显示警报次数	
				}
				if(tmp_buf[19]==98){
					time1=0;
	/*				if(time2++==4){	
					time2--;
					LCD_Fill(30,390,lcddev.width,390+16,WHITE);//清空显示	
					POINT_COLOR=RED;//设置字体为红色 			
					LCD_ShowString(30,390,lcddev.width-1,32,16,"Received Failed"); 
		//			for(j=0;j<3;j++){
	//				VOL1[j]=999.9;}
		//   	for(j=3;j<6;j++){
		//		VOL1[j]=9.999;}
						}*/
					OOL[4]=tmp_buf[18];
				for(i=0;i<3;i++)
				VOL2[i]=tmp_buf[3*i]*10+(float)tmp_buf[3*i+1]+(float)tmp_buf[3*i+2]/10.0f;
				for(i=3;i<6;i++)
				VOL2[i]=tmp_buf[3*i]+(float)tmp_buf[3*i+1]/100.0f+(float)tmp_buf[3*i+2]/1000.0f;						
for(j=0;j<3;j++){
				adcx=tmp_buf[3*j]*10+tmp_buf[3*j+1];
				LCD_ShowxNum(102+j*56,490,adcx,3,16,0); 		
				LCD_ShowxNum(134+j*56,490,tmp_buf[3*j+2],1,16,0X80);	
}	
for(j=3;j<6;j++){
				adcx=tmp_buf[3*j+1]*10+tmp_buf[3*j+2];
	 			LCD_ShowxNum(102+(j-3)*64,510,tmp_buf[3*j],2,16,0); 		
				LCD_ShowxNum(126+(j-3)*64,510,adcx,3,16,0);		
}	
		LCD_ShowNum(30+56,530,tmp_buf[18],3,16);	//显示警报次数	
			}					
			}
		 else{
				LCD_Fill(30,390,lcddev.width,390+16,WHITE);//清空显示	
				POINT_COLOR=RED;//设置字体为红色 			
				LCD_ShowString(30,390,lcddev.width-1,32,16,"Received Failed"); 
				for(j=0;j<3;j++){
					VOL1[j]=999.9;VOL2[j]=999.9;}
		   	for(j=3;j<6;j++){
					VOL1[j]=9.999;VOL2[j]=9.999;}
				
				LCD_Fill(30,470,lcddev.width,470+16,WHITE);//清空显示	
				POINT_COLOR=RED;//设置字体为红色 			
				LCD_ShowString(30,470,lcddev.width-1,32,16,"Received Failed"); 
		 }						
	test(mode,VOL,VOL1,VOL2,OOL);
	delay_ms(2000);
	}	
}		


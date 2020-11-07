#include "sim800c.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   	 	 	 	 	 	 
#include "lcd.h" 	   	  
#include "malloc.h"
#include "string.h"    	
#include "usart3.h" 
#include "timer.h"
#include "text.h"
#include "ff.h"	

static  char haoma[]="00310038003800300035003200370037003500300037";									 
//static char qqq[150];
//static char qq[150];

//usmart支持部分
//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART3_RX_STA;
//     1,清零USART3_RX_STA;
void sim_at_response(u8 mode)
{
	if(USART3_RX_STA&0X8000)		          //接收到一次数据了
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
		printf("%s",USART3_RX_BUF);	          //发送到串口
		if(mode)USART3_RX_STA=0;
	} 
}
//////////////////////////////////////////////////////////////////////////////////////////////////// 
//ATK-SIM800C 各项测试(拨号测试、短信测试、GPRS测试、蓝牙测试)共用代码
//SIM800C发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
u8* sim800c_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART3_RX_STA&0X8000)  //接收到一次数据了
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//向SIM800C发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 sim800c_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART3_RX_STA=0;
	if((u32)cmd<=0XFF)
	{
		while((USART3->SR&0X40)==0);//等待上一次数据发送完成  
		USART3->DR=(u32)cmd;
	}else u3_printf("%s\r\n",cmd);  //发送命令
	
	if(ack&&waittime)		        //需要等待应答
	{
		while(--waittime)	        //等待倒计时
		{ 
			delay_ms(10);
			if(USART3_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(sim800c_check_cmd(ack))break;//得到有效数据 
				USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
}


//将1个字符转换为16进制数字
//chr:字符,0~9/A~F/a~F
//返回值:chr对应的16进制数值
u8 sim800c_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//将1个16进制数字转换为字符
//hex:16进制数字,0~15;
//返回值:字符
u8 sim800c_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//GPRS测试部分代码q
const u8 *modetbl[2]={"TCP","UDP"};//连接模式
const u8 port[]="8087";	//端口固定为8086,当你的电脑8086端口被其他程序占用的时候,请修改为其他空闲端口
const u8 ipaddr[]="39.98.33.54"; 		//IP缓存; 
void connect(u8 mode){
	u8 *p;
	p=mymalloc(SRAMIN,100);		//申请100字节内存	
	USART3_RX_STA=0;
	sprintf((char*)p,"AT+CIPSTART=\"%s\",\"%s\",\"%s\"",modetbl[mode],ipaddr,port);
	if(sim800c_send_cmd(p,"OK",500)==0){
		POINT_COLOR=RED;//设置字体为蓝色 
		LCD_ShowString(30+9*8,570,200,16,16,"Connect OK");
		POINT_COLOR=BLUE;//设置字体为蓝色 		
		Show_Str(30,590,200,16,"连接方式:",16,0); 	//连接方式(TCP/UDP)
		Show_Str(30,610,200,16,"IP:",16,0);		//IP地址可以键盘设置
		Show_Str(30,630,200,16,"端口:",16,0);		//端口固定为8086
		POINT_COLOR=RED;//设置字体为红色 	
		Show_Str(30+72,590,200,16,(u8*)modetbl[mode],16,0);	//显示连接方式	
		Show_Str(30+40,610,200,16,(u8*)ipaddr,16,0);			//显示ip 	
		Show_Str(30+40,630,200,16,(u8*)port,16,0);			//显示端口 
	}
	else LCD_ShowString(30+9*8,570,200,16,16,"Connect Error");
	myfree(SRAMIN,p);
}

//tcp/udp测试
//带心跳功能,以维持连接
//mode:0:TCP测试;1,UDP测试)
//ipaddr:ip地址
//port:端口 
	static u8 l=0;
	static u8 m=0;
	static u8 n=0;
u8 test(u8 mode,float* data1,float* data2,float* data3,u8* data)
{ 
//	static  char haoma[]="00310038003800350032003700300033003000370033";
	u8 a,b,c;
	u8 j=0,i;
	u8 state=1;
	u8 *p,*p1,*p2;
	p=mymalloc(SRAMIN,100);		//申请100字节内存,duanx号码
	p1=mymalloc(SRAMIN,100);	//申请100字节内存,yun
	p2=mymalloc(SRAMIN,100);	//申请100字节内存,duanx内容
	POINT_COLOR=BLUE; 
	Show_Str(30,650,200,16,"状态:",16,0); 	 //连接状态
	Show_Str(30,670,200,16,"发送数据:",16,0);
	a='a';b='b';c='c';
	LCD_Fill(30+72,670,lcddev.width,670+16,WHITE);
	POINT_COLOR=RED;
	
	j+=sprintf((char*)(p1+j),"%d",data[0]);//温度
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%d",data[1]);//湿度
 	j+=sprintf((char*)(p1+j),"  ");
	
	LCD_Fill(30+72,670,lcddev.width,20,WHITE);//清空显示	
	Show_Str(30+72,670,300+8,20,p1,16,0); 
	
	j+=sprintf((char*)(p1+j),"%c",c);//发送的设备c
	for(i=0;i<3;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%4.1f",data1[i]);
	}
		for(i=3;i<6;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%5.3f",data1[i]);
	}//发送的内容
		j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%d",data[2]);
	j+=sprintf((char*)(p1+j),"  ");//发送的内容
	
	j+=sprintf((char*)(p1+j),"%c",a);//发送的设备a
	for(i=0;i<3;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%4.1f",data2[i]);
	}
		for(i=3;i<6;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%5.3f",data2[i]);
	}
	j+=sprintf((char*)(p1+j)," ");
		j+=sprintf((char*)(p1+j),"%d",data[3]);
	j+=sprintf((char*)(p1+j),"  ");//发送的内容

	j+=sprintf((char*)(p1+j),"%c",b);//发送的设备b
	for(i=0;i<3;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%4.1f",data3[i]);
	}
		for(i=3;i<6;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%5.3f",data3[i]);
	}
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%d",data[4]);
	LCD_Fill(30+72,690,lcddev.width,100,WHITE);//清空显示	
	Show_Str(30+72,690,300+40,100,p1+7,16,0); 

			
	sim800c_send_cmd("AT+CIPSTATUS","OK",500);	//查询连接状态
			if(strstr((const char*)USART3_RX_BUF,"CONNECT OK"))state=0;
		    if(strstr((const char*)USART3_RX_BUF,"CLOSED"))
					{
						LCD_Fill(30+40,650,lcddev.width,650+16,WHITE);//清空显示
						Show_Str(30+40,650,200,16,"GPRS连接失败",16,0); 	 //连接状态
						sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",500);	//关闭连接
						sim800c_send_cmd("AT+CIPSHUT","SHUT OK",500);		//关闭移动场景 
			
						sprintf((char*)p,"AT+CIPSTART=\"%s\",\"%s\",\"%s\"",modetbl[mode],ipaddr,port);
						if(sim800c_send_cmd(p,"OK",500)==0)	state=0;		//尝试重新连接	
					}		
	
			if(state==0){ 
					LCD_Fill(70,650,lcddev.width,650+16,WHITE);
					if(sim800c_send_cmd("AT+CIPSEND",">",100)==0)		//发送数据
					{ 
						Show_Str(30+40,650,200,16,"数据发送中",16,0); 		//提示数据发送中
						printf("CIPSEND DATA:%s\r\n",p1);	 			//发送数据打印到串口
						u3_printf("%s\r\n",p1);
						delay_ms(10);
						if(sim800c_send_cmd((u8*)0X1A,"SEND OK",1000)==0)
							{
								LCD_Fill(70,650,lcddev.width,650+16,WHITE);
								Show_Str(40+30,650,200,16,"数据发送成功!",16,0);//最长等待10s
							}
						else {LCD_Fill(70,650,lcddev.width,650+16,WHITE);Show_Str(40+30,650,200,12,"数据发送失败!",16,0);}
						delay_ms(10);
						state=0;				
					}else sim800c_send_cmd((u8*)0X1B,0,0);	//ESC,取消发送 			
	}
	
	
	
	
////短信报警	
 if(n!=data[2]){//c机器
		sprintf((char*)p,"AT+CMGS=\"%s\"",haoma); 	//发送短信命令+18852703073
		if(sim800c_send_cmd(p,">",200)==0)					
						{ 		 				 													 
							u3_printf("5C0A656C76847528623760A8597DFF0C60A87684673A566800636B63572862A58B66FF0C8BF7639267E5FF01");		 						//发送短信内容到GSM模块 
							if(sim800c_send_cmd((u8*)0X1A,"+CMGS:",1000)==0){
							Show_Str(70+100,650,200,16,"短信报警成功！",16,0); 		//提示数据发送中					
						}
					} 
				}	
	
	if(l!=data[3]){//a机器
		sprintf((char*)p,"AT+CMGS=\"%s\"",haoma); 	//发送短信命令+18852703073
		if(sim800c_send_cmd(p,">",200)==0)					
						{ 		 				 													 
							u3_printf("5C0A656C76847528623760A8597DFF0C60A87684673A566800616B63572862A58B66FF0C8BF7639267E5FF01");		 						//发送短信内容到GSM模块 
							if(sim800c_send_cmd((u8*)0X1A,"+CMGS:",1000)==0){
								Show_Str(70+100,650,200,16,"短信报警成功！",16,0); 		//提示数据发送中
						}
					} 
				}	
		
	if(m!=data[4]){//b机器
		sprintf((char*)p,"AT+CMGS=\"%s\"",haoma); 	//发送短信命令+18852703073
		if(sim800c_send_cmd(p,">",200)==0)					
						{ 		 				 													 
							u3_printf("5C0A656C76847528623760A8597DFF0C60A87684673A566800626B63572862A58B66FF0C8BF7639267E5FF01");		 						//发送短信内容到GSM模块 
							if(sim800c_send_cmd((u8*)0X1A,"+CMGS:",1000)==0){
								Show_Str(70+100,650,200,16,"短信报警成功！",16,0); 		//提示数据发送中						
						}
					} 
				}		
		l=data[3];
  	m=data[4];
  	n=data[2];
	 	myfree(SRAMIN,p);
		myfree(SRAMIN,p1);
		myfree(SRAMIN,p2);
		return state;		//发起连接
		
}


u8 gprs_init(void){
	u8 state=0;
	if(sim800c_send_cmd("AT","OK",100))
		{ state=1;return state;}//AT
	if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",100))
		{ state=2;	return state;}//设置GPRS移动台类别为B,支持包交换和数据交换 
	if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",100))
		{ state=3;return state;}//设置PDP上下文,互联网接协议,接入点等信息
	if(sim800c_send_cmd("AT+CGATT=1","OK",100))  
		{ state=4;return state;	}			//附着GPRS业务
//	if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",100))
//		{ state=5;return state; }	//设置为GPRS连接模式
	if(sim800c_send_cmd("AT+CIPHEAD=1","OK",100))
		{ state=5;return state;	}	
		
/////短信报警
	if(sim800c_send_cmd("AT+CMGF=1","OK",100))
		{ state=6;return state;	}	
	if(sim800c_send_cmd("AT+CSMP=17,167,1,8","OK",100))
		{ state=7;return state;	}	
	if(sim800c_send_cmd("AT+CSCS=\"UCS2\"","OK",100))
		{ state=8;return state;	}	
	return state;	
}
















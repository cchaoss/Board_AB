#include "lcd.h"
#include "bms.h"
#include "main.h"
#include "adc.h"
#include "acdc_module.h"
#include "electric_meter.h"

#define LCD_RX_DMA DMA2_Channel3
#define LCD_TX_DMA DMA2_Channel5

#define LCD_RX_SIZE	8//接受
unsigned char LCD_Rx_Buffer[LCD_RX_SIZE];//接受数据缓存

#define LCD_TX_SIZE	105//发送
unsigned char LCD_Tx_Buffer[LCD_TX_SIZE];//发送
Uart_Rx_FlagBits Uart_Flag;//串口接受标记

void LCD_UART_Init(uint32_t bound)
{
	GPIO_PinConfigure(GPIOC,10,GPIO_AF_PUSHPULL,GPIO_MODE_OUT50MHZ);//TX-复用推挽输出
	GPIO_PinConfigure(GPIOC,11,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//RX-下拉输入
	//初始化串口结构体
	USART_InitTypeDef USART_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//初始化串口时钟
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
	//设置串口中断优先级
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);//打开空闲中断,用于接收不定长数据时判断接受完一帧数据
	USART_Cmd(UART4, ENABLE);
	//USART_ClearFlag(UART4, USART_FLAG_TC);
	/*配置UART4 的DMA TX RX*/
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);//开启DMA2时钟
	//DMA2 Channel 3 triggerd by USART4 Rx event
	DMA_DeInit(DMA2_Channel3);//复位DMA控制器的通道
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)LCD_Rx_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//外设作为数据来源
	DMA_InitStructure.DMA_BufferSize = LCD_RX_SIZE;//缓存长度
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据宽度1字节
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存数据宽度1字节
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//非循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//外设与内存通讯，而非内存到内存
	DMA_Init(DMA2_Channel3,&DMA_InitStructure);
	DMA_Cmd (DMA2_Channel3,ENABLE);//启动DMA接受
	//DMA2 Channel 5 triggerd by USART4 Tx event
	DMA_DeInit(DMA2_Channel5);//复位DMA控制器的通道
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)LCD_Tx_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//外设作为传送数据目的地
	DMA_InitStructure.DMA_BufferSize = 0;//发送长度为0，默认关闭
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据宽度1字节
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存数据宽度1字节
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//非循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//外设与内存通讯，而非内存到内存
	DMA_Init(DMA2_Channel5,	&DMA_InitStructure);
	//DMA_Cmd (DMA2_Channel5,ENABLE);//默认关闭
	USART_DMACmd(UART4, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);//使能串口DMA发送和接受
}

//串口2中断服务函数,接收不定长数据---LCD
void UART4_IRQHandler(void)
{
	uint8_t clear = clear;
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)
	{
		clear = UART4->SR;
		clear = UART4->DR;//先读SR再读DR才能清除
		Uart_Flag.Lcd_Rx_Flag = true;
	}
}

void LCD_DMA_Reset(DMA_Channel_TypeDef* DMAy_Channelx, unsigned char len)
{
	//USART_ClearFlag(UART4, USART_FLAG_TC);
	DMA_Cmd (DMAy_Channelx,DISABLE);//要先关闭DMA才能设置长度
	DMAy_Channelx->CNDTR = len;			//重新设置开始地址
	DMA_Cmd (DMAy_Channelx,ENABLE); //启动DMA发送
}

unsigned char End_of_package[4] = {0xcc,0x33,0xc3,0x3c};//固定包尾
unsigned char Head_number[7] = {0xAA,0x14,0x02,0xff,0xff,0x00,0x00};//不显示背景色 无符号数 无效不显示 大小10*20 
unsigned char Head_string[7] = {0xAA,0x11,0x02,0xff,0xff,0x00,0x00};//不显示背景色 10*20 字符颜色 背景颜色
unsigned char Page[8] = {0xAA,0x22,0x00,0x00,0xCC,0x33,0xC3,0x3C};//切换页面指令
unsigned char Hand[6] = {0xAA,0x00,0xCC,0x33,0xC3,0x3C};//握手指令
void show_number(unsigned int number,unsigned short x,unsigned short y,unsigned char num1,unsigned char num2)
{
	memcpy(LCD_Tx_Buffer,Head_number,sizeof(Head_number));
	LCD_Tx_Buffer[7] = num1;//整数位数
	LCD_Tx_Buffer[8] = num2;//小数位数
	LCD_Tx_Buffer[9] = x>>8;
  LCD_Tx_Buffer[10]= x;
	LCD_Tx_Buffer[11]= y>>8;
  LCD_Tx_Buffer[12]= y;		
	LCD_Tx_Buffer[13]= number>>24;
	LCD_Tx_Buffer[14]= number>>16;			
	LCD_Tx_Buffer[15]= number>>8;
	LCD_Tx_Buffer[16]= number;		
	memcpy(&LCD_Tx_Buffer[17],End_of_package,sizeof(End_of_package));
	
	LCD_DMA_Reset(LCD_TX_DMA,21);
	while(DMA_GetCurrDataCounter(LCD_TX_DMA));//贼烦
}
//显示字符跟数字的xy坐标是反的
void Show_hanzi(unsigned char *hanzi,unsigned char size,unsigned short y,unsigned short x)
{
	memcpy(LCD_Tx_Buffer,Head_string,sizeof(Head_string));
	LCD_Tx_Buffer[7] = x>>8;
  LCD_Tx_Buffer[8] = x;
	LCD_Tx_Buffer[9] = y>>8;
  LCD_Tx_Buffer[10]= y;																									
	for(char i=0;i<(size);i++)	LCD_Tx_Buffer[11+i] = hanzi[i];//汉字存储区域
	memcpy(&LCD_Tx_Buffer[size+11],End_of_package,sizeof(End_of_package));
	
	LCD_DMA_Reset(LCD_TX_DMA,size+15);
	while(DMA_GetCurrDataCounter(LCD_TX_DMA));
}

void switch_page(unsigned char num)
{
	memcpy(LCD_Tx_Buffer,Page,sizeof(Page));
	LCD_Tx_Buffer[3] = num;
	LCD_DMA_Reset(LCD_TX_DMA,8);
	while(DMA_GetCurrDataCounter(LCD_TX_DMA));
}

void Check_Hand(void)
{
	memcpy(LCD_Tx_Buffer,Hand,sizeof(Hand));
	LCD_DMA_Reset(LCD_TX_DMA,6);//复位DMA发送首地址
}

												/* P0			 P0_CC		  P1_V			P1_A		 P1_SOC	  P1_KW   	P1_TIME		P2_Type		P2_EXP   P2_D*/
Display_Position SHOW = {{155,71},{125,289},{122,195},{122,219},{95,125},{122,241},{122,264},{135,71},{220,58},{15,128,294},{7,29,190},{295,30,100,175}};//俏皮UI
unsigned char Link_ok = 0;//LCD屏连接状态1连接 0未连接
void LcdShow(void)
{
	static unsigned char t;
	if(Uart_Flag.Lcd_Rx_Flag)
	{
		unsigned char LCD_DataLong = LCD_RX_SIZE - DMA_GetCurrDataCounter(LCD_RX_DMA);
		if((LCD_Rx_Buffer[0]==0xAA)&&(LCD_Rx_Buffer[1]==0x00)&&(LCD_Rx_Buffer[2]==0x4F)&&(LCD_Rx_Buffer[3]==0x4B))
			Link_ok = 1;//LCD设备存在
		memset(LCD_Rx_Buffer,0,LCD_DataLong);
		LCD_DMA_Reset(LCD_RX_DMA,LCD_RX_SIZE);//复位DMA接受首地址
		Uart_Flag.Lcd_Rx_Flag = false;
	}

	if(Link_ok)
	{
		if(Type_DM.DErr!=0)//桩自检错误(这里是针对简易桩显示3个内容)
		{
			switch_page(4);//系统自检界面
			if(Type_DM.DErr&Geodesic)	Show_hanzi("未接地线！",10,SHOW.P0[0],SHOW.P0[1]);
				else if(Type_DM.DErr&No_Module)	Show_hanzi("无电源模块",10,SHOW.P0[0],SHOW.P0[1]);
					else if(Type_DM.DErr&Disconnect_C)	Show_hanzi("C板通讯故障",11,SHOW.P0[0],SHOW.P0[1]);
						else if(Type_DM.DErr&Dc_Table_Err)	Show_hanzi("无直流电表",10,SHOW.P0[0],SHOW.P0[1]);
		}
		else//桩自检通过
		{
			switch(Type_BMS.Step)
			{
				case BEGIN:
				case LOCKED:
				{
					switch_page(1);//待机界面
					show_number(AD_DATA.CC*10,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);
					if(Type_DM.JiTing == 1)	Show_hanzi("急停已按下！",12,SHOW.P0[0],SHOW.P0[1]-8);
						else if((AD_DATA.CC>3)&&(AD_DATA.CC<5))	Show_hanzi("请按下启停健",12,SHOW.P0[0],SHOW.P0[1]-10);
							else Show_hanzi("请插充电枪",10,SHOW.P0[0],SHOW.P0[1]);
				}break;
				case SEND_9728:	switch_page(1);Show_hanzi("充电握手中",10,SHOW.P0[0],SHOW.P0[1]);show_number(AD_DATA.CC*10,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;
				case SEND_256:	switch_page(1);Show_hanzi("充电辨识中",10,SHOW.P0[0],SHOW.P0[1]);show_number(AD_DATA.CC*10,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;
				case SEND_2048:	switch_page(1);Show_hanzi(" BMS准备"  ,9 ,SHOW.P0[0],SHOW.P0[1]);show_number(AD_DATA.CC*10,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;
				case SEND_2560:	switch_page(1);Show_hanzi("充电机准备",10,SHOW.P0[0],SHOW.P0[1]);show_number(AD_DATA.CC*10,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;	
				case SEND_4608:	
					switch_page(2);//充电界面
					show_number(Type_VolCur.Soc,SHOW.P1_Soc[0],SHOW.P1_Soc[1],3,0);//SOC
					show_number(Type_VolCur.Vol,SHOW.P1_V[0],SHOW.P1_V[1],4,1);//电压
					show_number(Type_VolCur.Cur,SHOW.P1_A[0],SHOW.P1_A[1],4,1);//电流 
					show_number(Type_VolCur.KWh,SHOW.P1_KW[0],SHOW.P1_KW[1],4,1);//电量:电表数据或者定时器累加
					show_number(ChargTime>>1,SHOW.P1_Time[0],SHOW.P1_Time[1],3,0);//充电时间		
				
					if((Type_DM.MErr2&0x7f)!=0)	show_number(Type_DM.MErr2,SHOW.Sta[1],SHOW.Sta[0],3,0);//判断模块状态不正常
						else if((Type_DM.MErr1&0xbe)!=0)	show_number(Type_DM.MErr1,SHOW.Sta[2],SHOW.Sta[0],3,0);
							else if((Type_DM.MErr0&0x11)!=0)	show_number(Type_DM.MErr0,SHOW.Sta[3],SHOW.Sta[0],3,0);							
					break;
				case SEND_6656:
				case STOP:
					switch_page(3);//充电完成界面
					show_number(ChargTime>>1,SHOW.P2_D[0],SHOW.P2_D[2],3,0);//充电时间
					show_number(Type_VolCur.KWh,SHOW.P2_D[1],SHOW.P2_D[2],4,1);//电量:电表数据或者定时器累加
					switch(Type_BMS.Stop_Reason)
					{
						case Time_Out:
							Show_hanzi(" 通讯超时",9,SHOW.P2_Type[0],SHOW.P2_Type[1]);
							switch(Type_BMS.time_out)
							{
								case BRM512_Timeout:	Show_hanzi("   BRM512",9,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCP1536_Timeout:	Show_hanzi("  BCP 1536",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BRO2304_Timeout:	Show_hanzi("  BRO 2304",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCS4352_Timeout:	Show_hanzi("  BCS 4352",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCL4096_Timeout:	Show_hanzi("  BCL 4096",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BST6400_Timeout:	Show_hanzi("  BST 6400",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
						case Err_Stop:
							Show_hanzi(" 故障停止",9,SHOW.P2_Type[0],SHOW.P2_Type[1]);
							switch(Type_BMS.DErr)
							{
								case Lock_ERR:			Show_hanzi("  无法上锁",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case GUN_Relay_Err:	Show_hanzi("枪上继电器故障",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-10);break;
								case KK_Relay_Err:	Show_hanzi("中间继电器故障",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-10);break;
								case Insulation_ERR:Show_hanzi("  绝缘错误",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Bat_Vol_ERR:		Show_hanzi("电池电压不匹配",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case CC_ERR:				Show_hanzi("CC电压异常",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Temp_High:			Show_hanzi("枪温度过高",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
						case BMS_Stop:
							Show_hanzi(" BMS中止",9,SHOW.P2_Type[0],SHOW.P2_Type[1]);
							switch(Type_BMS.BErr)
							{
								case Soc_Full:			Show_hanzi(" 已充满！！",11,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Insulation:		Show_hanzi("   绝缘故障",11,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BmsOutNetTemp:	Show_hanzi("输出连接器过温",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-10);break;
								case ChargeNet:			Show_hanzi("充电连接器故障",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-10);break;
								case BatTemp:				Show_hanzi("电池温度过高",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case HighRelay:			Show_hanzi("高压继电器故障",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-10);break;
								case Vol_2:					Show_hanzi("点2电压错误",11,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case CurUnknown:		Show_hanzi(" 电流不可信",11,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case VolUnknown:		Show_hanzi(" 电压不可信",11,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
						case Mannul_Stop:
							Show_hanzi(" 人工中止",9,SHOW.P2_Type[0],SHOW.P2_Type[1]);
							switch(Type_BMS.Manual)
							{
								case JT_Stop:			Show_hanzi(" 急停按下！",11,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Card_Stop:		Show_hanzi("  刷卡停止",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case App_Stop:		Show_hanzi("   App停止",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Start_Stop:	Show_hanzi("  启停开关",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
					}break;
				default:break;
			}
		}
		show_number(Module_Status.num,SHOW.A_M[2],SHOW.A_M[0],1,0);//电源模块个数
		if(MeterSta != No_Link)	Show_hanzi("M",1,SHOW.A_M[0],SHOW.A_M[1]);//连上电表
	}else	if(t++ > 1)	Check_Hand();
}


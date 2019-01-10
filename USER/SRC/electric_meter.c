#include "electric_meter.h"
#include "lcd.h"

Ammeter single_cmd[3] =
{
	{0x00,0x00,0x01,0x00,0x04,0.01},	//查询正向总功
	{0x00,0x01,0x01,0x02,0x02,0.1},		//查询电压A
	{0x00,0x01,0x02,0x02,0x03,0.001},	//查询电流
};

enum _Meter_Type Meter_Type;
Meter_Data_Type MeterData;
unsigned char RS485_DataLong,Cmd_Step;
enum _ElecMeter_status MeterSta = No_Link;

#define METER_RX_DMA DMA1_Channel5
#define METER_TX_DMA DMA1_Channel4

#define METER_RX_SIZE	32//接受
unsigned char METER_Rx_Buffer[METER_RX_SIZE];//接受数据缓存
#define METER_TX_SIZE	20//发送
unsigned char METER_Tx_Buffer[METER_TX_SIZE];//发送
//UART 1
void METER_UART_Init(uint32_t bound)
{
	
	GPIO_PinConfigure(GPIOA,10,GPIO_IN_FLOATING,GPIO_MODE_INPUT);//RX-下拉输入	
	GPIO_PinConfigure(GPIOA,9,GPIO_AF_PUSHPULL,GPIO_MODE_OUT50MHZ);//TX-复用推挽输出
	GPIO_PinConfigure(GPIOA,8,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT50MHZ);//DE使能脚PA8
	//初始化串口结构体
	USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//初始化串口时钟
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_Even;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	//设置串口中断优先级
	NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//打开空闲中断,用于接收不定长数据时判断接受完一帧数据
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);//打开发送完成中断，用于发送完数据后拉低485DE脚到接受模式
	USART_Cmd(USART1, ENABLE);

	/*初始化UART1 DMA TX RX*/
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//开启DMA1时钟
	//DMA1 Channel 5 triggerd by USART1 Rx event
	DMA_DeInit(DMA1_Channel5);//复位DMA控制器的通道
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)METER_Rx_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//外设作为数据来源
	DMA_InitStructure.DMA_BufferSize = METER_RX_SIZE;//缓存长度
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据宽度1字节
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存数据宽度1字节
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//非循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//外设与内存通讯，而非内存到内存
	DMA_Init(DMA1_Channel5,&DMA_InitStructure);
	DMA_Cmd(DMA1_Channel5,ENABLE);//启动DMA接受
	//DMA1 Channel 4 triggerd by USART1 Tx event
	DMA_DeInit(DMA1_Channel4);//复位DMA控制器的通道
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)METER_Tx_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//外设作为传送数据目的地
	DMA_InitStructure.DMA_BufferSize = 0;//发送长度为0，默认关闭
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据宽度1字节
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存数据宽度1字节
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//非循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//外设与内存通讯，而非内存到内存
	DMA_Init(DMA1_Channel4,	&DMA_InitStructure);
	//DMA_Cmd (DMA1_Channel4,ENABLE);//默认关闭
	USART_DMACmd(USART1, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);//使能串口DMA发送和接受
}

void USART1_IRQHandler(void)
{
	uint8_t Clear=Clear;
	if(USART_GetITStatus(USART1,USART_IT_IDLE) != RESET)
	{
		Clear=USART1->SR;
		Clear=USART1->DR;
		Uart_Flag.Meter_Rx_Flag = true;
	}
	if(USART_GetITStatus(USART1,USART_IT_TC) != RESET)
	{
		USART_ClearFlag(USART1, USART_FLAG_TC);
		RS485_RX_EN();//485发送完成则拉低使能脚到发送模式！
	}
}

void METER_DMA_Reset(DMA_Channel_TypeDef* DMAy_Channelx, unsigned char len)
{
	RS485_TX_EN();//485发送
	DMA_Cmd (DMAy_Channelx,DISABLE);//要先关闭DMA才能设置长度
	DMAy_Channelx->CNDTR = len;			//重新设置开始地址
	DMA_Cmd (DMAy_Channelx,ENABLE); //启动DMA发送
}

unsigned char ReadAddrCmd[12]={0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0X68,0X13,0X00,0XDF,0X16};//控制码0x13-查询电表地址
static void Check_ElecMeter_Addr(void)
{
	memcpy(METER_Tx_Buffer,ReadAddrCmd,12);
	METER_DMA_Reset(METER_TX_DMA,12);
}

unsigned char ReadDataCmd[10] = {0x68,0x00,0x00,0x00,0x00,0x00,0x00,0x68,0x11,0x04};//控制码0x11-请求读电表数据
static void Send_read_cmd(void)
{
	unsigned short Sum;
	memcpy(METER_Tx_Buffer,ReadDataCmd,10);
	METER_Tx_Buffer[10] = single_cmd[Cmd_Step].u8DI0 + 0x33;//前面的已经被默认赋值不需要进行改动
	METER_Tx_Buffer[11] = single_cmd[Cmd_Step].u8DI1 + 0x33;
	METER_Tx_Buffer[12] = single_cmd[Cmd_Step].u8DI2 + 0x33;
	METER_Tx_Buffer[13] = single_cmd[Cmd_Step].u8DI3 + 0x33;
	Cmd_Step = (Cmd_Step+1)%3;//正向总功->电压->电流
	for(char i=0; i<14; i++)	Sum += METER_Tx_Buffer[i];
	METER_Tx_Buffer[14] = Sum&0x00FF;//校验位
	METER_Tx_Buffer[15] = 0x16;//包尾
	METER_DMA_Reset(METER_TX_DMA,16);
}


static enum _Meter_Type DataCheck(void)//包头包尾 数据长度全部符合
{
	if((METER_Rx_Buffer[1]==0xFE)&&(METER_Rx_Buffer[3]==0xFE)&&(METER_Rx_Buffer[4]==0x68)&&(METER_Rx_Buffer[11]==0x68))
	{
		if(((METER_Rx_Buffer[13]+16)==RS485_DataLong) && (METER_Rx_Buffer[RS485_DataLong-1]==0X16))	return Single_Meter;
	}
	else if((METER_Rx_Buffer[0]==0xFE) && (METER_Rx_Buffer[1]==0x68) && (METER_Rx_Buffer[8]==0x68))//40KW 3相交流表
	{
		if(((METER_Rx_Buffer[10]+13)==RS485_DataLong) && (METER_Rx_Buffer[RS485_DataLong-1]==0X16)) return Three_Meter;	
	}
	return No_Meter;
}

unsigned char hex_to_ten(unsigned char number)//BCD码(十六进制)转换为十进制数，例0x49-->49
{
	return (((number&0xf0)>>1) + ((number&0xf0)>>3) + (number&0x0f));
}

void Send_485_Data(void)
{
	static unsigned char t,BaudRate_t;

	if(MeterSta == No_Link)//电表是断线状态
	{
		if(BaudRate_t == 7)	METER_UART_Init(9600);/*尝试以9600波特率跟电表通讯*/
		if(BaudRate_t ==15)	METER_UART_Init(2400);/*尝试以2400波特率跟电表通讯*/
		BaudRate_t = (BaudRate_t+1)%16;//0-19 No_Link下8s切换一次波特率
		Check_ElecMeter_Addr();//发信号查询电表地址
	}
	else if(MeterSta == ReadData)//请求读数据
	{
		t = 0;	BaudRate_t = 0;/*连上电表将切换波特率计时清零*/
		Send_read_cmd();//发送读取数据的请求//正向功/电压/电流
		MeterSta = WaitData;
	}
	else if(MeterSta == WaitData)//电表等待电表数据状态
	{
		if(++t >= 5)//超时5s
		{
			t = 0;	MeterSta = No_Link;//电表离线，尝试重新连接
		}
	}
}

unsigned int array_x_min[4]= {1,100,10000,1000000};
void Deal_485_Data(void)
{
	float lData = 0;
	unsigned short Sum = 0;
	if(MeterSta == No_Link)
	{
		if(DataCheck() == Single_Meter)
		{
			for(char i=1;i<7;i++)	ReadDataCmd[i] = METER_Rx_Buffer[13+i]-0x33;				
			MeterSta = ReadData;
		}else if(DataCheck() == Three_Meter)	
		{
			for(char i=1;i<7;i++)	ReadDataCmd[i] = METER_Rx_Buffer[10+i]-0x33;
			MeterSta = ReadData;
		}
	}
	if(MeterSta == WaitData)//等待数据
	{
		if(DataCheck() == Single_Meter)
		{
			for(char i=4;i<RS485_DataLong-2;i++)	Sum += METER_Rx_Buffer[i];		
			if((uint8_t)Sum == METER_Rx_Buffer[RS485_DataLong-2])//校验码正确
			{				
				for(char i=0;i<(METER_Rx_Buffer[13]-4);i++)	lData = lData + hex_to_ten(METER_Rx_Buffer[18+i]-0x33)*array_x_min[i];	
				//lData = lData * single_cmd[Cmd_Step].ratio;
				if((METER_Rx_Buffer[15]==0x33)&&(METER_Rx_Buffer[16]==0x34)&&(METER_Rx_Buffer[17]==0x33))	MeterData.kwh_realtime = lData*single_cmd[0].ratio;//正向整功
					else if((METER_Rx_Buffer[15]==0x34)&&(METER_Rx_Buffer[16]==0x34)&&(METER_Rx_Buffer[17]==0x35))	 MeterData.vol = lData*single_cmd[1].ratio;//电压
						else if((METER_Rx_Buffer[15]==0x34)&&(METER_Rx_Buffer[16]==0x35)&&(METER_Rx_Buffer[17]==0x35)) MeterData.cur = lData*single_cmd[2].ratio;//电流	
				MeterSta = ReadData;
			}
		}			
		else if(DataCheck() == Three_Meter)
		{
			for(char i=1;i<RS485_DataLong-2;i++)	Sum += METER_Rx_Buffer[i];		
			if((uint8_t)Sum == METER_Rx_Buffer[RS485_DataLong-2])//校验码正确
			{				
				for(char i=0;i<(METER_Rx_Buffer[10]-4);i++)	lData = lData + hex_to_ten(METER_Rx_Buffer[15+i]-0x33)*array_x_min[i];
				if((METER_Rx_Buffer[12]==0x33)&&(METER_Rx_Buffer[13]==0x34)&&(METER_Rx_Buffer[14]==0x33))	MeterData.kwh_realtime = lData*single_cmd[0].ratio;//正向整功
					else if((METER_Rx_Buffer[12]==0x34)&&(METER_Rx_Buffer[13]==0x34)&&(METER_Rx_Buffer[14]==0x35))	 MeterData.vol = lData*single_cmd[1].ratio*1.732f;//电压（三相）
						else if((METER_Rx_Buffer[12]==0x34)&&(METER_Rx_Buffer[13]==0x35)&&(METER_Rx_Buffer[14]==0x35)) MeterData.cur = lData*single_cmd[2].ratio;//电流	
				MeterSta = ReadData;
			}
		}	
	}
}


void Read_ElectricMeter_Data(void)//1s一次
{
	if(Uart_Flag.Meter_Rx_Flag)
	{
		RS485_DataLong = METER_RX_SIZE - DMA_GetCurrDataCounter(METER_RX_DMA);
		Deal_485_Data();
		METER_DMA_Reset(METER_RX_DMA,METER_RX_SIZE);
		memset(METER_Rx_Buffer,0,RS485_DataLong);
		Uart_Flag.Meter_Rx_Flag = false;
	}
	else Send_485_Data();
	
	if(Board_C_Sta != 0)//只有连接了C板才将电表作为必要设备
	{
		if(MeterSta == No_Link)	Type_DM.DErr |= Dc_Table_Err;//电表是断线状态
			else Type_DM.DErr &= ~Dc_Table_Err;//连上电表
	}
}

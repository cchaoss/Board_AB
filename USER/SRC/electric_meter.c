#include "electric_meter.h"
#include "lcd.h"

Ammeter xianxing_cmd[3] =
{
	{0x00,0x00,0x01,0x00,0x04,0.01},//查询正向总功
	{0x00,0x01,0x01,0x02,0x02,0.1},//查询电压A
	{0x00,0x01,0x02,0x02,0x03,0.001},//查询电流
};


#define RS485_CommandBuff_Size 32
unsigned char RS485_CommandBuff[RS485_CommandBuff_Size];
unsigned char RS485_DataLong;
float test_data[3];
u8 xianxing=0;
enum ElecMeter_status ElecMeterStatus = Link_Start;//电表连接状态为请求连接状态
uint8_t u8ReadAddrBuff[32] = {0x68,0x00,0x00,0x00,0x00,0x00,0x00,0x68,0x11,0x04,0x00,0x00,0x00,0x00,0X00,0X16};//控制码0x11--请求读电表数据

//UART 1
void METER_UART_DMA_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);//开启DMA2时钟
	/*DMA1 Channel 5 triggerd by USART1 Rx event*/
	DMA_DeInit(DMA1_Channel5);//复位DMA控制器的通道
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RS485_CommandBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//外设作为数据来源
	DMA_InitStructure.DMA_BufferSize = RS485_CommandBuff_Size;//缓存长度
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据宽度1字节
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存数据宽度1字节
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//非循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//外设与内存通讯，而非内存到内存
	DMA_Init(DMA1_Channel5,&DMA_InitStructure);
	DMA_Cmd (DMA1_Channel5,ENABLE);//启动DMA接受
	/*DMA1 Channel 4 triggerd by USART1 Tx event*/
	DMA_DeInit(DMA1_Channel4);//复位DMA控制器的通道
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RS485_CommandBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//外设作为传送数据目的地
	DMA_InitStructure.DMA_BufferSize = 0;//发送长度为0，默认关闭
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据宽度1字节
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存数据宽度1字节
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//非循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//外设与内存通讯，而非内存到内存
	DMA_Init(DMA1_Channel4,	&DMA_InitStructure);
	//DMA_Cmd (DMA2_Channel5,ENABLE);//默认关闭
	USART_DMACmd(USART1, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);//使能串口DMA发送和接受
}

void METER_UART_Init(uint32_t bound)
{
	GPIO_PinConfigure(GPIOA,10,GPIO_AF_PUSHPULL,GPIO_MODE_OUT10MHZ);//TX-复用推挽输出
	GPIO_PinConfigure(GPIOA,9,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//RX-下拉输入
	GPIO_PinConfigure(RS485_DE_GPIO_PORT,RS485_DE_PIN,GPIO_AF_PUSHPULL,GPIO_MODE_OUT10MHZ);//DE使能脚PA8
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
	USART_Cmd(USART1, ENABLE);

	RS485_RX_EN();//使能接受
	METER_UART_DMA_Init();//初始化串口DMA
}
//这里配置的是空闲中断
void USART1_IRQHandler(void)
{
	uint8_t Clear=Clear;
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		Uart_Flag.Meter_Rx_Flag = true;
		Clear=USART1->SR;
		Clear=USART1->DR;
	}
}


//发讯号查询电表地址
void Check_ElecMeter_Addr(void)
{
	uint8_t u8ReadPresentAddrBuff[12]={0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0X68,0X13,0X00,0XDF,0X16};//初始的地址，控制码0x13--读通信地�
//	RS485_DMA_send(u8ReadPresentAddrBuff,12);
}

u8 hex_to_ten(u8 number)
{
	u8	temp = (number&0xf0)>>1 + (number&0xf0)>>3 + (number&0x0f);
	return temp;
}
//校验包头是否正确
u8 Check_data_start(void)
{
	if((RS485_CommandBuff[1]==0xFE) && (RS485_CommandBuff[2]==0xFE) && (RS485_CommandBuff[4]==0x68) && (RS485_CommandBuff[11]==0x68))	return 1;
		else	return 0;
}

void Save_485_Data(void)
{
	float lData;
	u8 i;
	for(i=0;i<(RS485_CommandBuff[13]-4);i++)
	{
		//得到的数据先减0x33(根据协议)，再把BCD码(十六进制)转换为十进制数，例0x49-->49
		//lData = lData + ((((RS485_CommandBuff[18+i]-0x33)&0xF0)>>4)*10+((RS485_CommandBuff[18+i]-0x33)&0x0F)) * pow(100,i);
		lData = lData + hex_to_ten(RS485_CommandBuff[18+i]-0x33)*pow(100,i);

	}
	lData = lData * xianxing_cmd[xianxing].ratio;//保存数据
	if((RS485_CommandBuff[14]==0x33) && (RS485_CommandBuff[15]==0x33) &&
	(RS485_CommandBuff[16]==0x34) && (RS485_CommandBuff[17]==0x33))
	{
			test_data[0]=lData;
	}
	else if((RS485_CommandBuff[14]==0x33) && (RS485_CommandBuff[15]==0x34) &&
	(RS485_CommandBuff[16]==0x34) && (RS485_CommandBuff[17]==0x35))
	{
		test_data[1] = lData;//电压
	}
	else if((RS485_CommandBuff[14]==0x33) && (RS485_CommandBuff[15]==0x34)&&
	(RS485_CommandBuff[16]==0x35) && (RS485_CommandBuff[17]==0x35))
	{
		test_data[2] = lData;//电流
	}
	xianxing++;
	if(xianxing >= 3)//一共保存ITEM个数据，这个看宏定义，目前是3个
	{
		xianxing = 0;
	}
	ElecMeterStatus = AskData;//状态切换为请求电表数据需等待一定时间后才能获取)
}


void Send_read_cmd(void)
{
		u16 u16Sum;
		u8 i;
		//前面的已经被默认赋值不需要进行改动
		u8ReadAddrBuff[10] = xianxing_cmd[xianxing].u8DI0 + 0x33;
		u8ReadAddrBuff[11] = xianxing_cmd[xianxing].u8DI1 + 0x33;
		u8ReadAddrBuff[12] = xianxing_cmd[xianxing].u8DI2 + 0x33;
		u8ReadAddrBuff[13] = xianxing_cmd[xianxing].u8DI3 + 0x33;
		for(i=0;i<14;i++)
		{
			u16Sum = u16Sum + u8ReadAddrBuff[i];
		}
		u8ReadAddrBuff[14] = u16Sum & 0x00FF;//校验位
//		RS485_DMA_send(u8ReadAddrBuff,16);//发送本次读取数据的请求
}

void Send_485_Data(void)
{
	static u16	Check_time = 0,		//查询地址的间隔发送时间
							WaitData_time = 0,//计算获取数据的时间
							AskData_time = 0;	//计算询问数据的时间

	if(Link_Start == ElecMeterStatus)//电表是断线状态
	{
		WaitData_time = 0;

		Check_time++;
		if(1 == Check_time)//间隔时间发送查询信息
		{
			Check_time = 0;
			Check_ElecMeter_Addr();//发信号查询电表地址
		}
	}
	else if(Link_Ok == ElecMeterStatus)//请求读数据
	{
			WaitData_time = 0;//每次发送读数据请求后等待时间计数清0
			Send_read_cmd();//发送读取数据的请求//正向功/电压/电流
			ElecMeterStatus = WaitData;//状态切换为等待电表数据状态
	}
	else if(WaitData == ElecMeterStatus)//电表等待电表数据状态
	{
			WaitData_time++;
			if(5 == WaitData_time)//超时了
			{
				WaitData_time = 0;//超时清0
				ElecMeterStatus = Link_Start;//等待超时。重新返回请求连接状态
			}
	}
	else if(AskData == ElecMeterStatus)//请求电表数据(需等待一定时间后才能获取)
	{
		WaitData_time = 0;
		AskData_time++;
		if(1== AskData_time)//500ms后读取下一数据
		{
			AskData_time = 0;
			ElecMeterStatus = Link_Ok;//状态变为发送电表地址
		}
	}
}


void RS485_DMA_send(u8 *SendBuff,u32 size)
{
	RS485_TX_EN();
//	USART3_DMA_send(SendBuff,size);
	RS485_RX_EN();
}


void Deal_485_Data(void)
{
		u8 data_long;
		//电表是断线状态
	//电表返回的数据
		if(Link_Start == ElecMeterStatus)
		{
			if (1==Check_data_start()&&(RS485_CommandBuff[12]== 0x93))
			{
					data_long = RS485_CommandBuff[13] + 13 + 3;  //获取整个帧的长度
					if((RS485_DataLong == data_long) && (0x16 == RS485_CommandBuff[data_long-1]))
					{
						u8 i =0;
						for(i=1;i<7;i++)
						{
								u8ReadAddrBuff[i] = RS485_CommandBuff[13+i]-0x33;
						}
						ElecMeterStatus = Link_Ok;
					}
			}
		}
		if(WaitData== ElecMeterStatus)//等待数据
		{
			if (1==Check_data_start())
			{
					data_long= RS485_CommandBuff[13]+16;
					if((RS485_DataLong==data_long) && (0x16==RS485_CommandBuff[data_long-1])) //接收一帧数据结束
					{
								u8 i =0;
								u16 u16Sum;
								for(i=4;i<data_long-2;i++)
								{
									u16Sum = u16Sum + RS485_CommandBuff[i];
								}
								u16Sum = u16Sum & 0x00FF;
								if(((u8)u16Sum) == RS485_CommandBuff[data_long-2])//校验码正确
								{
									Save_485_Data();
								}
					}
			}
		}
}


void Deal_YaDa(void)//1s一次
{
	if(Uart_Flag.Meter_Rx_Flag)
	{
		RS485_DataLong = RS485_CommandBuff_Size - DMA_GetCurrDataCounter(DMA1_Channel5);

		Deal_485_Data();
		memset(RS485_CommandBuff,0,RS485_DataLong);
		Uart_Flag.Meter_Rx_Flag = false;
	}
	else	Send_485_Data();//未收到新的串口指令则根据电表状态发送不同的指令
}

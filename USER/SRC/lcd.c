#include "lcd.h"
#include <string.h>
#include "bms.h"
#include "acdc_module.h"

#define LCD_Buff_Size	15//接受数据缓存大小
uint8_t LCD_CommandBuff[LCD_Buff_Size];//接受数据缓存
Uart_Rx_FlagBits Uart_Flag;//串口接受标记

void LCD_USART2_Config(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;		/*定义一个GPIO_InitTypeDef类型的结构体，保存串口需要使用的IO口信息*/
	USART_InitTypeDef USART_InitStructure;	/*定义一个USART_InitTypeDef类型的结构体，保存串口相关配置信息*/

	/* 第1步：打开串口所用GPIO口和USART外设的时钟 */
	LCD_USART_GPIO_APBxClkCmd(LCD_USART_GPIO_CLK, ENABLE);
	LCD_USART_APBxClkCmd(LCD_USART_CLK, ENABLE);

	/* 第2步：将USART Tx的GPIO配置为复用推挽输出模式 */
	GPIO_InitStructure.GPIO_Pin = LCD_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_USART_TX_GPIO_PORT, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式 */
	GPIO_InitStructure.GPIO_Pin = LCD_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(LCD_USART_RX_GPIO_PORT, &GPIO_InitStructure);

	/* 第4步：设置串口硬件参数，完成串口的初始化配置 */
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(LCD_USARTx, &USART_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;// 串口中断优先级配置
  NVIC_InitStructure.NVIC_IRQChannel = LCD_USART_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(LCD_USARTx, USART_IT_IDLE, ENABLE);//打开空闲中断,用于接收不定长数据时判断接受完一帧数据
	USART_ITConfig(LCD_USARTx, USART_IT_RXNE, ENABLE);//使能串口接收中断
	USART_Cmd(LCD_USARTx, ENABLE);

	USART_ClearFlag(LCD_USARTx, USART_FLAG_TC);
}

void USART2_DMA_receive(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(USART2_RX_DMA_CLK, ENABLE);//开启DMA时钟---正常是在这里打开时钟,不过因为这次该函数被循环调用,所以不在这里开启时钟，而是初始化

	DMA_DeInit(USART2_RX_DMA_CHANNEL);// 复位DMA控制器的通道
  USART_DMACmd(LCD_USARTx, USART_DMAReq_Rx, ENABLE);

	//配置 DMA 初始化结构体
	DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_ADDRESS;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)LCD_CommandBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = LCD_Buff_Size;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(USART2_RX_DMA_CHANNEL,&DMA_InitStructure);
	DMA_Cmd (USART2_RX_DMA_CHANNEL,ENABLE);
}

void USART2_DMA_send(u8 *SendBuff,u32 size)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(USART2_TX_DMA_CLK, ENABLE);

  /* USART 向 DMA发出TX请求 */
  USART_DMACmd(LCD_USARTx, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd (USART2_TX_DMA_CHANNEL,DISABLE);

	DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_ADDRESS;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)SendBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = size;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(USART2_TX_DMA_CHANNEL, &DMA_InitStructure);
	DMA_Cmd (USART2_TX_DMA_CHANNEL,ENABLE);

	while(DMA_GetCurrDataCounter(USART2_TX_DMA_CHANNEL));
}

// 串口2中断服务函数,接收不定长数据---LCD
void LCD_USART_IRQHandler(void)
{
	if(USART_GetITStatus(LCD_USARTx, USART_IT_IDLE) != RESET)
	{
		Uart_Flag.Lcd_Rx_Flag = true;
	}
}
/*************************************************************************/
void show_number(unsigned int number,unsigned short x,unsigned short y)
{
	u8 Show_buff[21] = {0xAA,0x14,0x01,0xff,0xff,0x00,0x00,
											0x08,0x02,//整数位数，小数位数
											0x00,0x00,0x00,0x00,//坐标X,Y
											0x00,0x00,0x00,0x00,//数值			
											0xcc,0x33,0xc3,0x3c};
	Show_buff[9] = 	(x&0xff00)>>8;
  Show_buff[10]=  x&0x00ff;
	Show_buff[11]=	(y&0xff00)>>8;
  Show_buff[12]=	y&0x00ff;		
	Show_buff[13]= (number>>24);
	Show_buff[14]= 	(number>>16)&0x00ff;			
	Show_buff[15]= 	(number&0xff00)>>8;
	Show_buff[16]= 	(number&0x00ff);																				
	USART2_DMA_send(Show_buff,21);
}
void Show_hanzi(unsigned char *hanzi,u8 size,u16 x,u16 y)
{
	u8 Show_buff[40] = {0xAA,0x11,0x01,0xff,0xff,0x00,0x00,
												0x00,0x00,0x00,0x00,
												0x00,0x00,0x00,0x00,};
	u8 i;//字显现的坐标位置
	Show_buff[7]= (x&0xff00)>>8;
  Show_buff[8]=  x&0x00ff;
	Show_buff[9]=	 (y&0xff00)>>8;
  Show_buff[10]=	y&0x00ff;																									
	for(i=0;i<(size);i++)
	{
		Show_buff[11+i] = hanzi[i];//汉字存储区域
	}
	Show_buff[size+11]= 0xcc;
	Show_buff[size+12]= 0x33;
	Show_buff[size+13]= 0xc3;
	Show_buff[size+14]= 0x3c;
	USART2_DMA_send(Show_buff,size+15);
}
void switch_page(unsigned char page)
{
	unsigned char Show_buff[8] = {0xAA,0x22,0x00,0x00,0xCC,0x33,0xC3,0x3C};
	Show_buff[3] = page;
	USART2_DMA_send(Show_buff,8);
}

Display_Position SHOW = {{60,160},//P0 //x-y
												 {60,120},//P1_V
												 {60,160},//P1_A
												 {60,200},//P1_Soc
												 {60,240},//P1_KW
												 {60,280},//P1_Time
												 {60,160},//P2_STOP
												 {60,240},//P2_Exp
												 {60,160}};//P3_Err

unsigned char Check_Hand[6] = {0xAA,0x00,0xCC,0x33,0xC3,0x3C};//握手指令
unsigned char Link_ok = 0;//LCD屏连接状态1连接 0未连接

void LcdShow(void)
{
	if(Uart_Flag.Lcd_Rx_Flag)
	{
		unsigned char LCD_DataLong = LCD_Buff_Size - DMA_GetCurrDataCounter(USART2_RX_DMA_CHANNEL);
		USART2_DMA_receive();
		if(LCD_CommandBuff[0]==0xAA)	Link_ok = 1;//LCD设备存在
		memset(LCD_CommandBuff,0,LCD_DataLong);
		Uart_Flag.Lcd_Rx_Flag = false;
	}

	if(Link_ok)
	{
		switch(BMS_STA)
		{
			case BEGIN:			switch_page(0);	Show_hanzi("请插抢",6,SHOW.P0[0],SHOW.P0[1]);break;
			case SEND_9728:	switch_page(0);	Show_hanzi("充电握手中",10,SHOW.P0[0],SHOW.P0[1]);break;
			case SEND_256:	switch_page(0);	Show_hanzi("充电辨识中",10,SHOW.P0[0],SHOW.P0[1]);break;
			case SEND_2048:	switch_page(0);	Show_hanzi("BMS准备中",12,SHOW.P0[0],SHOW.P0[1]);break;
			case SEND_2560:	switch_page(0);	Show_hanzi("充电机准备中",12,SHOW.P0[0],SHOW.P0[1]);break;
			
			case SEND_4608:	
			case SEND_6656:					
				switch_page(1);
				show_number(Module_Status.Output_Vol,SHOW.P1_V[0],SHOW.P1_V[0]);//电压
				show_number(Module_Status.Output_Cur,SHOW.P1_A[0],SHOW.P1_A[0]);//电流 
				show_number(Data_4352.PreSOC,SHOW.P1_Soc[0],SHOW.P1_Soc[0]);//SOC
				//show_number(Data_4352.PreSOC,SHOW.P1_KW[0],SHOW.P1_KW[0]);//电量(简易桩不显示)
				show_number(Data_4352.RemaChargTime,SHOW.P1_KW[0],SHOW.P1_KW[0]);//剩余时间
			break;
			
			case STOP:	
				switch_page(2);	
				if(Type_BMS.Stop_Reason == Time_Out)
				{
					Show_hanzi("通讯超时停止",12,SHOW.P2_STOP[0],SHOW.P2_STOP[1]);
				}else if(Type_BMS.Stop_Reason == Err_Stop)
				{
					Show_hanzi("故障停止",8,SHOW.P2_STOP[0],SHOW.P2_STOP[1]);
				}else if(Type_BMS.Stop_Reason == BMS_Stop)
				{
					Show_hanzi("BMS中止",8,SHOW.P2_STOP[0],SHOW.P2_STOP[1]);
				}else if(Type_BMS.Stop_Reason == Mannul_Stop)
				{
					Show_hanzi("人工中止",8,SHOW.P2_STOP[0],SHOW.P2_STOP[1]);
				}
			break;
		}
	}else	USART2_DMA_send(Check_Hand,6);
}


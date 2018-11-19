#include "electric_meter.h"
#include "lcd.h"

Ammeter xianxing_cmd[3] =
{
	{0x00,0x00,0x01,0x00,0x04,0.01},//²éÑ¯ÕıÏò×Ü¹¦
	{0x00,0x01,0x01,0x02,0x02,0.1},//²éÑ¯µçÑ¹A
	{0x00,0x01,0x02,0x02,0x03,0.001},//²éÑ¯µçÁ÷
};


#define RS485_CommandBuff_Size 32
unsigned char RS485_CommandBuff[RS485_CommandBuff_Size];
unsigned char RS485_DataLong;
float test_data[3];
u8 xianxing=0;
enum ElecMeter_status ElecMeterStatus = Link_Start;//µç±íÁ¬½Ó×´Ì¬ÎªÇëÇóÁ¬½Ó×´Ì¬
uint8_t u8ReadAddrBuff[32] = {0x68,0x00,0x00,0x00,0x00,0x00,0x00,0x68,0x11,0x04,0x00,0x00,0x00,0x00,0X00,0X16};//¿ØÖÆÂë0x11--ÇëÇó¶Áµç±íÊı¾İ

void RS485_Config(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RS485_USART_GPIO_CLK | RS485_DE_GPIO_CLK, ENABLE);//GPIO¿ÚÊ±ÖÓ
	RCC_APB1PeriphClockCmd(RS485_USART_CLK, ENABLE);		//USARTÍâÉèµÄÊ±ÖÓ

	GPIO_InitStructure.GPIO_Pin = RS485_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RS485_USART_TX_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = RS485_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(RS485_USART_RX_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = RS485_DE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(RS485_DE_GPIO_PORT, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_Even ;
	USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(RS485_USARTx, &USART_InitStructure);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel =RS485_USART_IRQ ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(RS485_USARTx, USART_IT_IDLE, ENABLE);
  USART_Cmd(RS485_USARTx, ENABLE);

	GPIO_ResetBits(RS485_DE_GPIO_PORT,RS485_DE_PIN);//ÊÕ·¢¿ØÖÆÒı½Å
}

// ´®¿Ú3ÖĞ¶Ï
//ÕâÀïÅäÖÃµÄÊÇ¿ÕÏĞÖĞ¶Ï
void RS485_USART_IRQHandler(void)
{
	uint8_t Clear=Clear;
	if(USART_GetITStatus(RS485_USARTx, USART_IT_IDLE) != RESET)
	{
		Uart_Flag.Meter_Rx_Flag = true;
		Clear=RS485_USARTx->SR;
		Clear=RS485_USARTx->DR;
	}
}

void USART3_DMA_send(u8 *SendBuff,u32 size)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(USART3_TX_DMA_CLK, ENABLE);

  USART_DMACmd(RS485_USARTx, USART_DMAReq_Tx, ENABLE);
	DMA_DeInit(USART3_TX_DMA_CHANNEL);
	DMA_Cmd(USART3_TX_DMA_CHANNEL,DISABLE);

	DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_ADDRESS;
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

	DMA_Init(USART3_TX_DMA_CHANNEL, &DMA_InitStructure);
  DMA_ClearFlag(USART3_TX_DMA_FLAG);
	DMA_Cmd (USART3_TX_DMA_CHANNEL,ENABLE);

	while(DMA_GetCurrDataCounter(USART3_TX_DMA_CHANNEL));
	while(USART_GetFlagStatus(RS485_USARTx, USART_FLAG_TC) == RESET);
}

void USART3_DMA_receive(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(USART3_RX_DMA_CLK, ENABLE);

	DMA_DeInit(USART3_RX_DMA_CHANNEL);// ¸´Î»DMA¿ØÖÆÆ÷µÄÍ¨µÀ
  USART_DMACmd(RS485_USARTx, USART_DMAReq_Rx, ENABLE);

	DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_ADDRESS;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RS485_CommandBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = RS485_CommandBuff_Size;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(USART3_RX_DMA_CHANNEL, &DMA_InitStructure);
	DMA_Cmd(USART3_RX_DMA_CHANNEL,ENABLE);
}


//·¢Ñ¶ºÅ²éÑ¯µç±íµØÖ·
void Check_ElecMeter_Addr(void)
{
	uint8_t u8ReadPresentAddrBuff[12]={0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0X68,0X13,0X00,0XDF,0X16};//³õÊ¼µÄµØÖ·£¬¿ØÖÆÂë0x13--¶ÁÍ¨ĞÅµØÖ
	RS485_DMA_send(u8ReadPresentAddrBuff,12);
}

u8 hex_to_ten(u8 number)
{
	u8	temp = (number&0xf0)>>1 + (number&0xf0)>>3 + (number&0x0f);
	return temp;
}
//Ğ£Ñé°üÍ·ÊÇ·ñÕıÈ·
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
		//µÃµ½µÄÊı¾İÏÈ¼õ0x33(¸ù¾İĞ­Òé)£¬ÔÙ°ÑBCDÂë(Ê®Áù½øÖÆ)×ª»»ÎªÊ®½øÖÆÊı£¬Àı0x49-->49
		//lData = lData + ((((RS485_CommandBuff[18+i]-0x33)&0xF0)>>4)*10+((RS485_CommandBuff[18+i]-0x33)&0x0F)) * pow(100,i);
		lData = lData + hex_to_ten(RS485_CommandBuff[18+i]-0x33)*pow(100,i);

	}
	lData = lData * xianxing_cmd[xianxing].ratio;//±£´æÊı¾İ
	if((RS485_CommandBuff[14]==0x33) && (RS485_CommandBuff[15]==0x33) &&
	(RS485_CommandBuff[16]==0x34) && (RS485_CommandBuff[17]==0x33))
	{
			test_data[0]=lData;
	}
	else if((RS485_CommandBuff[14]==0x33) && (RS485_CommandBuff[15]==0x34) &&
	(RS485_CommandBuff[16]==0x34) && (RS485_CommandBuff[17]==0x35))
	{
		test_data[1] = lData;//µçÑ¹
	}
	else if((RS485_CommandBuff[14]==0x33) && (RS485_CommandBuff[15]==0x34)&&
	(RS485_CommandBuff[16]==0x35) && (RS485_CommandBuff[17]==0x35))
	{
		test_data[2] = lData;//µçÁ÷
	}
	xianxing++;
	if(xianxing >= 3)//Ò»¹²±£´æITEM¸öÊı¾İ£¬Õâ¸ö¿´ºê¶¨Òå£¬Ä¿Ç°ÊÇ3¸ö
	{
		xianxing = 0;
	}
	ElecMeterStatus = AskData;//×´Ì¬ÇĞ»»ÎªÇëÇóµç±íÊı¾İĞèµÈ´ıÒ»¶¨Ê±¼äºó²ÅÄÜ»ñÈ¡)
}


void Send_read_cmd(void)
{
		u16 u16Sum;
		u8 i;
		//Ç°ÃæµÄÒÑ¾­±»Ä¬ÈÏ¸³Öµ²»ĞèÒª½øĞĞ¸Ä¶¯
		u8ReadAddrBuff[10] = xianxing_cmd[xianxing].u8DI0 + 0x33;
		u8ReadAddrBuff[11] = xianxing_cmd[xianxing].u8DI1 + 0x33;
		u8ReadAddrBuff[12] = xianxing_cmd[xianxing].u8DI2 + 0x33;
		u8ReadAddrBuff[13] = xianxing_cmd[xianxing].u8DI3 + 0x33;
		for(i=0;i<14;i++)
		{
			u16Sum = u16Sum + u8ReadAddrBuff[i];
		}
		u8ReadAddrBuff[14] = u16Sum & 0x00FF;//Ğ£ÑéÎ»
		RS485_DMA_send(u8ReadAddrBuff,16);//·¢ËÍ±¾´Î¶ÁÈ¡Êı¾İµÄÇëÇó
}

void Send_485_Data(void)
{
	static u16	Check_time = 0,		//²éÑ¯µØÖ·µÄ¼ä¸ô·¢ËÍÊ±¼ä
							WaitData_time = 0,//¼ÆËã»ñÈ¡Êı¾İµÄÊ±¼ä
							AskData_time = 0;	//¼ÆËãÑ¯ÎÊÊı¾İµÄÊ±¼ä

	if(Link_Start == ElecMeterStatus)//µç±íÊÇ¶ÏÏß×´Ì¬
	{
		WaitData_time = 0;

		Check_time++;
		if(1 == Check_time)//¼ä¸ôÊ±¼ä·¢ËÍ²éÑ¯ĞÅÏ¢
		{
			Check_time = 0;
			Check_ElecMeter_Addr();//·¢ĞÅºÅ²éÑ¯µç±íµØÖ·
		}
	}
	else if(Link_Ok == ElecMeterStatus)//ÇëÇó¶ÁÊı¾İ
	{
			WaitData_time = 0;//Ã¿´Î·¢ËÍ¶ÁÊı¾İÇëÇóºóµÈ´ıÊ±¼ä¼ÆÊıÇå0
			Send_read_cmd();//·¢ËÍ¶ÁÈ¡Êı¾İµÄÇëÇó//ÕıÏò¹¦/µçÑ¹/µçÁ÷
			ElecMeterStatus = WaitData;//×´Ì¬ÇĞ»»ÎªµÈ´ıµç±íÊı¾İ×´Ì¬
	}
	else if(WaitData == ElecMeterStatus)//µç±íµÈ´ıµç±íÊı¾İ×´Ì¬
	{
			WaitData_time++;
			if(5 == WaitData_time)//³¬Ê±ÁË
			{
				WaitData_time = 0;//³¬Ê±Çå0
				ElecMeterStatus = Link_Start;//µÈ´ı³¬Ê±¡£ÖØĞÂ·µ»ØÇëÇóÁ¬½Ó×´Ì¬
			}
	}
	else if(AskData == ElecMeterStatus)//ÇëÇóµç±íÊı¾İ(ĞèµÈ´ıÒ»¶¨Ê±¼äºó²ÅÄÜ»ñÈ¡)
	{
		WaitData_time = 0;
		AskData_time++;
		if(1== AskData_time)//500msºó¶ÁÈ¡ÏÂÒ»Êı¾İ
		{
			AskData_time = 0;
			ElecMeterStatus = Link_Ok;//×´Ì¬±äÎª·¢ËÍµç±íµØÖ·
		}
	}
}


void RS485_DMA_send(u8 *SendBuff,u32 size)
{
	RS485_TX_EN();
	USART3_DMA_send(SendBuff,size);
	RS485_RX_EN();
}


void Deal_485_Data(void)
{
		u8 data_long;
		//µç±íÊÇ¶ÏÏß×´Ì¬
	//µç±í·µ»ØµÄÊı¾İ
		if(Link_Start == ElecMeterStatus)
		{
			if (1==Check_data_start()&&(RS485_CommandBuff[12]== 0x93))
			{
					data_long = RS485_CommandBuff[13] + 13 + 3;  //»ñÈ¡Õû¸öÖ¡µÄ³¤¶È
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
		if(WaitData== ElecMeterStatus)//µÈ´ıÊı¾İ
		{
			if (1==Check_data_start())
			{
					data_long= RS485_CommandBuff[13]+16;
					if((RS485_DataLong==data_long) && (0x16==RS485_CommandBuff[data_long-1])) //½ÓÊÕÒ»Ö¡Êı¾İ½áÊø
					{
								u8 i =0;
								u16 u16Sum;
								for(i=4;i<data_long-2;i++)
								{
									u16Sum = u16Sum + RS485_CommandBuff[i];
								}
								u16Sum = u16Sum & 0x00FF;
								if(((u8)u16Sum) == RS485_CommandBuff[data_long-2])//Ğ£ÑéÂëÕıÈ·
								{
									Save_485_Data();
								}
					}
			}
		}
}


void Deal_YaDa(void)//1sÒ»´Î
{
	if(Uart_Flag.Meter_Rx_Flag)
	{
		RS485_DataLong = RS485_CommandBuff_Size - DMA_GetCurrDataCounter(USART3_RX_DMA_CHANNEL);
		USART3_DMA_receive();

		Deal_485_Data();
		memset(RS485_CommandBuff,0,RS485_DataLong);
		Uart_Flag.Meter_Rx_Flag = false;
	}
	else	Send_485_Data();//Î´ÊÕµ½ĞÂµÄ´®¿ÚÖ¸ÁîÔò¸ù¾İµç±í×´Ì¬·¢ËÍ²»Í¬µÄÖ¸Áî
}

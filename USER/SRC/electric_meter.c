#include "electric_meter.h"
#include "lcd.h"

Ammeter xianxing_cmd[3] =
{
	{0x00,0x00,0x01,0x00,0x04,0.01},//��ѯ�����ܹ�
	{0x00,0x01,0x01,0x02,0x02,0.1},//��ѯ��ѹA
	{0x00,0x01,0x02,0x02,0x03,0.001},//��ѯ����
};


#define RS485_CommandBuff_Size 32
unsigned char RS485_CommandBuff[RS485_CommandBuff_Size];
unsigned char RS485_DataLong;
float test_data[3];
u8 xianxing=0;
enum ElecMeter_status ElecMeterStatus = Link_Start;//�������״̬Ϊ��������״̬
uint8_t u8ReadAddrBuff[32] = {0x68,0x00,0x00,0x00,0x00,0x00,0x00,0x68,0x11,0x04,0x00,0x00,0x00,0x00,0X00,0X16};//������0x11--������������

//UART 1
void METER_UART_DMA_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);//����DMA2ʱ��
	/*DMA1 Channel 5 triggerd by USART1 Rx event*/
	DMA_DeInit(DMA1_Channel5);//��λDMA��������ͨ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RS485_CommandBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//������Ϊ������Դ
	DMA_InitStructure.DMA_BufferSize = RS485_CommandBuff_Size;//���泤��
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//�����ַ������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݿ���1�ֽ�
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//�ڴ����ݿ���1�ֽ�
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//��ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//�������ڴ�ͨѶ�������ڴ浽�ڴ�
	DMA_Init(DMA1_Channel5,&DMA_InitStructure);
	DMA_Cmd (DMA1_Channel5,ENABLE);//����DMA����
	/*DMA1 Channel 4 triggerd by USART1 Tx event*/
	DMA_DeInit(DMA1_Channel4);//��λDMA��������ͨ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RS485_CommandBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//������Ϊ��������Ŀ�ĵ�
	DMA_InitStructure.DMA_BufferSize = 0;//���ͳ���Ϊ0��Ĭ�Ϲر�
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//�����ַ������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݿ���1�ֽ�
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//�ڴ����ݿ���1�ֽ�
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//��ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//�������ڴ�ͨѶ�������ڴ浽�ڴ�
	DMA_Init(DMA1_Channel4,	&DMA_InitStructure);
	//DMA_Cmd (DMA2_Channel5,ENABLE);//Ĭ�Ϲر�
	USART_DMACmd(USART1, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);//ʹ�ܴ���DMA���ͺͽ���
}

void METER_UART_Init(uint32_t bound)
{
	GPIO_PinConfigure(GPIOA,10,GPIO_AF_PUSHPULL,GPIO_MODE_OUT10MHZ);//TX-�����������
	GPIO_PinConfigure(GPIOA,9,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//RX-��������
	GPIO_PinConfigure(RS485_DE_GPIO_PORT,RS485_DE_PIN,GPIO_AF_PUSHPULL,GPIO_MODE_OUT10MHZ);//DEʹ�ܽ�PA8
	//��ʼ�����ڽṹ��
	USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//��ʼ������ʱ��
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_Even;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	//���ô����ж����ȼ�
	NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//�򿪿����ж�,���ڽ��ղ���������ʱ�жϽ�����һ֡����
	USART_Cmd(USART1, ENABLE);

	RS485_RX_EN();//ʹ�ܽ���
	METER_UART_DMA_Init();//��ʼ������DMA
}
//�������õ��ǿ����ж�
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


//��Ѷ�Ų�ѯ�����ַ
void Check_ElecMeter_Addr(void)
{
	uint8_t u8ReadPresentAddrBuff[12]={0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0X68,0X13,0X00,0XDF,0X16};//��ʼ�ĵ�ַ��������0x13--��ͨ�ŵ��
//	RS485_DMA_send(u8ReadPresentAddrBuff,12);
}

u8 hex_to_ten(u8 number)
{
	u8	temp = (number&0xf0)>>1 + (number&0xf0)>>3 + (number&0x0f);
	return temp;
}
//У���ͷ�Ƿ���ȷ
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
		//�õ��������ȼ�0x33(����Э��)���ٰ�BCD��(ʮ������)ת��Ϊʮ����������0x49-->49
		//lData = lData + ((((RS485_CommandBuff[18+i]-0x33)&0xF0)>>4)*10+((RS485_CommandBuff[18+i]-0x33)&0x0F)) * pow(100,i);
		lData = lData + hex_to_ten(RS485_CommandBuff[18+i]-0x33)*pow(100,i);

	}
	lData = lData * xianxing_cmd[xianxing].ratio;//��������
	if((RS485_CommandBuff[14]==0x33) && (RS485_CommandBuff[15]==0x33) &&
	(RS485_CommandBuff[16]==0x34) && (RS485_CommandBuff[17]==0x33))
	{
			test_data[0]=lData;
	}
	else if((RS485_CommandBuff[14]==0x33) && (RS485_CommandBuff[15]==0x34) &&
	(RS485_CommandBuff[16]==0x34) && (RS485_CommandBuff[17]==0x35))
	{
		test_data[1] = lData;//��ѹ
	}
	else if((RS485_CommandBuff[14]==0x33) && (RS485_CommandBuff[15]==0x34)&&
	(RS485_CommandBuff[16]==0x35) && (RS485_CommandBuff[17]==0x35))
	{
		test_data[2] = lData;//����
	}
	xianxing++;
	if(xianxing >= 3)//һ������ITEM�����ݣ�������궨�壬Ŀǰ��3��
	{
		xianxing = 0;
	}
	ElecMeterStatus = AskData;//״̬�л�Ϊ������������ȴ�һ��ʱ�����ܻ�ȡ)
}


void Send_read_cmd(void)
{
		u16 u16Sum;
		u8 i;
		//ǰ����Ѿ���Ĭ�ϸ�ֵ����Ҫ���иĶ�
		u8ReadAddrBuff[10] = xianxing_cmd[xianxing].u8DI0 + 0x33;
		u8ReadAddrBuff[11] = xianxing_cmd[xianxing].u8DI1 + 0x33;
		u8ReadAddrBuff[12] = xianxing_cmd[xianxing].u8DI2 + 0x33;
		u8ReadAddrBuff[13] = xianxing_cmd[xianxing].u8DI3 + 0x33;
		for(i=0;i<14;i++)
		{
			u16Sum = u16Sum + u8ReadAddrBuff[i];
		}
		u8ReadAddrBuff[14] = u16Sum & 0x00FF;//У��λ
//		RS485_DMA_send(u8ReadAddrBuff,16);//���ͱ��ζ�ȡ���ݵ�����
}

void Send_485_Data(void)
{
	static u16	Check_time = 0,		//��ѯ��ַ�ļ������ʱ��
							WaitData_time = 0,//�����ȡ���ݵ�ʱ��
							AskData_time = 0;	//����ѯ�����ݵ�ʱ��

	if(Link_Start == ElecMeterStatus)//����Ƕ���״̬
	{
		WaitData_time = 0;

		Check_time++;
		if(1 == Check_time)//���ʱ�䷢�Ͳ�ѯ��Ϣ
		{
			Check_time = 0;
			Check_ElecMeter_Addr();//���źŲ�ѯ�����ַ
		}
	}
	else if(Link_Ok == ElecMeterStatus)//���������
	{
			WaitData_time = 0;//ÿ�η��Ͷ����������ȴ�ʱ�������0
			Send_read_cmd();//���Ͷ�ȡ���ݵ�����//����/��ѹ/����
			ElecMeterStatus = WaitData;//״̬�л�Ϊ�ȴ��������״̬
	}
	else if(WaitData == ElecMeterStatus)//����ȴ��������״̬
	{
			WaitData_time++;
			if(5 == WaitData_time)//��ʱ��
			{
				WaitData_time = 0;//��ʱ��0
				ElecMeterStatus = Link_Start;//�ȴ���ʱ�����·�����������״̬
			}
	}
	else if(AskData == ElecMeterStatus)//����������(��ȴ�һ��ʱ�����ܻ�ȡ)
	{
		WaitData_time = 0;
		AskData_time++;
		if(1== AskData_time)//500ms���ȡ��һ����
		{
			AskData_time = 0;
			ElecMeterStatus = Link_Ok;//״̬��Ϊ���͵����ַ
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
		//����Ƕ���״̬
	//������ص�����
		if(Link_Start == ElecMeterStatus)
		{
			if (1==Check_data_start()&&(RS485_CommandBuff[12]== 0x93))
			{
					data_long = RS485_CommandBuff[13] + 13 + 3;  //��ȡ����֡�ĳ���
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
		if(WaitData== ElecMeterStatus)//�ȴ�����
		{
			if (1==Check_data_start())
			{
					data_long= RS485_CommandBuff[13]+16;
					if((RS485_DataLong==data_long) && (0x16==RS485_CommandBuff[data_long-1])) //����һ֡���ݽ���
					{
								u8 i =0;
								u16 u16Sum;
								for(i=4;i<data_long-2;i++)
								{
									u16Sum = u16Sum + RS485_CommandBuff[i];
								}
								u16Sum = u16Sum & 0x00FF;
								if(((u8)u16Sum) == RS485_CommandBuff[data_long-2])//У������ȷ
								{
									Save_485_Data();
								}
					}
			}
		}
}


void Deal_YaDa(void)//1sһ��
{
	if(Uart_Flag.Meter_Rx_Flag)
	{
		RS485_DataLong = RS485_CommandBuff_Size - DMA_GetCurrDataCounter(DMA1_Channel5);

		Deal_485_Data();
		memset(RS485_CommandBuff,0,RS485_DataLong);
		Uart_Flag.Meter_Rx_Flag = false;
	}
	else	Send_485_Data();//δ�յ��µĴ���ָ������ݵ��״̬���Ͳ�ͬ��ָ��
}
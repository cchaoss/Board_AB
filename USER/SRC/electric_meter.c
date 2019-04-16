#include "electric_meter.h"
#include "lcd.h"

Ammeter single_cmd[3] =
{
	{0x00,0x00,0x01,0x00,0x04,0.01},	//��ѯ�����ܹ�
	{0x00,0x01,0x01,0x02,0x02,0.1},		//��ѯ��ѹA
	{0x00,0x01,0x02,0x02,0x03,0.001},	//��ѯ����
};

Meter_Data_Type MeterData;
unsigned char RS485_DataLong;
enum _Meter_Type Meter_Type = No_Meter;
enum _ElecMeter_status MeterSta = No_Link;

#define METER_RX_DMA DMA1_Channel5
#define METER_TX_DMA DMA1_Channel4

#define METER_RX_SIZE	32//�����ܳ���
unsigned char METER_Rx_Buffer[METER_RX_SIZE];//�������ݻ���
#define METER_TX_SIZE	20
unsigned char METER_Tx_Buffer[METER_TX_SIZE];
//UART 1
void METER_UART_Init(uint32_t bound)
{
	
	GPIO_PinConfigure(GPIOA,10,GPIO_IN_FLOATING,GPIO_MODE_INPUT);//RX-��������	
	GPIO_PinConfigure(GPIOA,9,GPIO_AF_PUSHPULL,GPIO_MODE_OUT50MHZ);//TX-�����������
	GPIO_PinConfigure(GPIOA,8,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT50MHZ);//DEʹ�ܽ�PA8
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
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);//�򿪷�������жϣ����ڷ��������ݺ�����485DE�ŵ�����ģʽ
	USART_Cmd(USART1, ENABLE);

	/*��ʼ��UART1 DMA TX RX*/
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//����DMA1ʱ��
	//DMA1 Channel 5 triggerd by USART1 Rx event
	DMA_DeInit(DMA1_Channel5);//��λDMA��������ͨ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)METER_Rx_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//������Ϊ������Դ
	DMA_InitStructure.DMA_BufferSize = METER_RX_SIZE;//���泤��
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//�����ַ������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݿ��1�ֽ�
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//�ڴ����ݿ��1�ֽ�
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//��ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//�������ڴ�ͨѶ�������ڴ浽�ڴ�
	DMA_Init(DMA1_Channel5,&DMA_InitStructure);
	DMA_Cmd(DMA1_Channel5,ENABLE);//����DMA����
	//DMA1 Channel 4 triggerd by USART1 Tx event
	DMA_DeInit(DMA1_Channel4);//��λDMA��������ͨ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)METER_Tx_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//������Ϊ��������Ŀ�ĵ�
	DMA_InitStructure.DMA_BufferSize = 0;//���ͳ���Ϊ0��Ĭ�Ϲر�
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//�����ַ������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݿ��1�ֽ�
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//�ڴ����ݿ��1�ֽ�
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//��ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//�������ڴ�ͨѶ�������ڴ浽�ڴ�
	DMA_Init(DMA1_Channel4,	&DMA_InitStructure);
	//DMA_Cmd (DMA1_Channel4,ENABLE);//Ĭ�Ϲر�
	USART_DMACmd(USART1, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);//ʹ�ܴ���DMA���ͺͽ���
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
		RS485_RX_EN();//485�������������ʹ�ܽŵ�����ģʽ��
	}
}

void METER_DMA_Reset(DMA_Channel_TypeDef* DMAy_Channelx, unsigned char len)
{
	RS485_TX_EN();//485����
	DMA_Cmd (DMAy_Channelx,DISABLE);//Ҫ�ȹر�DMA�������ó���
	DMAy_Channelx->CNDTR = len;			//�������ÿ�ʼ��ַ
	DMA_Cmd (DMAy_Channelx,ENABLE); //����DMA����
}

unsigned char ReadAddrCmd[12]={0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0X68,0X13,0X00,0XDF,0X16};//������0x13-��ѯ����ַ
static void Check_ElecMeter_Addr(void)
{
	memcpy(METER_Tx_Buffer,ReadAddrCmd,12);
	METER_DMA_Reset(METER_TX_DMA,12);
}

unsigned char ReadDataCmd[10] = {0x68,0x00,0x00,0x00,0x00,0x00,0x00,0x68,0x11,0x04};//������0x11-������������
static void Send_read_cmd(void)
{
	static char Cmd_Step;
	unsigned short Sum;
	memcpy(METER_Tx_Buffer,ReadDataCmd,10);
	METER_Tx_Buffer[10] = single_cmd[Cmd_Step].u8DI0 + 0x33;//ǰ����Ѿ���Ĭ�ϸ�ֵ����Ҫ���иĶ�
	METER_Tx_Buffer[11] = single_cmd[Cmd_Step].u8DI1 + 0x33;
	METER_Tx_Buffer[12] = single_cmd[Cmd_Step].u8DI2 + 0x33;
	METER_Tx_Buffer[13] = single_cmd[Cmd_Step].u8DI3 + 0x33;
	Cmd_Step = (Cmd_Step+1)%3;//�����ܹ�->��ѹ->����
	for(char i=0; i<14; i++)	Sum += METER_Tx_Buffer[i];
	METER_Tx_Buffer[14] = Sum&0x00FF;//У��λ
	METER_Tx_Buffer[15] = 0x16;//��β
	METER_DMA_Reset(METER_TX_DMA,16);
}


static enum _Meter_Type DataCheck(void)//��ͷ��β ���ݳ���ȫ������
{
	if((METER_Rx_Buffer[1]==0xFE)&&(METER_Rx_Buffer[3]==0xFE)&&(METER_Rx_Buffer[4]==0x68)&&(METER_Rx_Buffer[11]==0x68))
	{
		if(((METER_Rx_Buffer[13]+16)==RS485_DataLong) && (METER_Rx_Buffer[RS485_DataLong-1]==0X16))	return Single_Meter;
	}
	else if((METER_Rx_Buffer[0]==0xFE) && (METER_Rx_Buffer[1]==0x68) && (METER_Rx_Buffer[8]==0x68))//40KW 3�ཻ����
	{
		if(((METER_Rx_Buffer[10]+13)==RS485_DataLong) && (METER_Rx_Buffer[RS485_DataLong-1]==0X16)) return Three_Meter;	
	}
	return No_Meter;
}

unsigned char hex_to_ten(unsigned char number)//BCD��(ʮ������)ת��Ϊʮ����������0x49-->49
{
	return (((number&0xf0)>>1) + ((number&0xf0)>>3) + (number&0x0f));
}

void Send_485_Data(void)
{
	static unsigned char t,BaudRate_t;

	if(MeterSta == No_Link)//����Ƕ���״̬
	{
		if(BaudRate_t == 15)	METER_UART_Init(9600);/*������9600�����ʸ����ͨѶ*/
		if(BaudRate_t == 31)	METER_UART_Init(2400);/*������2400�����ʸ����ͨѶ*/
		BaudRate_t = (BaudRate_t+1)%32;//0-31 No_Link��8s�л�һ�β�����
		Check_ElecMeter_Addr();//���źŲ�ѯ����ַ
	}
	else if(MeterSta == ReadData)//���������
	{
		t = 0;	BaudRate_t = 0;/*���ϵ���л������ʼ�ʱ����*/
		Send_read_cmd();//���Ͷ�ȡ���ݵ�����//����/��ѹ/����
		MeterSta = WaitData;
	}
	else if(MeterSta == WaitData)//���ȴ��������״̬
	{
		if(++t >= 10)//��ʱ5s
		{
			t = 0;	MeterSta = No_Link;//������ߣ�������������
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
	if(MeterSta == WaitData)//�ȴ�����
	{
		if(DataCheck() == Single_Meter)
		{
			for(char i=4;i<RS485_DataLong-2;i++)	Sum += METER_Rx_Buffer[i];		
			if((uint8_t)Sum == METER_Rx_Buffer[RS485_DataLong-2])//У������ȷ
			{				
				for(char i=0;i<(METER_Rx_Buffer[13]-4);i++)	lData = lData + hex_to_ten(METER_Rx_Buffer[18+i]-0x33)*array_x_min[i];	
				//lData = lData * single_cmd[Cmd_Step].ratio;
				if((METER_Rx_Buffer[15]==0x33)&&(METER_Rx_Buffer[16]==0x34)&&(METER_Rx_Buffer[17]==0x33))	MeterData.kwh_realtime = lData*single_cmd[0].ratio;//��������
					else if((METER_Rx_Buffer[15]==0x34)&&(METER_Rx_Buffer[16]==0x34)&&(METER_Rx_Buffer[17]==0x35))	 MeterData.vol = lData*single_cmd[1].ratio;//��ѹ
						else if((METER_Rx_Buffer[15]==0x34)&&(METER_Rx_Buffer[16]==0x35)&&(METER_Rx_Buffer[17]==0x35)) MeterData.cur = lData*single_cmd[2].ratio;//����	
				MeterSta = ReadData;
			}
		}			
		else if(DataCheck() == Three_Meter)
		{
			for(char i=1;i<RS485_DataLong-2;i++)	Sum += METER_Rx_Buffer[i];		
			if((uint8_t)Sum == METER_Rx_Buffer[RS485_DataLong-2])//У������ȷ
			{				
				for(char i=0;i<(METER_Rx_Buffer[10]-4);i++)	lData = lData + hex_to_ten(METER_Rx_Buffer[15+i]-0x33)*array_x_min[i];
				if((METER_Rx_Buffer[12]==0x33)&&(METER_Rx_Buffer[13]==0x34)&&(METER_Rx_Buffer[14]==0x33))	MeterData.kwh_realtime = lData*single_cmd[0].ratio;//��������
					else if((METER_Rx_Buffer[12]==0x34)&&(METER_Rx_Buffer[13]==0x34)&&(METER_Rx_Buffer[14]==0x35))	 MeterData.vol = lData*single_cmd[1].ratio*1.732f;//��ѹ�����ࣩ
						else if((METER_Rx_Buffer[12]==0x34)&&(METER_Rx_Buffer[13]==0x35)&&(METER_Rx_Buffer[14]==0x35)) MeterData.cur = lData*single_cmd[2].ratio;//����	
				MeterSta = ReadData;
			}
		}	
	}
}


void Read_ElectricMeter_Data(void)//500msһ��
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
	
	if(Board_C_Sta != 0)//ֻ��������C��Ž������Ϊ��Ҫ�豸
	{
		if(MeterSta == No_Link)	Type_DM.DErr |= Dc_Table_Err;//����Ƕ���״̬
			else Type_DM.DErr &= ~Dc_Table_Err;//���ϵ��
	}
}

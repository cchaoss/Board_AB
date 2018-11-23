#include "lcd.h"
#include "bms.h"
#include "main.h"
#include "acdc_module.h"

#define LCD_RX_DMA DMA2_Channel3
#define LCD_TX_DMA DMA2_Channel5

#define LCD_RX_SIZE	8//����
unsigned char LCD_Rx_Buffer[LCD_RX_SIZE];//�������ݻ���

#define LCD_TX_SIZE	105//����
unsigned char LCD_Tx_Buffer[LCD_TX_SIZE];//����
Uart_Rx_FlagBits Uart_Flag;//���ڽ��ܱ��

void LCD_UART_Init(uint32_t bound)
{
	GPIO_PinConfigure(GPIOC,10,GPIO_AF_PUSHPULL,GPIO_MODE_OUT50MHZ);//TX-�����������
	GPIO_PinConfigure(GPIOC,11,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//RX-��������
	//��ʼ�����ڽṹ��
	USART_InitTypeDef USART_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//��ʼ������ʱ��
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
	//���ô����ж����ȼ�
	NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);//�򿪿����ж�,���ڽ��ղ���������ʱ�жϽ�����һ֡����
	USART_Cmd(UART4, ENABLE);
	USART_ClearFlag(UART4, USART_FLAG_TC);
	/*����UART4 ��DMA TX RX*/
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);//����DMA2ʱ��
	//DMA2 Channel 3 triggerd by USART4 Rx event
	DMA_DeInit(DMA2_Channel3);//��λDMA��������ͨ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)LCD_Rx_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//������Ϊ������Դ
	DMA_InitStructure.DMA_BufferSize = LCD_RX_SIZE;//���泤��
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//�����ַ������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݿ��1�ֽ�
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//�ڴ����ݿ��1�ֽ�
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//��ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//�������ڴ�ͨѶ�������ڴ浽�ڴ�
	DMA_Init(DMA2_Channel3,&DMA_InitStructure);
	DMA_Cmd (DMA2_Channel3,ENABLE);//����DMA����
	//DMA2 Channel 5 triggerd by USART4 Tx event
	DMA_DeInit(DMA2_Channel5);//��λDMA��������ͨ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)LCD_Tx_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//������Ϊ��������Ŀ�ĵ�
	DMA_InitStructure.DMA_BufferSize = 0;//���ͳ���Ϊ0��Ĭ�Ϲر�
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//�����ַ������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݿ��1�ֽ�
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//�ڴ����ݿ��1�ֽ�
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//��ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//�������ڴ�ͨѶ�������ڴ浽�ڴ�
	DMA_Init(DMA2_Channel5,	&DMA_InitStructure);
	//DMA_Cmd (DMA2_Channel5,ENABLE);//Ĭ�Ϲر�
	USART_DMACmd(UART4, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);//ʹ�ܴ���DMA���ͺͽ���
}

//����2�жϷ�����,���ղ���������---LCD
void UART4_IRQHandler(void)
{
	uint8_t clear = clear;
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)
	{
		clear = UART4->SR;
		clear = UART4->DR;//�ȶ�SR�ٶ�DR�������
		Uart_Flag.Lcd_Rx_Flag = true;
	}
}

void LCD_DMA_Reset(DMA_Channel_TypeDef* DMAy_Channelx, unsigned char len)
{
	DMA_Cmd (DMAy_Channelx,DISABLE);//Ҫ�ȹر�DMA�������ó���
	DMAy_Channelx->CNDTR = len;			//�������ÿ�ʼ��ַ
	DMA_Cmd (DMAy_Channelx,ENABLE); //����DMA����
}
/*************************************************************************/
unsigned char End_of_package[4] = {0xcc,0x33,0xc3,0x3c};//�̶���β
unsigned char Head_number[7] = {0xAA,0x14,0x03,0xff,0xff,0x00,0x00};//����ʾ����ɫ �޷����� ��Ч����ʾ ��С8*16 
unsigned char Head_string[7] = {0xAA,0x11,0x02,0xff,0xff,0x00,0x00};//����ʾ����ɫ 8*16 �ַ���ɫ ������ɫ
unsigned char Page[8] = {0xAA,0x22,0x00,0x00,0xCC,0x33,0xC3,0x3C};//�л�ҳ��ָ��
unsigned char Hand[6] = {0xAA,0x00,0xCC,0x33,0xC3,0x3C};//����ָ��
void show_number(unsigned int number,unsigned short x,unsigned short y,unsigned char num1,unsigned char num2)
{
	memcpy(LCD_Tx_Buffer,Head_number,sizeof(Head_number));
	LCD_Tx_Buffer[7] = num1;//����λ��
	LCD_Tx_Buffer[8] = num2;//С��λ��
	LCD_Tx_Buffer[9] = (x&0xff00)>>8;
  LCD_Tx_Buffer[10]= x&0x00ff;
	LCD_Tx_Buffer[11]= (y&0xff00)>>8;
  LCD_Tx_Buffer[12]= y&0x00ff;		
	LCD_Tx_Buffer[13]= (number>>24);
	LCD_Tx_Buffer[14]= (number>>16)&0x00ff;			
	LCD_Tx_Buffer[15]= (number&0xff00)>>8;
	LCD_Tx_Buffer[16]= (number&0x00ff);		
	memcpy(&LCD_Tx_Buffer[17],End_of_package,sizeof(End_of_package));

	LCD_DMA_Reset(LCD_TX_DMA,21);
	while(DMA_GetCurrDataCounter(LCD_TX_DMA));//����
}
//��ʾ�ַ������ֵ�xy�����Ƿ���
void Show_hanzi(unsigned char *hanzi,unsigned char size,unsigned short y,unsigned short x)
{
	memcpy(LCD_Tx_Buffer,Head_string,sizeof(Head_string));
	LCD_Tx_Buffer[7] = (x&0xff00)>>8;
  LCD_Tx_Buffer[8] = x&0x00ff;
	LCD_Tx_Buffer[9] = (y&0xff00)>>8;
  LCD_Tx_Buffer[10]= y&0x00ff;																									
	for(char i=0;i<(size);i++)	LCD_Tx_Buffer[11+i] = hanzi[i];//���ִ洢����
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
	LCD_DMA_Reset(LCD_TX_DMA,6);//��λDMA�����׵�ַ
}
												/* P0			 P0_CC		  P1_V			P1_A		  P1_SOC	P1_KW 		 P1_TIME		P2_EXP*/
Display_Position SHOW = {{150,70},{122,275},{100,188},{100,217},{86,139},{100,244},{122,275},{256,58}};
			
unsigned char Link_ok = 0;//LCD������״̬1���� 0δ����
unsigned char LCD_DataLong;
unsigned char qqq = 1;
void LcdShow(void)
{
	if(Uart_Flag.Lcd_Rx_Flag)
	{
		LCD_DataLong = LCD_RX_SIZE - DMA_GetCurrDataCounter(LCD_RX_DMA);
		if((LCD_Rx_Buffer[0]==0xAA)&&(LCD_Rx_Buffer[1]==0x00)&&(LCD_Rx_Buffer[2]==0x4F)&&(LCD_Rx_Buffer[3]==0x4B))
			Link_ok = 1;//LCD�豸����
		memset(LCD_Rx_Buffer,0,LCD_DataLong);
		LCD_DMA_Reset(LCD_RX_DMA,LCD_RX_SIZE);//��λDMA�����׵�ַ
		Uart_Flag.Lcd_Rx_Flag = false;
	}

	if(Link_ok)
	{
		if((Type_DM.DErr==Geodesic)||(Type_DM.DErr==No_Module)||(Type_DM.DErr==Relay_Err))//׮�Լ����(��������Լ���׮��ʾ3������)
		{
			switch_page(3);
			switch(Type_DM.DErr)//notes:Disconnect_C Dc_Table_Err	(����׮����ʾ) 
			{
				case Geodesic:		Show_hanzi(" �ӵع���",9,SHOW.P0[0],SHOW.P0[1]);break;
				case No_Module:		Show_hanzi("�޵�Դģ��",10,SHOW.P0[0],SHOW.P0[1]);break;
				case Relay_Err:		Show_hanzi("�̵�������",10,SHOW.P0[0],SHOW.P0[1]);break;
				default:break;
			}
		}
		else//׮�Լ�ͨ��
		{
			switch(Type_BMS.Step)
			{
				case BEGIN:			switch_page(0);Show_hanzi("�����ǹ",10,SHOW.P0[0],SHOW.P0[1]);show_number(Type_VolCur.CC,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;
				case SEND_9728:	switch_page(0);Show_hanzi("���������",10,SHOW.P0[0],SHOW.P0[1]);show_number(Type_VolCur.CC,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;
				case SEND_256:	switch_page(0);Show_hanzi("����ʶ��",10,SHOW.P0[0],SHOW.P0[1]);show_number(Type_VolCur.CC,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;
				case SEND_2048:	switch_page(0);Show_hanzi(" BMS׼��"  ,9 ,SHOW.P0[0],SHOW.P0[1]);show_number(Type_VolCur.CC,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;
				case SEND_2560:	switch_page(0);Show_hanzi("����׼��",10,SHOW.P0[0],SHOW.P0[1]);show_number(Type_VolCur.CC,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;	
				case SEND_4608:	
				case SEND_6656:				
					switch_page(1);
					show_number(Type_VolCur.Soc,SHOW.P1_Soc[0],SHOW.P1_Soc[1],3,0);//SOC
					show_number(Type_VolCur.Vol,SHOW.P1_V[0],SHOW.P1_V[1],4,1);//��ѹ
					show_number(Type_VolCur.Cur,SHOW.P1_A[0],SHOW.P1_A[1],4,1);//���� 
					show_number(Type_VolCur.KWh,SHOW.P1_KW[0],SHOW.P1_KW[1],4,1);//����(���ӵ�����ʾ)
					show_number(Data_4352.RemaChargTime++,SHOW.P1_Time[0],SHOW.P1_Time[1],4,0);//ʣ��ʱ��
					break;
				case STOP:
					switch_page(2);
					switch(Type_BMS.Stop_Reason)
					{
						case Time_Out:
							Show_hanzi(" ͨѶ��ʱ",9,SHOW.P0[0],SHOW.P0[1]);
							switch(Type_BMS.time_out)
							{
								case BRM512_Timeout:	Show_hanzi("   512" ,6,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCP1536_Timeout:	Show_hanzi("   1536",7,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BRO2304_Timeout:	Show_hanzi("   2304",7,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCS4352_Timeout:	Show_hanzi("   4352",7,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCL4096_Timeout:	Show_hanzi("   4096",7,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BST6400_Timeout:	Show_hanzi("   6400",7,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
						case Err_Stop:
							Show_hanzi(" ����ֹͣ",9,SHOW.P0[0],SHOW.P0[1]);
							switch(Type_BMS.DErr)
							{
								case Lock_ERR:			Show_hanzi("  �޷�����",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Insulation_ERR:Show_hanzi("  ��Ե����",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Bat_Vol_ERR:		Show_hanzi("��ص�ѹ�쳣",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
						case BMS_Stop:
							Show_hanzi(" BMS��ֹ",9,SHOW.P0[0],SHOW.P0[1]);
							switch(Type_BMS.BErr)
							{
								case Soc_Full:			Show_hanzi("  �ѳ�����",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Insulation:		Show_hanzi("  ��Ե����",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BmsOutNetTemp:	Show_hanzi("�������������",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-8);break;
								case ChargeNet:			Show_hanzi("�������������",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-8);break;
								case BatTemp:				Show_hanzi("����¶ȹ���",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case HighRelay:			Show_hanzi("��ѹ�̵�������",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-8);break;
								case Vol_2:					Show_hanzi(" ��2��ѹ����",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case CurOver:				Show_hanzi("  ��������",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case CurUnknown:		Show_hanzi("  ����������",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case VolErr:				Show_hanzi("  ��ѹ�쳣",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case VolUnknown:		Show_hanzi("  ��ѹ������",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
						case Mannul_Stop:
							Show_hanzi(" �˹���ֹ",9,SHOW.P0[0],SHOW.P0[1]);
							switch(Type_BMS.Manual)
							{
								case JT_Stop:			Show_hanzi("  ��ͣ����",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Card_Stop:		Show_hanzi("  ˢ��ֹͣ",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case App_Stop:		Show_hanzi("   Appֹͣ",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Start_Stop:	Show_hanzi("  ��ͣ����",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
					}break;
				default:break;
			}
		}
	}
	else	Check_Hand();
}


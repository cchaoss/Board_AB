#include "lcd.h"
#include <string.h>
#include "bms.h"
#include "main.h"
#include "acdc_module.h"

#define LCD_Buff_Size	15//�������ݻ����С
uint8_t LCD_CommandBuff[LCD_Buff_Size];//�������ݻ���
Uart_Rx_FlagBits Uart_Flag;//���ڽ��ܱ��

void USART2_DMA_receive(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(USART2_RX_DMA_CLK, ENABLE);//����DMAʱ��---�������������ʱ��,������Ϊ��θú�����ѭ������,���Բ������￪��ʱ�ӣ����ǳ�ʼ��

	DMA_DeInit(USART2_RX_DMA_CHANNEL);// ��λDMA��������ͨ��
  USART_DMACmd(LCD_USARTx, USART_DMAReq_Rx, ENABLE);

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

  /* USART �� DMA����TX���� */
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

void LCD_USART2_Config(uint32_t bound)
{
	GPIO_PinConfigure(LCD_USART_TX_PORT,LCD_USART_TX_PIN,GPIO_AF_PUSHPULL,GPIO_MODE_OUT10MHZ);//TX-�����������
	GPIO_PinConfigure(LCD_USART_RX_PORT,LCD_USART_RX_PIN,GPIO_IN_PULL_UP,GPIO_MODE_OUT10MHZ);//RX-��������
	
	USART_InitTypeDef USART_InitStructure;
	LCD_USART_APBxClkCmd(LCD_USART_CLK, ENABLE);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(LCD_USARTx, &USART_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = LCD_USART_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(LCD_USARTx, USART_IT_IDLE, ENABLE);//�򿪿����ж�,���ڽ��ղ���������ʱ�жϽ�����һ֡����
	USART_ITConfig(LCD_USARTx, USART_IT_RXNE, ENABLE);//ʹ�ܴ��ڽ����ж�
	USART_Cmd(LCD_USARTx, ENABLE);

	USART_ClearFlag(LCD_USARTx, USART_FLAG_TC);
	
	USART2_DMA_receive();//
}

//����2�жϷ�����,���ղ���������---LCD
void LCD_USART_IRQHandler(void)
{
	uint8_t clear;
	if(USART_GetITStatus(LCD_USARTx, USART_IT_IDLE) != RESET)
	{
		clear = LCD_USARTx->SR;
		clear = LCD_USARTx->DR;//�ȶ�SR�ٶ�DR�������
		clear = clear;//��������
		Uart_Flag.Lcd_Rx_Flag = true;
	}
}
/*************************************************************************/

void show_number(unsigned int number,unsigned short x,unsigned short y)
{
	u8 Show_buff[21] = {0xAA,0x14,0x01,0xff,0xff,0x00,0x00,
											0x08,0x02,//����λ����С��λ��
											0x00,0x00,0x00,0x00,//����X,Y
											0x00,0x00,0x00,0x00,//��ֵ			
											0xcc,0x33,0xc3,0x3c};
	Show_buff[9] = 	(x&0xff00)>>8;
  Show_buff[10]=  x&0x00ff;
	Show_buff[11]=	(y&0xff00)>>8;
  Show_buff[12]=	y&0x00ff;		
	Show_buff[13]=  (number>>24);
	Show_buff[14]= 	(number>>16)&0x00ff;			
	Show_buff[15]= 	(number&0xff00)>>8;
	Show_buff[16]= 	(number&0x00ff);																				
	USART2_DMA_send(Show_buff,21);
}

void Show_hanzi(unsigned char *hanzi,unsigned char size,unsigned short x,unsigned short y)
{
	u8 Show_buff[45] = {0xAA,0x11,0x01,0xff,0xff,0x00,0x00,
												0x00,0x00,0x00,0x00,
												0x00,0x00,0x00,0x00,};
	Show_buff[7]= (x&0xff00)>>8;
  Show_buff[8]= x&0x00ff;
	Show_buff[9]=	(y&0xff00)>>8;
  Show_buff[10]=y&0x00ff;																									
	for(char i=0;i<(size);i++)	Show_buff[11+i] = hanzi[i];//���ִ洢����
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
unsigned char Check_Hand[6] = {0xAA,0x00,0xCC,0x33,0xC3,0x3C};//����ָ��
unsigned char Link_ok = 0;//LCD������״̬1���� 0δ����

void LcdShow(void)
{
	if(Uart_Flag.Lcd_Rx_Flag)
	{
		unsigned char LCD_DataLong = LCD_Buff_Size - DMA_GetCurrDataCounter(USART2_RX_DMA_CHANNEL);
		USART2_DMA_receive();
		if(LCD_CommandBuff[0]==0xAA)	Link_ok = 1;//LCD�豸����
		memset(LCD_CommandBuff,0,LCD_DataLong);
		Uart_Flag.Lcd_Rx_Flag = false;
	}

	if(Link_ok)
	{
		if((Type_DM.DErr==Geodesic)||(Type_DM.DErr==No_Module)||(Type_DM.DErr==Relay_Err))//׮�Լ����(��������Լ���׮��ʾ3������)
		{
			switch_page(3);
			switch(Type_DM.DErr)//notes:Disconnect_C Dc_Table_Err	(����׮����ʾ) 
			{
				case Geodesic:		Show_hanzi("�ӵع���",8,SHOW.P3_Err[0],SHOW.P3_Err[1]);break;
				case No_Module:		Show_hanzi("�޵�Դģ��",10,SHOW.P3_Err[0],SHOW.P3_Err[1]);break;
				case Relay_Err:		Show_hanzi("�̵�������",10,SHOW.P3_Err[0],SHOW.P3_Err[1]);break;
				default:break;
			}
		}
		else//׮�Լ�ͨ��
		{
			switch(Type_BMS.Step)
			{
				case BEGIN:			switch_page(0);	Show_hanzi("�����",6,SHOW.P0[0],SHOW.P0[1]);break;
				case SEND_9728:	switch_page(0);	Show_hanzi("���������",10,SHOW.P0[0],SHOW.P0[1]);break;
				case SEND_256:	switch_page(0);	Show_hanzi("����ʶ��",10,SHOW.P0[0],SHOW.P0[1]);break;
				case SEND_2048:	switch_page(0);	Show_hanzi("BMS׼����",9,SHOW.P0[0],SHOW.P0[1]);break;
				case SEND_2560:	switch_page(0);	Show_hanzi("����׼����",12,SHOW.P0[0],SHOW.P0[1]);break;	
				case SEND_4608:	
				case SEND_6656:					
					switch_page(1);
					show_number(Module_Status.Output_Vol,SHOW.P1_V[0],SHOW.P1_V[0]);//��ѹ
					show_number(Module_Status.Output_Cur,SHOW.P1_A[0],SHOW.P1_A[0]);//���� 
					show_number(Data_4352.PreSOC,SHOW.P1_Soc[0],SHOW.P1_Soc[0]);//SOC
					//show_number(Data_4352.PreSOC,SHOW.P1_KW[0],SHOW.P1_KW[0]);//����(����׮����ʾ)
					show_number(Data_4352.RemaChargTime,SHOW.P1_KW[0],SHOW.P1_KW[0]);//ʣ��ʱ��
					break;
				case STOP:
					switch_page(2);
					switch(Type_BMS.Stop_Reason)
					{
						case Time_Out:
							Show_hanzi("ͨѶ��ʱֹͣ",12,SHOW.P2_STOP[0],SHOW.P2_STOP[1]);
							switch(Type_BMS.time_out)
							{
								case BRM512_Timeout:	Show_hanzi("512" ,3,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCP1536_Timeout:	Show_hanzi("1536",4,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BRO2304_Timeout:	Show_hanzi("2304",4,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCS4352_Timeout:	Show_hanzi("4352",4,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCL4096_Timeout:	Show_hanzi("4096",4,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BST6400_Timeout:	Show_hanzi("6400",4,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}
							break;
						case Err_Stop:
							Show_hanzi("����ֹͣ",8,SHOW.P2_STOP[0],SHOW.P2_STOP[1]);
							switch(Type_BMS.DErr)
							{
								case Lock_ERR:			Show_hanzi("�޷�����",8,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case Insulation_ERR:Show_hanzi("��Ե������",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case Bat_Vol_ERR:		Show_hanzi("��ص�ѹ�뱨��ƫ�����",22,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								default:break;
							}
							break;
						case BMS_Stop:
							Show_hanzi("BMS��ֹ",8,SHOW.P2_STOP[0],SHOW.P2_STOP[1]);
							switch(Type_BMS.BErr)
							{
								case Soc_Full:			Show_hanzi("����ֹͣ",8,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);//BMSֹͣԭ�򣺳���
								case Insulation:		Show_hanzi("��Ե����",8,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case BmsOutNetTemp:	Show_hanzi("�������������",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case ChargeNet:			Show_hanzi("�������������",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case BatTemp:				Show_hanzi("����¶ȹ���",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case HighRelay:			Show_hanzi("��ѹ�̵�������",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case Vol_2:					Show_hanzi("����2��ѹ����",15,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case CurOver:				Show_hanzi("��������",8,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case CurUnknown:		Show_hanzi("����������",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case VolErr:				Show_hanzi("��ѹ�쳣",8,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								case VolUnknown:		Show_hanzi("��ѹ������",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);
								default:break;
							}
							break;
						case Mannul_Stop:
							Show_hanzi("�˹���ֹ",8,SHOW.P2_STOP[0],SHOW.P2_STOP[1]);
							switch(Type_BMS.Manual)
							{
								case JT_Stop:			Show_hanzi("����ֹͣ��ť",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Card_Stop:		Show_hanzi("ˢ��ֹͣ",8,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case App_Stop:		Show_hanzi("App��ֹ̨ͣ",11,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Start_Stop:	Show_hanzi("��ͣ���ذ���",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}
							break;
					}
					break;
				default:break;
			}
		}
	}else	USART2_DMA_send(Check_Hand,6);
}


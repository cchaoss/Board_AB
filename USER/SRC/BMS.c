#include "main.h"
#include "bms.h"
#include "can.h"

#pragma pack(1)   //ǿ��1�ֽڶ���
/*���׮����BMS������*/
stuPGN9728Type Data_9728 = {0x01,0x01,0x00};
stuPGN256Type	 Data_256  = {0x00,0x0100000,};
stuPGN1792Type Data_1792;
stuPGN2048Type Data_2048;
stuPGN2560Type Data_2560;
stuPGN4608Type Data_4608;
stuPGN6656Type Data_6656;
stuPGN7424Type Data_7424;
stuPGN7936Type Data_7936;
/*BMS������������*/
stuPGN9984Type Data_9984;
stuPGN512Type	 Data_512;
stuPGN1536Type Data_1536;
stuPGN2304Type Data_2304;
stuPGN4096Type Data_4096;
stuPGN4352Type Data_4352;
stuPGN4864Type Data_4864;
stuPGN6400Type Data_6400;
stuPGN7168Type Data_7168;
stuPGN7680Type Data_7680;
#pragma pack()

									/*����PGN			���ȼ�	Ŀ�ĵ�ַ	Դ��ַ	���ݳ���		������*/		
TX_BMS CHM_9728 = {(9728>>8),			6,		 0xf4,		 0x56,		3,			&Data_9728};//�������ֱ���
TX_BMS CRM_256 = 	{(256 >>8),			6,		 0xf4,		 0x56,		8,			&Data_256 };//������ʶ����
//TX_BMS CTS_1792 = {(1792>>8),			6,		 0xf4,		 0x56,		7,			&Data_1792};//��������ʱ��ͬ��(��ѡ)
TX_BMS CML_2048 = {(2048>>8),			6,		 0xf4,		 0x56,		8,			&Data_2048};//��������������
TX_BMS CRO_2560 = {(2560>>8),			4,		 0xf4,		 0x56,		1,			&Data_2560};//�������׼������
TX_BMS CCS_4608 = {(4608>>8),			6,		 0xf4,		 0x56,		8,			&Data_4608};//�������״̬
TX_BMS CST_6656 = {(6656>>8),			4,		 0xf4,		 0x56,		4,			&Data_6656};//������ֹ���
TX_BMS CSD_7424 = {(7424>>8),			6,		 0xf4,		 0x56,		8,			&Data_7424};//����ͳ������
TX_BMS CEM_7936 = {(7936>>8),			2,		 0xf4,		 0x56,		4,			&Data_7936};//����������

#define RX_BMS_NUM 10							/*���ܱ�־		����PGN					������*/
RX_BMS RX_BMS_TAB[RX_BMS_NUM] =	{	{		0,			(9984>>8),			&Data_9984},	//BMS���ֱ���
																	{		0,			(512>>8),				&Data_512	},	//BMS������ʶ����
																	{		0,			(1536>>8),			&Data_1536},	//�������س�����
																	{		0,			(2304>>8),			&Data_2304},	//���׼����������
																	{		0,			(4096>>8),			&Data_4096},	//��س��������
																	{		0,			(4352>>8),			&Data_4352},	//��س����״̬����
																	{		0,			(4864>>8),			&Data_4864},	//��������״̬��Ϣ
																	{		0,			(6400>>8),			&Data_6400},	//BMS��ֹ��籨��
																	{		0,			(7168>>8),			&Data_7168},	//BMSͳ�����ݱ���
																	{		0,			(7680>>8),			&Data_7680}};//BMS������

CanTxMsg TxMsg1 =  {0, 0, CAN_Id_Extended, CAN_RTR_Data, 0, {0}};//��չ֡ ����֡					
unsigned char BMS_STA;
void BMS_Task(void const *argument)
{
	const unsigned char BMS_Task_Time = 10U;//10msѭ��
	static unsigned short t;
	
	while(1)
	{
		
		if(BMS_Recevie_Flag == 1)
		{
//			RX_BMS.IFArbition  		= (uint8_t)((RxMsg1.ExtId&0xfc000000)>>26);
//			RX_BMS.PGN_S   		 		= (uint8_t)((RxMsg1.ExtId&0x00ff0000)>>16);
//			RX_BMS.PDUSpecific 		= (uint8_t)((RxMsg1.ExtId&0x0000ff00)>>8);	
//			RX_BMS.SourceAddress  = (uint8_t)(RxMsg1.ExtId&0x000000ff);	
			for(char i = 0; i < RX_BMS_NUM; i++)
			{
				unsigned char PGN_t = (RxMsg1.ExtId&0x00ff0000)>>16;//��ȡ����PGN��
				if(PGN_t == RX_BMS_TAB[i].PGN_S)
				{
					memcpy(RX_BMS_TAB[i].Data,RxMsg1.Data,RxMsg1.DLC);
					RX_BMS_TAB[i].Rx_status = 1;
				}
			}				
			BMS_Recevie_Flag = 0;
		}
	
		switch(BMS_STA)
		{
			case WAIT_9984_BHM:
			{
						t = (t+1)%(250/BMS_Task_Time);//����250ms
						if(t == 1)	BMS_Send(CRM_256);
						
						if(RX_BMS_TAB[WAIT_9984_BHM].Rx_status == 1)	BMS_STA = WAIT_512_BRM;
						break;
			}
			case WAIT_512_BRM:
			{
						break;
			}
			default:break;
		
		}


		
		
		osDelay(BMS_Task_Time);
	}
}



//����ֵ��0�ɹ���1ʧ��
unsigned char BMS_Send(TX_BMS Pbuf)
{
	TxMsg1.ExtId = (Pbuf.IFArbition<<26) | (Pbuf.PGN_S<<16) | (Pbuf.PDUSpecific<<8) | (Pbuf.SourceAddress);
	TxMsg1.DLC = Pbuf.DLC;
	memcpy(TxMsg1.Data,Pbuf.Data,Pbuf.DLC);
	return Can_Send_Msg(CAN1, &TxMsg1);;
}


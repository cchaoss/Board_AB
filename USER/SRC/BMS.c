#include "main.h"
#include "bms.h"
#include "can.h"

#pragma pack(1)   //ǿ��1�ֽڶ���
/*���׮����BMS������*/
stuPGN9728Type Data_9728 = {0x01,0x01,0x00};
stuPGN256Type	 Data_256  = {0x00,0x0100000,};
stuPGN1792Type Data_1792;
stuPGN2048Type Data_2048 = {7500,	2000,	4000-2000};//���������ѹ��200-750V ���������0-200A
stuPGN2560Type Data_2560;
stuPGN4608Type Data_4608 = {0,0,0,0x01};//Ĭ��������2015GB
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

									/*����PGN		���ȼ�	Ŀ�ĵ�ַ	Դ��ַ	���ݳ���		��������ms		������*/		
TX_BMS CHM_9728 = {(9728>>8),		6,		 0xf4,		 0x56,		 3,					250,			&Data_9728};//�������ֱ���
TX_BMS CRM_256 = 	{(256 >>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_256 };//������ʶ����
TX_BMS CTS_1792 = {(1792>>8),		6,		 0xf4,		 0x56,	 	 7,					250,			&Data_1792};//��������ʱ��ͬ��(��ѡ)
TX_BMS CML_2048 = {(2048>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_2048};//��������������
TX_BMS CRO_2560 = {(2560>>8),		4,		 0xf4,		 0x56,		 1,					250,			&Data_2560};//�������׼������
TX_BMS CCS_4608 = {(4608>>8),		6,		 0xf4,		 0x56,		 8,					50,				&Data_4608};//�������״̬
TX_BMS CST_6656 = {(6656>>8),		4,		 0xf4,		 0x56,		 4,					10,				&Data_6656};//������ֹ���
TX_BMS CSD_7424 = {(7424>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_7424};//����ͳ������
TX_BMS CEM_7936 = {(7936>>8),		2,		 0xf4,		 0x56,		 4,					250,			&Data_7936};//����������

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
																	{		0,			(7680>>8),			&Data_7680}}; //BMS������
uint8_t buf[8];
unsigned char J1939_Multi_Package[8];/*0x10,len_L,len_H,����,0xff,PGN[3]*/
CanTxMsg TxMsg1 =  {0, 0, CAN_Id_Extended, CAN_RTR_Data, 0, {0}};//��չ֡ ����֡					
unsigned char BMS_STA = SEND_256;
void BMS_Task(void const *argument)
{
	const unsigned char BMS_Task_Time = 10U;//10msѭ��
	static unsigned short t,time_out;
	unsigned char *P = NULL;
	while(1)
	{
		//����BMS����
		if(BMS_Recevie_Flag == 1)
		{		
			BMS_Recevie_Flag = 0;
			if(RxMsg1.ExtId == 0X1CEC56F4)//BMS�����������������
			{	memcpy(buf,RxMsg1.Data,RxMsg1.DLC);
				if(RxMsg1.Data[0] == 0x10)
				{
					memcpy(J1939_Multi_Package,RxMsg1.Data,RxMsg1.DLC);//�������������ӵ���������					
					TxMsg1.ExtId = 0X1CECF456;			 //Ӧ������������
					TxMsg1.DLC = 8;																	
					TxMsg1.Data[0] = 0x11;									//Ӧ��ͷ	
					TxMsg1.Data[1] = J1939_Multi_Package[3];//�ɷ��Ͱ���
					TxMsg1.Data[2] = 1;											//����
					TxMsg1.Data[3] = TxMsg1.Data[4] = 0xff;//ȱʡֵ		
					memcpy(&TxMsg1.Data[5],&J1939_Multi_Package[5],3);//PGN*/							
					Can_Send_Msg(CAN1, &TxMsg1);
				}
			}
			else if(RxMsg1.ExtId == 0X1CEB56F4)//������ݴ���
			{				
				P = (unsigned char*)malloc(J1939_Multi_Package[1]);//�����ڴ����ڴ洢�������
				memcpy(P+((RxMsg1.Data[0]-1)*7),&RxMsg1.Data[1],7);//����������			
				if(RxMsg1.Data[0] == J1939_Multi_Package[3])//���������ɣ�
				{
					TxMsg1.ExtId = 0X1CECF456;//��Ӧ��ɶ������
					TxMsg1.DLC = 8;					
					TxMsg1.Data[0] = 0x13;//Ӧ��ͷ						
					memcpy(&TxMsg1.Data[1],&J1939_Multi_Package[1],7);					
					Can_Send_Msg(CAN1, &TxMsg1);//Ӧ�����������
					
					if(J1939_Multi_Package[6] == (uint8_t)(512>>8))		{memcpy(&Data_512,P,sizeof(Data_512));		RX_BMS_TAB[WAIT_512_BRM].Rx_status  = 1;}
					if(J1939_Multi_Package[6] == (uint8_t)(1536>>8))	{memcpy(&Data_1536,P,sizeof(Data_1536));	RX_BMS_TAB[WAIT_1536_BCP].Rx_status = 1;}
					if(J1939_Multi_Package[6] == (uint8_t)(4352>>8))	{memcpy(&Data_4352,P,sizeof(Data_4352));	RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 1;}
					free(P);//�ͷ��ڴ�		
				}
			}
			else
			{
				for(char i = 0; i < RX_BMS_NUM; i++)
				{
					unsigned char PGN_t = (RxMsg1.ExtId&0x00ff0000)>>16;//��ȡ����PGN��
					if(PGN_t == RX_BMS_TAB[i].PGN_S)
					{
						memcpy(RX_BMS_TAB[i].Data,RxMsg1.Data,RxMsg1.DLC);
						RX_BMS_TAB[i].Rx_status = 1;
					}
				}
			}
		}
	
		//BMS����
		switch(BMS_STA)
		{
			case SEND_9728://�������
			{
				//if(everything is ok)��ͷ cc �� ������Դ�պϣ�
				t = (t+1)%(CHM_9728.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CHM_9728);}
				
				if((RX_BMS_TAB[WAIT_9984_BHM].Rx_status == 1) || (time_out >= 20))//�յ�9984||��ʱ5s	GB2015
				{
					RX_BMS_TAB[WAIT_9984_BHM].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_256;
				}
				break;
			}
			
			case SEND_256://����ʶ
			{
				//if(��Ե й�� k1k2�Ӵ�����ѹ)
				if(RX_BMS_TAB[WAIT_512_BRM].Rx_status == 1)	
				{
					Data_256.IdentifyResult = 0xAA;//��ʶ�ɹ�
					time_out = 0;
				}
				else 
				{
					Data_256.IdentifyResult = 0x00;
					//if(time_out >= 20)//����512���ĳ�ʱ
				}
				t = (t+1)%(CRM_256.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CRM_256);}
				
				if(RX_BMS_TAB[WAIT_1536_BCP].Rx_status == 1)
				{
					RX_BMS_TAB[WAIT_1536_BCP].Rx_status = RX_BMS_TAB[WAIT_512_BRM].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_2048;				
				}//else if(time_out >= 20)//����1536���ĳ�ʱ
				break;
			}
			
			case SEND_2048://��������������/ʱ��ͬ��
			{
				t = (t+1)%(CML_2048.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CML_2048);	BMS_Send(CTS_1792);}
				
				if((RX_BMS_TAB[WAIT_2304_BRO].Rx_status==1) && (Data_2304.BMSSta==0xAA))
				{
					RX_BMS_TAB[WAIT_2304_BRO].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_2560;				
				}//else if(time_out >=20)//����2304BMS׼����������ʱ
				break;
			}
			
			case SEND_2560://�������׼������
			{
				if(1)Data_2560.PrepareOK = 0xAA;//�ж������ѹ�뱨�ĵ�ص�ѹ���5%���ڣ��ڳ��������С��ѹ֮�䡣����
					else Data_2560.PrepareOK = 0x00;
				t = (t+1)%(CRO_2560.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CRO_2560);}

				if((RX_BMS_TAB[WAIT_4096_BCL].Rx_status==1))
				{
					RX_BMS_TAB[WAIT_4096_BCL].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_4608;				
				}//else if(time_out >= 20)//����4096��س������+4352��س����״̬���ĳ�ʱ
				break;
			}
			
			case SEND_4608://�������״̬
			{
				t = (t+1)%(CCS_4608.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CCS_4608);
					Data_4608.OutputCurr = Data_4096.NeedCurr+10;//������
					Data_4608.OutputVolt = Data_4096.NeedVolt-10;//ʵ��ʹ����ACDCģ������
					Data_4608.ChargingTime++;}
				
				if(RX_BMS_TAB[WAIT_4352_BCS].Rx_status == 1)
				{
					time_out = 0;
					//RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 0;
				}//else if(time_out>=100)//4352��س����״̬���ĳ�ʱ->ֹͣ��磡
				

				if(RX_BMS_TAB[WAIT_4864_BSM].Rx_status == 1)
				{
					RX_BMS_TAB[WAIT_4864_BSM].Rx_status = 0;
					
					//if(1)//���״̬�Ƿ�����	����ֹ���?�����������Ƿ����
					//BMS_STA = SEND_2048;				
				}			
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
	memset(TxMsg1.Data,0,sizeof(TxMsg1.Data));
	memcpy(TxMsg1.Data,Pbuf.Data,Pbuf.DLC);
	return Can_Send_Msg(CAN1, &TxMsg1);
}


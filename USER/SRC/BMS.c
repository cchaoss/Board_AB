#include "main.h"
#include "bms.h"
#include "can.h"

#pragma pack(1)   //ǿ��1�ֽڶ���
/*���׮����BMS������*/
stuPGN9728Type Data_9728 = {0x01,0x01,0x00};
stuPGN256Type	 Data_256  = {0x00,0,};
stuPGN1792Type Data_1792;
stuPGN2048Type Data_2048 = {7500,	2000,	4000-2000};//���������ѹ��200-750V ���������0-200A
stuPGN2560Type Data_2560;
stuPGN4608Type Data_4608 = {0,0,0,0x01};//Ĭ��������2015GB
stuPGN6656Type Data_6656;
stuPGN7424Type Data_7424;
stuPGN7936Type Data_7936;
/*BMS������������*/
stuPGN9984Type Data_9984;
stuPGN2304Type Data_2304;
stuPGN4096Type Data_4096;
stuPGN512Type	 Data_512;
stuPGN1536Type Data_1536;
stuPGN4352Type Data_4352;
stuPGN4864Type Data_4864;
stuPGN6400Type Data_6400;
stuPGN7168Type Data_7168;
stuPGN7680Type Data_7680;
#pragma pack()

#define RX_BMS_NUM 10							/*���ܱ�־		����PGN					������*/
RX_BMS RX_BMS_TAB[RX_BMS_NUM] =	{	{		0,			(9984>>8),			&Data_9984},	//BMS���ֱ���
																	{		0,			(2304>>8),			&Data_2304},	//���׼����������
																	{		0,			(4096>>8),			&Data_4096},	//��س��������
																	{		0,			(4864>>8),			&Data_4864},	//��������״̬��Ϣ
																	{		0,			(6400>>8),			&Data_6400},	//BMS��ֹ��籨��
																	{		0,			(7168>>8),			&Data_7168},	//BMSͳ�����ݱ���
																	{		0,			(7680>>8),			&Data_7680}, //BMS������																	
																	{		0,			(512>>8),				&Data_512	},	//BMS������ʶ����
																	{		0,			(1536>>8),			&Data_1536},	//�������س�����
																	{		0,			(4352>>8),			&Data_4352}};	//��س����״̬����
									/*����PGN		���ȼ�	Ŀ�ĵ�ַ	Դ��ַ	���ݳ���		��������ms		������*/		
TX_BMS CHM_9728 = {(9728>>8),		6,		 0xf4,		 0x56,		 3,					250,			&Data_9728};//�������ֱ���
TX_BMS CRM_256 = 	{(256 >>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_256 };//������ʶ����
TX_BMS CTS_1792 = {(1792>>8),		6,		 0xf4,		 0x56,	 	 7,					250,			&Data_1792};//��������ʱ��ͬ��
TX_BMS CML_2048 = {(2048>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_2048};//��������������
TX_BMS CRO_2560 = {(2560>>8),		4,		 0xf4,		 0x56,		 1,					250,			&Data_2560};//�������׼������
TX_BMS CCS_4608 = {(4608>>8),		6,		 0xf4,		 0x56,		 8,					50,				&Data_4608};//�������״̬
TX_BMS CST_6656 = {(6656>>8),		4,		 0xf4,		 0x56,		 4,					10,				&Data_6656};//������ֹ���
TX_BMS CSD_7424 = {(7424>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_7424};//����ͳ������
TX_BMS CEM_7936 = {(7936>>8),		2,		 0xf4,		 0x56,		 4,					250,			&Data_7936};//����������

CanTxMsg TxMsg1 =  {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//��չ֡ ����֡
unsigned char OUT,	BMS_STA = SEND_256;//��������ֱ�ӱ�ʶ
void BMS_Task(void const *argument)
{
	const unsigned char BMS_Task_Time = 10U;//10msѭ��
	static unsigned short t,time_out;
	while(1)
	{	//������ʹ���������
		Single_Package_Deal();
		//BMS����
		switch(BMS_STA)
		{
			case BEGIN://����׼��
			{
				//CC�ź��Ƿ�3.2v-4.8v
				//��������������������
				//������Դ�̵����պ�K3K4
				//��ok�Ļ�->{��ر�־λ��0��BMS_STA = SEND_9728;}//���Ľ��ձ�־��0
				break;
			}	
			case SEND_9728://�������
			{
				t = (t+1)%(CHM_9728.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CHM_9728);}		
				if((RX_BMS_TAB[WAIT_9984_BHM].Rx_status == 1)||(time_out > 20))//�յ�9984||��ʱ5s->GB2015
				{
					//if(k1k2�Ӵ�����ѹ<10V)
					//if(��Ե���ok��)//��Ҫ��ѹģ�������ѹ
					//if(й�ŵ�·���ok��)
					t = time_out = 0;
					Data_256.IdentifyResult = 0x00;//��ʶ״̬��δ��ʶ
					BMS_STA = SEND_256;
				}
				break;
			}		
			case SEND_256://����ʶ
			{
				t = (t+1)%(CRM_256.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CRM_256);}
				
				if(RX_BMS_TAB[WAIT_512_BRM].Rx_status == 1)	{time_out = 0;	Data_256.IdentifyResult = 0xAA;}//��ʶ�ɹ�	
					else if(time_out > 20)	BMS_STA = TIME_OUT;//���ճ�����ʶ����BRM��ʱ5s
				
				if(RX_BMS_TAB[WAIT_1536_BCP].Rx_status == 1)
				{
					//RX_BMS_TAB[WAIT_1536_BCP].Rx_status = RX_BMS_TAB[WAIT_512_BRM].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_2048;				
				}else if(time_out > 20)	BMS_STA = TIME_OUT;//����1536���ĳ�ʱ5s
				break;
			}		
			case SEND_2048://��������������/ʱ��ͬ��
			{
				t = (t+1)%(CML_2048.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CML_2048);	BMS_Send(CTS_1792);}
				
				if(RX_BMS_TAB[WAIT_2304_BRO].Rx_status == 1)
				{
					if(Data_2304.BMSSta==0xAA)
					{
						//RX_BMS_TAB[WAIT_2304_BRO].Rx_status = 0;
						t = time_out = 0;
						Data_2560.PrepareOK = 0x00;//����׼��δ���
						BMS_STA = SEND_2560;
					}else if(time_out > 240)	BMS_STA = TIME_OUT;//BRO 60s��δ׼����
				}else if(time_out >20)	BMS_STA = TIME_OUT;//����2304_BRO BMS׼����������ʱ5s
				break;
			}
			
			case SEND_2560://�������׼������
			{	
				t = (t+1)%(CRO_2560.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CRO_2560);}
				//�ж�ǹ��ѹ�뱨�ĵ�ص�ѹ���5%���ڣ��ڳ��������С��ѹ֮��
				//������Դģ�������ѹ���ڵ�ص�ѹ1-10v�󣬱պϼ̵���K1K2����Ϊ0xAA
				if(1)	Data_2560.PrepareOK = 0xAA;
				if(RX_BMS_TAB[WAIT_4096_BCL].Rx_status == 1)
				{
					//RX_BMS_TAB[WAIT_4096_BCL].Rx_status = RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 0;
					time_out = 0;				
				}else if(time_out > 4)	BMS_STA = TIME_OUT;//����4096��س������ʱ250*4=1s
				if(RX_BMS_TAB[WAIT_4352_BCS].Rx_status == 1)
				{
					t = time_out = 0;
					BMS_STA = SEND_4608;
				}else if(time_out > 20)	BMS_STA = TIME_OUT;//4352��س����״̬���ĳ�ʱ5s
				break;
			}		
			case SEND_4608://�������״̬
			{
				t = (t+1)%(CCS_4608.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CCS_4608);
					Data_4608.OutputCurr = Data_4096.NeedCurr+10;//������
					Data_4608.OutputVolt = Data_4096.NeedVolt-10;//ʵ��ʹ����ACDCģ������
					Data_4608.ChargingTime++;}
				
				if(RX_BMS_TAB[WAIT_4864_BSM].Rx_status == 1)
				{
					RX_BMS_TAB[WAIT_4864_BSM].Rx_status = 0;
					//if(1)//���״̬��ǩ�Ƿ�ȫ��������else err
					//if(1)����ֹ���ON?->�����ͣ�����������ж�4864BSM���״̬�Ƿ������	
				}
				//if(1)�����������Ƿ��������������6656������ֹ���CTS
				break;
			}
			case SEND_6656://������ֹ���
			{
				t = (t+1)%(CST_6656.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CST_6656);}

				if((RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)||(time_out > 500))//�յ�BMS��ֹ���BST||����CTS10*100*5=5s��
				{
					//�������ֹͣ����
					t = time_out = 0;
					BMS_STA = SEND_7424;
				}
				break;
			}			
			case SEND_7424://����ͳ�����ݱ���
			{
				t = (t+1)%(CSD_7424.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CSD_7424);}
				//�Ͽ�DC�̵���K1K2
				//й�ŵ�·����

				if(RX_BMS_TAB[WAIT_7168_BSD].Rx_status == 1)//�յ�BMSͳ������BSD
				{
					//������Դ�̵����Ͽ�K3K4����������
					t = time_out = 0;
					BMS_STA = BEGIN;
				}//else if(time_out > 40)	BMS_STA = TIME_OUT;//����BMSͳ�����ݱ��ĳ�ʱ10s		
				break;
			}
			case TIME_OUT:
			{
				if(BMS_STA != TIME_OUT)	OUT++;
				if(OUT > 3)//��������3�β�������
				{
					//�������ֹͣ����
					if(time_out > 1)
					{
						//�Ͽ�K1K2
						//й�ŵ�·����
						//������Դ�̵���K3K4�Ͽ�
						//����������
						t = time_out = 0;
						//BMS_STA = ;//�������°�ǹ�����ſ��Լ�����ʼ
					}							
				}
				else 
				{
					t = (t+1)%(CEM_7936.Period/BMS_Task_Time);
					if(t == 1)	{time_out++;	BMS_Send(CEM_7936);}
					//�������ֹͣ����
					if(time_out > 1)
					{
						//�Ͽ�K1K2
						t = time_out = 0;
						BMS_STA = SEND_256;//���¿�ʼ��ʶ
					}					
				}
				break;
			}
			default:break;
		}
		
		osDelay(BMS_Task_Time);
	}
}



static void BMS_Send(TX_BMS Pbuf)
{
	TxMsg1.ExtId = (Pbuf.IFArbition<<26) | (Pbuf.PGN_S<<16) | (Pbuf.PDUSpecific<<8) | (Pbuf.SourceAddress);
	TxMsg1.DLC = Pbuf.DLC;
	memset(TxMsg1.Data,0,sizeof(TxMsg1.Data));
	memcpy(TxMsg1.Data,Pbuf.Data,Pbuf.DLC);
	CAN_Transmit(CAN1, &TxMsg1);
}




static void Single_Package_Deal(void)
{
	if(BMS_Rx_Flag1 == 1)
	{	
		for(char i = WAIT_9984_BHM; i <= WAIT_7680_BEM; i++)
		{
			if((unsigned char)((BMS_RX_1.ExtId&0x00ff0000)>>16) == RX_BMS_TAB[i].PGN_S)//��ȡ����PGN��
			{
				RX_BMS_TAB[i].Rx_status = 1;
				memcpy(RX_BMS_TAB[i].Data,BMS_RX_1.Data,BMS_RX_1.DLC);
			}
		}
		BMS_Rx_Flag1 = 0;
	}
}
		

unsigned char *P = NULL;
unsigned char J1939_Multi_Package[8];//0x10,len_L,len_H,����,0xff,PGN[3]
CanTxMsg TxMsg_Ack  = {0, 0X1CECF456, CAN_Id_Extended, CAN_RTR_Data, 8, {0x11,0x00,1,0xff,0xff,0x00,0x00,0x00}};
CanTxMsg TxMsg_Done = {0, 0X1CECF456, CAN_Id_Extended, CAN_RTR_Data, 8, {0x13,0x00,0,0x00,0xff,0x00,0x00,0x00}};
//����BMS�������	
void Multi_Package_Deal(void)
{
	if((BMS_RX_0.ExtId == 0X1CEC56F4)&&(BMS_RX_0.Data[0] == 0x10))//BMS�����������������
	{		
		memcpy(&J1939_Multi_Package[1],&BMS_RX_0.Data[1],7);//�������������ӵ���������	
		for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++) \
			if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	P = (unsigned char*)RX_BMS_TAB[i].Data;
		
		TxMsg_Ack.Data[1] = J1939_Multi_Package[3];//�ɷ��Ͱ���	
		TxMsg_Ack.Data[6] = J1939_Multi_Package[6];//PGN*/							
		CAN_Transmit(CAN1, &TxMsg_Ack);
		if(TxMsg_Ack.Data[6] == 0x11)	CAN_Transmit(CAN1, &TxMsg_Ack);//Ӧ��4352�÷����飡����ж�������
	}
	else if(BMS_RX_0.ExtId == 0X1CEB56F4)//������ݴ���
	{				
		if(BMS_RX_0.Data[0] == 1)	memcpy(P,&BMS_RX_0.Data[1],7); \
			else if(BMS_RX_0.Data[0] == 2)	memcpy(P+7,&BMS_RX_0.Data[1],7);//����������
				
		if(BMS_RX_0.Data[0] == J1939_Multi_Package[3])//���������ɣ�
		{
			for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++)
				if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	RX_BMS_TAB[i].Rx_status  = 1;				
			TxMsg_Done.Data[1] = J1939_Multi_Package[1];
			TxMsg_Done.Data[3] = J1939_Multi_Package[3];
			TxMsg_Done.Data[6] = J1939_Multi_Package[6];
			CAN_Transmit(CAN1, &TxMsg_Done);//Ӧ�����������
		}
	}
}

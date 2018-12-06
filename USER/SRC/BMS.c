#include "main.h"
#include "bms.h"
#include "can.h"
#include "adc.h"
#include "acdc_module.h"

#pragma pack(1)//ǿ��1�ֽڶ���
stuPGN9728Type Data_9728 = {0x01,0x01,0x00};
stuPGN256Type	 Data_256  = {0x00,0,};
stuPGN1792Type Data_1792;
stuPGN2048Type Data_2048 = {ACDC_MAX_VOL,ACDC_MIN_VOL,ACDC_MAX_CUR};//���������ѹ��200-750V ���������0-200A
stuPGN2560Type Data_2560;
stuPGN4608Type Data_4608 = {0,0,0,0x01};//Ĭ��������2015GB
stuPGN6656Type Data_6656;
stuPGN7424Type Data_7424;
stuPGN7936Type Data_7936;/*���׮����BMS������*/
stuPGN9984Type Data_9984;/*BMS������������*/
stuPGN2304Type Data_2304;
stuPGN4096Type Data_4096;
stuPGN512Type	 Data_512;
stuPGN1536Type Data_1536;
stuPGN4352Type Data_4352;
stuPGN4864Type Data_4864;
stuPGN6400Type Data_6400;
stuPGN7168Type Data_7168;
stuPGN7680Type Data_7680;
#pragma pack()							/*���ܱ�־		����PGN					������*/
RX_BMS RX_BMS_TAB[10] =	{	{		0,			(9984>>8),			&Data_9984},	//BMS���ֱ���
													{		0,			(2304>>8),			&Data_2304},	//���׼����������
													{		0,			(4096>>8),			&Data_4096},	//��س��������
													{		0,			(4864>>8),			&Data_4864},	//��������״̬��Ϣ
													{		0,			(6400>>8),			&Data_6400},	//BMS��ֹ��籨��
													{		0,			(7168>>8),			&Data_7168},	//BMSͳ�����ݱ���
													{		0,			(7680>>8),			&Data_7680},  //BMS������																	
													{		0,			(512>>8),				&Data_512	},	//BMS������ʶ����
													{		0,			(1536>>8),			&Data_1536},	//�������س�����
													{		0,			(4352>>8),			&Data_4352}};	//��س����״̬����
									/*����PGN		���ȼ�	Ŀ�ĵ�ַ	Դ��ַ	���ݳ���		��������ms		������*/		
TX_BMS CHM_9728 = {(9728>>8),		6,		 0xf4,		 0x56,		 3,					250,			&Data_9728};//�������ֱ���
TX_BMS CRM_256  =	{(256 >>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_256 };//������ʶ����
TX_BMS CTS_1792 = {(1792>>8),		6,		 0xf4,		 0x56,	 	 7,					250,			&Data_1792};//��������ʱ��ͬ��
TX_BMS CML_2048 = {(2048>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_2048};//��������������
TX_BMS CRO_2560 = {(2560>>8),		4,		 0xf4,		 0x56,		 1,					250,			&Data_2560};//�������׼������
TX_BMS CCS_4608 = {(4608>>8),		6,		 0xf4,		 0x56,		 8,					50,				&Data_4608};//�������״̬
TX_BMS CST_6656 = {(6656>>8),		4,		 0xf4,		 0x56,		 4,					10,				&Data_6656};//������ֹ���
TX_BMS CSD_7424 = {(7424>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_7424};//����ͳ������
TX_BMS CEM_7936 = {(7936>>8),		2,		 0xf4,		 0x56,		 4,					250,			&Data_7936};//����������
unsigned char OUT,guzhang;
enum _BMS_STA BMS_STA = BEGIN;
bool once_run = true;

void BMS_Task(void const *argument)
{
	const unsigned char BMS_Task_Time = 25U;//10msѭ��
	static unsigned short t,t1,timeout,timeout1;

	while(1)
	{	
		Single_Package_Deal();//������ʹ���������
		
		switch(BMS_STA)//BMS���ʹ���
		{
			case BEGIN://����׼��
			{			
				if((Type_DM.JiTing==0)&&((AD_DATA.CC>3)&&(AD_DATA.CC<5))&&(Type_DM.DErr==0))//��ͣδ����	CC����	׮�޹���
				{
					//δ����C��ͨ����ͣ���ؿ������->��ǹ�汾��			//����C��ͨ��C���·��Ŀ�������
					if(((Board_C_Sta == 0)&&(DI_Ack.START == 1))||((Board_C_Sta != 0)&&(Type_Control_Cmd.Start_Stop == 0x02)))
					{
						if(DI_Ack.LOCK == 1)//��������
						{	
							BMS_Data_Init();
							GPIO_PinWrite(BMS_POWER_RELAY_PORT,BMS_POWER_RELAY_PIN,1);//������Դ�̵����պ�K3K4
							OUT = t = timeout = timeout1 = 0;
							BMS_STA = SEND_9728;
						}
						else//��״̬-δ����-��ǹ
						{
							t = (t+1)%(2500/BMS_Task_Time);
							if(t == 1)	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN1,1);
							if(t == (100/BMS_Task_Time))	{timeout++;GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN1,0);}
							if(timeout==2)	
							{
								t=timeout=0;	
								Data_6656.ChargStopChargingReason = Err_Stop;	
								Data_6656.ChargFaultReason |=0x0400;//��������
								guzhang = Lock_ERR;	
								BMS_STA = SEND_6656;
							}//��ʱ5s��δ����->���������Ҫ�ٴΰ�ǹ
						}
					}
				}
			}break;	
			
			case SEND_9728://�������
			{
				t = (t+1)%(CHM_9728.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CHM_9728);}	
//				if(timeout == 1)	{ACDC_Set_Vol_Cur(5000,0);GPIO_PinWrite(K_GUN_PORT,K_GUN_PIN,1);}//��ѹģ�������ѹ �պ�ǹ�ϼ̵���K1K2
//				if(timeout == 20)	Start_Insulation_Check();������Ե���(��ѹ�ﵽ500v���ٿ������)5s��
//				if(timeout == 23)//750ms��ȡ��Ե�����
//				{
//					ACDC_Set_Vol_Cur(0,0);//�رյ�Դ
//					if(AD_DATA.VT_Return == 1)
//					{
//	 					t = timeout = 0;
//						Data_6656.ChargStopChargingReason = Err_Stop;
//						Data_6656.ChargFaultReason |=0x0400;//��������
//						guzhang = Insulation_ERR;//��Ե������
//						BMS_STA = SEND_6656;
//					}
//				}
/*			if(k1k2�Ӵ�����ѹ<10V)//guzhang = Gun_Vol_ERR;//�Ӵ�������ѹ>10V//��Ҫ���°�ǹ
				if(й�ŵ�·���ok��)//guzhang = Tap_Check_ERR;//й�ż�����//ͣ�ñ�׮*/	
				//if((RX_BMS_TAB[WAIT_9984_BHM].Rx_status == 1)||(timeout > 20))//�յ�9984||��ʱ5s->GB2015
				if(timeout == 24)	{t = timeout = 0;	BMS_STA = SEND_256;	GPIO_PinWrite(K_GUN_PORT,K_GUN_PIN,0);}//�Ͽ�ǹ�ϼ̵���K1K2	
			}break;	
			
			case SEND_256://����ʶ
			{
				t = (t+1)%(CRM_256.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CRM_256);}
				
				if(RX_BMS_TAB[WAIT_512_BRM].Rx_status == 1)	{timeout = 0;	Data_256.IdentifyResult = 0xAA;}//��ʶ�ɹ�	
					else if(timeout > 20)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.IdentifyTimeOut |= 0x01;}//���ճ�����ʶ����BRM��ʱ5s
				
				if(RX_BMS_TAB[WAIT_1536_BCP].Rx_status == 1)	{t = timeout = 0;	BMS_STA = SEND_2048;}
					else if(timeout > 30)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x01;}//����1536��س�������ʱ7s(default 5s)
			}break;
			
			case SEND_2048://��������������/ʱ��ͬ��
			{
				t = (t+1)%(CML_2048.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CTS_1792);	BMS_Send(CML_2048);}
				
				if(RX_BMS_TAB[WAIT_2304_BRO].Rx_status == 1)
				{
					if(Data_2304.BMSSta==0xAA)	{t = timeout = 0;	BMS_STA = SEND_2560;}
						else if(timeout>240)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x04;}//BRO��60s��δ׼����
				}else if(timeout>20)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x04;}//����BROBMS׼����������ʱ5s	
			}break;
			
			case SEND_2560://�������׼������
			{
				t = (t+1)%(CRO_2560.Period/BMS_Task_Time);
				if(t == 1)	
				{
					timeout1++;	BMS_Send(CRO_2560);
					if(Data_2560.PrepareOK == 0xAA) timeout++;//����׼��������ż��㳬ʱ
				}
//				if(Data_1536.BatPreVolt)//�ж�ǹ��ѹ�뱨�ĵ�ص�ѹ���5%���ڣ��ڳ��������С��ѹ֮��(���ʵ��ſ�10%)
//				{
//					t = timeou = 0;	
//					Data_6656.ChargStopChargingReason = Err_Stop;	
//					Data_6656.ChargFaultReason |=0x0400;//��������
//					guzhang = Bat_Vol_ERR;//��Ҫ�ٴΰ�ǹ
//					BMS_STA = SEND_6656;
//				}
				if(timeout1 == 1)	ACDC_Set_Vol_Cur(Data_1536.BatPreVolt+55,0);
//				if(Module_Status.Output_Vol*10 > Data_1536.BatPreVolt+10)//������Դģ�������ѹ���ڵ�ص�ѹ1-10v
				{
					GPIO_PinWrite(K_GUN_PORT,K_GUN_PIN,1);//�պ�ǹ�����̵���K1K2
					Data_2560.PrepareOK = 0xAA;//����׼������
				}			
				if((RX_BMS_TAB[WAIT_4352_BCS].Rx_status==1)&&(RX_BMS_TAB[WAIT_4096_BCL].Rx_status==1))	
				{
					t1=t=timeout=timeout1= 0;	
					BMS_STA = SEND_4608;
					Tim2_Init(60000,36000);//��ʼ�ۼƳ��ʱ��
				}
				else if(timeout>20)	{t1=t=timeout=timeout1= 0;	Data_7936.BMSChargingStaTimeOut|=0x05;	BMS_STA=TIME_OUT;}//4096��س������ʱ5s 4352��س����״̬���ĳ�5s(��������)
			}break;
			
			case SEND_4608:/*�������״̬(���׶�)*/
			{
				OUT = 0;//ͨѶ��ʱ�����������㣺�����׶�˵�����б��Ķ��յ�һ��
				t = (t+1)%(CCS_4608.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;timeout1++;	BMS_Send(CCS_4608);}
				
				if((RX_BMS_TAB[WAIT_4096_BCL].Rx_status==0)&&(timeout>4))//����4096��س������ʱ1s
				{
					Data_7936.BMSChargingStaTimeOut|=0x04;
					t = timeout = 0;	
					BMS_STA=TIME_OUT;	
				}
				if((RX_BMS_TAB[WAIT_4352_BCS].Rx_status==0)&&(timeout1>20))//4352��س����״̬���ĳ�5s
				{
					Data_7936.BMSChargingStaTimeOut|=0x01;
					t = timeout = 0;	
					BMS_STA=TIME_OUT;
				}
				if(RX_BMS_TAB[WAIT_4096_BCL].Rx_status==1)	{timeout = 0;	RX_BMS_TAB[WAIT_4096_BCL].Rx_status = 0;}//���ܵ�������ձ��
				if(RX_BMS_TAB[WAIT_4352_BCS].Rx_status==1)	{timeout1= 0;	RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 0;}
				
				if(RX_BMS_TAB[WAIT_4864_BSM].Rx_status==1)
				{
					RX_BMS_TAB[WAIT_4864_BSM].Rx_status = 0;
					if((Data_4864.BatSta==0)&&((Data_4864.BatConnetSta&0x0f)==0))//���״̬λ������
					{
						if(Data_4864.BatConnetSta&0x10)	ACDC_Set_Vol_Cur(Data_4096.NeedVolt,4000-Data_4096.NeedCurr);//������
							else	ACDC_Set_Vol_Cur(0,0);//�����ͣ	
					}else{t = timeout = 0;	BMS_STA = SEND_6656;}//����쳣ֹͣ���
				}
				if(RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)//�յ�BMS��ֹ���
				{
					Data_6656.ChargStopChargingReason = BMS_Stop;//BMSֹͣ
					t = timeout = 0;	
					BMS_STA = STOP;
				}
				
				t1 = (t1+1)%(300/BMS_Task_Time);
				if(t1 == (300/BMS_Task_Time)-1)
				{
					Data_4608.OutputVolt = Module_Status.Output_Vol*10;
					Data_4608.OutputCurr = 4000-Module_Status.Output_Cur*10;
					Data_4608.ChargingTime = Bound(ChargTime>>1,600,0);
					
					if((DI_Ack.GUN!=0xFF)&&(DI_Ack.GUN!=((K_GUN_PORT->IDR>>K_GUN_PIN)&1)))	guzhang = GUN_Relay_Err;//ǹ�ϼ̵�������
					if((DI_Ack.KK_H!=0xFF)&&(DI_Ack.KK_H!=((KK_PORT->IDR>>KK_PIN1)&1)))			guzhang = KK_Relay_Err;
					if((DI_Ack.KK_L!=0xFF)&&(DI_Ack.KK_L!=((KK_PORT->IDR>>KK_PIN2)&1)))			guzhang = KK_Relay_Err;//�м�̵�������
					
					if((guzhang==GUN_Relay_Err)||(guzhang==KK_Relay_Err))
					{	
						Data_6656.ChargStopChargingReason = Err_Stop;	
						Data_6656.ChargFaultReason |=0x04;//��������������
						t = timeout = 0;	
						BMS_STA = SEND_6656;
					}
				}
				if(Type_Control_Cmd.Start_Stop == 0x01)	ACDC_Set_Vol_Cur(0,0);//�����ͣ->�����Դģ�飡
				
				if(Type_DM.JiTing==1)	{Data_6656.ChargStopChargingReason=Mannul_Stop;Type_BMS.Manual = JT_Stop;	t=timeout=0;	BMS_STA = SEND_6656;}//�˹�ֹͣ-��ͣ����
				if(Board_C_Sta == 0)//δ����C��ͨ����ͣ���ؿ������->��ǹ�汾��
				{
					if(DI_Ack.START==0)	{Data_6656.ChargStopChargingReason=Mannul_Stop;Type_BMS.Manual = Start_Stop;	t=timeout=0;	BMS_STA = SEND_6656;}//�˹�ֹͣ-��ͣ����
				}
				else if(Type_Control_Cmd.Start_Stop==0)
				{Data_6656.ChargStopChargingReason=Mannul_Stop;	Type_BMS.Manual=Type_Control_Cmd.Type;	t=timeout=0;	BMS_STA = SEND_6656;}//�˹�ֹͣ-ˢ��APPֹͣ
			}break;
			
			case SEND_6656://������ֹ���
			{
				BMS_Send(CST_6656);//10ms���ڷ���
				if(guzhang == 0)
				{
					timeout++;
					if((RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)||(timeout > 5000/BMS_Task_Time))//�յ�BMS��ֹ���BST||����CTS10*500=5s��
					{
						ACDC_Set_Vol_Cur(0,0);//�������ֹͣ����
						BMS_Send(CSD_7424);//����ͳ�����ݱ���
						t = timeout = 0;
						BMS_STA = STOP;
					}
				}
				else{t = timeout = 0;	BMS_STA = STOP;}
			}break;			
//			case SEND_7424://����ͳ�����ݱ���
//			{
//				t = (t+1)%(CSD_7424.Period/BMS_Task_Time);
//				if(t == 1)	BMS_Send(CSD_7424);
//				if((t>20)||(0))//delay20*10ms(���ߵ���5A����)-�����̵�������
//					{Charge_Close();	t = timeout = 0;	BMS_STA = BEGIN;}//��������7168BMSͳ�����ݱ���										
//			}break;
			case TIME_OUT://��ʱ��������
			{
				if(OUT < 3)//3�����ڳ�ʱ������
				{
					t = (t+1)%(CEM_7936.Period/BMS_Task_Time);
					if(t == 1)	
					{
						BMS_Send(CEM_7936);//���ͳ���������
						ACDC_Set_Vol_Cur(0,0);//�������ֹͣ����
					}	
					if((t == 200/BMS_Task_Time))//delay200ms(���ߵ���5A����)-�����̵�������
					{	
						GPIO_PinWrite(K_GUN_PORT,K_GUN_PIN,0);//�Ͽ�ǹ�����̵���K1K2
						OUT++;//��������+1
						BMS_Data_Init();//���Ľ��ܱ��
						t = timeout = timeout1 = 0;						
						BMS_STA = SEND_256;//���¿�ʼ��ʶ
					}					
				}
				else {t = timeout = 0;	BMS_STA = STOP;}//3��ͨѶ��ʱ�������ٳ�ʱ�������� ֱ�ӽ������					
			}break;
			
			case STOP://������������ȴ��ٴβ���->BEGIN
			{
				if(once_run)
				{
					t = (t+1)%(500/BMS_Task_Time);
					if(t == 1)	ACDC_Set_Vol_Cur(0,0);//�������ֹͣ����
					if(t == (200/BMS_Task_Time))//delay200ms(���ߵ���5A����)-�����̵�������
					{	
						GPIO_PinWrite(BMS_POWER_RELAY_PORT,BMS_POWER_RELAY_PIN,0);//�رո�����ԴK3K4
						GPIO_PinWrite(K_GUN_PORT,K_GUN_PIN,0);//�Ͽ�ǹ�����̵���K1K2				
					}		
					if(t == (200/BMS_Task_Time))	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,1);//����
					if(t == (300/BMS_Task_Time))	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,0);
					if(t == (350/BMS_Task_Time))	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,1);//����
					if(t == (450/BMS_Task_Time))	{GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,0);once_run = false;}
				}
				if(AD_DATA.CC > 5.5f)	{t = timeout = 0;BMS_STA = BEGIN;}//�������°�ǹ�����ſ�����һ�γ��
			}break;
			default:break;
		}	
		osDelay(BMS_Task_Time);
	}
}


static void BMS_Data_Init(void)
{
	for(char j = 0; j<10; j++)	RX_BMS_TAB[j].Rx_status = 0;//������Ľ��ܱ�־
	Data_256.IdentifyResult = 0x00;//��ʶ״̬��δ��ʶ
	Data_2560.PrepareOK = 0x00;		 //׼��״̬��׼��δ���
	memset(&Data_1792,0,sizeof(Data_1792));
	memset(&Data_4608,0,sizeof(Data_4608));
	memset(&Data_6656,0,sizeof(Data_6656));
	memset(&Data_7424,0,sizeof(Data_7424));
	memset(&Data_7936,0,sizeof(Data_7936));//TX
	memset(&Data_4864,0,sizeof(Data_4864));//RX���״̬������
	guzhang = 0;//����λ����
	once_run = true;
}


CanTxMsg TxMsg1 = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//��չ֡ ����֡
static void BMS_Send(TX_BMS Pbuf)
{
	TxMsg1.ExtId = (Pbuf.IFArbition<<26)|(Pbuf.PGN_S<<16)|(Pbuf.PDUSpecific<<8)|(Pbuf.SourceAddress);
	TxMsg1.DLC = Pbuf.DLC;
	memset(TxMsg1.Data,0,sizeof(TxMsg1.Data));
	memcpy(TxMsg1.Data,Pbuf.Data,Pbuf.DLC);
	CAN_Transmit(CAN1, &TxMsg1);
}

//����������
static void Single_Package_Deal(void)
{
	if(RX_Flag.BMS_Rx_Flag1)
	{	
		for(char i = WAIT_9984_BHM; i <= WAIT_7680_BEM; i++)
		{
			if((unsigned char)((BMS_RX_1.ExtId&0x00ff0000)>>16) == RX_BMS_TAB[i].PGN_S)//��ȡ����PGN��
			{
				RX_BMS_TAB[i].Rx_status = 1;	memcpy(RX_BMS_TAB[i].Data,BMS_RX_1.Data,BMS_RX_1.DLC);
			}
		}
		RX_Flag.BMS_Rx_Flag1 = false;
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
		for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++)
		{
			if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	
				P = (unsigned char*)RX_BMS_TAB[i].Data;
		}
		TxMsg_Ack.Data[1] = J1939_Multi_Package[3];//�ɷ��Ͱ���	
		TxMsg_Ack.Data[6] = J1939_Multi_Package[6];//PGN*/							
		CAN_Transmit(CAN1, &TxMsg_Ack);
		if(TxMsg_Ack.Data[6] == 0x11)	CAN_Transmit(CAN1, &TxMsg_Ack);//Ӧ��4352�÷����飡����ж�������
	}
	else if(BMS_RX_0.ExtId == 0X1CEB56F4)//������ݴ���
	{				
		if(BMS_RX_0.Data[0] == 1)	memcpy(P,&BMS_RX_0.Data[1],7); \
			else if(BMS_RX_0.Data[0] == 2)	memcpy(P+7,&BMS_RX_0.Data[1],7);//����������ǰ2����������Ч
				
		if(BMS_RX_0.Data[0] == J1939_Multi_Package[3])//���������ɣ�
		{
			for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++)
			{
				if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	
					RX_BMS_TAB[i].Rx_status  = 1;				
			}
			TxMsg_Done.Data[1] = J1939_Multi_Package[1];
			TxMsg_Done.Data[3] = J1939_Multi_Package[3];
			TxMsg_Done.Data[6] = J1939_Multi_Package[6];
			CAN_Transmit(CAN1, &TxMsg_Done);//Ӧ�����������
		}
	}
}


static void ACDC_Set_Vol_Cur(short vol,	short cur)
{
	int V = Bound(vol,ACDC_MAX_VOL,0)*100;//0-700V
	int C = Bound(cur,4000-ACDC_MAX_CUR,0)*100;//0-300V
	ACDC_VolCur_Buffer[0] = V>>24;
	ACDC_VolCur_Buffer[1] = V>>16;
	ACDC_VolCur_Buffer[2] = V>>8;
	ACDC_VolCur_Buffer[3] = V>>0;
	ACDC_VolCur_Buffer[4] = C>>24;
	ACDC_VolCur_Buffer[5] = C>>16;
	ACDC_VolCur_Buffer[6] = C>>8;
	ACDC_VolCur_Buffer[7] = C>>0;
}

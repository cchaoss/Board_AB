#include "main.h"
#include "can.h"
#include "bms.h"
#include "lcd.h"

osThreadId MAIN_ID;//����ID
osThreadId	System_Task_ID;
osThreadId	BMS_Task_ID;
osThreadId	ACDC_Module_Task_ID;

//�̶߳���ṹ��:�����������ȼ���ȱʡ������ջ�ռ䣨byte��
osThreadDef(System_Task, 	osPriorityHigh, 1, 300);
osThreadDef(BMS_Task, 		osPriorityHigh, 1, 600); 
osThreadDef(ACDC_Module_Task, osPriorityNormal, 1, 200);

#define TIMER1_Delay	50U//ms
osTimerId TIMER1_ID;//��ʱ��ID
osTimerDef (Timer1, Timer1_Callback);//��ʱ���ṹ�壺��ʱ��������Ӧ�Ļص�����


unsigned char Board_Type = 0x0A;//Ĭ��ΪA��
int main(void)
{
	Bsp_init();						//��ʼ���弶�豸	
	osKernelInitialize();	//��ʼ��RTXϵͳ
	System_Task_ID 			= osThreadCreate(osThread(System_Task), NULL);			//����������
	BMS_Task_ID					= osThreadCreate(osThread(BMS_Task), NULL);					//��������BMSͨѶ����
	ACDC_Module_Task_ID	= osThreadCreate(osThread(ACDC_Module_Task), NULL);	//��������-ֱ����Դģ��ͨѶ����
	osKernelStart();    	//��ʼ���񴴽���������
	
	TIMER1_ID = osTimerCreate(osTimer(Timer1), osTimerPeriodic, NULL);
	osTimerStart(TIMER1_ID, TIMER1_Delay);	//������ʱ��TIMER1
	
	MAIN_ID = osThreadGetId();
	osThreadTerminate(MAIN_ID);	//ɾ��MAIN����
}


uint16_t kkk;
static void Timer1_Callback(void const *arg)
{	
	kkk = (kkk+1)%(500/TIMER1_Delay);
	if(kkk == 1)
	{
		GPIO_PinWrite(LED_BOARD_PORT,LED_RUN_PIN,!GPIO_PinRead(LED_BOARD_PORT,LED_RUN_PIN));//RUN_LED 500s��תһ��
		IWDG_ReloadCounter();//�ڲ����Ź�ι����
		GPIO_PinWrite(GPIOD,2,0);GPIO_PinWrite(GPIOD,2,1);//�ⲿӲ�����Ź�ι���
	}
	DI_Ack.JT 	 = DI_Status_Check(DI_Filter.JT,GPIO_PinRead(JT_PORT,JT_PIN));
	DI_Ack.START = DI_Status_Check(DI_Filter.START,GPIO_PinRead(START_PORT,START_PIN));
	DI_Ack.GUN   = DI_Status_Check(DI_Filter.GUN,GPIO_PinRead(K_GUN_ACK_PORT,K_GUN_ACK_PIN));
	DI_Ack.LOCK  = DI_Status_Check(DI_Filter.LOCK,GPIO_PinRead(LOCK_ACK_PORT,LOCK_ACK_PIN));
	DI_Ack.KK_H  = DI_Status_Check(DI_Filter.KK_H,GPIO_PinRead(KK_ACK_PORT,KK_ACK_PIN1));
	DI_Ack.KK_L  = DI_Status_Check(DI_Filter.KK_L,GPIO_PinRead(KK_ACK_PORT,KK_ACK_PIN2));
}	


Bms_Type	Type_BMS;
VolCur_Type	Type_VolCur;
Device_Module_Type Type_DM;
Control_Type	Type_Control_Cmd;
CanTxMsg TxMsg_ABC = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//��չ֡ ����֡
unsigned char Board_C_Sta = 0;//0:C�岻���� 1:C��ͨѶ���� 0xFF:C��ͨѶ��ʱ
//����AB�������ϱ���C��
void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 75U;
	static unsigned char t,STEP;
	if(Check_PE())		Type_DM.DErr = Geodesic;//�ӵع���
	if(GPIO_PinRead(DIP_SWITCH_PORT1,DIP_SWITCH_PIN1))	Board_Type  = 0X0A;//��ȡ���뿪�ص�ַ ȷ��A B��
		else	{Board_Type  = 0X0B;	Type_DM.DErr = 0;}//�ӵؼ����A����B���޽ӵع���
	if((DI_Ack.GUN&DI_Ack.KK_H&DI_Ack.KK_H)!=0)	Type_DM.DErr = Relay_Err;//�̵����޷��Ͽ�
	Type_DM.JiTing = 0;//Ĭ�ϼ�ͣδ����
	while(1)
	{
		if(DI_Ack.JT == 0)	Type_DM.JiTing = 1;//��ͣ��ť����
			else if(DI_Ack.JT == 1)Type_DM.JiTing = 0;
		
		ABC_Data_Deal(System_Task_Time);//����C������
		switch(STEP)
		{
			case 0://׮״̬/ģ��״̬֡
			{
				if(Type_DM.DErr != 0)	Type_DM.DSta = 2;//׮����:ģ�����//���ͨѶ����//ABCʧ��
					else if((BMS_STA == BEGIN)||(BMS_STA == STOP))	Type_DM.DSta = 0;//׮����
						else Type_DM.DSta = 1;//׮���������
				memcpy(TxMsg_ABC.Data,&Type_DM,sizeof(Type_DM));
				STEP = 1;
			}break;
			case 1://BMS״̬֡
			{
				Type_BMS.Step = BMS_STA;
				if(Data_6656.ChargStopChargingReason==0)	Type_BMS.Stop_Reason = Time_Out;//��ʱ��������3��ֹͣ
					else Type_BMS.Stop_Reason = Data_6656.ChargStopChargingReason;
				if(Data_7936.IdentifyTimeOut == 0x01)	Type_BMS.time_out = BRM512_Timeout;
					else if(Data_7936.ChargingParamTimeOut&0x01) Type_BMS.time_out = BCP1536_Timeout;
						else if(Data_7936.ChargingParamTimeOut&0x04)	Type_BMS.time_out = BRO2304_Timeout;
							else if(Data_7936.BMSChargingStaTimeOut&0x01) Type_BMS.time_out = BCS4352_Timeout;
								else if(Data_7936.BMSChargingStaTimeOut&0x01) Type_BMS.time_out = BCL4096_Timeout;
				Type_BMS.DErr = guzhang;			
				if(Data_6400.BMSFaultReason&0x03)	Type_BMS.BErr = Insulation;//��Ե����
					else if(Data_6400.BMSFaultReason&0x3c)	Type_BMS.BErr = BmsOutNetTemp;//BMSԪ��/�������������(2��1)
						else if(Data_6400.BMSFaultReason&0xc0)	Type_BMS.BErr = ChargeNet;//�������������
							else if(Data_6400.BMSFaultReason&0x0300)	Type_BMS.BErr = BatTemp;//������¶ȹ���
								else if(Data_6400.BMSFaultReason&0x0c00)	Type_BMS.BErr = HighRelay;//��ѹ�̵�������
									else if(Data_6400.BMSFaultReason&0x3000)	Type_BMS.BErr = Vol_2;////����2��ѹ������
				if(Data_6400.BMSStopChargingReason&0x01)	Type_BMS.BErr = CurOver;//��������
					else if(Data_6400.BMSStopChargingReason&0x02)	Type_BMS.BErr = CurUnknown;//����������
						else if(Data_6400.BMSStopChargingReason&0x04)	Type_BMS.BErr = CurOver;//��ѹ�쳣
							else if(Data_6400.BMSStopChargingReason&0x08)	Type_BMS.BErr = VolUnknown;//��ѹ������
				if((Data_6400.BMSStopChargingReason&0x05)||(Data_4352.PreSOC>=98))	Type_BMS.BErr = Soc_Full;//����ֹͣ(�ﵽ����SOC)	
//				if(Type_DM.JiTing == 1)	Type_BMS.Manual = JT;//3������������������θ�ֵ�Ƿ��Щ��
				STEP = 2;
			}break;
			case 2://��ѹ����SOC֡
			{
				Type_VolCur.Vol = Data_4608.OutputVolt;//ģ�����
				Type_VolCur.Cur =	Data_4608.OutputCurr;
				Type_VolCur.Soc = Data_4352.PreSOC;		 //������
				Type_VolCur.KW  =	305;//30.5kw
				STEP = 0;
			}break;
			default:break;
		}
		
		if(Board_C_Sta == 0)//δ����C�������ʾ(����)
		{
			t = (t+1)%(1000/System_Task_Time);//��ʾ����1sˢ��һ��
			//if(t == 1)	LcdShow();		
		}
		else//ֻҪ������C���ѭ������
		{
			CAN_Transmit(CAN2, &TxMsg_ABC);
			memset(TxMsg_ABC.Data,0,sizeof(TxMsg_ABC.Data));
		}
		
		osDelay(System_Task_Time);
	}
}


unsigned char count;
static void ABC_Data_Deal(unsigned short Task_Time)
{
	if(RX_Flag.ABC_Data_Rx_Flag)
	{
		RX_Flag.ABC_Data_Rx_Flag = false;
		if(ABC_DATA_RX.ExtId==0xC0000ABC)//�����ͣ/��ͣ��ģ�����
		{
			Board_C_Sta = 0x01;//��C��ͨѶ����
			memcpy(&Type_Control_Cmd,ABC_DATA_RX.Data,sizeof(Type_Control_Cmd));
		}
		if(ABC_DATA_RX.ExtId==0xC1000ABC);//TODO:��������
		count = 0;
	}else count++;
	if((count > (5000/Task_Time))&&(Board_C_Sta==1))	
	{
		Board_C_Sta = 0xFF;//ͨѶ���ϣ�5s��δ�յ�C�����ݣ�
		Type_DM.DErr = Disconnect_C;
	}
}

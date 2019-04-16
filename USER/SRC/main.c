#include "main.h"
#include "can.h"
#include "bms.h"
#include "lcd.h"
#include "adc.h"
#include "acdc_module.h"
#include "electric_meter.h"

osThreadId MAIN_ID;//����ID
osThreadId	System_Task_ID;
osThreadId	BMS_Task_ID;
osThreadId	ACDC_Module_Task_ID;

//�̶߳���ṹ��:�����������ȼ���ȱʡ������ջ�ռ䣨byte��
osThreadDef(System_Task, 	osPriorityHigh, 1, 300);
osThreadDef(BMS_Task, 		osPriorityHigh, 1, 500); 
osThreadDef(ACDC_Module_Task, osPriorityNormal, 1, 200);

#define TIMER1_Delay	50U//ms
osTimerId TIMER1_ID;//��ʱ��ID
osTimerDef (Timer1, Timer1_Callback);//��ʱ���ṹ�壺��ʱ��������Ӧ�Ļص�����

DIPSwitchBits DIPSwitch = {0};
unsigned char Board_Type = 0x0A;//Ĭ��ΪA��
int main(void)
{
	Bsp_init();						//��ʼ���弶�豸	
	osKernelInitialize();	//��ʼ��RTXϵͳ
	System_Task_ID 			= osThreadCreate(osThread(System_Task), NULL);			//����������
	BMS_Task_ID					= osThreadCreate(osThread(BMS_Task), NULL);					//��������BMSͨѶ����
	ACDC_Module_Task_ID	= osThreadCreate(osThread(ACDC_Module_Task), NULL);	//��������-ֱ����Դģ��ͨѶ����
	osKernelStart();    	//��ʼ���ϴ���������
	
	TIMER1_ID = osTimerCreate(osTimer(Timer1), osTimerPeriodic, NULL);
	osTimerStart(TIMER1_ID, TIMER1_Delay);	//������ʱ��TIMER1
	
	MAIN_ID = osThreadGetId();
	osThreadTerminate(MAIN_ID);	//ɾ��MAIN����
}


static void Timer1_Callback(void const *arg)
{	
	static char t;
	t = (t+1)%(500/TIMER1_Delay);
	if(t == 1)
	{
		LED_RUN_TOGGLE;//RUN_LED 500ms��תһ��
		IWDG_ReloadCounter();//�ڲ����Ź�ι��	
	}
	DI_Ack.JT 	 = DI_Status_Check(&DI_Filter.JT,		((JT_PORT->IDR>>JT_PIN)&1));//��ȡ����IO�����źŲ��˲�
	DI_Ack.START = DI_Status_Check(&DI_Filter.START,((START_PORT->IDR>>START_PIN)&1));
	DI_Ack.GUN   = DI_Status_Check(&DI_Filter.GUN,	((K_GUN_ACK_PORT->IDR>>K_GUN_ACK_PIN)&1));
	DI_Ack.LOCK  = DI_Status_Check(&DI_Filter.LOCK,	((LOCK_ACK_PORT->IDR>>LOCK_ACK_PIN)&1));
	DI_Ack.KK_H  = DI_Status_Check(&DI_Filter.KK_H,	((KK_ACK_PORT->IDR>>KK_ACK_PIN1)&1));
	DI_Ack.KK_L  = DI_Status_Check(&DI_Filter.KK_L,	((KK_ACK_PORT->IDR>>KK_ACK_PIN2)&1));
	Get_Adc_Status();//����AD����ֵ
	if(DI_Ack.JT == 0)	Type_DM.JiTing = 1;//��ͣ��ť����
		else if(DI_Ack.JT == 1)	Type_DM.JiTing = 0;
}	


CanTxMsg TxMsg_ABC = {0, 0x0ABC00A0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//��չ֡ ����֡
unsigned char Board_C_Sta = 0;//0:C�岻���� 1:C��ͨѶ���� 0xFF:C��ͨѶ��ʱ
//����AB�������ϱ���C��
void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 100U;
	static unsigned char t0,t1;
	/*-���뿪��Ĭ���Ǹߵ�ƽ��*/
	DIPSwitch.Bits_1 = GPIO_PinRead(DIP_SWITCH_PORT1,DIP_SWITCH_PIN1);//1-A��  0-B��
	DIPSwitch.Bits_2 = GPIO_PinRead(DIP_SWITCH_PORT2,DIP_SWITCH_PIN2);//1-750Vģ��0-500vģ��
	DIPSwitch.Bits_3 = GPIO_PinRead(DIP_SWITCH_PORT3,DIP_SWITCH_PIN3);//1���� 0�ر� ��Ե���	
	DIPSwitch.Bits_4 = GPIO_PinRead(DIP_SWITCH_PORT4,DIP_SWITCH_PIN4);//1��������  0�ر�����
	if(Check_PE())		Type_DM.DErr |= Geodesic;//�ӵع���
	if(DIPSwitch.Bits_1 != 1)	{Board_Type = 0X0B; TxMsg_ABC.ExtId=0x0ABC00B0; Type_DM.DErr &= ~Geodesic;}//��ȡ���뿪�ص�ַ:Ĭ��A	�ӵؼ����A����
	if(DIPSwitch.Bits_2 == 1) ACDC_MAX_VOL = 7500;	else ACDC_MAX_VOL = 5000;//���뿪�ص�2���� 1=750V 0=500V
	
	while(1)
	{	
		ABC_Data_Deal_RX(System_Task_Time);//����C������
		ABC_Data_Deal_TX();//��C�巢������
		
		t0 = (t0+1)%(500/System_Task_Time);
		if(t0 == 1)	Read_ElectricMeter_Data();//���������
		
		t1 = (t1+1)%(1000/System_Task_Time);
		if((t1 == 0)&&(Board_Type==0X0A))	LcdShow();//��ʾ����1sˢ��һ��
		
		osDelay(System_Task_Time);
	}
}


Bms_Type	Type_BMS;
VolCur_Type	Type_VolCur;
Device_Module_Type Type_DM;
Control_Type	Type_Control_Cmd;
static void ABC_Data_Deal_TX(void)
{
	static char STEP;
	STEP = (STEP+1)%3;//0-1-2
	switch(STEP)
	{
		case 0://׮״̬/ģ��״̬֡
			if(Type_DM.DErr != 0)	Type_DM.DSta = 2;//׮����
				else if((BMS_STA == BEGIN)||(BMS_STA == STOP))	Type_DM.DSta = 0;//׮����
					else Type_DM.DSta = 1;//׮���������
			TxMsg_ABC.DLC = sizeof(Type_DM);
			memcpy(TxMsg_ABC.Data,&Type_DM,sizeof(Type_DM));
		break;
		
		case 1://BMS״̬֡
			Type_BMS.Step = BMS_STA;
			Type_BMS.DErr = guzhang;
			if((AD_DATA.CC>CC_Connect_MIN)&&(AD_DATA.CC<CC_Connect_MAX))	Type_BMS.Gun_link = 1;//����
				else if(AD_DATA.CC > CC_Disconnect)	Type_BMS.Gun_link = 0;//��ǹ
			if(Data_6656.ChargStopChargingReason==0)	Type_BMS.Stop_Reason = Time_Out;//��ʱ��������3��ֹͣ
				else Type_BMS.Stop_Reason = Data_6656.ChargStopChargingReason;
			if(Data_7936.IdentifyTimeOut == 0x01)	Type_BMS.time_out = BRM512_Timeout;
				else if(Data_7936.ChargingParamTimeOut&0x01) Type_BMS.time_out = BCP1536_Timeout;
					else if(Data_7936.ChargingParamTimeOut&0x04)	Type_BMS.time_out = BRO2304_Timeout;
						else if(Data_7936.BMSChargingStaTimeOut&0x01) Type_BMS.time_out = BCS4352_Timeout;
							else if(Data_7936.BMSChargingStaTimeOut&0x04) Type_BMS.time_out = BCL4096_Timeout;						
			if(Data_6400.BMSFaultReason&0x03)	Type_BMS.BErr = Insulation;//��Ե����
				else if(Data_6400.BMSFaultReason&0x3c)	Type_BMS.BErr = BmsOutNetTemp;//BMSԪ��/�������������(2��1)
					else if(Data_6400.BMSFaultReason&0xc0)	Type_BMS.BErr = ChargeNet;//�������������
						else if(Data_6400.BMSFaultReason&0x0300)	Type_BMS.BErr = BatTemp;//������¶ȹ���
							else if(Data_6400.BMSFaultReason&0x0c00)	Type_BMS.BErr = HighRelay;//��ѹ�̵�������
								else if(Data_6400.BMSFaultReason&0x3000)	Type_BMS.BErr = Vol_2;//����2��ѹ������
			if(Data_6400.BMSErrorReason&0x03)	Type_BMS.BErr = CurUnknown;//����������
					else if(Data_6400.BMSErrorReason&0x0c)	Type_BMS.BErr = VolUnknown;//��ѹ������
			if((Data_6400.BMSStopChargingReason&0x05)||(Data_4352.PreSOC>98))	Type_BMS.BErr = Soc_Full;//����ֹͣ���ﵽ��ѹ����soc����
			TxMsg_ABC.DLC = sizeof(Type_BMS);
			memcpy(TxMsg_ABC.Data,&Type_BMS,sizeof(Type_BMS));
		break;
		
		case 2://��ѹ����SOC֡
			Type_VolCur.Soc = Data_4352.PreSOC;//������		
			if(MeterSta == No_Link)
			{
				Type_VolCur.KWh =	dianliang*10;//�޵��ʹ���ۼӼ������
				Type_VolCur.Vol = Module_Status.Output_Vol*10;//�޵���ѹ����ʹ��ģ��ֵ
				Type_VolCur.Cur =	Module_Status.Output_Cur*10;
			}else 
			{
				Type_VolCur.KWh = (MeterData.kwh_realtime-MeterData.kwh_start)*10;//���ϵ��ʹ�õ������
				Type_VolCur.Vol = MeterData.vol*10;
				Type_VolCur.Cur =	MeterData.cur*10;
			}
			TxMsg_ABC.DLC = sizeof(Type_VolCur);
			memcpy(TxMsg_ABC.Data,&Type_VolCur,sizeof(Type_VolCur));
		break;
		default:break;
	}
	
	if(Board_C_Sta != 0)//ֻҪ������C���ѭ������
	{
		TxMsg_ABC.ExtId = (TxMsg_ABC.ExtId&0XFFFFFFF0)|STEP;
		CAN_Transmit(CAN2, &TxMsg_ABC);
		memset(TxMsg_ABC.Data,0,8);
	}
}

static void ABC_Data_Deal_RX(unsigned short Task_Time)
{
	static unsigned char count,count1;
	if(RX_Flag.ABC_Data_Rx_Flag)
	{
		RX_Flag.ABC_Data_Rx_Flag = false;
		if(ABC_DATA_RX.ExtId==0x0ABC00C0)//�����ͣ/��ͣ��ģ�����
		{
			if(++count1 > (1000/Task_Time))	
			{
				Board_C_Sta = 0x01;//��C��ͨѶ����
				Type_DM.DErr &= ~Disconnect_C;//���ϻָ�
				LED5_ON;//�յ����ݵ���
			}
			Type_Control_Cmd.KK_Sta = ABC_DATA_RX.Data[3];
			Type_Control_Cmd.Module_Assign = ABC_DATA_RX.Data[2];
			if(Board_Type == 0x0A)	memcpy(&Type_Control_Cmd.CMD,&ABC_DATA_RX.Data[0],1);
			else if(Board_Type == 0x0B)	memcpy(&Type_Control_Cmd.CMD,&ABC_DATA_RX.Data[1],1);
		}
		else if(ABC_DATA_RX.ExtId==0x0ABC00C1);//TODO:��������
		count = 0;
	}else {count++;count1 = 0;}
	if((count > (3000/Task_Time))&&(Board_C_Sta==1))	
	{
		Board_C_Sta = 0xFF;//ͨѶ���ϣ�2s��δ�յ�C�����ݣ�
		Type_Control_Cmd.CMD.Start_Stop = false;//�رճ��
		Type_DM.DErr |= Disconnect_C;//ֻ�������Ϲ�C���ٶϿ��Ż��ù���λ��
		LED5_OFF;//ʧ����Ϩ��
	}
	
	if(Board_C_Sta == 0)	
	{
		Type_Control_Cmd.Module_Assign = 0XAA;/*û������C��ʱ A����Կ�������ģ��*/
		Type_Control_Cmd.CMD.Account = true;	/*û������C��ʱ ��λ�������*/
	}
}

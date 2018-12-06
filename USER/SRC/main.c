#include "main.h"
#include "can.h"
#include "bms.h"
#include "lcd.h"
#include "adc.h"
#include "electric_meter.h"

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

DIPSwitchBits DIPSwitch = {0};
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
		if(Board_Type == 0X0A)	GPIO_PinWrite(LED_BOARD_PORT,LED_RUN_PIN,!((LED_BOARD_PORT->IDR >> LED_RUN_PIN) & 1));
			else GPIO_PinWrite(LED_BOARD_PORT,LED3_PIN,!((LED_BOARD_PORT->IDR >> LED3_PIN) & 1));
		IWDG_ReloadCounter();//�ڲ����Ź�ι��	RUN_LED 500ms��תһ��
	}
	
	DI_Ack.JT 	 = DI_Status_Check(&DI_Filter.JT,		((JT_PORT->IDR >> JT_PIN) & 1));//��ȡ����IO�����źŲ��˲�
	DI_Ack.START = DI_Status_Check(&DI_Filter.START,((START_PORT->IDR >> START_PIN) & 1));
	DI_Ack.GUN   = DI_Status_Check(&DI_Filter.GUN,	((K_GUN_ACK_PORT->IDR >> K_GUN_ACK_PIN) & 1));
	DI_Ack.LOCK  = DI_Status_Check(&DI_Filter.LOCK,	((LOCK_ACK_PORT->IDR >> LOCK_ACK_PIN) & 1));
	DI_Ack.KK_H  = DI_Status_Check(&DI_Filter.KK_H,	((KK_ACK_PORT->IDR >> KK_ACK_PIN1) & 1));
	DI_Ack.KK_L  = DI_Status_Check(&DI_Filter.KK_L,	((KK_ACK_PORT->IDR >> KK_ACK_PIN2) & 1));

	Get_Adc_Status();//����AD����ֵ
}	


Bms_Type	Type_BMS;
VolCur_Type	Type_VolCur;
Device_Module_Type Type_DM;
Control_Type	Type_Control_Cmd;
CanTxMsg TxMsg_ABC = {0, 0x0ABC00A0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//��չ֡ ����֡
unsigned char Board_C_Sta = 0;//0:C�岻���� 1:C��ͨѶ���� 0xFF:C��ͨѶ��ʱ
//����AB�������ϱ���C��
void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 100U;
	static unsigned char STEP,t;
	
//	if(Check_PE())		Type_DM.DErr |= Geodesic;//�ӵع���
	DIPSwitch.Bits_1 = GPIO_PinRead(DIP_SWITCH_PORT1,DIP_SWITCH_PIN1);//���뿪��Ĭ���Ǹߵ�ƽ��
	DIPSwitch.Bits_2 = GPIO_PinRead(DIP_SWITCH_PORT2,DIP_SWITCH_PIN2);
	DIPSwitch.Bits_3 = GPIO_PinRead(DIP_SWITCH_PORT3,DIP_SWITCH_PIN3);
	DIPSwitch.Bits_4 = GPIO_PinRead(DIP_SWITCH_PORT4,DIP_SWITCH_PIN4);
	if(DIPSwitch.Bits_1 == 0)	{Board_Type = 0X0B; TxMsg_ABC.ExtId=0x0ABC00B0; Type_DM.DErr &= ~Geodesic;}//��ȡ���뿪�ص�ַ:Ĭ��A	�ӵؼ����A����
	
	while(1)
	{
		if(DI_Ack.JT == 0)	Type_DM.JiTing = 1;//��ͣ��ť����
			else if(DI_Ack.JT == 1)	Type_DM.JiTing = 0;
		
		ABC_Data_Deal(System_Task_Time);//����C������
		switch(STEP)
		{
			case 0://׮״̬/ģ��״̬֡
			{
				if(Type_DM.DErr != 0)	Type_DM.DSta = 2;//׮����
					else if((BMS_STA == BEGIN)||(BMS_STA == STOP))	Type_DM.DSta = 0;//׮����
						else Type_DM.DSta = 1;//׮���������
				memcpy(TxMsg_ABC.Data,&Type_DM,sizeof(Type_DM));
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
				if((Data_6400.BMSStopChargingReason&0x05)||(Data_4352.PreSOC>98))	Type_BMS.BErr = Soc_Full;//����ֹͣ���ﵽ��ѹ����soc����
				Type_BMS.RemaChargTime = Data_4352.RemaChargTime;//0-600min
				memcpy(TxMsg_ABC.Data,&Type_BMS,sizeof(Type_BMS));
			}break;
			case 2://��ѹ����SOC֡
			{
				Type_VolCur.Soc = Data_4352.PreSOC;//������
				Type_VolCur.CC  = AD_DATA.CC*10;	 //4.5v=45				
				if(MeterSta == No_Link)
				{
					Type_VolCur.KWh =	dianliang;//�޵��ʹ���ۼӼ������
					Type_VolCur.Vol = Data_4608.OutputVolt;//�޵���ѹ����ʹ��ģ��ֵ
					Type_VolCur.Cur =	4000-Data_4608.OutputCurr;
				}else 
				{
					if(BMS_STA==SEND_2560)	MeterData.kwh_start = MeterData.kwh_realtime;//׼�����ʱ��¼����������
					Type_VolCur.KWh = MeterData.kwh_realtime-MeterData.kwh_start;//���ϵ��ʹ�õ������
					Type_VolCur.Vol = MeterData.vol;
					Type_VolCur.Cur =	MeterData.cur;
				}
				memcpy(TxMsg_ABC.Data,&Type_VolCur,sizeof(Type_VolCur));
			}break;
			default:break;
		}STEP = (STEP+1)%3;//0-1-2
		
		if(Board_C_Sta != 0)//ֻҪ������C���ѭ������
		{
			TxMsg_ABC.ExtId = (TxMsg_ABC.ExtId&0XFFFFFFF0)|STEP;
			CAN_Transmit(CAN2, &TxMsg_ABC);
			memset(TxMsg_ABC.Data,0,8);
		}
		
		t = (t+1)%(1000/System_Task_Time);
		if(t == 1)	
		{
			Read_ElectricMeter_Data();//���������
			LcdShow();//��ʾ����1sˢ��һ��
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
		if(ABC_DATA_RX.ExtId==0x0ABC00C0)//�����ͣ/��ͣ��ģ�����
		{
			Board_C_Sta = 0x01;//��C��ͨѶ����
			Type_DM.DErr &= ~Disconnect_C;//���ϻָ�
			GPIO_PinWrite(LED_BOARD_PORT,LED5_PIN,1);//�յ����ݵ�5��
			Type_Control_Cmd.Module_Assign = ABC_DATA_RX.Data[4];
			if(Board_Type == 0x0A)	{Type_Control_Cmd.Start_Stop = ABC_DATA_RX.Data[0];Type_Control_Cmd.Type = ABC_DATA_RX.Data[1];}
			else if(Board_Type == 0x0B){Type_Control_Cmd.Start_Stop = ABC_DATA_RX.Data[2];Type_Control_Cmd.Type = ABC_DATA_RX.Data[3];}
		}
		else if(ABC_DATA_RX.ExtId==0x0ABC00C1);//TODO:��������
		count = 0;
	}else count++;
	if((count > (5000/Task_Time))&&(Board_C_Sta==1))	
	{
		Board_C_Sta = 0xFF;//ͨѶ���ϣ�5s��δ�յ�C�����ݣ�
		Type_DM.DErr |= Disconnect_C;//ֻ�������Ϲ�C���ٶϿ��Ż��ù���λ��
		GPIO_PinWrite(LED_BOARD_PORT,LED5_PIN,0);//ʧ����5Ϩ��
	}
}

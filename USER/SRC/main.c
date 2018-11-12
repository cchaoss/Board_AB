#include "main.h"
#include "can.h"
#include "bms.h"

osThreadId MAIN_ID;//����ID
osThreadId	System_Task_ID;
osThreadId	BMS_Task_ID;
osThreadId	ACDC_Module_Task_ID;

//�̶߳���ṹ��:�����������ȼ���ȱʡ������ջ�ռ䣨byte��
osThreadDef(System_Task, 	osPriorityHigh, 1, 300);
osThreadDef(BMS_Task, 		osPriorityHigh, 1, 300); 
osThreadDef(ACDC_Module_Task, osPriorityNormal, 1, 300);

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
		GPIO_PinWrite(LED_RUN_PORT,LED_RUN_PIN,!GPIO_PinRead(LED_RUN_PORT,LED_RUN_PIN));//RUN_LED 500s��תһ��
		IWDG_ReloadCounter();//�ڲ����Ź�ι����
		GPIO_PinWrite(GPIOD,2,0);GPIO_PinWrite(GPIOD,2,1);//�ⲿӲ�����Ź�ι���
	}
}	


Bms_Type	Type_BMS;
VolCur_Type	Type_VolCur;
Device_Module_Type Type_DM;
Control_Type	Type_Control_Cmd;
CanTxMsg TxMsg_ABC = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//��չ֡ ����֡
unsigned char Board_C_Sta;//0:C�岻���� 1:C��ͨѶ���� 0xFF:C��ͨѶ��ʱ
//����AB�������ϱ���C��
void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 75U;
	static unsigned char STEP;
	//Board_Type���
	//�ӵؼ��		{Type_DM.DSta = 2;//׮���� Type_DM.DErr = Geodesic;//�ӵؼ��ֻ��A��}
	//�Ͽ�S1S2S3S4
	//���S1S2S3S4״̬�Ƿ�����{Type_DM.DSta = 2;//׮���� Type_DM.DErr = Relay_Err;//�̵���״̬������}
	while(1)
	{
		if(0)	Type_DM.JiTing = 1;//��鼱ͣ�Ƿ���
			else Type_DM.JiTing = 0;
		ABC_Data_Deal();//����C������
		
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
				if((Data_6400.BMSStopChargingReason&0x05)!=0)	Type_BMS.BErr = Soc_Full;//����ֹͣ(�ﵽ����SOC)
				if(Data_6400.BMSFaultReason&0x01)	Type_BMS.BErr = Insulation;
				
				STEP = 2;
			}break;
			case 2://��ѹ����SOC֡
			{
				
				STEP = 0;
			}break;
			default:break;
		}
		
		if(Board_C_Sta!=0)	
		{
			CAN_Transmit(CAN2, &TxMsg_ABC);//ֻҪ������C���ѭ������
			memset(TxMsg_ABC.Data,0,sizeof(TxMsg_ABC.Data));
		}
		osDelay(System_Task_Time);
	}
}


unsigned char count;
static void ABC_Data_Deal(void)
{
	if(RX_Flag.ABC_Data_Rx_Flag)
	{
		RX_Flag.ABC_Data_Rx_Flag = false;
		if(ABC_DATA_RX.ExtId==0xC0000ABC)//�����ͣ/��ͣ��ģ�����
		{
			Board_C_Sta = 0x01;//��C��ͨѶ����
		}
		if(ABC_DATA_RX.ExtId==0xC1000ABC)//��������
		{}
		count = 0;
	}else count++;
	if((count > 50)&&(Board_C_Sta==1))	Board_C_Sta = 0xFF;//ͨѶ���ϣ�5s��δ�յ�C�����ݣ�
	
	if(Board_C_Sta != 0x01)	Type_DM.DErr = Disconnect_C;//û������C��
}

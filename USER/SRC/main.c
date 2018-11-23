#include "main.h"
#include "can.h"
#include "bms.h"
#include "lcd.h"
#include "adc.h"

osThreadId MAIN_ID;//任务ID
osThreadId	System_Task_ID;
osThreadId	BMS_Task_ID;
osThreadId	ACDC_Module_Task_ID;

//线程定义结构体:任务名、优先级、缺省、任务栈空间（byte）
osThreadDef(System_Task, 	osPriorityHigh, 1, 300);
osThreadDef(BMS_Task, 		osPriorityHigh, 1, 600); 
osThreadDef(ACDC_Module_Task, osPriorityNormal, 1, 200);

#define TIMER1_Delay	50U//ms
osTimerId TIMER1_ID;//软定时器ID
osTimerDef (Timer1, Timer1_Callback);//软定时器结构体：定时器名、对应的回调函数


unsigned char Board_Type = 0x0A;//默认为A板
int main(void)
{
	Bsp_init();						//初始化板级设备	
	osKernelInitialize();	//初始化RTX系统
	System_Task_ID 			= osThreadCreate(osThread(System_Task), NULL);			//创建主任务
	BMS_Task_ID					= osThreadCreate(osThread(BMS_Task), NULL);					//创建汽车BMS通讯任务
	ACDC_Module_Task_ID	= osThreadCreate(osThread(ACDC_Module_Task), NULL);	//创建交流-直流电源模块通讯任务
	osKernelStart();    	//开始任务创建以上任务
	
	TIMER1_ID = osTimerCreate(osTimer(Timer1), osTimerPeriodic, NULL);
	osTimerStart(TIMER1_ID, TIMER1_Delay);	//开启软定时器TIMER1
	
	MAIN_ID = osThreadGetId();
	osThreadTerminate(MAIN_ID);	//删除MAIN任务
}


uint16_t kkk;
static void Timer1_Callback(void const *arg)
{	
	//内部看门狗喂狗	RUN_LED 500ms反转一次
	kkk = (kkk+1)%(500/TIMER1_Delay);
	if(kkk == 1)
	{
		if(Board_Type == 0X0A)	GPIO_PinWrite(LED_BOARD_PORT,LED_RUN_PIN,!((LED_BOARD_PORT->IDR >> LED_RUN_PIN) & 1));
			else GPIO_PinWrite(LED_BOARD_PORT,LED3_PIN,!((LED_BOARD_PORT->IDR >> LED3_PIN) & 1));
		IWDG_ReloadCounter();
	}
	//读取所有IO输入信号并滤波
	DI_Ack.JT 	 = DI_Status_Check(&DI_Filter.JT,		((JT_PORT->IDR >> JT_PIN) & 1));
	DI_Ack.START = DI_Status_Check(&DI_Filter.START,((START_PORT->IDR >> START_PIN) & 1));
	DI_Ack.GUN   = DI_Status_Check(&DI_Filter.GUN,	((K_GUN_ACK_PORT->IDR >> K_GUN_ACK_PIN) & 1));
	DI_Ack.LOCK  = DI_Status_Check(&DI_Filter.LOCK,	((LOCK_ACK_PORT->IDR >> LOCK_ACK_PIN) & 1));
	DI_Ack.KK_H  = DI_Status_Check(&DI_Filter.KK_H,	((KK_ACK_PORT->IDR >> KK_ACK_PIN1) & 1));
	DI_Ack.KK_L  = DI_Status_Check(&DI_Filter.KK_L,	((KK_ACK_PORT->IDR >> KK_ACK_PIN2) & 1));
	//计算AD采样值
	Get_Adc_Status();
}	


Bms_Type	Type_BMS;
VolCur_Type	Type_VolCur;
Device_Module_Type Type_DM;
Control_Type	Type_Control_Cmd;
CanTxMsg TxMsg_ABC = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//扩展帧 数据帧
unsigned char Board_C_Sta = 0;//0:C板不存在 1:C板通讯正常 0xFF:C板通讯超时
//处理AB板数据上报到C板
char gg;
void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 75U;
	static unsigned char t,STEP;
	
//	if(Check_PE())		Type_DM.DErr = Geodesic;//接地故障
	if(GPIO_PinRead(DIP_SWITCH_PORT1,DIP_SWITCH_PIN1)==0)	{Board_Type  = 0X0B;	Type_DM.DErr = 0;}//读取拨码开关地址:默认A	接地检查由A板检查	
	if((DI_Ack.GUN&DI_Ack.KK_H&DI_Ack.KK_H)!=0)	Type_DM.DErr = Relay_Err;//继电器无法断开
	
	while(1)
	{
				//if(gg)	{ Start_Insulation_Check();gg=0;}
		
		if(DI_Ack.JT == 0)	Type_DM.JiTing = 1;//急停按钮按下
			else if(DI_Ack.JT == 1)Type_DM.JiTing = 0;
		
		ABC_Data_Deal(System_Task_Time);//解析C板数据
		switch(STEP)
		{
			case 0://桩状态/模块状态帧
			{
				if(Type_DM.DErr != 0)	Type_DM.DSta = 2;//桩故障:模块个数//电表通讯错误//ABC失联
					else if((BMS_STA == BEGIN)||(BMS_STA == STOP))	Type_DM.DSta = 0;//桩待机
						else Type_DM.DSta = 1;//桩充电流程中
				memcpy(TxMsg_ABC.Data,&Type_DM,sizeof(Type_DM));
				STEP = 1;
			}break;
			case 1://BMS状态帧
			{
				Type_BMS.Step = BMS_STA;
				if(Data_6656.ChargStopChargingReason==0)	Type_BMS.Stop_Reason = Time_Out;//超时重连超过3次停止
					else Type_BMS.Stop_Reason = Data_6656.ChargStopChargingReason;
				if(Data_7936.IdentifyTimeOut == 0x01)	Type_BMS.time_out = BRM512_Timeout;
					else if(Data_7936.ChargingParamTimeOut&0x01) Type_BMS.time_out = BCP1536_Timeout;
						else if(Data_7936.ChargingParamTimeOut&0x04)	Type_BMS.time_out = BRO2304_Timeout;
							else if(Data_7936.BMSChargingStaTimeOut&0x01) Type_BMS.time_out = BCS4352_Timeout;
								else if(Data_7936.BMSChargingStaTimeOut&0x01) Type_BMS.time_out = BCL4096_Timeout;
				Type_BMS.DErr = guzhang;			
				if(Data_6400.BMSFaultReason&0x03)	Type_BMS.BErr = Insulation;//绝缘故障
					else if(Data_6400.BMSFaultReason&0x3c)	Type_BMS.BErr = BmsOutNetTemp;//BMS元件/输出连接器过温(2合1)
						else if(Data_6400.BMSFaultReason&0xc0)	Type_BMS.BErr = ChargeNet;//充电连接器故障
							else if(Data_6400.BMSFaultReason&0x0300)	Type_BMS.BErr = BatTemp;//电池组温度过高
								else if(Data_6400.BMSFaultReason&0x0c00)	Type_BMS.BErr = HighRelay;//高压继电器故障
									else if(Data_6400.BMSFaultReason&0x3000)	Type_BMS.BErr = Vol_2;////检查点2电压检查故障
				if(Data_6400.BMSStopChargingReason&0x01)	Type_BMS.BErr = CurOver;//电流过大
					else if(Data_6400.BMSStopChargingReason&0x02)	Type_BMS.BErr = CurUnknown;//电流不可信
						else if(Data_6400.BMSStopChargingReason&0x04)	Type_BMS.BErr = CurOver;//电压异常
							else if(Data_6400.BMSStopChargingReason&0x08)	Type_BMS.BErr = VolUnknown;//电压不可信
				if((Data_6400.BMSStopChargingReason&0x05)||(Data_4352.PreSOC>=98))	Type_BMS.BErr = Soc_Full;//充满停止(达到所需SOC)	
//				if(Type_DM.JiTing == 1)	Type_BMS.Manual = JT;//3种情况在情况发生代码段赋值是否好些？
				STEP = 2;
			}break;
			case 2://电压电流SOC帧
			{
				Type_VolCur.Vol = Data_4608.OutputVolt;//模块测量
				Type_VolCur.Cur =	4000-Data_4608.OutputCurr;
				Type_VolCur.Soc = Data_4352.PreSOC;		 //车反馈
				Type_VolCur.KWh =	305;//30.5kwh
				Type_VolCur.CC  = AD_DATA.CC*10;//4v=40
				STEP = 0;
			}break;
			default:break;
		}
		
		if(Board_C_Sta == 0)//未连接C板才做显示(简易)
		{
			t = (t+1)%(975/System_Task_Time);//显示任务~1s刷新一次
			if(t == 1)	LcdShow();
		}
		else//只要连接了C板就循环发送
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
		if(ABC_DATA_RX.ExtId==0xC0000ABC)//充电启停/暂停，模块分配
		{
			Board_C_Sta = 0x01;//与C板通讯正常
			memcpy(&Type_Control_Cmd,ABC_DATA_RX.Data,sizeof(Type_Control_Cmd));
		}
		if(ABC_DATA_RX.ExtId==0xC1000ABC);//TODO:参数设置
		count = 0;
	}else count++;
	if((count > (5000/Task_Time))&&(Board_C_Sta==1))	
	{
		Board_C_Sta = 0xFF;//通讯故障（5s内未收到C板数据）
		Type_DM.DErr = Disconnect_C;
	}
}

#include "main.h"
#include "can.h"
#include "bms.h"
#include "lcd.h"
#include "adc.h"
#include "acdc_module.h"
#include "electric_meter.h"

osThreadId MAIN_ID;//任务ID
osThreadId	System_Task_ID;
osThreadId	BMS_Task_ID;
osThreadId	ACDC_Module_Task_ID;

//线程定义结构体:任务名、优先级、缺省、任务栈空间（byte）
osThreadDef(System_Task, 	osPriorityHigh, 1, 300);
osThreadDef(BMS_Task, 		osPriorityHigh, 1, 500); 
osThreadDef(ACDC_Module_Task, osPriorityNormal, 1, 200);

#define TIMER1_Delay	50U//ms
osTimerId TIMER1_ID;//软定时器ID
osTimerDef (Timer1, Timer1_Callback);//软定时器结构体：定时器名、对应的回调函数

DIPSwitchBits DIPSwitch = {0};
unsigned char Board_Type = 0x0A;//默认为A板
int main(void)
{
	Bsp_init();						//初始化板级设备	
	osKernelInitialize();	//初始化RTX系统
	System_Task_ID 			= osThreadCreate(osThread(System_Task), NULL);			//创建主任务
	BMS_Task_ID					= osThreadCreate(osThread(BMS_Task), NULL);					//创建汽车BMS通讯任务
	ACDC_Module_Task_ID	= osThreadCreate(osThread(ACDC_Module_Task), NULL);	//创建交流-直流电源模块通讯任务
	osKernelStart();    	//开始以上创建的任务
	
	TIMER1_ID = osTimerCreate(osTimer(Timer1), osTimerPeriodic, NULL);
	osTimerStart(TIMER1_ID, TIMER1_Delay);	//开启软定时器TIMER1
	
	MAIN_ID = osThreadGetId();
	osThreadTerminate(MAIN_ID);	//删除MAIN任务
}


static void Timer1_Callback(void const *arg)
{	
	static char t;
	t = (t+1)%(500/TIMER1_Delay);
	if(t == 1)
	{
		LED_RUN_TOGGLE;//RUN_LED 500ms反转一次
		IWDG_ReloadCounter();//内部看门狗喂狗	
	}
	DI_Ack.JT 	 = DI_Status_Check(&DI_Filter.JT,		((JT_PORT->IDR>>JT_PIN)&1));//读取所有IO输入信号并滤波
	DI_Ack.START = DI_Status_Check(&DI_Filter.START,((START_PORT->IDR>>START_PIN)&1));
	DI_Ack.GUN   = DI_Status_Check(&DI_Filter.GUN,	((K_GUN_ACK_PORT->IDR>>K_GUN_ACK_PIN)&1));
	DI_Ack.LOCK  = DI_Status_Check(&DI_Filter.LOCK,	((LOCK_ACK_PORT->IDR>>LOCK_ACK_PIN)&1));
	DI_Ack.KK_H  = DI_Status_Check(&DI_Filter.KK_H,	((KK_ACK_PORT->IDR>>KK_ACK_PIN1)&1));
	DI_Ack.KK_L  = DI_Status_Check(&DI_Filter.KK_L,	((KK_ACK_PORT->IDR>>KK_ACK_PIN2)&1));
	Get_Adc_Status();//计算AD采样值
	if(DI_Ack.JT == 0)	Type_DM.JiTing = 1;//急停按钮按下
		else if(DI_Ack.JT == 1)	Type_DM.JiTing = 0;
}	


CanTxMsg TxMsg_ABC = {0, 0x0ABC00A0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//扩展帧 数据帧
unsigned char Board_C_Sta = 0;//0:C板不存在 1:C板通讯正常 0xFF:C板通讯超时
//处理AB板数据上报到C板
void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 100U;
	static unsigned char t,STEP;
	/*-拨码开关默认是高电平！*/
	DIPSwitch.Bits_1 = GPIO_PinRead(DIP_SWITCH_PORT1,DIP_SWITCH_PIN1);//1-A板  0-B板
	DIPSwitch.Bits_2 = GPIO_PinRead(DIP_SWITCH_PORT2,DIP_SWITCH_PIN2);//1-750V模块0-500v模块
	DIPSwitch.Bits_3 = GPIO_PinRead(DIP_SWITCH_PORT3,DIP_SWITCH_PIN3);//1开启 0关闭 绝缘检查	
	DIPSwitch.Bits_4 = GPIO_PinRead(DIP_SWITCH_PORT4,DIP_SWITCH_PIN4);//1开启上锁  0关闭上锁
	if(Check_PE())		Type_DM.DErr |= Geodesic;//接地故障
	if(DIPSwitch.Bits_1 != 1)	{Board_Type = 0X0B; TxMsg_ABC.ExtId=0x0ABC00B0; Type_DM.DErr &= ~Geodesic;}//读取拨码开关地址:默认A	接地检查由A板检查
	if(DIPSwitch.Bits_2 == 1) ACDC_MAX_VOL = 7500;	else ACDC_MAX_VOL = 5000;//拨码开关第2决定 1=750V 0=500V
	
	while(1)
	{	
		ABC_Data_Deal_RX(System_Task_Time);//解析C板数据
		STEP = (STEP+1)%3;//0-1-2
		ABC_Data_Deal_TX(STEP);
		
		t = (t+1)%(1000/System_Task_Time);
		if(t == 1)	Read_ElectricMeter_Data();//读电表数据
		if((t == 6)&&(Board_Type==0X0A))	LcdShow();//显示任务1s刷新一次
		
		osDelay(System_Task_Time);
	}
}


Bms_Type	Type_BMS;
VolCur_Type	Type_VolCur;
Device_Module_Type Type_DM;
Control_Type	Type_Control_Cmd;
static void ABC_Data_Deal_TX(unsigned char Sta)
{
	switch(Sta)
	{
		case 0://桩状态/模块状态帧
			if(Type_DM.DErr != 0)	Type_DM.DSta = 2;//桩故障
				else if((BMS_STA == BEGIN)||(BMS_STA == STOP))	Type_DM.DSta = 0;//桩待机
					else Type_DM.DSta = 1;//桩充电流程中
			memcpy(TxMsg_ABC.Data,&Type_DM,sizeof(Type_DM));
		break;
		
		case 1://BMS状态帧
			Type_BMS.Step = BMS_STA;
			Type_BMS.DErr = guzhang;
			if(Data_6656.ChargStopChargingReason==0)	Type_BMS.Stop_Reason = Time_Out;//超时重连超过3次停止
				else Type_BMS.Stop_Reason = Data_6656.ChargStopChargingReason;
			if(Data_7936.IdentifyTimeOut == 0x01)	Type_BMS.time_out = BRM512_Timeout;
				else if(Data_7936.ChargingParamTimeOut&0x01) Type_BMS.time_out = BCP1536_Timeout;
					else if(Data_7936.ChargingParamTimeOut&0x04)	Type_BMS.time_out = BRO2304_Timeout;
						else if(Data_7936.BMSChargingStaTimeOut&0x01) Type_BMS.time_out = BCS4352_Timeout;
							else if(Data_7936.BMSChargingStaTimeOut&0x04) Type_BMS.time_out = BCL4096_Timeout;						
			if(Data_6400.BMSFaultReason&0x03)	Type_BMS.BErr = Insulation;//绝缘故障
				else if(Data_6400.BMSFaultReason&0x3c)	Type_BMS.BErr = BmsOutNetTemp;//BMS元件/输出连接器过温(2合1)
					else if(Data_6400.BMSFaultReason&0xc0)	Type_BMS.BErr = ChargeNet;//充电连接器故障
						else if(Data_6400.BMSFaultReason&0x0300)	Type_BMS.BErr = BatTemp;//电池组温度过高
							else if(Data_6400.BMSFaultReason&0x0c00)	Type_BMS.BErr = HighRelay;//高压继电器故障
								else if(Data_6400.BMSFaultReason&0x3000)	Type_BMS.BErr = Vol_2;////检查点2电压检查故障
			if(Data_6400.BMSErrorReason&0x03)	Type_BMS.BErr = CurUnknown;//电流不可信
					else if(Data_6400.BMSErrorReason&0x0c)	Type_BMS.BErr = VolUnknown;//电压不可信
			if((Data_6400.BMSStopChargingReason&0x05)||(Data_4352.PreSOC>98))	Type_BMS.BErr = Soc_Full;//充满停止（达到电压或者soc？）
			Type_BMS.RemaChargTime = Data_4352.RemaChargTime;//0-600min
			memcpy(TxMsg_ABC.Data,&Type_BMS,sizeof(Type_BMS));
		break;
		
		case 2://电压电流SOC帧
			Type_VolCur.Soc = Data_4352.PreSOC;//车反馈
			Type_VolCur.CC  = AD_DATA.CC*10;	 //4.5v=45				
			if(MeterSta == No_Link)
			{
				Type_VolCur.KWh =	dianliang*10;//无电表使用累加计算电量
				Type_VolCur.Vol = Module_Status.Output_Vol*10;//无电表电压电流使用模块值
				Type_VolCur.Cur =	Module_Status.Output_Cur*10;
			}else 
			{
				Type_VolCur.KWh = (MeterData.kwh_realtime-MeterData.kwh_start)*10;//连上电表使用电表数据
				Type_VolCur.Vol = MeterData.vol*10;
				Type_VolCur.Cur =	MeterData.cur*10;
			}
			memcpy(TxMsg_ABC.Data,&Type_VolCur,sizeof(Type_VolCur));
		break;
		default:break;
	}
	
	if(Board_C_Sta != 0)//只要连接了C板就循环发送
	{
		TxMsg_ABC.ExtId = (TxMsg_ABC.ExtId&0XFFFFFFF0)|Sta;
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
		if(ABC_DATA_RX.ExtId==0x0ABC00C0)//充电启停/暂停，模块分配
		{
			if(++count1 > (1000/Task_Time))	
			{
				Board_C_Sta = 0x01;//与C板通讯正常
				Type_DM.DErr &= ~Disconnect_C;//故障恢复
				LED5_ON;//收到数据灯4亮
			}
			Type_Control_Cmd.KK_Sta = ABC_DATA_RX.Data[3];
			Type_Control_Cmd.Module_Assign = ABC_DATA_RX.Data[2];
			if(Board_Type == 0x0A)	memcpy(&Type_Control_Cmd.CMD,&ABC_DATA_RX.Data[0],1);
			else if(Board_Type == 0x0B)	memcpy(&Type_Control_Cmd.CMD,&ABC_DATA_RX.Data[1],1);
		}
		else if(ABC_DATA_RX.ExtId==0x0ABC00C1);//TODO:参数设置
		count = 0;
	}else {count++;count1 = 0;}
	if((count > (3000/Task_Time))&&(Board_C_Sta==1))	
	{
		Board_C_Sta = 0xFF;//通讯故障（2s内未收到C板数据）
		Type_Control_Cmd.CMD.Start_Stop = false;//关闭充电
		Type_DM.DErr |= Disconnect_C;//只有连接上过C板再断开才会置故障位！
		LED5_OFF;//失联灯4熄灭
	}
	
	if(Board_C_Sta == 0)	
	{
		Type_Control_Cmd.Module_Assign = 0XAA;/*没有连接C板时 A板可以控制所有模块*/
		Type_Control_Cmd.CMD.Account = true;	/*没有连接C板时 置位结算完成*/
	}
}

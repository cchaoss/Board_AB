#include "main.h"
#include "can.h"
#include "bms.h"

osThreadId MAIN_ID;//任务ID
osThreadId	System_Task_ID;
osThreadId	BMS_Task_ID;
osThreadId	ACDC_Module_Task_ID;

//线程定义结构体:任务名、优先级、缺省、任务栈空间（byte）
osThreadDef(System_Task, 	osPriorityHigh, 1, 300);
osThreadDef(BMS_Task, 		osPriorityHigh, 1, 300); 
osThreadDef(ACDC_Module_Task, osPriorityNormal, 1, 300);

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
	kkk = (kkk+1)%(500/TIMER1_Delay);
	if(kkk == 1)
	{
		GPIO_PinWrite(LED_RUN_PORT,LED_RUN_PIN,!GPIO_PinRead(LED_RUN_PORT,LED_RUN_PIN));//RUN_LED 500s反转一次
		IWDG_ReloadCounter();//内部看门狗喂狗！
		GPIO_PinWrite(GPIOD,2,0);GPIO_PinWrite(GPIOD,2,1);//外部硬件开门狗喂狗�
	}
}	


Bms_Type	Type_BMS;
VolCur_Type	Type_VolCur;
Device_Module_Type Type_DM;
Control_Type	Type_Control_Cmd;
CanTxMsg TxMsg_ABC = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//扩展帧 数据帧
unsigned char Board_C_Sta;//0:C板不存在 1:C板通讯正常 0xFF:C板通讯超时
//处理AB板数据上报到C板
void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 75U;
	static unsigned char STEP;
	//Board_Type检查
	//接地检查		{Type_DM.DSta = 2;//桩故障 Type_DM.DErr = Geodesic;//接地检查只对A板}
	//断开S1S2S3S4
	//检查S1S2S3S4状态是否正常{Type_DM.DSta = 2;//桩故障 Type_DM.DErr = Relay_Err;//继电器状态不正常}
	while(1)
	{
		if(0)	Type_DM.JiTing = 1;//检查急停是否按下
			else Type_DM.JiTing = 0;
		ABC_Data_Deal();//解析C板数据
		
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
				if((Data_6400.BMSStopChargingReason&0x05)!=0)	Type_BMS.BErr = Soc_Full;//充满停止(达到所需SOC)
				if(Data_6400.BMSFaultReason&0x01)	Type_BMS.BErr = Insulation;
				
				STEP = 2;
			}break;
			case 2://电压电流SOC帧
			{
				
				STEP = 0;
			}break;
			default:break;
		}
		
		if(Board_C_Sta!=0)	
		{
			CAN_Transmit(CAN2, &TxMsg_ABC);//只要连接了C板就循环发送
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
		if(ABC_DATA_RX.ExtId==0xC0000ABC)//充电启停/暂停，模块分配
		{
			Board_C_Sta = 0x01;//与C板通讯正常
		}
		if(ABC_DATA_RX.ExtId==0xC1000ABC)//参数设置
		{}
		count = 0;
	}else count++;
	if((count > 50)&&(Board_C_Sta==1))	Board_C_Sta = 0xFF;//通讯故障（5s内未收到C板数据）
	
	if(Board_C_Sta != 0x01)	Type_DM.DErr = Disconnect_C;//没有连上C板
}

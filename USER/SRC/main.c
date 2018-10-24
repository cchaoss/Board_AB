#include "main.h"
#include "can.h"
#include "bms.h"

//任务ID
osThreadId MAIN_ID;
osThreadId	System_Task_ID;
osThreadId	BMS_Task_ID;
osThreadId	ACDC_Module_Task_ID;

//线程定义结构体:任务名、优先级、缺省、任务栈空间（byte）
osThreadDef(System_Task, 	osPriorityHigh, 1, 300);
osThreadDef(BMS_Task, 		osPriorityHigh, 1, 300); 
osThreadDef(ACDC_Module_Task, osPriorityNormal, 1, 300); 

//软定时器ID
osTimerId TIMER1_ID;
#define TIMER1_Delay	500U//ms
//软定时器结构体：定时器名、对应的回调函数
osTimerDef (Timer1, Timer1_Callback);   


//note:Reload <=4095
static void IWDG_Init(uint8_t Prescaler ,uint16_t Reload)
{
	IWDG->KR = IWDG_WriteAccess_Enable;
	IWDG->PR = Prescaler;
	IWDG->RLR = Reload;
	IWDG->KR = (uint16_t)0xAAAA;
	IWDG->KR = (uint16_t)0xCCCC;
}


static void Bsp_init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//嵌套向量中断控制器组选择，中断分组
	
	GPIO_PinConfigure(GPIOC,15,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//初始化RUN_LED
	GPIO_PinConfigure(GPIOD,2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//初始化硬件开门狗IO

	BMS_Can_Init();//初始化与BMS通讯的CAN口
	ACDC_Module_Can_Init();//初始化与电源模块通讯的CAN口

	IWDG_Init(IWDG_Prescaler_64 ,1250);//Tout=(4*2^Prescaler*Reload)/40单位:ms 这里是2s溢出
}


int main(void)
{
		
	Bsp_init();//初始化设备	
	osKernelInitialize();	//初始化RTX系统
	
	System_Task_ID 			= osThreadCreate(osThread(System_Task), NULL);//创建主任务
	BMS_Task_ID					= osThreadCreate(osThread(BMS_Task), NULL);//创建汽车BMS通讯任务
	ACDC_Module_Task_ID	= osThreadCreate(osThread(ACDC_Module_Task), NULL);//创建交流-直流电源模块通讯任务
	osKernelStart();    	//开始任务创建以上任务
	
	TIMER1_ID = osTimerCreate(osTimer(Timer1), osTimerPeriodic, NULL);
	osTimerStart(TIMER1_ID, TIMER1_Delay);//开启软定时器TIMER1
	
	MAIN_ID = osThreadGetId();
	osThreadTerminate(MAIN_ID);//删除MAIN任务
}


void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 20U;
	while(1)
	{
		
		
		osDelay(System_Task_Time);
	}
}


uint32_t kkk = 0;
//TIMER1_Delay = 500ms调用一次
static void Timer1_Callback(void const *arg)
{	kkk++;
		GPIO_PinWrite(GPIOC,15,!GPIO_PinRead(GPIOC,15));//RUN_LED 500s反转一次
		IWDG_ReloadCounter();//内部看门狗喂狗！
		GPIO_PinWrite(GPIOD,2,0);GPIO_PinWrite(GPIOD,2,1);//外部硬件开门狗喂狗！	
}	

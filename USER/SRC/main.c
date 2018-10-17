#include "main.h"
#include "can.h"
#include "bms.h"

//����ID
osThreadId MAIN_ID;
osThreadId	System_Task_ID;
osThreadId	BMS_Task_ID;
osThreadId	ACDC_Module_Task_ID;

//�̶߳���ṹ��:�����������ȼ���ȱʡ������ջ�ռ䣨byte��
osThreadDef(System_Task, 	osPriorityHigh, 1, 400);
osThreadDef(BMS_Task, 		osPriorityHigh, 1, 800); 
osThreadDef(ACDC_Module_Task, osPriorityNormal, 1, 400); 


//��ʱ��ID
osTimerId TIMER1_ID;
#define TIMER1_Delay	500U//ms
//��ʱ���ṹ�壺��ʱ��������Ӧ�Ļص�����
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//Ƕ�������жϿ�������ѡ���жϷ���
	
	GPIO_PinConfigure(GPIOC,15,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ��RUN_LED
	GPIO_PinConfigure(GPIOD,2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ��Ӳ�����Ź�IO

	BMS_Can_Init();//��ʼ����BMSͨѶ��CAN��
	ACDC_Module_Can_Init();//��ʼ�����Դģ��ͨѶ��CAN��

	IWDG_Init(IWDG_Prescaler_64 ,1250);//Tout=(4*2^Prescaler*Reload)/40��λ:ms ������2s���
}


int main(void)
{
		
	Bsp_init();//��ʼ���豸	
	osKernelInitialize();	//��ʼ��RTXϵͳ
	
	System_Task_ID 			= osThreadCreate(osThread(System_Task), NULL);//����������
	BMS_Task_ID					= osThreadCreate(osThread(BMS_Task), NULL);//��������BMSͨѶ����
	ACDC_Module_Task_ID	= osThreadCreate(osThread(ACDC_Module_Task), NULL);//��������-ֱ����Դģ��ͨѶ����
	osKernelStart();    	//��ʼ���񴴽���������
	
	TIMER1_ID = osTimerCreate(osTimer(Timer1), osTimerPeriodic, NULL);
	osTimerStart(TIMER1_ID, TIMER1_Delay);//������ʱ��TIMER1
	
	MAIN_ID = osThreadGetId();
	osThreadTerminate(MAIN_ID);//ɾ��MAIN����
}


void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 20U;
	while(1)
	{
		
		
		osDelay(System_Task_Time);
	}
}


//TIMER1_Delay = 500ms����һ��
static void Timer1_Callback(void const *arg)
{
		GPIO_PinWrite(GPIOC,15,!GPIO_PinRead(GPIOC,15));//RUN_LED 500s��תһ��
		IWDG_ReloadCounter();//�ڲ����Ź�ι����
		GPIO_PinWrite(GPIOD,2,0);GPIO_PinWrite(GPIOD,2,1);//�ⲿӲ�����Ź�ι����	
}	

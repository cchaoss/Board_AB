#include "bsp.h"
#include "main.h"
#include "can.h"
#include "lcd.h"

static void DIDO_init(void)
{
	GPIO_PinConfigure(LED_RUN_PORT,LED_RUN_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ��RUN_LED
	GPIO_PinConfigure(GPIOD,2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ��Ӳ�����Ź�IO
}




//note:Reload <=4095
static void IWDG_Init(uint8_t Prescaler ,uint16_t Reload)
{
	IWDG->KR = IWDG_WriteAccess_Enable;
	IWDG->PR = Prescaler;
	IWDG->RLR = Reload;
	IWDG->KR = (uint16_t)0xAAAA;
	IWDG->KR = (uint16_t)0xCCCC;
}


void Bsp_init(void)
{
	//Notes:system_stm32f10x.c 1036���޸ľ�������
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//Ƕ�������жϿ�������ѡ���жϷ���
//	DIDO_init();//��ʼ��Ӳ��IO
//	BMS_Can_Init();//��ʼ����BMSͨѶ��CAN��
//	ACDC_Module_Can_Init();//��ʼ�����Դģ��ͨѶ��CAN��
	
	LCD_USART2_Config(9600);//��ʼ��LCD��Ļ����
	
	IWDG_Init(IWDG_Prescaler_64 ,1250);//Tout=(4*2^Prescaler*Reload)/40��λ:ms ������2s���
}

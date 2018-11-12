#include "bsp.h"
#include "main.h"
#include "can.h"

static void DIDO_init(void)
{
	GPIO_PinConfigure(GPIOC,15,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//初始化RUN_LED
	GPIO_PinConfigure(GPIOD,2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//初始化硬件开门狗IO
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
	//Notes:system_stm32f10x.c 1036行修改晶振配置
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//嵌套向量中断控制器组选择，中断分组
	DIDO_init();//初始化硬件IO
	BMS_Can_Init();//初始化与BMS通讯的CAN口
	ACDC_Module_Can_Init();//初始化与电源模块通讯的CAN口
	IWDG_Init(IWDG_Prescaler_64 ,1250);//Tout=(4*2^Prescaler*Reload)/40单位:ms 这里是2s溢出
}

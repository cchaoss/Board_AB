#include "bsp.h"
#include "main.h"
#include "can.h"
#include "lcd.h"
#include "adc.h"
#include "acdc_module.h"
#include "electric_meter.h"

static void DIDO_init(void)
{
	//LED
	GPIO_PinConfigure(LED_BOARD_PORT,LED_RUN_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//初始化RUN_LED
	GPIO_PinConfigure(LED_BOARD_PORT,LED3_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//初始化LED3
	GPIO_PinConfigure(LED_BOARD_PORT,LED4_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//初始化LED4
	GPIO_PinConfigure(LED_BOARD_PORT,LED5_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//初始化LED5
	GPIO_PinConfigure(LED_CHARGE_PORT,LED_CHARGE_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//初始化充电灯
	GPIO_PinConfigure(LED_GUZHANG_PORT,LED_GUZHANG_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//初始化故障灯
	//检查信号输入DI
	GPIO_PinConfigure(JT_PORT,JT_PIN,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//急停输入:下拉输入
	GPIO_PinConfigure(START_PORT,START_PIN,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//启停输入
	GPIO_PinConfigure(K_GUN_ACK_PORT,K_GUN_ACK_PIN,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//枪继电器反馈输入
	GPIO_PinConfigure(LOCK_ACK_PORT,LOCK_ACK_PIN,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//锁反馈输入
	GPIO_PinConfigure(KK_ACK_PORT,KK_ACK_PIN2,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//中间继电器火线反馈输入
	GPIO_PinConfigure(KK_ACK_PORT,KK_ACK_PIN1,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//中间继电器底线反馈输入

	//控制继电器DO
	GPIO_PinConfigure(LOCK_GUN_PORT,LOCK_GUN_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(LOCK_GUN_PORT,LOCK_GUN_PIN2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//枪锁控制
	GPIO_PinConfigure(K_GUN_PORT,K_GUN_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//枪上继电器控制开关
	GPIO_PinConfigure(KK_PORT,KK_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(KK_PORT,KK_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//中间继电器控制开关
	GPIO_PinConfigure(PE_RELAY_PORT,PE_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//接地检查开关
	GPIO_PinConfigure(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//绝缘检查开关
	GPIO_PinConfigure(XIEFANG_RELAY_PORT,XIEFANG_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//泄放检查开关
	GPIO_PinConfigure(BMS_POWER_RELAY_PORT,BMS_POWER_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//BMS辅助电源开关
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//禁用JTAG功能 作普通引脚
	GPIO_PinConfigure(DIP_SWITCH_PORT1,DIP_SWITCH_PIN1,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//AB板选择：上拉输入
	GPIO_PinConfigure(DIP_SWITCH_PORT2,DIP_SWITCH_PIN2,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//预留的拨码开关输入
	GPIO_PinConfigure(PE_CHECK_PORT,PE_CHECK_PIN,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//接地检测输入口
	
	//上电配置所有开关断开
	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN1,0);
	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,1);
	delay_us(100000);//100ms
	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,0);//打开锁
	GPIO_PinWrite(LED_CHARGE_PORT,LED_CHARGE_PIN,0);
	GPIO_PinWrite(LED_GUZHANG_PORT,LED_GUZHANG_PIN,0);//充电/故障灯熄灭
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,0);			//关闭接地检测relay
	GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,0);
	GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN2,0);//关闭绝缘检查relay
	GPIO_PinWrite(XIEFANG_RELAY_PORT,XIEFANG_RELAY_PIN,0);//关闭泄放检查relay
	GPIO_PinWrite(LED_BOARD_PORT,LED5_PIN,0);//失联灯5默认熄灭
	Open_K1K2;//断开枪上继电器
	Open_K3K4;//关闭辅助电源
	Open_KK;//断开中间继电器
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

unsigned short ChargTime;//充电计费时间，现在是使用 TIM2进行计时，每30S加1
float dianliang;
void Tim2_Init(unsigned short Psc,unsigned short Arr)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM_TimeBaseStructure.TIM_Prescaler= Psc-1;
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period= Arr-1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM2, ENABLE);	
	ChargTime = 0;
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{	
		TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);  	
		ChargTime++;
		dianliang += Module_Status.Output_Vol * Module_Status.Output_Cur / 120 / 1000;//算出已充电量(kwh度) = 电压*电流*时间(小时)/1000
	}
}

DI_Ack_Flags DI_Ack;
DI_Data_Type DI_Filter;
//类似滑动窗口滤波：数据流连续DI_Filter_Size个数据均一致则认为可信
//返回值：0低电平 1高电平 FF无效数据
unsigned char DI_Status_Check(_Filter_TYPE*	Filter, unsigned char GPIOX_Status)
{
	Filter->Buffer[Filter->Count] = GPIOX_Status;
	if(++Filter->Count == DI_Filter_Size)
	{
		Filter->Count = 0;
		Filter->Ready = true;
	}
	if(Filter->Ready)
	{
		for(char i = 0;i<DI_Filter_Size-1;i++)
		{
			if(Filter->Buffer[i] != Filter->Buffer[i+1])	
				return 0XFF;//只要有不一致的值则认为数据不可信
		}
		return Filter->Buffer[0];
	}else return 0XFF;
}

//接地检查:0正常 1接地故障
unsigned char Check_PE(void)
{
	unsigned short PE_SUM;
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,1);//打开接地检查继电器
	for(short i = 0;i<3000;i++)//~600ms
	{
		delay_us(200);
		PE_SUM += GPIO_PinRead(PE_CHECK_PORT,PE_CHECK_PIN);
	}
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,0);//关闭接地检查继电器
	return ((3000-PE_SUM) < 300);//检查到低电平个数>300
}

//误差5%
void delay_us(u32 nTimer)
{
	for(u32 i=0;i<nTimer;i++){
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();//__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();//for语句也需要时间
	}
}

void Bsp_init(void)
{
	//Notes:1.system_stm32f10x.c 1036行修改晶振8821 stm32F10x.h 122行25M改为8M
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//嵌套向量中断控制器组选择，中断分组
	DIDO_init();//初始化硬件IO
	BMS_Can_Init();//初始化与BMS通讯的CAN口
	ACDC_Module_Can_Init();//初始化与电源模块通讯的CAN口
	
	ADCx_Init();
	LCD_UART_Init(435200);//初始化LCD屏幕串口	244800	870400
	METER_UART_Init(2400);//初始化电表485串口 2400 9600
	IWDG_Init(IWDG_Prescaler_64 ,1250);//Tout=(4*2^Prescaler*Reload)/40单位:ms 这里是2s溢出
}

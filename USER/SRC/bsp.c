#include "bsp.h"
#include "main.h"
#include "can.h"
#include "lcd.h"

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
	GPIO_AFConfigure(AFIO_SWJ_JTAG_NO_SW);//禁用JTAG IO口
	GPIO_PinConfigure(DIP_SWITCH_PORT1,DIP_SWITCH_PIN1,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//AB板选择：上拉输入
	GPIO_PinConfigure(DIP_SWITCH_PORT2,DIP_SWITCH_PIN2,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//预留的拨码开关输入
	//控制继电器DO
	GPIO_PinConfigure(LOCK_GUN_PORT,LOCK_GUN_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(LOCK_GUN_PORT,LOCK_GUN_PIN2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//枪锁控制
	GPIO_PinConfigure(K_GUN_PORT,K_GUN_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//枪上继电器控制开关
	GPIO_PinConfigure(KK_PORT,KK_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(KK_PORT,KK_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//中间继电器控制开关
	GPIO_PinConfigure(PE_RELAY_PORT,PE_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//接地检查开关
	GPIO_PinConfigure(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//绝缘检查开关
	GPIO_PinConfigure(XIEFANG_RELAY_PORT,XIEFANG_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//泄放检查开关
	GPIO_PinConfigure(BMS_POWER_RELAY_PORT,BMS_POWER_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//BMS辅助电源开关
	//上电配置所有开关断开
	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN1,0);
	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN1,0);//打开锁
	GPIO_PinWrite(K_GUN_PORT,K_GUN_PIN,0);//关闭枪上继电器
	GPIO_PinWrite(KK_PORT,KK_PIN1,0);
	GPIO_PinWrite(KK_PORT,KK_PIN2,0);//关闭中间继电器
	GPIO_PinWrite(LED_CHARGE_PORT,LED_CHARGE_PIN,0);
	GPIO_PinWrite(LED_GUZHANG_PORT,LED_GUZHANG_PIN,0);//充电/故障灯熄灭
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,0);			//关闭接地检测relay
	GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,0);
	GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN2,0);//关闭绝缘检查relay
	GPIO_PinWrite(XIEFANG_RELAY_PORT,XIEFANG_RELAY_PIN,0);//关闭泄放检查relay
	GPIO_PinWrite(BMS_POWER_RELAY_PORT,BMS_POWER_RELAY_PIN,0);//关闭辅助电源
	/**/
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
	//Notes:1.system_stm32f10x.c 1036行修改晶振8821 stm32F10x.h 122行25M改为8M
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//嵌套向量中断控制器组选择，中断分组
	DIDO_init();//初始化硬件IO
	BMS_Can_Init();//初始化与BMS通讯的CAN口
	ACDC_Module_Can_Init();//初始化与电源模块通讯的CAN口
	
//	LCD_USART2_Config(9600);//初始化LCD屏幕串口
	
	IWDG_Init(IWDG_Prescaler_64 ,1250);//Tout=(4*2^Prescaler*Reload)/40单位:ms 这里是2s溢出
}


DI_Ack_Flags DI_Ack;
DI_Data_Type DI_Filter;
//类似滑动窗口滤波：数据流连续DI_Filter_Size个数据均一致则认为可信
//返回值：0低电平 1高电平 -1无效数据
short DI_Status_Check(_Filter_TYPE	Filter, unsigned char GPIOX_Status)
{
	Filter.Buffer[Filter.Count] = GPIOX_Status;
	if(++Filter.Count == DI_Filter_Size)
	{
		Filter.Count = 0;
		Filter.Ready = true;
	}
	if(Filter.Ready)
	{
		for(char i = 0;i<DI_Filter_Size-1;i++)
		{
			if(Filter.Buffer[i] != Filter.Buffer[i+1])	
				return -1;//只要有不一致的值则认为数据不可信
		}
		if(Filter.Buffer[0]==1)	return 1;
			else return 0;
	}else return -1;
}

//接地检查
unsigned char Check_PE(void)
{
	unsigned short PE_SUM;
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,1);//打开接地检查继电器
	for(short i = 0;i<3000;i++)//~600ms
	{
		delay_us(200);
		PE_SUM += GPIO_PinRead(PE_RELAY_PORT,PE_RELAY_PIN);
	}
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,0);//关闭接地检查继电器
	if((3000-PE_SUM) > 50)//检查到低电平个数50以上
		return 0;//接地正常
	else return 1;//接地故障
}

//误差5%
void delay_us(u32 nTimer)
{
	u32 i=0;
	for(i=0;i<nTimer;i++){
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();//__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();//for语句也需要时间
	}
}

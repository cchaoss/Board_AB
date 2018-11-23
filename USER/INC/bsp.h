#ifndef __BSP_H
#define __BSP_H

#include "main.h"
/*硬件IO口定义*/
//LED
#define LED_BOARD_PORT		GPIOC//板载灯
#define LED_RUN_PIN				6
#define LED3_PIN					7
#define LED4_PIN					8
#define LED5_PIN					9
#define LED_CHARGE_PORT		GPIOB//充电灯
#define LED_CHARGE_PIN		9
#define LED_GUZHANG_PORT	GPIOC//故障灯
#define LED_GUZHANG_PIN		13
//GPIO_IN-ACK
#define DIP_SWITCH_PORT1	GPIOB//拨码开关1
#define DIP_SWITCH_PIN1		3
#define DIP_SWITCH_PORT2	GPIOD//拨码开关2
#define DIP_SWITCH_PIN2		2
#define PE_CHECK_PORT			GPIOB//接地检测输入
#define PE_CHECK_PIN			4
#define JT_PORT						GPIOC//急停按钮
#define JT_PIN						5
#define START_PORT				GPIOA//启停按钮
#define START_PIN					6
#define K_GUN_ACK_PORT		GPIOC//枪继电器反馈
#define K_GUN_ACK_PIN			4
#define LOCK_ACK_PORT			GPIOA//锁反馈
#define LOCK_ACK_PIN			7
#define KK_ACK_PORT				GPIOA//中间继电器反馈
#define KK_ACK_PIN1				5		 //+
#define KK_ACK_PIN2				4		 //-
//GPIO_OUT_RELAY
#define LOCK_GUN_PORT	GPIOB//枪锁控制
#define LOCK_GUN_PIN1	15
#define LOCK_GUN_PIN2	14
#define K_GUN_PORT	GPIOB//枪上继电器控制
#define K_GUN_PIN		2
#define KK_PORT			GPIOB//中间继电器控制
#define KK_PIN1			0		 //+
#define KK_PIN2			1		 //-
#define PE_RELAY_PORT				GPIOB//接地检查开关
#define PE_RELAY_PIN				5
#define JUEYUAN_RELAY_PORT	GPIOB//绝缘检查开关
#define JUYUAN_RELAY_PIN1		6
#define JUYUAN_RELAY_PIN2		7
#define XIEFANG_RELAY_PORT	GPIOB//泄放检查开关
#define XIEFANG_RELAY_PIN		8
#define BMS_POWER_RELAY_PORT	GPIOC//BMS辅助电源开关
#define BMS_POWER_RELAY_PIN		14


#define DI_Filter_Size 3
//DI信号滤波缓存
typedef struct
{
	bool	Ready;
	unsigned char Count;
	unsigned char Buffer[DI_Filter_Size];
}_Filter_TYPE;
typedef struct
{
	_Filter_TYPE JT;
	_Filter_TYPE START;
	_Filter_TYPE GUN;
	_Filter_TYPE LOCK;
	_Filter_TYPE KK_H;//中间继电器反馈
	_Filter_TYPE KK_L;
}DI_Data_Type;
extern DI_Data_Type DI_Filter;
//DI反馈信号状态
typedef struct
{
	unsigned char JT;
	unsigned char START;
	unsigned char GUN;
	unsigned char LOCK;
	unsigned char KK_H;
	unsigned char KK_L;
}DI_Ack_Flags;
extern DI_Ack_Flags DI_Ack;

void delay_us(u32 nTimer);
unsigned char Check_PE(void);
unsigned char DI_Status_Check(_Filter_TYPE*	Filter, unsigned char GPIOX_Status);
void Bsp_init(void);//板级硬件初始化

#endif

#ifndef __BSP_H
#define __BSP_H

#include "main.h"
/*硬件IO口定义*/
//LED
#define LED_BOARD_PORT		GPIOC//板载灯
#define LED_RUN_PIN				15
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

/*LCD-USART2*/
#define LCD_USART_TX_PORT       GPIOA   
#define LCD_USART_TX_PIN        2
#define LCD_USART_RX_PORT       GPIOA
#define LCD_USART_RX_PIN        3

#define LCD_USARTx              USART2
#define LCD_USART_CLK           RCC_APB1Periph_USART2
#define LCD_USART_APBxClkCmd    RCC_APB1PeriphClockCmd	//UART1挂着APB2其他串口挂着APB1

#define	LCD_USART_IRQ           USART2_IRQn
#define	LCD_USART_IRQHandler    USART2_IRQHandler

#define	USART2_TX_DMA_CLK			 	RCC_AHBPeriph_DMA1		//DMA1的时钟
#define	USART2_TX_DMA_CHANNEL		DMA1_Channel7				//串口2 TX对应的DMA请求通道是DMA1的第七通道
#define	USART2_RX_DMA_CLK			 	RCC_AHBPeriph_DMA1		//DMA1的时钟
#define	USART2_RX_DMA_CHANNEL  	DMA1_Channel6				//串口2 RX对应的DMA请求通道是DMA1的第六通道
#define USART2_DR_ADDRESS       (u32)(&(USART2->DR))	// 外设寄存器地址---串口2
/**/



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
	char JT;
	char START;
	char GUN;
	char LOCK;
	char KK_H;
	char KK_L;
}DI_Ack_Flags;
extern DI_Ack_Flags DI_Ack;

void delay_us(u32 nTimer);
unsigned char Check_PE(void);
short DI_Status_Check(_Filter_TYPE	Filter, unsigned char GPIOX_Status);
void Bsp_init(void);//板级硬件初始化

#endif

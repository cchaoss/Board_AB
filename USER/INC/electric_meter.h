#ifndef __485_H
#define	__485_H

#include "main.h"

/*485收发控制引脚*/
#define RS485_DE_GPIO_PORT	GPIOA
#define RS485_DE_PIN				GPIO_Pin_8

#define RS485_RX_EN()			GPIO_ResetBits(RS485_DE_GPIO_PORT,RS485_DE_PIN);//进入接收模式
#define RS485_TX_EN()			GPIO_SetBits(RS485_DE_GPIO_PORT,RS485_DE_PIN);	//进入发送模式

//充电桩的电表连接状态
enum ElecMeter_status
{
	Link_Start,			//断线状态
	Link_Ok,	//发送电表地址
	WaitData,	//等待电表数据状态
	AskData,	//请求电表数据(需等待一定时间后才能获取)
};

typedef struct
{
	uint8_t u8DI0;
	uint8_t u8DI1;
	uint8_t u8DI2;
	uint8_t u8DI3;

	uint8_t u8Len;//指令长度
	float ratio;//转化比率
}Ammeter;//电表指令数据结构体


void Deal_YaDa(void);
void METER_UART_Init(uint32_t bound);

#endif

#ifndef __ELECTRIC_METER_H
#define	__ELECTRIC_METER_H

#include "main.h"

/*485收发控制引脚*/
#define RS485_RX_EN()		GPIO_ResetBits(GPIOA,GPIO_Pin_8)	//进入接收模式
#define RS485_TX_EN()		GPIO_SetBits(GPIOA,GPIO_Pin_8)	//进入发送模式

enum _ElecMeter_status
{
	No_Link,		//断线状态
	ReadData,		//发送电表地址
	WaitData,		//等待电表数据状态
	AskData,		//请求电表数据(需等待一定时间后才能获取)
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

typedef struct
{
	float kwh;
	float vol;
	float cur;
}Meter_Data_Type;
extern Meter_Data_Type MeterData;

void Read_ElectricMeter_Data(void);
void METER_UART_Init(uint32_t bound);

#endif

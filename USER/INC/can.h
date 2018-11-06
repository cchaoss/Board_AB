#ifndef __CAN_H
#define __CAN_H

#include "GPIO_STM32F10x.h"
#include "stm32f10x_can.h"

typedef struct
{
	char	BMS_Rx_Flag1:1;
	char  ACDC_Rx_Flag:1;
	char 	ABC_Data_Rx_Flag:1;
	char	reserved:5;
}Can_Rx_FlagBits;
extern Can_Rx_FlagBits	RX_Flag;//CAN接受报文分类标记
extern CanRxMsg BMS_RX_0;//多包接受
extern CanRxMsg BMS_RX_1;//单包接受
extern CanRxMsg ACDC_RX;//充电模块应答接受
extern CanRxMsg ABC_DATA_RX;


void BMS_Can_Init(void);
void ACDC_Module_Can_Init(void);

#endif

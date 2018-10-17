#ifndef __CAN_H
#define __CAN_H

#include "GPIO_STM32F10x.h"
#include "stm32f10x_can.h"

extern CanRxMsg RxMsg1;
extern unsigned char BMS_Recevie_Flag;

void BMS_Can_Init(void);
void ACDC_Module_Can_Init(void);

unsigned char Can_Send_Msg(CAN_TypeDef* CANx, CanTxMsg* TxMessage);

#endif

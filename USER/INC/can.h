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
extern Can_Rx_FlagBits	RX_Flag;//CAN���ܱ��ķ�����
extern CanRxMsg BMS_RX_0;//�������
extern CanRxMsg BMS_RX_1;//��������
extern CanRxMsg ADCD_RX;//���ģ��Ӧ�����


void BMS_Can_Init(void);
void ACDC_Module_Can_Init(void);

#endif

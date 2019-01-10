#ifndef __LCD_H
#define __LCD_H

#include "main.h"
#include "bms.h"

typedef struct
{
	char Lcd_Rx_Flag:1;
	char Meter_Rx_Flag:1;
	char reserved:6;
}Uart_Rx_FlagBits;

typedef struct
{
	uint16_t P0[2];
	uint16_t P0_CC[2];
	uint16_t P1_V[2];
	uint16_t P1_A[2];
	uint16_t P1_Soc[2];
	uint16_t P1_KW[2];
	uint16_t P1_Time[2];
	uint16_t P2_Type[2];
	uint16_t P2_Exp[2];
	uint16_t P2_D[3];
	uint16_t A_M[3];
	uint16_t Sta[4];
}Display_Position;


extern Uart_Rx_FlagBits Uart_Flag;
void LCD_UART_Init(uint32_t bound);
void LcdShow(void);
#endif

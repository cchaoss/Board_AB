#ifndef __LCD_H
#define __LCD_H

#include "main.h"
#include "bms.h"

typedef struct
{
	char Lcd_Rx_Flag:1;
	char	reserved:7;
}Uart_Rx_FlagBits;

typedef struct
{
	uint16_t P0[2];
	uint16_t P1_V[2];
	uint16_t P1_A[2];
	uint16_t P1_Soc[2];
	uint16_t P1_KW[2];
	uint16_t P1_Time[2];
	uint16_t P2_STOP[2];
	uint16_t P2_Exp[2];
	uint16_t P3_Err[2];
}Display_Position;




extern void LCD_USART2_Config(u32 bound);
extern void LcdShow(void);
#endif

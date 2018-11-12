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


/* 
 * 串口宏定义，不同的串口挂载的总线和IO不一样，移植时需要修改这几个宏
 * 1-修改总线时钟的宏，uart1挂载到apb2总线，其他uart挂载到apb1总线
 * 2-修改GPIO的宏
 */
/****************************************************** 串口2-USART2 ******************************************************/
#define  LCD_USARTx                   USART2
#define  LCD_USART_CLK                RCC_APB1Periph_USART2
#define  LCD_USART_APBxClkCmd         RCC_APB1PeriphClockCmd
//#define  LCD_USART_BAUDRATE           115200

// USART2 GPIO 引脚宏定义
#define  LCD_USART_GPIO_CLK           (RCC_APB2Periph_GPIOA)
#define  LCD_USART_GPIO_APBxClkCmd    RCC_APB2PeriphClockCmd
    
#define  LCD_USART_TX_GPIO_PORT       GPIOA   
#define  LCD_USART_TX_GPIO_PIN        GPIO_Pin_2
#define  LCD_USART_RX_GPIO_PORT       GPIOA
#define  LCD_USART_RX_GPIO_PIN        GPIO_Pin_3

#define  LCD_USART_IRQ                USART2_IRQn
#define  LCD_USART_IRQHandler         USART2_IRQHandler


/* 
 * 串口对应DMA的宏定义，不同的串口对应的DMA不同，串口1、2、3在DMA1上，串口4在DMA2上
 */
/************************************************** 串口2DMA配置相关的宏 **************************************************/
//串口2 TX对应的DMA请求通道是DMA1的第七通道
#define		USART2_TX_DMA_CLK				RCC_AHBPeriph_DMA1		//DMA1的时钟
#define		USART2_TX_DMA_CHANNEL		DMA1_Channel7

//串口2 RX对应的DMA请求通道是DMA1的第六通道
#define		USART2_RX_DMA_CLK				RCC_AHBPeriph_DMA1		//DMA1的时钟
#define		USART2_RX_DMA_CHANNEL   DMA1_Channel6

// 外设寄存器地址---串口2
#define  USART2_DR_ADDRESS        (u32)(&(USART2->DR))




extern void LCD_USART2_Config(u32 bound);
extern void USART2_DMA_receive(void);
extern void LcdShow(void);
#endif

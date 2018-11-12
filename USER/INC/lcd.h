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
 * ���ں궨�壬��ͬ�Ĵ��ڹ��ص����ߺ�IO��һ������ֲʱ��Ҫ�޸��⼸����
 * 1-�޸�����ʱ�ӵĺ꣬uart1���ص�apb2���ߣ�����uart���ص�apb1����
 * 2-�޸�GPIO�ĺ�
 */
/****************************************************** ����2-USART2 ******************************************************/
#define  LCD_USARTx                   USART2
#define  LCD_USART_CLK                RCC_APB1Periph_USART2
#define  LCD_USART_APBxClkCmd         RCC_APB1PeriphClockCmd
//#define  LCD_USART_BAUDRATE           115200

// USART2 GPIO ���ź궨��
#define  LCD_USART_GPIO_CLK           (RCC_APB2Periph_GPIOA)
#define  LCD_USART_GPIO_APBxClkCmd    RCC_APB2PeriphClockCmd
    
#define  LCD_USART_TX_GPIO_PORT       GPIOA   
#define  LCD_USART_TX_GPIO_PIN        GPIO_Pin_2
#define  LCD_USART_RX_GPIO_PORT       GPIOA
#define  LCD_USART_RX_GPIO_PIN        GPIO_Pin_3

#define  LCD_USART_IRQ                USART2_IRQn
#define  LCD_USART_IRQHandler         USART2_IRQHandler


/* 
 * ���ڶ�ӦDMA�ĺ궨�壬��ͬ�Ĵ��ڶ�Ӧ��DMA��ͬ������1��2��3��DMA1�ϣ�����4��DMA2��
 */
/************************************************** ����2DMA������صĺ� **************************************************/
//����2 TX��Ӧ��DMA����ͨ����DMA1�ĵ���ͨ��
#define		USART2_TX_DMA_CLK				RCC_AHBPeriph_DMA1		//DMA1��ʱ��
#define		USART2_TX_DMA_CHANNEL		DMA1_Channel7

//����2 RX��Ӧ��DMA����ͨ����DMA1�ĵ���ͨ��
#define		USART2_RX_DMA_CLK				RCC_AHBPeriph_DMA1		//DMA1��ʱ��
#define		USART2_RX_DMA_CHANNEL   DMA1_Channel6

// ����Ĵ�����ַ---����2
#define  USART2_DR_ADDRESS        (u32)(&(USART2->DR))




extern void LCD_USART2_Config(u32 bound);
extern void USART2_DMA_receive(void);
extern void LcdShow(void);
#endif

#ifndef __BSP_H
#define __BSP_H

/*硬件IO口定义*/
#define BMS_RX_PORT		GPIOA
#define BMS_RX_PIN		11
#define BMS_TX_PORT		GPIOA
#define BMS_TX_PIN		12

#define ACDC_RX_PORT	GPIOB
#define ACDC_RX_PIN		12
#define	ACDC_TX_PORT	GPIOB
#define ACDC_TX_PIN		13

#define LED_RUN_PORT	GPIOC
#define LED_RUN_PIN		15

//LCD-USART2
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



void Bsp_init(void);//板级硬件初始化

#endif

#ifndef __485_H
#define	__485_H

#include "main.h"

#define  RS485_USARTx                   USART3
#define  RS485_USART_CLK                RCC_APB1Periph_USART3
#define  RS485_USART_APBxClkCmd         RCC_APB1PeriphClockCmd
// USART3 GPIO 引脚宏定义
#define  RS485_USART_GPIO_CLK           (RCC_APB2Periph_GPIOB)
#define  RS485_USART_GPIO_APBxClkCmd    RCC_APB2PeriphClockCmd

#define  RS485_USART_TX_GPIO_PORT       GPIOB
#define  RS485_USART_TX_GPIO_PIN        GPIO_Pin_10
#define  RS485_USART_RX_GPIO_PORT       GPIOB
#define  RS485_USART_RX_GPIO_PIN        GPIO_Pin_11
#define  RS485_USART_IRQ                USART3_IRQn
#define  RS485_USART_IRQHandler         USART3_IRQHandler

/*一个收发控制脚PB6***************************************/
/*485收发控制引脚*/
#define RS485_DE_GPIO_PORT							GPIOA
#define RS485_DE_GPIO_CLK								RCC_APB2Periph_GPIOA
#define RS485_DE_PIN										GPIO_Pin_8
/*控制收发引脚-------PX修改*/
//进入接收模式
#define RS485_RX_EN()			GPIO_ResetBits(RS485_DE_GPIO_PORT,RS485_DE_PIN);
//进入发送模式
#define RS485_TX_EN()			GPIO_SetBits(RS485_DE_GPIO_PORT,RS485_DE_PIN);

//串口对应DMA的宏定义，不同的串口对应的DMA不同，串口1、2、3在DMA1上，串口4在DMA2上
//串口3TX对应的DMA请求通道是DMA1的第二通道
#define		USART3_TX_DMA_CLK				RCC_AHBPeriph_DMA1		//DMA1的时钟
#define		USART3_TX_DMA_CHANNEL   DMA1_Channel2

//串口3RX对应的DMA请求通道是DMA1的第三通道
#define		USART3_RX_DMA_CLK				RCC_AHBPeriph_DMA1		//DMA1的时钟
#define		USART3_RX_DMA_CHANNEL   DMA1_Channel3

//外设寄存器地址---串口3
#define  USART3_DR_ADDRESS        (u32)(&(USART3->DR))


//串口3 TX对应的中断
#define  USART3_TX_DMA_IRQ                DMA1_Channel2_IRQn
#define  USART3_TX_DMA_IRQHandler         DMA1_Channel2_IRQHandler

//串口3 TX对应的DMA传输完成标识位
#define  USART3_TX_DMA_FLAG       DMA1_FLAG_TC2






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
void USART3_DMA_receive(void);
void RS485_Config(u32 bound);
void RS485_DMA_send(u8 *SendBuff,u32 size);

#endif

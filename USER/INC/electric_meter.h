#ifndef __485_H
#define	__485_H

#include "main.h"

#define  RS485_USARTx                   USART3
#define  RS485_USART_CLK                RCC_APB1Periph_USART3
#define  RS485_USART_APBxClkCmd         RCC_APB1PeriphClockCmd
// USART3 GPIO ���ź궨��
#define  RS485_USART_GPIO_CLK           (RCC_APB2Periph_GPIOB)
#define  RS485_USART_GPIO_APBxClkCmd    RCC_APB2PeriphClockCmd

#define  RS485_USART_TX_GPIO_PORT       GPIOB
#define  RS485_USART_TX_GPIO_PIN        GPIO_Pin_10
#define  RS485_USART_RX_GPIO_PORT       GPIOB
#define  RS485_USART_RX_GPIO_PIN        GPIO_Pin_11
#define  RS485_USART_IRQ                USART3_IRQn
#define  RS485_USART_IRQHandler         USART3_IRQHandler

/*һ���շ����ƽ�PB6***************************************/
/*485�շ���������*/
#define RS485_DE_GPIO_PORT							GPIOA
#define RS485_DE_GPIO_CLK								RCC_APB2Periph_GPIOA
#define RS485_DE_PIN										GPIO_Pin_8
/*�����շ�����-------PX�޸�*/
//�������ģʽ
#define RS485_RX_EN()			GPIO_ResetBits(RS485_DE_GPIO_PORT,RS485_DE_PIN);
//���뷢��ģʽ
#define RS485_TX_EN()			GPIO_SetBits(RS485_DE_GPIO_PORT,RS485_DE_PIN);

//���ڶ�ӦDMA�ĺ궨�壬��ͬ�Ĵ��ڶ�Ӧ��DMA��ͬ������1��2��3��DMA1�ϣ�����4��DMA2��
//����3TX��Ӧ��DMA����ͨ����DMA1�ĵڶ�ͨ��
#define		USART3_TX_DMA_CLK				RCC_AHBPeriph_DMA1		//DMA1��ʱ��
#define		USART3_TX_DMA_CHANNEL   DMA1_Channel2

//����3RX��Ӧ��DMA����ͨ����DMA1�ĵ���ͨ��
#define		USART3_RX_DMA_CLK				RCC_AHBPeriph_DMA1		//DMA1��ʱ��
#define		USART3_RX_DMA_CHANNEL   DMA1_Channel3

//����Ĵ�����ַ---����3
#define  USART3_DR_ADDRESS        (u32)(&(USART3->DR))


//����3 TX��Ӧ���ж�
#define  USART3_TX_DMA_IRQ                DMA1_Channel2_IRQn
#define  USART3_TX_DMA_IRQHandler         DMA1_Channel2_IRQHandler

//����3 TX��Ӧ��DMA������ɱ�ʶλ
#define  USART3_TX_DMA_FLAG       DMA1_FLAG_TC2






//���׮�ĵ������״̬
enum ElecMeter_status
{
	Link_Start,			//����״̬
	Link_Ok,	//���͵���ַ
	WaitData,	//�ȴ��������״̬
	AskData,	//����������(��ȴ�һ��ʱ�����ܻ�ȡ)
};

typedef struct
{
	uint8_t u8DI0;
	uint8_t u8DI1;
	uint8_t u8DI2;
	uint8_t u8DI3;

	uint8_t u8Len;//ָ���
	float ratio;//ת������
}Ammeter;//���ָ�����ݽṹ��


void Deal_YaDa(void);
void USART3_DMA_receive(void);
void RS485_Config(u32 bound);
void RS485_DMA_send(u8 *SendBuff,u32 size);

#endif

#ifndef __485_H
#define	__485_H

#include "main.h"

/*485�շ���������*/
#define RS485_DE_GPIO_PORT	GPIOA
#define RS485_DE_PIN				GPIO_Pin_8

#define RS485_RX_EN()			GPIO_ResetBits(RS485_DE_GPIO_PORT,RS485_DE_PIN);//�������ģʽ
#define RS485_TX_EN()			GPIO_SetBits(RS485_DE_GPIO_PORT,RS485_DE_PIN);	//���뷢��ģʽ

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
void METER_UART_Init(uint32_t bound);

#endif

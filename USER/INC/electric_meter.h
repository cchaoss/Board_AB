#ifndef __ELECTRIC_METER_H
#define	__ELECTRIC_METER_H

#include "main.h"

/*485�շ���������*/
#define RS485_RX_EN()		GPIO_ResetBits(GPIOA,GPIO_Pin_8)	//�������ģʽ
#define RS485_TX_EN()		GPIO_SetBits(GPIOA,GPIO_Pin_8)	//���뷢��ģʽ

enum _ElecMeter_status
{
	No_Link,		//����״̬
	ReadData,		//���͵���ַ
	WaitData,		//�ȴ��������״̬
	AskData,		//����������(��ȴ�һ��ʱ�����ܻ�ȡ)
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

typedef struct
{
	float kwh;
	float vol;
	float cur;
}Meter_Data_Type;
extern Meter_Data_Type MeterData;

void Read_ElectricMeter_Data(void);
void METER_UART_Init(uint32_t bound);

#endif

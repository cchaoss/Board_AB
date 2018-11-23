#ifndef __BSP_H
#define __BSP_H

#include "main.h"
/*Ӳ��IO�ڶ���*/
//LED
#define LED_BOARD_PORT		GPIOC//���ص�
#define LED_RUN_PIN				6
#define LED3_PIN					7
#define LED4_PIN					8
#define LED5_PIN					9
#define LED_CHARGE_PORT		GPIOB//����
#define LED_CHARGE_PIN		9
#define LED_GUZHANG_PORT	GPIOC//���ϵ�
#define LED_GUZHANG_PIN		13
//GPIO_IN-ACK
#define DIP_SWITCH_PORT1	GPIOB//���뿪��1
#define DIP_SWITCH_PIN1		3
#define DIP_SWITCH_PORT2	GPIOD//���뿪��2
#define DIP_SWITCH_PIN2		2
#define PE_CHECK_PORT			GPIOB//�ӵؼ������
#define PE_CHECK_PIN			4
#define JT_PORT						GPIOC//��ͣ��ť
#define JT_PIN						5
#define START_PORT				GPIOA//��ͣ��ť
#define START_PIN					6
#define K_GUN_ACK_PORT		GPIOC//ǹ�̵�������
#define K_GUN_ACK_PIN			4
#define LOCK_ACK_PORT			GPIOA//������
#define LOCK_ACK_PIN			7
#define KK_ACK_PORT				GPIOA//�м�̵�������
#define KK_ACK_PIN1				5		 //+
#define KK_ACK_PIN2				4		 //-
//GPIO_OUT_RELAY
#define LOCK_GUN_PORT	GPIOB//ǹ������
#define LOCK_GUN_PIN1	15
#define LOCK_GUN_PIN2	14
#define K_GUN_PORT	GPIOB//ǹ�ϼ̵�������
#define K_GUN_PIN		2
#define KK_PORT			GPIOB//�м�̵�������
#define KK_PIN1			0		 //+
#define KK_PIN2			1		 //-
#define PE_RELAY_PORT				GPIOB//�ӵؼ�鿪��
#define PE_RELAY_PIN				5
#define JUEYUAN_RELAY_PORT	GPIOB//��Ե��鿪��
#define JUYUAN_RELAY_PIN1		6
#define JUYUAN_RELAY_PIN2		7
#define XIEFANG_RELAY_PORT	GPIOB//й�ż�鿪��
#define XIEFANG_RELAY_PIN		8
#define BMS_POWER_RELAY_PORT	GPIOC//BMS������Դ����
#define BMS_POWER_RELAY_PIN		14


#define DI_Filter_Size 3
//DI�ź��˲�����
typedef struct
{
	bool	Ready;
	unsigned char Count;
	unsigned char Buffer[DI_Filter_Size];
}_Filter_TYPE;
typedef struct
{
	_Filter_TYPE JT;
	_Filter_TYPE START;
	_Filter_TYPE GUN;
	_Filter_TYPE LOCK;
	_Filter_TYPE KK_H;//�м�̵�������
	_Filter_TYPE KK_L;
}DI_Data_Type;
extern DI_Data_Type DI_Filter;
//DI�����ź�״̬
typedef struct
{
	unsigned char JT;
	unsigned char START;
	unsigned char GUN;
	unsigned char LOCK;
	unsigned char KK_H;
	unsigned char KK_L;
}DI_Ack_Flags;
extern DI_Ack_Flags DI_Ack;

void delay_us(u32 nTimer);
unsigned char Check_PE(void);
unsigned char DI_Status_Check(_Filter_TYPE*	Filter, unsigned char GPIOX_Status);
void Bsp_init(void);//�弶Ӳ����ʼ��

#endif
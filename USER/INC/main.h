#ifndef __MAIN_H
#define __MAIN_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stm32f10x.h"                  
#include "cmsis_os.h"
#include "GPIO_STM32F10x.h"
#include "bsp.h"

enum _Device_err
{
	Geodesic = 1<<0,			//�ӵع��ϣ�ֻ��Ҫ���A��*
	Disconnect_C = 1<<1,	//��C��ͨѶ����
	No_Module = 1<<2,			//�޵�Դģ������				 *
	Dc_Table_Err = 1<<3,	//��ǹֱ����������(�����汾)
};
enum _manual_rea
{
	JT_Stop = 1,
	Start_Stop = 2,
	App_Stop = 3,
	Card_Stop = 4,
};
enum _Stop_rea
{
	Condition_Stop = 0x01,		
	Mannul_Stop = 0x04,
	Err_Stop = 0x10,
	BMS_Stop = 0x40,
	Time_Out = 0xFF,
};
enum _Timeout_Bms
{
	BRM512_Timeout = 1,
	BCP1536_Timeout= 2,
	BRO2304_Timeout= 3,
	BCS4352_Timeout= 4,
	BCL4096_Timeout= 5,
	BST6400_Timeout= 6,
	BSD7168_Timeout= 7,
};
enum _guzhang
{
	None = 0,					//�޹���
	Lock_ERR = 1,			//�޷�����
	GUN_Relay_Err = 2,//��ǹ�̵���״̬����
	KK_Relay_Err  = 3,//�м�̵�������
//	Gun_Vol_ERR = 4,//ǹ�˵�ѹ>10V
//	Tap_Check_ERR=5,//й�ż�����
	Insulation_ERR= 6,//��Ե������
	Bat_Vol_ERR 	= 7,//��ص�ѹ��ƥ��
	CC_ERR = 8,			  //CC�źŴ���
	Temp_High = 9,		//ǹ�¶ȹ���
};
enum _Err_Bms
{
	Soc_Full = 1,//�ﵽsoc ��ѹ
	Insulation=2,//��Ե����
	BmsOutNetTemp=4,//BMSԪ��/�������������
	ChargeNet = 5,//�������������
	BatTemp = 6,//������¶ȹ���
	HighRelay = 7,//��ѹ�̵�������
	Vol_2 = 8,//����2��ѹ������
//	CurOver = 9,//��������
	CurUnknown=10,//����������
//	VolErr = 11,//��ѹ�쳣
	VolUnknown=12,//��ѹ������
};

/*A/B<����>C���ݽ���֡����*/
typedef struct
{
	uint8_t JiTing;	//��ͣ��ť1���£�0�ɿ�
	uint8_t DSta;		//׮״̬��0������1����У�2����
	uint8_t DErr;		//׮����ԭ��
	uint8_t MNum;		//A/B����ģ�����
	uint8_t MErr2;	//ģ�����ԭ��sta2
	uint8_t MErr1;	//ģ�����ԭ��sta1
	uint8_t MErr0;	//ģ�����ԭ��sta0
	uint8_t M_Suspend_ACK;//ģ����ͣ���Ӧ��
}Device_Module_Type;
typedef struct
{
	uint8_t Step;				//���� ��ʶ ׼����� ����� ���ֹͣ
	uint8_t Stop_Reason;//ֹͣԭ�򣺳��� ͨѶ��ʱ ���� BMS �˹�
	uint8_t time_out;		//��ʱԭ��
	uint8_t DErr;				//׮���ϣ��� ����ѹ ��Ե DC����ѹ���ص�ѹ<5% �̵�������
	uint8_t BErr;				//BMS��ֹԭ��
	uint8_t Manual;			//�˹���ֹԭ��1��ͣ 2��ͣ 3APP 4ˢ��
	uint8_t Gun_link;		//����״̬ 1��ǹ 0δ����
}Bms_Type;
typedef struct
{
	uint16_t Vol;	//5505=550.5V
	uint16_t Cur;	//1205=120.5A
	uint16_t KWh;	//5505=550.5KW
	uint8_t  Soc;	//88%
}VolCur_Type;
/**C->AB��������**/
typedef struct
{
	char	Start_Stop:2;//0X00�ر�Aǹ��0X01���� 0x02��ͣ���
	char  Suspend:1;	 //0X01ģ����ͣʹ��
	char	Type:1;			 //0x00 APP��ͣ 0x01 ˢ����ͣ
	char	Account:1;	 //������0δ���㣬1�������
	char 	reserved:3;
}Start_Stop_Cmd;
typedef struct
{
	Start_Stop_Cmd CMD;
	uint8_t Module_Assign;//0XAB�����ñ���ģ�飬0XAAȫ��ģ���Aǹ�ã�0XBBȫ��ģ���Bǹ��
	uint8_t KK_Sta;//�м�̵�������״̬ 0�Ͽ� 1�պ�
}Control_Type;
/******************/
typedef struct
{
	char Bits_1:1;//AB���ַ
	char Bits_2:1;
	char Bits_3:1;
	char Bits_4:1;
	char reserved:4;
}DIPSwitchBits;//���뿪��
extern DIPSwitchBits DIPSwitch;
extern Bms_Type	Type_BMS;
extern VolCur_Type	Type_VolCur;
extern Device_Module_Type Type_DM;
extern Control_Type	Type_Control_Cmd;

extern unsigned char Board_Type;//A B�嶨��0X0A 0X0B
extern unsigned char Board_C_Sta;//AB����C������״̬��0������ 1 �������� FFͨѶ��ʱ����
static void ABC_Data_Deal_RX(unsigned short Task_Time);
static void ABC_Data_Deal_TX(void);
static void Timer1_Callback(void const *arg);

void System_Task(void const *argument);
void BMS_Task(void const *argument);                
void ACDC_Module_Task(void const *argument); 

#endif

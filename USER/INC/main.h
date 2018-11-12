#ifndef __MAIN_H
#define __MAIN_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"                  
#include "cmsis_os.h"
#include "GPIO_STM32F10x.h"
#include "bsp.h"

enum _Device_err
{
	Geodesic = 1,			//�ӵع��ϣ�ֻ��Ҫ���A��
	Disconnect_C,			//��C��������
	No_Module,				//�޵�Դģ������
	Relay_Err,				//��ǹ�̵���״̬����
	Dc_Table_Err,			//��ǹֱ����������
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
enum _Err_Bms
{
	Soc_Full = 1,//�ﵽsoc ��ѹ
	Insulation=2,//��Ե����
	OutNetTemp=3,//�������������
	BmsOutNetTemp=4,//BMSԪ���������������
	ChargeNet = 5,//�������������
	BatTemp = 6,//������¶ȹ���
	HighRelay = 7,//��ѹ�̵�������
	Vol_2 = 8,//����2��ѹ������
	CurOver = 9,//��������
	VolErr = 10,//��ѹ�쳣
};

/*A/B<����>C���ݽ���֡����*/
typedef struct
{
	uint8_t JiTing;	//��ͣ��ť1���£�0�ɿ�
	uint8_t DSta;		//׮״̬��0������1����У�2����
	uint8_t DErr;		//׮����ԭ��1�ӵ�,2ģ����� <1, 3S1234�����޷��رգ�4�����ͨѶ
	uint8_t MNum;		//A/B����ģ�����
	uint8_t MErr2;		//ģ�����ԭ��sta2
	uint8_t MErr1;		//ģ�����ԭ��sta1
	uint8_t MErr0;		//ģ�����ԭ��sta0
}Device_Module_Type;
typedef struct
{
	uint8_t Step;				//���� ��ʶ ׼����� ����� ���ֹͣ
	uint8_t Stop_Reason;//ֹͣԭ�򣺳��� ͨѶ��ʱ ���� BMS �˹�
	uint8_t time_out;		//��ʱԭ��
	uint8_t DErr;				//׮���ϣ��� ����ѹ ��Ե DC����ѹ���ص�ѹ<5%
	uint8_t BErr;				//BMS��ֹԭ��
	uint8_t Manual;			//�˹���ֹԭ�򣺼�ͣ ˢ�� APP
}Bms_Type;
typedef struct
{
	uint16_t Vol;	//5505=550.5V
	uint16_t Cur;	//1205=120.5A
	uint8_t  Soc;	//88%
}VolCur_Type;
typedef struct
{
	uint8_t A_Start_Stop;//0X00�ر�Aǹ��0X01��ͣ��0X02����
	uint8_t B_Start_Stop;//0X00�ر�Bǹ��0X01��ͣ��0X02����
	uint8_t Module_Assign;//0XAB�����ñ���ģ�飬0XAAȫ��ģ���Aǹ�ã�0XBBȫ��ģ���Bǹ��
}Control_Type;
extern Device_Module_Type Type_DM;
extern Bms_Type	Type_BMS;
extern VolCur_Type	Type_VolCur;
extern Control_Type	Type_Control_C;

extern unsigned char Board_Type;//A B�嶨��

static void ABC_Data_Deal(void);
static void Timer1_Callback(void const *arg);
void System_Task(void const *argument);
void BMS_Task(void const *argument);                
void ACDC_Module_Task(void const *argument); 

#endif


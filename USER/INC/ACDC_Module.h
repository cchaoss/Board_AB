#ifndef __ACDC_MODULE_H
#define __ACDC_MODULE_H

#include "main.h"

enum _ACDC_STA
{
	Set_Group 	= 0,
	Group_Verify= 1,
	Read_Status	= 2,
	Read_Vol_Cur= 3,
	Set_Vol_Cur = 4,
};
typedef struct
{
	char All_num	:1;
	char A_num		:1;
	char B_num		:1;
	char Vol_cur	:1;
	char Temp_sta	:1;
	char ON_OFF_STA:1;
	char Reserved :2;
}Module_Rx_Flag_Bits;

typedef struct
{
	float Output_Vol;
	float Output_Cur;
	uint8_t num;
	uint8_t	numA;
	uint8_t numB;
}ACDC_Status;

typedef struct
{
	uint8_t reserved[2];
	uint8_t group;
	uint8_t reserve;
	uint8_t temptrue;
	uint8_t sta2;
	uint8_t sta1;
	uint8_t sta0;
}sta_ack_type;

//#define MAX_GROUP_NUM 8//һ��ģ�����8��2*8=16��
#define Total_Number_Read		0x02823FF0U//��ȡ����ģ��������
#define Total_Number_Ack		0x0282F03FU
#define GroupA_Number_Read	0x02C200F0U//����0ģ��������
#define GroupA_Number_Ack		0x02C2F000U
#define GroupB_Number_Read	0x02C201F0U//����1ģ��������
#define GroupB_Number_Ack		0x02C2F001U
#define Total_Vol_Cur_Read	0x02813FF0U//������ģ�������ѹ �ܵ���
#define Total_Vol_Cur_Ack		0x0281F03FU
#define GroupA_Vol_Cur_Read	0x02C100F0U//����0ģ�������ѹ �ܵ���
#define GroupA_Vol_Cur_Ack	0x02C1F000U
#define GroupB_Vol_Cur_Read	0x02C101F0U//����1ģ�������ѹ �ܵ���
#define GroupB_Vol_Cur_Ack	0x02C1F001U
#define Single_Module_Sta_Read	0x028400F0U//��ȡģ��0��š��¶ȡ�״̬
#define Single_Module_Sta_Ack		0x0284F000U

#define Set_Total_Vol_Cur		0x029B3FF0U//��������ģ�������ѹ �ܵ���
#define Set_GroupA_Vol_Cur	0x02DB00F0U//������0������ģ���ѹ �ܵ���
#define Set_GroupB_Vol_Cur	0x02DB01F0U//������1������ģ���ѹ �ܵ���
#define Total_Module_ONOFF	0x029A3FF0U//��������ģ�鿪�ػ�
#define GroupA_Module_ONOFF	0x02DA00F0U//������0����ģ�鿪�ػ�
#define GroupB_Module_ONOFF	0x02DA01F0U//������1����ģ�鿪�ػ�
#define Set_Single_Module_Group	0x029600F0U//����ģ��0�����		

extern uint8_t ACDC_VolCur_Buffer[8];
extern ACDC_Status Module_Status;//��Դģ��ĸ��� �¶� ״̬ ��ѹ����	

static void ACDC_RxMsg_Deal(void);
#endif

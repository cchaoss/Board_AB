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

//#define MAX_GROUP_NUM 8//一组模块最多8个2*8=16个
#define Total_Number_Read		0x02823FF0U//读取所有模块总数量
#define Total_Number_Ack		0x0282F03FU
#define GroupA_Number_Read	0x02C200F0U//读组0模块总数量
#define GroupA_Number_Ack		0x02C2F000U
#define GroupB_Number_Read	0x02C201F0U//读组1模块总数量
#define GroupB_Number_Ack		0x02C2F001U
#define Total_Vol_Cur_Read	0x02813FF0U//读所有模块输出电压 总电流
#define Total_Vol_Cur_Ack		0x0281F03FU
#define GroupA_Vol_Cur_Read	0x02C100F0U//读组0模块输出电压 总电流
#define GroupA_Vol_Cur_Ack	0x02C1F000U
#define GroupB_Vol_Cur_Read	0x02C101F0U//读组1模块输出电压 总电流
#define GroupB_Vol_Cur_Ack	0x02C1F001U
#define Single_Module_Sta_Read	0x028400F0U//读取模块0组号、温度、状态
#define Single_Module_Sta_Ack		0x0284F000U

#define Set_Total_Vol_Cur		0x029B3FF0U//设置所有模块输出电压 总电流
#define Set_GroupA_Vol_Cur	0x02DB00F0U//设置组0下所有模块电压 总电流
#define Set_GroupB_Vol_Cur	0x02DB01F0U//设置组1下所有模块电压 总电流
#define Total_Module_ONOFF	0x029A3FF0U//设置所有模块开关机
#define GroupA_Module_ONOFF	0x02DA00F0U//设置组0所有模块开关机
#define GroupB_Module_ONOFF	0x02DA01F0U//设置组1所有模块开关机
#define Set_Single_Module_Group	0x029600F0U//设置模块0的组号		

extern uint8_t ACDC_VolCur_Buffer[8];
extern ACDC_Status Module_Status;//电源模块的个数 温度 状态 电压电流	

static void ACDC_RxMsg_Deal(void);
#endif

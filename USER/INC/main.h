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
	Geodesic = 1,			//接地故障（只需要检查A）
	Disconnect_C,			//与C板无连接
	No_Module,				//无电源模块连接
	Relay_Err,				//本枪继电器状态错误
	Dc_Table_Err,			//本枪直流表无连接
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
	Soc_Full = 1,//达到soc 电压
	Insulation=2,//绝缘故障
	OutNetTemp=3,//输出连接器过温
	BmsOutNetTemp=4,//BMS元件输出连接器过温
	ChargeNet = 5,//充电连接器故障
	BatTemp = 6,//电池组温度过高
	HighRelay = 7,//高压继电器故障
	Vol_2 = 8,//检查点2电压检查故障
	CurOver = 9,//电流过大
	VolErr = 10,//电压异常
};

/*A/B<——>C数据交互帧内容*/
typedef struct
{
	uint8_t JiTing;	//急停按钮1按下，0松开
	uint8_t DSta;		//桩状态：0待机，1充电中，2故障
	uint8_t DErr;		//桩故障原因：1接地,2模块个数 <1, 3S1234开关无法关闭，4电表无通讯
	uint8_t MNum;		//A/B本组模块个数
	uint8_t MErr2;		//模块故障原因sta2
	uint8_t MErr1;		//模块故障原因sta1
	uint8_t MErr0;		//模块故障原因sta0
}Device_Module_Type;
typedef struct
{
	uint8_t Step;				//握手 辨识 准备充电 充电中 充电停止
	uint8_t Stop_Reason;//停止原因：充满 通讯超时 故障 BMS 人工
	uint8_t time_out;		//超时原因
	uint8_t DErr;				//桩故障：锁 外侧电压 绝缘 DC外侧电压与电池电压<5%
	uint8_t BErr;				//BMS中止原因
	uint8_t Manual;			//人工中止原因：急停 刷卡 APP
}Bms_Type;
typedef struct
{
	uint16_t Vol;	//5505=550.5V
	uint16_t Cur;	//1205=120.5A
	uint8_t  Soc;	//88%
}VolCur_Type;
typedef struct
{
	uint8_t A_Start_Stop;//0X00关闭A枪，0X01暂停，0X02开启
	uint8_t B_Start_Stop;//0X00关闭B枪，0X01暂停，0X02开启
	uint8_t Module_Assign;//0XAB各自用本组模块，0XAA全部模块给A枪用，0XBB全部模块给B枪用
}Control_Type;
extern Device_Module_Type Type_DM;
extern Bms_Type	Type_BMS;
extern VolCur_Type	Type_VolCur;
extern Control_Type	Type_Control_C;

extern unsigned char Board_Type;//A B板定义

static void ABC_Data_Deal(void);
static void Timer1_Callback(void const *arg);
void System_Task(void const *argument);
void BMS_Task(void const *argument);                
void ACDC_Module_Task(void const *argument); 

#endif


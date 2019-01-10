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
	Geodesic = 1<<0,			//接地故障（只需要检查A）*
	Disconnect_C = 1<<1,	//与C板通讯故障
	No_Module = 1<<2,			//无电源模块连接				 *
	Dc_Table_Err = 1<<3,	//本枪直流表无连接(联网版本)
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
	None = 0,					//无故障
	Lock_ERR = 1,			//无法上锁
	GUN_Relay_Err = 2,//本枪继电器状态错误
	KK_Relay_Err  = 3,//中间继电器错误
//	Gun_Vol_ERR = 4,//枪端电压>10V
//	Tap_Check_ERR=5,//泄放检查错误
	Insulation_ERR= 6,//绝缘检测错误
	Bat_Vol_ERR 	= 7,//电池电压不匹配
	CC_ERR = 8,			  //CC信号错误
};
enum _Err_Bms
{
	Soc_Full = 1,//达到soc 电压
	Insulation=2,//绝缘故障
	BmsOutNetTemp=4,//BMS元件/输出连接器过温
	ChargeNet = 5,//充电连接器故障
	BatTemp = 6,//电池组温度过高
	HighRelay = 7,//高压继电器故障
	Vol_2 = 8,//检查点2电压检查故障
//	CurOver = 9,//电流过大
	CurUnknown=10,//电流不可信
//	VolErr = 11,//电压异常
	VolUnknown=12,//电压不可信
};

/*A/B<——>C数据交互帧内容*/
typedef struct
{
	uint8_t JiTing;	//急停按钮1按下，0松开
	uint8_t DSta;		//桩状态：0待机，1充电中，2故障
	uint8_t DErr;		//桩故障原因
	uint8_t MNum;		//A/B本组模块个数
	uint8_t MErr2;	//模块故障原因sta2
	uint8_t MErr1;	//模块故障原因sta1
	uint8_t MErr0;	//模块故障原因sta0
	uint8_t M_Suspend_ACK;//模块暂停结果应答
}Device_Module_Type;
typedef struct
{
	uint8_t Step;				//握手 辨识 准备充电 充电中 充电停止
	uint8_t Stop_Reason;//停止原因：充满 通讯超时 故障 BMS 人工
	uint8_t time_out;		//超时原因
	uint8_t DErr;				//桩故障：锁 外侧电压 绝缘 DC外侧电压与电池电压<5% 继电器错误
	uint8_t BErr;				//BMS中止原因
	uint8_t Manual;			//人工中止原因：1急停 2启停 3APP 4刷卡
	uint16_t RemaChargTime;//剩余充电时间0-600min
}Bms_Type;
typedef struct
{
	uint16_t Vol;	//5505=550.5V
	uint16_t Cur;	//1205=120.5A
	uint16_t KWh;	//5505=550.5KW
	uint8_t  Soc;	//88%
	uint8_t   CC;	//CC电压
}VolCur_Type;
/**C->AB数据内容**/
typedef struct
{
	char	Start_Stop:1;//0X00关闭A枪，0X01开启
	char  Suspend:1;	 //0X01暂停使能
	char	Type:1;			 //0x00 APP启停 0x01 刷卡启停
	char	Account:1;	 //结算标记0未结算，1结算完成
	char 	reserved:4;
}Start_Stop_Cmd;
typedef struct
{
	Start_Stop_Cmd CMD;
	uint8_t Module_Assign;//0XAB各自用本组模块，0XAA全部模块给A枪用，0XBB全部模块给B枪用
	uint8_t KK_Sta;//中间继电器控制状态 0断开 1闭合
}Control_Type;
/******************/
typedef struct
{
	char Bits_1:1;//AB板地址
	char Bits_2:1;
	char Bits_3:1;
	char Bits_4:1;
	char reserved:4;
}DIPSwitchBits;//拨码开关
extern DIPSwitchBits DIPSwitch;
extern Bms_Type	Type_BMS;
extern VolCur_Type	Type_VolCur;
extern Device_Module_Type Type_DM;
extern Control_Type	Type_Control_Cmd;

extern unsigned char Board_Type;//A B板定义0X0A 0X0B
extern unsigned char Board_C_Sta;//AB板与C板连接状态：0无连接 1 连接正常 FF通讯超时重连
static void ABC_Data_Deal_RX(unsigned short Task_Time);
static void ABC_Data_Deal_TX(unsigned char Sta);
static void Timer1_Callback(void const *arg);

void System_Task(void const *argument);
void BMS_Task(void const *argument);                
void ACDC_Module_Task(void const *argument); 

#endif

#ifndef __BMS_H
#define __BMS_H

#include "main.h"

//ACDC_MAX_VOL 由拨码开关第2决定 1=750V 0=500V
extern unsigned short ACDC_MAX_VOL;
#define ACDC_MIN_VOL	2000				//200V
#define ACDC_MAX_CUR	(4000-2500)	//250A
#define ACDC_MIN_CUR	(4000-20)		//2A

#define CC_Connect_MIN	2.8f
#define CC_Connect_MAX	5.2f
#define CC_Disconnect		5.5f

enum _BMS_STA
{
	BEGIN 		= 0,//待插抢
	LOCKED	  = 1,
	SEND_9728 = 2,//握手
	SEND_256  = 3,//辨识
	SEND_2048 = 4,//电池准备
	SEND_2560 = 5,//充电机准备
	SEND_4608 = 6,//充电中
	TIME_OUT  = 7,//超时
	SEND_6656 = 8,//停止
	STOP 			= 9,//停止
};
enum _bms_rx_num
{
	WAIT_9984_BHM = 0,
	WAIT_2304_BRO = 1,
	WAIT_4096_BCL = 2,
	WAIT_4864_BSM = 3,
	WAIT_6400_BST = 4,
	WAIT_7168_BSD = 5,
	WAIT_7680_BEM = 6,
	
	WAIT_512_BRM  = 7,
	WAIT_1536_BCP = 8,
	WAIT_4352_BCS = 9,
};

//接受BMS报文的数据结构
typedef struct
{
	uint8_t Rx_status;//接受到对应报文将此位置1
	uint8_t PGN_S;		//报文PGN
	void *Data;
}RX_BMS;  	
//发送给BMS的数据结构
typedef struct
{
	uint8_t PGN_S;				//报文PGN
	uint8_t IFArbition;		//优先级 保留位 数据页
	uint8_t PDUSpecific;	//目的地址
	uint8_t SourceAddress;//源地址
	uint8_t DLC;					//数据长度
	uint8_t Period;				//发送周期ms
	void *Data;
}TX_BMS;  	
#pragma pack(1)//强制1字节对齐
/*************************充电桩发给BMS的数据****************************/
typedef struct
{
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
}stuPGN9728Type;
typedef struct
{
	uint8_t  IdentifyResult;//辨识结果
	uint32_t ChargSerial;   //充电机编号
	uint8_t  ChargRegionalcode[3];//充电桩所在区域编码
}stuPGN256Type;
typedef struct
{
	uint8_t TimeSync[7];
}stuPGN1792Type;
typedef struct
{
	uint16_t OutPutHVolt;//最高输出电压
	uint16_t OutPutLVolt;//最低输出电压
	uint16_t OutPutHCurr;//最大输出电流
	uint16_t OutPutLCurr;//最小输出电流
}stuPGN2048Type;
typedef struct
{
	uint8_t PrepareOK;
}stuPGN2560Type;
typedef struct
{
	uint16_t OutputVolt;
	uint16_t OutputCurr;
	uint16_t ChargingTime;
	uint16_t AllowCharging;
}stuPGN4608Type;//充电机充电状态
typedef struct
{
	uint8_t  ChargStopChargingReason;//桩中止充电原因
	uint16_t ChargFaultReason;       //桩中止充电故障原因
  uint8_t	 ChargErrorReason;       //桩中止充电错误原因
}stuPGN6656Type;
typedef struct
{
	uint16_t ChargTime;
	uint16_t OutputEnergy;
	uint32_t ChargSerial;
}stuPGN7424Type;
typedef struct
{
	uint8_t IdentifyTimeOut;  		//辨识通讯超时
	uint8_t ChargingParamTimeOut;	//1532 2304超时
	uint8_t BMSChargingStaTimeOut;//4096 4352 6400超时
	uint8_t Summary;							//充电统计超时
}stuPGN7936Type;
/*************************BMS发过来的数据****************************/
typedef struct 
{
	uint16_t AllowHightVolt;
}stuPGN9984Type;//车辆BMS握手报文
typedef struct 
{
	uint8_t Ver[3];
	uint8_t BatType;
	uint16_t RatedCapacity; //额定容量
  uint16_t RatdeVolt;			//电池额定总电压0-750V
	uint8_t  BatBusinessName[4];
	uint8_t  BatSerial[4];
	uint8_t  BatPrdY;
	uint8_t  BatPrdM;    
	uint8_t  BatPrdD;
	uint8_t  BatChargeCount[3];
	uint8_t  BatOwmer;
	uint8_t  Test;
	uint8_t  CarVIN[17];//车辆识别码
	uint8_t  BMSVer[8]; //BMS软件版本号
  char test;
}stuPGN512Type;//车辆辨识报文
typedef struct 
{
	uint16_t SigAllowHightVolt;
	uint16_t AllowHightCurr;		 //最高允许充电电流
	uint16_t RatedCapacity; 
  uint16_t TotalAllowHightVolt;//最高允许充电总电压
	uint8_t  AllowTemp;
	uint16_t SOC;				//电池组电量状态
	uint16_t BatPreVolt;//电池组总电压 
	char test;
}stuPGN1536Type;//动力蓄电池充电参数报文
typedef struct 
{
	uint8_t BMSSta;//0xAA?
}stuPGN2304Type;//电池充电准备就绪
typedef struct 
{
	uint16_t NeedVolt;//电压需求
	uint16_t NeedCurr;//电流需求
	uint8_t  ChargingMode;//充电模式：1：恒压	2：恒流
}stuPGN4096Type;//电池充电需求
typedef struct
{
	uint16_t ChargVoltMeas;//充电电压测量值
	uint16_t ChargCurrMeas;//充电电流测量值
	uint16_t BatVoltH;
	uint8_t  PreSOC;			 //当前SOC
	uint16_t RemaChargTime;//估算剩余充电时间0-600min
	char test1[5];
}stuPGN4352Type;//电池充电总状态
typedef struct
{
	uint8_t BatHVNum;
  uint8_t BatHTemp;
  uint8_t BatHTNum;
	uint8_t BatLTemp;
  uint8_t BatLTNum;
	uint8_t BatSta;
  uint8_t BatConnetSta;
}stuPGN4864Type;//动力蓄电池状态信息
typedef struct
{
	uint8_t  BMSStopChargingReason;//BMS中止充电原因
	uint16_t BMSFaultReason;       //BMS中止充电故障原因
  uint8_t	 BMSErrorReason;       //BMS中止充电错误原因
}stuPGN6400Type;//BMS中止充电
typedef struct
{
	uint8_t  BMSStopSOC;
	uint16_t SigBatLVolt;
	uint16_t SigBatHVolt;
	uint8_t  BatTempL;
	uint8_t  BatTempH;
}stuPGN7168Type;//BMS统计数据
typedef struct
{
	uint8_t HandshakeTimeOut;   //握手通讯超时
	uint8_t ChargingTimeOut;
	uint8_t ChargingEndTimeOut;
	uint8_t test;
}stuPGN7680Type;//BMS错误报文
#pragma pack()

#define Bound(val,max,min) ((val) > (max)? (max) : (val) < (min)? (min) : (val))

extern stuPGN4352Type Data_4352;
extern stuPGN4608Type Data_4608;
extern stuPGN6656Type Data_6656;
extern stuPGN6400Type Data_6400;
extern stuPGN7936Type Data_7936;
extern stuPGN9984Type Data_9984;
extern enum _guzhang guzhang;
extern enum _BMS_STA BMS_STA;
static void ACDC_Set_Vol_Cur(short vol, short cur);
static void BMS_Data_Init(void);
static void BMS_Send(TX_BMS Pbuf);

static void Single_Package_Deal(void);
void Multi_Package_Deal(void);
#endif

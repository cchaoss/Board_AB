#ifndef __BMS_H
#define __BMS_H

#include "main.h"

enum _sta
{
	WAIT_9984_BHM,
	WAIT_512_BRM,
	WAIT_1536_BCP,
	WAIT_2304_BRO,
	WAIT_4096_BCL,
	WAIT_4352_BCS,
	WAIT_4864_BSM,
	WAIT_6400_BST,
	WAIT_7168_BSD,
	WAIT_7680_BEM
};

//接受BMS报文的数据结构
typedef struct
{
	uint8_t Rx_status;			//接受到对应报文将此位置1
	uint8_t PGN_S;					//报文PGN
//	uint8_t IFArbition;			//优先级 保留位 数据页
//	uint8_t PDUSpecific;		//目的地址
//	uint8_t SourceAddress;	//源地址
//	uint8_t DLC;						//数据长度
	void *Data;
}RX_BMS;  	

//发送给BMS的数据结构
typedef struct
{
	uint8_t PGN_S;					//报文PGN
	uint8_t IFArbition;			//优先级 保留位 数据页
	uint8_t PDUSpecific;		//目的地址
	uint8_t SourceAddress;	//源地址
	uint8_t DLC;						//数据长度
	void *Data;
}TX_BMS;  	

#pragma pack(1)   //强制1字节对齐
/*************************充电桩发给BMS的数据****************************/
typedef struct
{
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
}stuPGN9728Type;
typedef struct
{
	uint8_t  IdentifyResult;   //辨识结果
	uint32_t ChargSerial;      //充电机编号
	uint8_t  ChargRegionalcode[3];//充电桩所在区域编码
}stuPGN256Type;
typedef struct
{
	uint8_t TimeSync[7];
}stuPGN1792Type;
typedef struct
{
	uint16_t OutPutHVolt;  
	uint16_t OutPutLVolt;
	uint16_t OutPutHCurr;
	uint16_t OutPutLCurr;
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
}stuPGN4608Type;
typedef struct
{
	uint8_t  ChargStopChargingReason;  //BMS中止充电原因
	uint16_t ChargFaultReason;         //BMS中止充电故障原因
  uint8_t	 ChargErrorReason;         //BMS中止充电错误原因
}stuPGN6656Type;
typedef struct
{
	uint16_t ChargTime;
	uint16_t OutputEnergy;
	uint32_t ChargSerial;
}stuPGN7424Type;
typedef struct
{
	uint8_t IdentifyTimeOut;   //辨识通讯超时
	uint8_t ChargingParamTimeOut;
	uint8_t BMSChargingStaTimeOut;
	uint8_t Summary;//充电统计状态
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
  uint16_t RatdeVolt;
	uint8_t  BatBusinessName[4]; //电池厂商名称
	uint8_t  BatSerial[4]; //电池组序号
	uint8_t  BatPrdY;    //电池生产的年份
	uint8_t  BatPrdM;    
	uint8_t  BatPrdD;
	uint8_t  BatChargeCount[3];
	uint8_t  BatOwmer;  //电池组产权标识
	uint8_t  Test;
	uint8_t  CarVIN[17];//车辆识别码
	uint8_t  BMSVer[8]; //BMS软件版本号
  	
}stuPGN512Type;//车辆辨识报文
typedef struct 
{
	uint16_t SigAllowHightVolt;
	uint16_t AllowHightCurr;
	uint16_t RatedCapacity; 
  uint16_t TotalAllowHightVolt;
	uint8_t  AllowTemp;
	uint16_t SOC;
	uint16_t BatPreVolt;  
}stuPGN1536Type;//动力蓄电池充电参数报文
typedef struct 
{
	uint8_t BMSSta;
}stuPGN2304Type;//电池充电准备就绪
typedef struct 
{
	uint16_t NeedVolt;
	uint16_t NeedCurr;
	uint8_t  ChargingMode;
}stuPGN4096Type;//电池充电需求
typedef struct
{
	uint16_t ChargVoltMeas;
	uint16_t ChargCurrMeas;
	uint16_t BatVoltH;
	uint8_t  PreSOC;
	uint16_t RemaChargTime;  //估算剩余充电时间
}stuPGN4352Type;//电池充电总状态
typedef struct
{
	uint8_t BatHVoltSerialNumber;
  uint8_t BatHTemp;
  uint8_t BatHTempSerialNumber;
	uint8_t BatLTemp;
  uint8_t BatLTempSerialNumber;
	uint8_t BatSta;
  uint8_t BatConnetSta;
}stuPGN4864Type;//动力蓄电池状态信息
typedef struct
{
	uint8_t  BMSStopChargingReason;  //BMS中止充电原因
	uint16_t BMSFaultReason;         //BMS中止充电故障原因
  uint8_t	 BMSErrorReason;         //BMS中止充电错误原因
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



static unsigned char BMS_Send(TX_BMS Pbuf);

	
#endif

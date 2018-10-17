#ifndef __BMS_H
#define __BMS_H

#include "main.h"

enum _BMS_STA
{
	SEND_9728,
	SEND_256,
	SEND_2048,
	SEND_2560,
	SEND_4608,
	SEND_6656,
	SEND_7424,
	SEND_7936
};
enum _rx_bms_sta
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

//����BMS���ĵ����ݽṹ
typedef struct
{
	uint8_t Rx_status;			//���ܵ���Ӧ���Ľ���λ��1
	uint8_t PGN_S;					//����PGN
//	uint8_t IFArbition;			//���ȼ� ����λ ����ҳ
//	uint8_t PDUSpecific;		//Ŀ�ĵ�ַ
//	uint8_t SourceAddress;	//Դ��ַ
//	uint8_t DLC;						//���ݳ���
	void *Data;
}RX_BMS;  	

//���͸�BMS�����ݽṹ
typedef struct
{
	uint8_t PGN_S;					//����PGN
	uint8_t IFArbition;			//���ȼ� ����λ ����ҳ
	uint8_t PDUSpecific;		//Ŀ�ĵ�ַ
	uint8_t SourceAddress;	//Դ��ַ
	uint8_t DLC;						//���ݳ���
	uint8_t Period;					//��������ms
	void *Data;
}TX_BMS;  	

#pragma pack(1)   //ǿ��1�ֽڶ���
/*************************���׮����BMS������****************************/
typedef struct
{
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
}stuPGN9728Type;
typedef struct
{
	uint8_t  IdentifyResult;   //��ʶ���
	uint32_t ChargSerial;      //�������
	uint8_t  ChargRegionalcode[3];//���׮�����������
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
	uint8_t  ChargStopChargingReason;  //BMS��ֹ���ԭ��
	uint16_t ChargFaultReason;         //BMS��ֹ������ԭ��
  uint8_t	 ChargErrorReason;         //BMS��ֹ������ԭ��
}stuPGN6656Type;
typedef struct
{
	uint16_t ChargTime;
	uint16_t OutputEnergy;
	uint32_t ChargSerial;
}stuPGN7424Type;
typedef struct
{
	uint8_t IdentifyTimeOut;   //��ʶͨѶ��ʱ
	uint8_t ChargingParamTimeOut;
	uint8_t BMSChargingStaTimeOut;
	uint8_t Summary;//���ͳ��״̬
}stuPGN7936Type;
/*************************BMS������������****************************/
typedef struct 
{
	uint16_t AllowHightVolt;
}stuPGN9984Type;//����BMS���ֱ���
typedef struct 
{
	uint8_t Ver[3];
	uint8_t BatType;
	uint16_t RatedCapacity; //�����
  uint16_t RatdeVolt;
	uint8_t  BatBusinessName[4]; //��س�������
	uint8_t  BatSerial[4]; //��������
	uint8_t  BatPrdY;    //������������
	uint8_t  BatPrdM;    
	uint8_t  BatPrdD;
	uint8_t  BatChargeCount[3];
	uint8_t  BatOwmer;  //������Ȩ��ʶ
	uint8_t  Test;
	uint8_t  CarVIN[17];//����ʶ����
	uint8_t  BMSVer[8]; //BMS����汾��
  	
}stuPGN512Type;//������ʶ����
typedef struct 
{
	uint16_t SigAllowHightVolt;
	uint16_t AllowHightCurr;
	uint16_t RatedCapacity; 
  uint16_t TotalAllowHightVolt;
	uint8_t  AllowTemp;
	uint16_t SOC;
	uint16_t BatPreVolt;  
}stuPGN1536Type;//�������س���������
typedef struct 
{
	uint8_t BMSSta;
}stuPGN2304Type;//��س��׼������
typedef struct 
{
	uint16_t NeedVolt;
	uint16_t NeedCurr;
	uint8_t  ChargingMode;
}stuPGN4096Type;//��س������
typedef struct
{
	uint16_t ChargVoltMeas;
	uint16_t ChargCurrMeas;
	uint16_t BatVoltH;
	uint8_t  PreSOC;
	uint16_t RemaChargTime;  //����ʣ����ʱ��
}stuPGN4352Type;//��س����״̬
typedef struct
{
	uint8_t BatHVoltSerialNumber;
  uint8_t BatHTemp;
  uint8_t BatHTempSerialNumber;
	uint8_t BatLTemp;
  uint8_t BatLTempSerialNumber;
	uint8_t BatSta;
  uint8_t BatConnetSta;
}stuPGN4864Type;//��������״̬��Ϣ
typedef struct
{
	uint8_t  BMSStopChargingReason;  //BMS��ֹ���ԭ��
	uint16_t BMSFaultReason;         //BMS��ֹ������ԭ��
  uint8_t	 BMSErrorReason;         //BMS��ֹ������ԭ��
}stuPGN6400Type;//BMS��ֹ���
typedef struct
{
	uint8_t  BMSStopSOC;
	uint16_t SigBatLVolt;
	uint16_t SigBatHVolt;
	uint8_t  BatTempL;
	uint8_t  BatTempH;
}stuPGN7168Type;//BMSͳ������
typedef struct
{
	uint8_t HandshakeTimeOut;   //����ͨѶ��ʱ
	uint8_t ChargingTimeOut;
	uint8_t ChargingEndTimeOut;
	uint8_t test;
}stuPGN7680Type;//BMS������
#pragma pack()



static unsigned char BMS_Send(TX_BMS Pbuf);

	
#endif

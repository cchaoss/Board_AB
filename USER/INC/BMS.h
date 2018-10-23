#ifndef __BMS_H
#define __BMS_H

#include "main.h"
#include "stm32f10x_can.h"

#define ACDC_MAX_VOL	7500				//750V
#define ACDC_MIN_VOL	2000				//200V
#define ACDC_MAX_CUR	(4000-2000)	//200A
enum _BMS_STA
{
	BEGIN = 0,
	SEND_9728 = 1,
	SEND_256  = 2,
	SEND_2048 = 3,
	SEND_2560 = 4,
	SEND_4608 = 5,
	SEND_6656 = 6,
	SEND_7424 = 7,
	TIME_OUT = 8,
};
enum _rx_bms_sta
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
	uint16_t OutPutHVolt;	//��������ѹ
	uint16_t OutPutLVolt;	//��������ѹ
	uint16_t OutPutHCurr;	//����������
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
}stuPGN4608Type;//�������״̬
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
	uint8_t IdentifyTimeOut;  		//��ʶͨѶ��ʱ
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
  uint16_t RatdeVolt;			//��ض�ܵ�ѹ0-750V
	uint8_t  BatBusinessName[4];
	uint8_t  BatSerial[4];
	uint8_t  BatPrdY;
	uint8_t  BatPrdM;    
	uint8_t  BatPrdD;
	uint8_t  BatChargeCount[3];
	uint8_t  BatOwmer;
	uint8_t  Test;
	uint8_t  CarVIN[17];//����ʶ����
	uint8_t  BMSVer[8]; //BMS����汾��
  char test;
}stuPGN512Type;//������ʶ����
typedef struct 
{
	uint16_t SigAllowHightVolt;
	uint16_t AllowHightCurr;	//������������
	uint16_t RatedCapacity; 
  uint16_t TotalAllowHightVolt;//����������ܵ�ѹ
	uint8_t  AllowTemp;
	uint16_t SOC;				//��������״̬
	uint16_t BatPreVolt;//������ܵ�ѹ 
	char test;
}stuPGN1536Type;//�������س���������
typedef struct 
{
	uint8_t BMSSta;//0xAA?
}stuPGN2304Type;//��س��׼������
typedef struct 
{
	uint16_t NeedVolt;//��ѹ����
	uint16_t NeedCurr;//��������
	uint8_t  ChargingMode;//���ģʽ��1����ѹ	2������
}stuPGN4096Type;//��س������
typedef struct
{
	uint16_t ChargVoltMeas;//����ѹ����ֵ
	uint16_t ChargCurrMeas;//����������ֵ
	uint16_t BatVoltH;
	uint8_t  PreSOC;			 //��ǰSOC
	uint16_t RemaChargTime;//����ʣ����ʱ��0-600min
	char test1[5];
}stuPGN4352Type;//��س����״̬
typedef struct
{
	uint8_t BatHVNum;
  uint8_t BatHTemp;
  uint8_t BatHTNum;
	uint8_t BatLTemp;
  uint8_t BatLTNum;
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

#define Bound(val,max,min) ((val) > (max)? (max) : (val) < (min)? (min) : (val))

static void ACDC_Start(unsigned short vol,unsigned short cur);
static void ACDC_Stop(void);
static void Charge_Close(void);
static void BMS_Data_Init(void);
static void BMS_Send(TX_BMS Pbuf);
static void Single_Package_Deal(void);
void Multi_Package_Deal(void);
#endif

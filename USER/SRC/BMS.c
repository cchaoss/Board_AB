#include "main.h"
#include "bms.h"
#include "can.h"
#include "acdc_module.h"

#pragma pack(1)//强制1字节对齐
stuPGN9728Type Data_9728 = {0x01,0x01,0x00};
stuPGN256Type	 Data_256  = {0x00,0,};
stuPGN1792Type Data_1792;
stuPGN2048Type Data_2048 = {ACDC_MAX_VOL,ACDC_MIN_VOL,ACDC_MAX_CUR};//充电机输出电压：200-750V 输出电流：0-200A
stuPGN2560Type Data_2560;
stuPGN4608Type Data_4608 = {0,0,0,0x01};//默认允许充电2015GB
stuPGN6656Type Data_6656;
stuPGN7424Type Data_7424;
stuPGN7936Type Data_7936;/*充电桩发给BMS的数据*/
stuPGN9984Type Data_9984;/*BMS发过来的数据*/
stuPGN2304Type Data_2304;
stuPGN4096Type Data_4096;
stuPGN512Type	 Data_512;
stuPGN1536Type Data_1536;
stuPGN4352Type Data_4352;
stuPGN4864Type Data_4864;
stuPGN6400Type Data_6400;
stuPGN7168Type Data_7168;
stuPGN7680Type Data_7680;
#pragma pack()							/*接受标志		报文PGN					数据区*/
RX_BMS RX_BMS_TAB[10] =	{	{		0,			(9984>>8),			&Data_9984},	//BMS握手报文
													{		0,			(2304>>8),			&Data_2304},	//电池准备就绪报文
													{		0,			(4096>>8),			&Data_4096},	//电池充电需求报文
													{		0,			(4864>>8),			&Data_4864},	//动力蓄电池状态信息
													{		0,			(6400>>8),			&Data_6400},	//BMS中止充电报文
													{		0,			(7168>>8),			&Data_7168},	//BMS统计数据报文
													{		0,			(7680>>8),			&Data_7680},  //BMS错误报文																	
													{		0,			(512>>8),				&Data_512	},	//BMS车辆辨识报文
													{		0,			(1536>>8),			&Data_1536},	//动力蓄电池充电参数
													{		0,			(4352>>8),			&Data_4352}};	//电池充电总状态报文
									/*报文PGN		优先级	目的地址	源地址	数据长度		发送周期ms		数据区*/		
TX_BMS CHM_9728 = {(9728>>8),		6,		 0xf4,		 0x56,		 3,					250,			&Data_9728};//充电机握手报文
TX_BMS CRM_256  =	{(256 >>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_256 };//充电机辨识报文
TX_BMS CTS_1792 = {(1792>>8),		6,		 0xf4,		 0x56,	 	 7,					250,			&Data_1792};//充电机发送时间同步
TX_BMS CML_2048 = {(2048>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_2048};//充电机最大输出能力
TX_BMS CRO_2560 = {(2560>>8),		4,		 0xf4,		 0x56,		 1,					250,			&Data_2560};//充电机输出准备就绪
TX_BMS CCS_4608 = {(4608>>8),		6,		 0xf4,		 0x56,		 8,					50,				&Data_4608};//充电机充电状态
TX_BMS CST_6656 = {(6656>>8),		4,		 0xf4,		 0x56,		 4,					10,				&Data_6656};//充电机中止充电
TX_BMS CSD_7424 = {(7424>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_7424};//充电机统计数据
TX_BMS CEM_7936 = {(7936>>8),		2,		 0xf4,		 0x56,		 4,					250,			&Data_7936};//充电机错误报文
float CC = 12;
unsigned char OUT,guzhang,BMS_STA = BEGIN;
void BMS_Task(void const *argument)
{
	const unsigned char BMS_Task_Time = 25U;//10ms循环
	static unsigned short t,timeout,timeout1;
	static bool once_run = true;
	
	while(1)
	{	
		Single_Package_Deal();//负责接送处理单包数据
		
		switch(BMS_STA)//BMS发送处理
		{
			case BEGIN://插抢准备
			{	//前提：充电机自身状态正常
				//CC信号是否3.2v-4.8v
				//上锁，上锁反馈就绪？//超时5s锁未锁好->结束充电需要再次拔枪	guzhang = Lock_ERR;
				//辅助电源继电器闭合K3K4
				//都ok的话->{相关标志位清0，BMS_STA = SEND_9728;}//报文接收标志清0
				if((CC>3)&&(CC<5))
				{
					BMS_Data_Init();
					once_run = true;
					OUT = 0;
					t = timeout = timeout1 = 0;
					BMS_STA = SEND_9728;
				}
			}break;	
			
			case SEND_9728://充电握手
			{
				t = (t+1)%(CHM_9728.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CHM_9728);}		
				if((RX_BMS_TAB[WAIT_9984_BHM].Rx_status == 1)||(timeout > 20))//收到9984||超时5s->GB2015
				{
					//if(k1k2接触器电压<10V)//guzhang = Gun_Vol_ERR;//接触器外侧电压>10V//需要重新拔枪
					//if(绝缘监测ok？)//需要电压模块输出电压//guzhang = Insulation_ERR;//绝缘检查错误//停用本桩
					//if(泄放电路检查ok？)//guzhang = Tap_Check_ERR;//泄放检查错误//停用本桩
					t = timeout = 0;	
					BMS_STA = SEND_256;
				}
			}break;	
			
			case SEND_256://充电辨识
			{
				t = (t+1)%(CRM_256.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CRM_256);}
				
				if(RX_BMS_TAB[WAIT_512_BRM].Rx_status == 1)	{timeout = 0;	Data_256.IdentifyResult = 0xAA;}//辨识成功	
					else if(timeout > 20)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.IdentifyTimeOut |= 0x01;}//接收车辆辨识报文BRM超时5s
				
				if(RX_BMS_TAB[WAIT_1536_BCP].Rx_status == 1)	{t = timeout = 0;	BMS_STA = SEND_2048;}
					else if(timeout > 20)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x01;}//接受1536电池充电参数超时5s
			}break;
			
			case SEND_2048://充电机最大输出能力/时间同步
			{
				t = (t+1)%(CML_2048.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CTS_1792);	BMS_Send(CML_2048);}
				
				if(RX_BMS_TAB[WAIT_2304_BRO].Rx_status == 1)
				{
					if(Data_2304.BMSSta==0xAA)	{t = timeout = 0;	BMS_STA = SEND_2560;}
						else if(timeout>240)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x04;}//BRO在60s内未准备好
				}else if(timeout>20)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x04;}//接受BROBMS准备充电就绪超时5s	
			}break;
			
			case SEND_2560://充电机输出准备就绪
			{
				t = (t+1)%(CRO_2560.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CRO_2560);}
				//if(Data_1536.BatPreVolt)判断枪电压与报文电池电压相差5%以内，在充电机最大最小电压之间//guzhang = Bat_Vol_ERR;//需要再次拔枪
				//调整电源模块输出电压高于电池电压1-10v OK?  ACDC_Set_Vol_Cur(Data_1536.BatPreVolt+5.5f,0);
				//if(Module_Status.Output_Vol*10>Data_1536.BatPreVolt+1)//闭合继电器K1K2
				if(1)	Data_2560.PrepareOK = 0xAA;
				if((RX_BMS_TAB[WAIT_4352_BCS].Rx_status==1)&&(RX_BMS_TAB[WAIT_4096_BCL].Rx_status==1))	{t=timeout=0;	BMS_STA = SEND_4608;}
					else if(timeout>20)	{t=timeout=0;	Data_7936.BMSChargingStaTimeOut|=0x05;	BMS_STA=TIME_OUT;}//4096电池充电需求超时5s 4352电池充电总状态报文超5s(共生共死)
			}break;
			
			case SEND_4608:/*充电机充电状态(充电阶段)*/
			{
				OUT = 0;//通讯超时重连次数清零：到充电阶段说明所有报文都收到一遍
				//if(充电机故障){Data_6656.ChargStopChargingReason|=Err_Stop;	Data_6656.ChargFaultReason = 故障原因;	BMS_Send(CST_6656);	t=timeout=0;	BMS_STA = STOP;}//充电机故障停止
				//if(急停)->{Data_6656.ChargStopChargingReason|=Mannul_Stop;	t=timeout=0;	BMS_STA = SEND_6656;}//人工停止
				Data_4608.OutputVolt = Module_Status.Output_Vol*10;
				Data_4608.OutputCurr = 4000-Module_Status.Output_Cur*10;
				Data_4608.ChargingTime++;
				
				t = (t+1)%(CCS_4608.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;timeout1++;	BMS_Send(CCS_4608);}
				
				if((RX_BMS_TAB[WAIT_4096_BCL].Rx_status==0)&&(timeout>4))//接受4096电池充电需求超时1s
				{
					Data_7936.BMSChargingStaTimeOut|=0x04;
					t = timeout = 0;	
					BMS_STA=TIME_OUT;	
				}
				if((RX_BMS_TAB[WAIT_4352_BCS].Rx_status==0)&&(timeout1>20))//4352电池充电总状态报文超5s
				{
					Data_7936.BMSChargingStaTimeOut|=0x01;
					t = timeout = 0;	
					BMS_STA=TIME_OUT;
				}
				if(RX_BMS_TAB[WAIT_4096_BCL].Rx_status==1)	{timeout = 0;	RX_BMS_TAB[WAIT_4096_BCL].Rx_status = 0;}//接受到报文清空标记
				if(RX_BMS_TAB[WAIT_4352_BCS].Rx_status==1)	{timeout1= 0;	RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 0;}
				
				if(RX_BMS_TAB[WAIT_4864_BSM].Rx_status==1)
				{
					RX_BMS_TAB[WAIT_4864_BSM].Rx_status = 0;
					if((Data_4864.BatSta==0)&&((Data_4864.BatConnetSta&0x0f)==0))//电池状态位都正常
					{
						if(Data_4864.BatConnetSta&0x10)	
							ACDC_Set_Vol_Cur(Data_4096.NeedVolt,4000-Data_4096.NeedCurr);//允许充电
						else	ACDC_Set_Vol_Cur(0,0);//充电暂停	
					}else{t = timeout = 0;	BMS_STA = SEND_6656;}//电池异常停止充电
				}
				
				if(RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)//收到BMS中止充电
				{
					Data_6656.ChargStopChargingReason |= BMS_Stop;//BMS停止
					t = timeout = 0;	
					BMS_STA = STOP;
				}
			}break;
			
			case SEND_6656://充电机中止充电
			{
				BMS_Send(CST_6656);//10ms周期发送
				timeout++;
				if((RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)||(timeout > 5000/BMS_Task_Time))//收到BMS中止充电BST||发送CTS10*500=5s后
				{
					ACDC_Set_Vol_Cur(0,0);//电力输出停止操作
					BMS_Send(CSD_7424);//充电机统计数据报文
					t = timeout = 0;
					BMS_STA = STOP;
				}
			}break;			
//			case SEND_7424://充电机统计数据报文
//			{
//				t = (t+1)%(CSD_7424.Period/BMS_Task_Time);
//				if(t == 1)	BMS_Send(CSD_7424);
//				if((t>20)||(0))//delay20*10ms(或者电流5A以下)-保护继电器寿命
//					{Charge_Close();	t = timeout = 0;	BMS_STA = BEGIN;}//跳过接受7168BMS统计数据报文										
//			}break;
			case TIME_OUT://超时重连处理
			{
				if(OUT < 3)//3次以内超时则重连
				{
					t = (t+1)%(CEM_7936.Period/BMS_Task_Time);
					if(t == 1)	
					{
						BMS_Send(CEM_7936);//发送充电机错误报文
						ACDC_Set_Vol_Cur(0,0);//电力输出停止操作
					}	
					if((t>200/BMS_Task_Time)||(0))//delay200ms(或者电流5A以下)-保护继电器寿命
					{	//断开K1K2
						OUT++;//重连次数+1
						BMS_Data_Init();//情况接受标记
						t = timeout = timeout1 = 0;						
						BMS_STA = SEND_256;//重新开始辨识
					}					
				}
				else {t = timeout = 0;	BMS_STA = STOP;}//3次通讯超时重连后再超时则不再重连 直接结束充电					
			}break;
			
			case STOP://结束充电操作后等待再次插抢->BEGIN
			{
				if(t == 0)	ACDC_Set_Vol_Cur(0,0);//电力输出停止操作
				if((t==200/BMS_Task_Time)||(0))	{once_run = false;	Charge_Close();}//delay200ms(或者电流5A以下)-保护继电器寿命	
				if(once_run)	t++;	else t = 100;
				if(CC > 10)	BMS_STA = BEGIN;//必须重新拔枪插抢才可以下一次充电
			}break;
			default:break;
		}	
		osDelay(BMS_Task_Time);
	}
}


static void BMS_Data_Init(void)
{
	for(char j = 0; j<10; j++)	RX_BMS_TAB[j].Rx_status = 0;//清除报文接受标志
	Data_256.IdentifyResult = 0x00;//辨识状态：未辨识
	Data_2560.PrepareOK = 0x00;		 //准备状态：准备未完成
	memset(&Data_1792,0,sizeof(Data_1792));
	memset(&Data_4608,0,sizeof(Data_4608));
	memset(&Data_6656,0,sizeof(Data_6656));
	memset(&Data_7424,0,sizeof(Data_7424));
	memset(&Data_7936,0,sizeof(Data_7936));//TX
	memset(&Data_4864,0,sizeof(Data_4864));//RX电池状态标记清空
}


CanTxMsg TxMsg1 = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//扩展帧 数据帧
static void BMS_Send(TX_BMS Pbuf)
{
	TxMsg1.ExtId = (Pbuf.IFArbition<<26)|(Pbuf.PGN_S<<16)|(Pbuf.PDUSpecific<<8)|(Pbuf.SourceAddress);
	TxMsg1.DLC = Pbuf.DLC;
	memset(TxMsg1.Data,0,sizeof(TxMsg1.Data));
	memcpy(TxMsg1.Data,Pbuf.Data,Pbuf.DLC);
	CAN_Transmit(CAN1, &TxMsg1);
}

//处理单包数据
static void Single_Package_Deal(void)
{
	if(RX_Flag.BMS_Rx_Flag1)
	{	
		for(char i = WAIT_9984_BHM; i <= WAIT_7680_BEM; i++)
		{
			if((unsigned char)((BMS_RX_1.ExtId&0x00ff0000)>>16) == RX_BMS_TAB[i].PGN_S)//提取报文PGN号
			{
				RX_BMS_TAB[i].Rx_status = 1;
				memcpy(RX_BMS_TAB[i].Data,BMS_RX_1.Data,BMS_RX_1.DLC);
			}
		}
		RX_Flag.BMS_Rx_Flag1 = false;
	}
}
		

unsigned char *P = NULL;
unsigned char J1939_Multi_Package[8];//0x10,len_L,len_H,包数,0xff,PGN[3]
CanTxMsg TxMsg_Ack  = {0, 0X1CECF456, CAN_Id_Extended, CAN_RTR_Data, 8, {0x11,0x00,1,0xff,0xff,0x00,0x00,0x00}};
CanTxMsg TxMsg_Done = {0, 0X1CECF456, CAN_Id_Extended, CAN_RTR_Data, 8, {0x13,0x00,0,0x00,0xff,0x00,0x00,0x00}};
//处理BMS多包传输	
void Multi_Package_Deal(void)
{
	if((BMS_RX_0.ExtId == 0X1CEC56F4)&&(BMS_RX_0.Data[0] == 0x10))//BMS请求建立多包发送连接
	{		
		memcpy(&J1939_Multi_Package[1],&BMS_RX_0.Data[1],7);//保存多包发送连接的配置数据	
		for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++)
		{
			if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	
				P = (unsigned char*)RX_BMS_TAB[i].Data;
		}
		TxMsg_Ack.Data[1] = J1939_Multi_Package[3];//可发送包数	
		TxMsg_Ack.Data[6] = J1939_Multi_Package[6];//PGN*/							
		CAN_Transmit(CAN1, &TxMsg_Ack);
		if(TxMsg_Ack.Data[6] == 0x11)	CAN_Transmit(CAN1, &TxMsg_Ack);//应答4352得发两遍！真的有毒！！！
	}
	else if(BMS_RX_0.ExtId == 0X1CEB56F4)//多包数据传输
	{				
		if(BMS_RX_0.Data[0] == 1)	memcpy(P,&BMS_RX_0.Data[1],7); \
			else if(BMS_RX_0.Data[0] == 2)	memcpy(P+7,&BMS_RX_0.Data[1],7);//重组多包数据前2个包数据有效
				
		if(BMS_RX_0.Data[0] == J1939_Multi_Package[3])//多包接受完成？
		{
			for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++)
			{
				if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	
					RX_BMS_TAB[i].Rx_status  = 1;				
			}
			TxMsg_Done.Data[1] = J1939_Multi_Package[1];
			TxMsg_Done.Data[3] = J1939_Multi_Package[3];
			TxMsg_Done.Data[6] = J1939_Multi_Package[6];
			CAN_Transmit(CAN1, &TxMsg_Done);//应答多包接受完成
		}
	}
}


static void ACDC_Set_Vol_Cur(short vol,	short cur)
{
	int V = Bound(vol,ACDC_MAX_VOL,0)*100;//0-700V
	int C = Bound(cur,4000-ACDC_MAX_CUR,0)*100;//0-300V
	ACDC_VolCur_Buffer[0] = V>>24;
	ACDC_VolCur_Buffer[1] = V>>16;
	ACDC_VolCur_Buffer[2] = V>>8;
	ACDC_VolCur_Buffer[3] = V>>0;
	ACDC_VolCur_Buffer[4] = C>>24;
	ACDC_VolCur_Buffer[5] = C>>16;
	ACDC_VolCur_Buffer[6] = C>>8;
	ACDC_VolCur_Buffer[7] = C>>0;
}

static void Charge_Close(void)
{
	//3断开K1K2
	//4泄放电路动作
	//5辅助电源继电器K3K4断开
	//6电子锁解锁
}

#include "main.h"
#include "bms.h"
#include "can.h"

#pragma pack(1)   //强制1字节对齐
/*充电桩发给BMS的数据*/
stuPGN9728Type Data_9728 = {0x01,0x01,0x00};
stuPGN256Type	 Data_256  = {0x00,0,};
stuPGN1792Type Data_1792;
stuPGN2048Type Data_2048 = {7500,	2000,	4000-2000};//充电机输出电压：200-750V 输出电流：0-200A
stuPGN2560Type Data_2560;
stuPGN4608Type Data_4608 = {0,0,0,0x01};//默认允许充电2015GB
stuPGN6656Type Data_6656;
stuPGN7424Type Data_7424;
stuPGN7936Type Data_7936;
/*BMS发过来的数据*/
stuPGN9984Type Data_9984;
stuPGN2304Type Data_2304;
stuPGN4096Type Data_4096;
stuPGN512Type	 Data_512;
stuPGN1536Type Data_1536;
stuPGN4352Type Data_4352;
stuPGN4864Type Data_4864;
stuPGN6400Type Data_6400;
stuPGN7168Type Data_7168;
stuPGN7680Type Data_7680;
#pragma pack()

#define RX_BMS_NUM 10							/*接受标志		报文PGN					数据区*/
RX_BMS RX_BMS_TAB[RX_BMS_NUM] =	{	{		0,			(9984>>8),			&Data_9984},	//BMS握手报文
																	{		0,			(2304>>8),			&Data_2304},	//电池准备就绪报文
																	{		0,			(4096>>8),			&Data_4096},	//电池充电需求报文
																	{		0,			(4864>>8),			&Data_4864},	//动力蓄电池状态信息
																	{		0,			(6400>>8),			&Data_6400},	//BMS中止充电报文
																	{		0,			(7168>>8),			&Data_7168},	//BMS统计数据报文
																	{		0,			(7680>>8),			&Data_7680}, //BMS错误报文																	
																	{		0,			(512>>8),				&Data_512	},	//BMS车辆辨识报文
																	{		0,			(1536>>8),			&Data_1536},	//动力蓄电池充电参数
																	{		0,			(4352>>8),			&Data_4352}};	//电池充电总状态报文
									/*报文PGN		优先级	目的地址	源地址	数据长度		发送周期ms		数据区*/		
TX_BMS CHM_9728 = {(9728>>8),		6,		 0xf4,		 0x56,		 3,					250,			&Data_9728};//充电机握手报文
TX_BMS CRM_256 = 	{(256 >>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_256 };//充电机辨识报文
TX_BMS CTS_1792 = {(1792>>8),		6,		 0xf4,		 0x56,	 	 7,					250,			&Data_1792};//充电机发送时间同步
TX_BMS CML_2048 = {(2048>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_2048};//充电机最大输出能力
TX_BMS CRO_2560 = {(2560>>8),		4,		 0xf4,		 0x56,		 1,					250,			&Data_2560};//充电机输出准备就绪
TX_BMS CCS_4608 = {(4608>>8),		6,		 0xf4,		 0x56,		 8,					50,				&Data_4608};//充电机充电状态
TX_BMS CST_6656 = {(6656>>8),		4,		 0xf4,		 0x56,		 4,					10,				&Data_6656};//充电机中止充电
TX_BMS CSD_7424 = {(7424>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_7424};//充电机统计数据
TX_BMS CEM_7936 = {(7936>>8),		2,		 0xf4,		 0x56,		 4,					250,			&Data_7936};//充电机错误报文

CanTxMsg TxMsg1 =  {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//扩展帧 数据帧
unsigned char OUT,	BMS_STA = SEND_256;//跳过握手直接辨识
void BMS_Task(void const *argument)
{
	const unsigned char BMS_Task_Time = 10U;//10ms循环
	static unsigned short t,time_out;
	while(1)
	{	//负责接送处理单包数据
		Single_Package_Deal();
		//BMS发送
		switch(BMS_STA)
		{
			case BEGIN://插抢准备
			{
				//CC信号是否3.2v-4.8v
				//上锁，上锁反馈就绪？
				//辅助电源继电器闭合K3K4
				//都ok的话->{相关标志位清0，BMS_STA = SEND_9728;}//报文接收标志清0
				break;
			}	
			case SEND_9728://充电握手
			{
				t = (t+1)%(CHM_9728.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CHM_9728);}		
				if((RX_BMS_TAB[WAIT_9984_BHM].Rx_status == 1)||(time_out > 20))//收到9984||超时5s->GB2015
				{
					//if(k1k2接触器电压<10V)
					//if(绝缘监测ok？)//需要电压模块输出电压
					//if(泄放电路检查ok？)
					t = time_out = 0;
					Data_256.IdentifyResult = 0x00;//辨识状态：未辨识
					BMS_STA = SEND_256;
				}
				break;
			}		
			case SEND_256://充电辨识
			{
				t = (t+1)%(CRM_256.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CRM_256);}
				
				if(RX_BMS_TAB[WAIT_512_BRM].Rx_status == 1)	{time_out = 0;	Data_256.IdentifyResult = 0xAA;}//辨识成功	
					else if(time_out > 20)	BMS_STA = TIME_OUT;//接收车辆辨识报文BRM超时5s
				
				if(RX_BMS_TAB[WAIT_1536_BCP].Rx_status == 1)
				{
					//RX_BMS_TAB[WAIT_1536_BCP].Rx_status = RX_BMS_TAB[WAIT_512_BRM].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_2048;				
				}else if(time_out > 20)	BMS_STA = TIME_OUT;//接受1536报文超时5s
				break;
			}		
			case SEND_2048://充电机最大输出能力/时间同步
			{
				t = (t+1)%(CML_2048.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CML_2048);	BMS_Send(CTS_1792);}
				
				if(RX_BMS_TAB[WAIT_2304_BRO].Rx_status == 1)
				{
					if(Data_2304.BMSSta==0xAA)
					{
						//RX_BMS_TAB[WAIT_2304_BRO].Rx_status = 0;
						t = time_out = 0;
						Data_2560.PrepareOK = 0x00;//充电机准备未完成
						BMS_STA = SEND_2560;
					}else if(time_out > 240)	BMS_STA = TIME_OUT;//BRO 60s内未准备好
				}else if(time_out >20)	BMS_STA = TIME_OUT;//接受2304_BRO BMS准备充电就绪超时5s
				break;
			}
			
			case SEND_2560://充电机输出准备就绪
			{	
				t = (t+1)%(CRO_2560.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CRO_2560);}
				//判断枪电压与报文电池电压相差5%以内，在充电机最大最小电压之间
				//调整电源模块输出电压高于电池电压1-10v后，闭合继电器K1K2后变更为0xAA
				if(1)	Data_2560.PrepareOK = 0xAA;
				if(RX_BMS_TAB[WAIT_4096_BCL].Rx_status == 1)
				{
					//RX_BMS_TAB[WAIT_4096_BCL].Rx_status = RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 0;
					time_out = 0;				
				}else if(time_out > 4)	BMS_STA = TIME_OUT;//接受4096电池充电需求超时250*4=1s
				if(RX_BMS_TAB[WAIT_4352_BCS].Rx_status == 1)
				{
					t = time_out = 0;
					BMS_STA = SEND_4608;
				}else if(time_out > 20)	BMS_STA = TIME_OUT;//4352电池充电总状态报文超时5s
				break;
			}		
			case SEND_4608://充电机充电状态
			{
				t = (t+1)%(CCS_4608.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CCS_4608);
					Data_4608.OutputCurr = Data_4096.NeedCurr+10;//测试用
					Data_4608.OutputVolt = Data_4096.NeedVolt-10;//实际使用拿ACDC模块数据
					Data_4608.ChargingTime++;}
				
				if(RX_BMS_TAB[WAIT_4864_BSM].Rx_status == 1)
				{
					RX_BMS_TAB[WAIT_4864_BSM].Rx_status = 0;
					//if(1)//电池状态标签是否全部正常？else err
					//if(1)充电禁止标记ON?->充电暂停，继续接受判断4864BSM电池状态是否正常�	
				}
				//if(1)充电结束条件是否成立？成立发送6656充电机中止充电CTS
				break;
			}
			case SEND_6656://充电机中止充电
			{
				t = (t+1)%(CST_6656.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CST_6656);}

				if((RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)||(time_out > 500))//收到BMS中止充电BST||发送CTS10*100*5=5s后
				{
					//电力输出停止操作
					t = time_out = 0;
					BMS_STA = SEND_7424;
				}
				break;
			}			
			case SEND_7424://充电机统计数据报文
			{
				t = (t+1)%(CSD_7424.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CSD_7424);}
				//断开DC继电器K1K2
				//泄放电路动作

				if(RX_BMS_TAB[WAIT_7168_BSD].Rx_status == 1)//收到BMS统计数据BSD
				{
					//辅助电源继电器断开K3K4电子锁解锁
					t = time_out = 0;
					BMS_STA = BEGIN;
				}//else if(time_out > 40)	BMS_STA = TIME_OUT;//接受BMS统计数据报文超时10s		
				break;
			}
			case TIME_OUT:
			{
				if(BMS_STA != TIME_OUT)	OUT++;
				if(OUT > 3)//重连超过3次不再重连
				{
					//电力输出停止操作
					if(time_out > 1)
					{
						//断开K1K2
						//泄放电路动作
						//辅助电源继电器K3K4断开
						//电子锁解锁
						t = time_out = 0;
						//BMS_STA = ;//必须重新拔枪插抢才可以继续开始
					}							
				}
				else 
				{
					t = (t+1)%(CEM_7936.Period/BMS_Task_Time);
					if(t == 1)	{time_out++;	BMS_Send(CEM_7936);}
					//电力输出停止操作
					if(time_out > 1)
					{
						//断开K1K2
						t = time_out = 0;
						BMS_STA = SEND_256;//重新开始辨识
					}					
				}
				break;
			}
			default:break;
		}
		
		osDelay(BMS_Task_Time);
	}
}



static void BMS_Send(TX_BMS Pbuf)
{
	TxMsg1.ExtId = (Pbuf.IFArbition<<26) | (Pbuf.PGN_S<<16) | (Pbuf.PDUSpecific<<8) | (Pbuf.SourceAddress);
	TxMsg1.DLC = Pbuf.DLC;
	memset(TxMsg1.Data,0,sizeof(TxMsg1.Data));
	memcpy(TxMsg1.Data,Pbuf.Data,Pbuf.DLC);
	CAN_Transmit(CAN1, &TxMsg1);
}




static void Single_Package_Deal(void)
{
	if(BMS_Rx_Flag1 == 1)
	{	
		for(char i = WAIT_9984_BHM; i <= WAIT_7680_BEM; i++)
		{
			if((unsigned char)((BMS_RX_1.ExtId&0x00ff0000)>>16) == RX_BMS_TAB[i].PGN_S)//提取报文PGN号
			{
				RX_BMS_TAB[i].Rx_status = 1;
				memcpy(RX_BMS_TAB[i].Data,BMS_RX_1.Data,BMS_RX_1.DLC);
			}
		}
		BMS_Rx_Flag1 = 0;
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
		for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++) \
			if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	P = (unsigned char*)RX_BMS_TAB[i].Data;
		
		TxMsg_Ack.Data[1] = J1939_Multi_Package[3];//可发送包数	
		TxMsg_Ack.Data[6] = J1939_Multi_Package[6];//PGN*/							
		CAN_Transmit(CAN1, &TxMsg_Ack);
		if(TxMsg_Ack.Data[6] == 0x11)	CAN_Transmit(CAN1, &TxMsg_Ack);//应答4352得发两遍！真的有毒！！！
	}
	else if(BMS_RX_0.ExtId == 0X1CEB56F4)//多包数据传输
	{				
		if(BMS_RX_0.Data[0] == 1)	memcpy(P,&BMS_RX_0.Data[1],7); \
			else if(BMS_RX_0.Data[0] == 2)	memcpy(P+7,&BMS_RX_0.Data[1],7);//重组多包数据
				
		if(BMS_RX_0.Data[0] == J1939_Multi_Package[3])//多包接受完成？
		{
			for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++)
				if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	RX_BMS_TAB[i].Rx_status  = 1;				
			TxMsg_Done.Data[1] = J1939_Multi_Package[1];
			TxMsg_Done.Data[3] = J1939_Multi_Package[3];
			TxMsg_Done.Data[6] = J1939_Multi_Package[6];
			CAN_Transmit(CAN1, &TxMsg_Done);//应答多包接受完成
		}
	}
}

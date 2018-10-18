#include "main.h"
#include "bms.h"
#include "can.h"

#pragma pack(1)   //强制1字节对齐
/*充电桩发给BMS的数据*/
stuPGN9728Type Data_9728 = {0x01,0x01,0x00};
stuPGN256Type	 Data_256  = {0x00,0x0100000,};
stuPGN1792Type Data_1792;
stuPGN2048Type Data_2048 = {7500,	2000,	4000-2000};//充电机输出电压：200-750V 输出电流：0-200A
stuPGN2560Type Data_2560;
stuPGN4608Type Data_4608 = {0,0,0,0x01};//默认允许充电2015GB
stuPGN6656Type Data_6656;
stuPGN7424Type Data_7424;
stuPGN7936Type Data_7936;
/*BMS发过来的数据*/
stuPGN9984Type Data_9984;
stuPGN512Type	 Data_512;
stuPGN1536Type Data_1536;
stuPGN2304Type Data_2304;
stuPGN4096Type Data_4096;
stuPGN4352Type Data_4352;
stuPGN4864Type Data_4864;
stuPGN6400Type Data_6400;
stuPGN7168Type Data_7168;
stuPGN7680Type Data_7680;
#pragma pack()

									/*报文PGN		优先级	目的地址	源地址	数据长度		发送周期ms		数据区*/		
TX_BMS CHM_9728 = {(9728>>8),		6,		 0xf4,		 0x56,		 3,					250,			&Data_9728};//充电机握手报文
TX_BMS CRM_256 = 	{(256 >>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_256 };//充电机辨识报文
TX_BMS CTS_1792 = {(1792>>8),		6,		 0xf4,		 0x56,	 	 7,					250,			&Data_1792};//充电机发送时间同步(可选)
TX_BMS CML_2048 = {(2048>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_2048};//充电机最大输出能力
TX_BMS CRO_2560 = {(2560>>8),		4,		 0xf4,		 0x56,		 1,					250,			&Data_2560};//充电机输出准备就绪
TX_BMS CCS_4608 = {(4608>>8),		6,		 0xf4,		 0x56,		 8,					50,				&Data_4608};//充电机充电状态
TX_BMS CST_6656 = {(6656>>8),		4,		 0xf4,		 0x56,		 4,					10,				&Data_6656};//充电机中止充电
TX_BMS CSD_7424 = {(7424>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_7424};//充电机统计数据
TX_BMS CEM_7936 = {(7936>>8),		2,		 0xf4,		 0x56,		 4,					250,			&Data_7936};//充电机错误报文

#define RX_BMS_NUM 10							/*接受标志		报文PGN					数据区*/
RX_BMS RX_BMS_TAB[RX_BMS_NUM] =	{	{		0,			(9984>>8),			&Data_9984},	//BMS握手报文
																	{		0,			(512>>8),				&Data_512	},	//BMS车辆辨识报文
																	{		0,			(1536>>8),			&Data_1536},	//动力蓄电池充电参数
																	{		0,			(2304>>8),			&Data_2304},	//电池准备就绪报文
																	{		0,			(4096>>8),			&Data_4096},	//电池充电需求报文
																	{		0,			(4352>>8),			&Data_4352},	//电池充电总状态报文
																	{		0,			(4864>>8),			&Data_4864},	//动力蓄电池状态信息
																	{		0,			(6400>>8),			&Data_6400},	//BMS中止充电报文
																	{		0,			(7168>>8),			&Data_7168},	//BMS统计数据报文
																	{		0,			(7680>>8),			&Data_7680}}; //BMS错误报文

unsigned char J1939_Multi_Package[8];/*0x10,len_L,len_H,包数,0xff,PGN[3]*/
CanTxMsg TxMsg1 =  {0, 0, CAN_Id_Extended, CAN_RTR_Data, 0, {0}};//扩展帧 数据帧					
unsigned char BMS_STA = SEND_256;
void BMS_Task(void const *argument)
{
	const unsigned char BMS_Task_Time = 10U;//10ms循环
	static unsigned short t,time_out;
	unsigned char *P = NULL;
	while(1)
	{
		//处理BMS接受
		if(BMS_Recevie_Flag == 1)
		{		
			BMS_Recevie_Flag = 0;
//可以先试着把这一部分放在中断里，快速响应多包传输请求
//			if(RxMsg1.ExtId == 0X1CEC56F4)//BMS请求建立多包发送连接
//			{	memcpy(buf,RxMsg1.Data,RxMsg1.DLC);
//				if(RxMsg1.Data[0] == 0x10)
//				{
//					memcpy(J1939_Multi_Package,RxMsg1.Data,RxMsg1.DLC);//保存多包发送连接的配置数据					
//					TxMsg1.ExtId = 0X1CECF456;			 //应答多包发送请求
//					TxMsg1.DLC = 8;																	
//					TxMsg1.Data[0] = 0x11;									//应答头	
//					TxMsg1.Data[1] = J1939_Multi_Package[3];//可发送包数
//					TxMsg1.Data[2] = 1;											//包号
//					TxMsg1.Data[3] = TxMsg1.Data[4] = 0xff;//缺省值		
//					memcpy(&TxMsg1.Data[5],&J1939_Multi_Package[5],3);//PGN*/							
//					Can_Send_Msg(CAN1, &TxMsg1);
//				}
//			}
//			else
				if(RxMsg1.ExtId == 0X1CEB56F4)//多包数据传输
			{				
				P = (unsigned char*)malloc(J1939_Multi_Package[1]);//申请内存用于存储多包数据
				memcpy(P+((RxMsg1.Data[0]-1)*7),&RxMsg1.Data[1],7);//重组多包数据			
				if(RxMsg1.Data[0] == J1939_Multi_Package[3])//多包接受完成？
				{
					TxMsg1.ExtId = 0X1CECF456;//响应完成多包接受
					TxMsg1.DLC = 8;					
					TxMsg1.Data[0] = 0x13;//应答头						
					memcpy(&TxMsg1.Data[1],&J1939_Multi_Package[1],7);					
					Can_Send_Msg(CAN1, &TxMsg1);//应答多包接受完成
					
					if(J1939_Multi_Package[6] == (uint8_t)(512>>8))		{memcpy(&Data_512,P,sizeof(Data_512));		RX_BMS_TAB[WAIT_512_BRM].Rx_status  = 1;}
					if(J1939_Multi_Package[6] == (uint8_t)(1536>>8))	{memcpy(&Data_1536,P,sizeof(Data_1536));	RX_BMS_TAB[WAIT_1536_BCP].Rx_status = 1;}
					if(J1939_Multi_Package[6] == (uint8_t)(4352>>8))	{memcpy(&Data_4352,P,sizeof(Data_4352));	RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 1;}
					free(P);//释放内存		
				}
			}
			else
			{
				for(char i = 0; i < RX_BMS_NUM; i++)
				{
					unsigned char PGN_t = (RxMsg1.ExtId&0x00ff0000)>>16;//提取报文PGN号
					if(PGN_t == RX_BMS_TAB[i].PGN_S)
					{
						memcpy(RX_BMS_TAB[i].Data,RxMsg1.Data,RxMsg1.DLC);
						RX_BMS_TAB[i].Rx_status = 1;
					}
				}
			}
		}
	
		//BMS发送
		switch(BMS_STA)
		{
			case SEND_9728://充电握手
			{
				//if(everything is ok)插头 cc 锁 辅助电源闭合？
				t = (t+1)%(CHM_9728.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CHM_9728);}
				
				if((RX_BMS_TAB[WAIT_9984_BHM].Rx_status == 1) || (time_out >= 20))//收到9984||超时5s	GB2015
				{
					RX_BMS_TAB[WAIT_9984_BHM].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_256;
				}
				break;
			}
			
			case SEND_256://充电辨识
			{
				//if(绝缘 泄放 k1k2接触器电压)
				if(RX_BMS_TAB[WAIT_512_BRM].Rx_status == 1)	
				{
					Data_256.IdentifyResult = 0xAA;//辨识成功
					time_out = 0;
				}
				else 
				{
					Data_256.IdentifyResult = 0x00;
					//if(time_out >= 20)//接受512报文超时
				}
				t = (t+1)%(CRM_256.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CRM_256);}
				
				if(RX_BMS_TAB[WAIT_1536_BCP].Rx_status == 1)
				{
					//RX_BMS_TAB[WAIT_1536_BCP].Rx_status = RX_BMS_TAB[WAIT_512_BRM].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_2048;				
				}//else if(time_out >= 20)//接受1536报文超时
				break;
			}
			
			case SEND_2048://充电机最大输出能力/时间同步
			{
				t = (t+1)%(CML_2048.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CML_2048);	BMS_Send(CTS_1792);}
				
				if((RX_BMS_TAB[WAIT_2304_BRO].Rx_status==1) && (Data_2304.BMSSta==0xAA))
				{
					RX_BMS_TAB[WAIT_2304_BRO].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_2560;				
				}//else if(time_out >=20)//接受2304BMS准备充电就绪超时
				break;
			}
			
			case SEND_2560://充电机输出准备就绪
			{
				if(1)Data_2560.PrepareOK = 0xAA;//判断输出电压与报文电池电压相差5%以内，在充电机最大最小电压之间。。。
					else Data_2560.PrepareOK = 0x00;
				t = (t+1)%(CRO_2560.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CRO_2560);}

				if((RX_BMS_TAB[WAIT_4096_BCL].Rx_status==1))
				{
					RX_BMS_TAB[WAIT_4096_BCL].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_4608;				
				}//else if(time_out >= 20)//接受4096电池充电需求+4352电池充电总状态报文超时
				break;
			}
			
			case SEND_4608://充电机充电状态
			{
				t = (t+1)%(CCS_4608.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CCS_4608);
					Data_4608.OutputCurr = Data_4096.NeedCurr+10;//测试用
					Data_4608.OutputVolt = Data_4096.NeedVolt-10;//实际使用拿ACDC模块数据
					Data_4608.ChargingTime++;}
				
				if(RX_BMS_TAB[WAIT_4352_BCS].Rx_status == 1)
				{
					time_out = 0;
					//RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 0;
				}//else if(time_out>=100)//4352电池充电总状态报文超时->停止充电！
				

				if(RX_BMS_TAB[WAIT_4864_BSM].Rx_status == 1)
				{
					RX_BMS_TAB[WAIT_4864_BSM].Rx_status = 0;
					
					//if(1)//电池状态是否正常	充电禁止标记?充电结束条件是否成立
					//BMS_STA = SEND_2048;				
				}			
				break;
			}
			
			default:break;
	
		}


		
		
		osDelay(BMS_Task_Time);
	}
}



//返回值：0成功；1失败
unsigned char BMS_Send(TX_BMS Pbuf)
{
	TxMsg1.ExtId = (Pbuf.IFArbition<<26) | (Pbuf.PGN_S<<16) | (Pbuf.PDUSpecific<<8) | (Pbuf.SourceAddress);
	TxMsg1.DLC = Pbuf.DLC;
	memset(TxMsg1.Data,0,sizeof(TxMsg1.Data));
	memcpy(TxMsg1.Data,Pbuf.Data,Pbuf.DLC);
	return Can_Send_Msg(CAN1, &TxMsg1);
}


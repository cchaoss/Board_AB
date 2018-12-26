#include "main.h"
#include "bms.h"
#include "can.h"
#include "adc.h"
#include "acdc_module.h"
#include "electric_meter.h"

#pragma pack(1)//强制1字节对齐
stuPGN9728Type Data_9728 = {0x01,0x01,0x00};
stuPGN256Type	 Data_256;
stuPGN1792Type Data_1792;
stuPGN2048Type Data_2048 = {ACDC_MAX_VOL,ACDC_MIN_VOL,ACDC_MAX_CUR,ACDC_MIN_CUR};//充电机输出电压：200-750V 输出电流：0-200A
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
unsigned char OUT,guzhang;
enum _BMS_STA BMS_STA = BEGIN;

void BMS_Task(void const *argument)
{
	const unsigned char BMS_Task_Time = 25U;//10ms循环
	static unsigned short t,t1,timeout,timeout1;
	static bool once_run = true,once_run_lock = true;
	static unsigned short fuck1,fuck2,count;
	while(1)
	{	
		if((Board_Type == 0X0A)&&(Board_C_Sta!=0))//中间继电器由A板负责控制检测
		{
			if(Type_Control_Cmd.KK_Sta == 1)/*单独控制中间继电器状态*/
			{
				fuck1=0;fuck2++;
				if(++count==(5000/BMS_Task_Time))	{Close_KK;	count = 0;}//5s后再闭合中间继电器
			}
			else if(Type_Control_Cmd.KK_Sta == 0)	{Open_KK;	count = 0;	fuck1++;fuck2=0;}
			if((fuck1 == (5000/BMS_Task_Time))||(fuck2 == (10000/BMS_Task_Time)))/*专门为了检查中间继电器*/	
			{
				fuck1 = fuck2 = 0;
				if((DI_Ack.KK_H!=0xFF)&&(DI_Ack.KK_H!=((KK_PORT->IDR>>KK_PIN1)&1)))	guzhang = KK_Relay_Err;
				if((DI_Ack.KK_L!=0xFF)&&(DI_Ack.KK_L!=((KK_PORT->IDR>>KK_PIN2)&1)))	guzhang = KK_Relay_Err;//中间继电器错误（这个继电器有点鸡贼）
			}
		}
		
		if((BMS_STA>=SEND_9728)&&(BMS_STA<=SEND_4608))
		{
			if(Type_DM.JiTing==1)//人工停止-急停按下
			{
				Data_6656.ChargStopChargingReason = Mannul_Stop;
				Type_BMS.Manual = JT_Stop;	
				t = timeout = 0;	
				BMS_STA = SEND_6656;
			}
			if((AD_DATA.CC<2.8)||(AD_DATA.CC>5.2))//充电过程中CC电压异常
			{
				Data_6656.ChargStopChargingReason = Err_Stop;
				guzhang = CC_ERR;//CC信号错误		
				t = timeout = 0;	
				BMS_STA = SEND_6656;
			}
		}
		
		Single_Package_Deal();//负责接送处理单包数据
		switch(BMS_STA)//BMS发送处理
		{
			case BEGIN://插抢准备		
				if((Type_DM.DErr==0)&&(Type_DM.JiTing==0)&&((AD_DATA.CC>2.8)&&(AD_DATA.CC<5.2)))//桩无故障 急停未按下	CC插抢	
				{	//未连接C板通过启停开关开启充电->单枪版本！	//连接C板通过C板下发的开启充电指令->双枪联网版本！
					if(((Board_C_Sta==0)&&(DI_Ack.START==1))||((Board_C_Sta==0x01)&&Type_Control_Cmd.CMD.Start_Stop))
						BMS_STA = LOCKED;
				}
			break;
				
			case LOCKED:
				if((DI_Ack.LOCK == 1)||(DIPSwitch.Bits_4 == 0))//上锁就绪||关闭上锁控制
				{	
					Close_K3K4;//闭合辅助电源继电器K3K4
					Tim2_Init(60000,36000);//开始累计充电时间
					MeterData.kwh_start = MeterData.kwh_realtime;//记录本次开始充电是的电表初始值
					t = timeout = 0;
					BMS_STA = SEND_9728;
				}
				else//锁状态-未上锁-锁枪
				{
					t = (t+1)%(2500/BMS_Task_Time);
					if(t == 1)	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN1,1);
					if(t == (100/BMS_Task_Time))	{GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN1,0);	timeout++;}
					if(timeout == 2)//超时5s锁未锁好->结束充电需要再次拔枪
					{	
						Data_6656.ChargStopChargingReason = Err_Stop;	
						Data_6656.ChargFaultReason |=0x0400;//其他故障	
						guzhang = Lock_ERR;//无法上锁
						t = timeout = 0;	
						BMS_STA = SEND_6656;
					}
				}
			break;
			
			case SEND_9728://充电握手
				t = (t+1)%(CHM_9728.Period/BMS_Task_Time);
				if(t == 1)	BMS_Send(CHM_9728);
				
				if(((Board_Type==0x0A)&&((Type_Control_Cmd.Module_Assign == 0xAA)||(Type_Control_Cmd.Module_Assign == 0xAB))) \
					||((Board_Type==0x0B)&&((Type_Control_Cmd.Module_Assign == 0xBB)||(Type_Control_Cmd.Module_Assign == 0xAB))))//无暂停使能+已分配模块可用
				{
					timeout = (timeout+1)%(7000/BMS_Task_Time);
					if(DIPSwitch.Bits_3 == 1)//第3位拨码开关开启绝缘检查
					{
						if(timeout == (200/BMS_Task_Time))	{ACDC_Set_Vol_Cur(5000,0);	Close_K1K2;}//电压模块输出电压 闭合枪上继电器K1K2
						if(timeout == (5000/BMS_Task_Time))	Start_Insulation_Check();//开启绝缘检查(电压达到500v后再开启检查)5s后
						if(timeout == (5800/BMS_Task_Time))//800ms后取绝缘检查结果
						{
							ACDC_Set_Vol_Cur(0,0);//关闭电源
							if(AD_DATA.VT_Return == 1)
							{	
								Data_6656.ChargStopChargingReason = Err_Stop;
								Data_6656.ChargFaultReason |=0x0400;//其他故障	
								guzhang = Insulation_ERR;//绝缘检查错误						
								t = timeout = 0;	
								BMS_STA = SEND_6656;
							}
						}
					}
					if(timeout == (6000/BMS_Task_Time))	
					{
						Open_K1K2;//200ms后断开枪上继电器K1K2
						t = timeout = 0;
						BMS_STA = SEND_256;
					}
				}
				//if((RX_BMS_TAB[WAIT_9984_BHM].Rx_status == 1)||(timeout > 20))//收到9984||超时5s->GB2015
				//if(k1k2接触器电压<10V)//guzhang = Gun_Vol_ERR;//接触器外侧电压>10V//需要重新拔枪
				//if(泄放电路检查ok？)//guzhang = Tap_Check_ERR;//泄放检查错误//停用本桩	
			break;	
			
			case SEND_256://充电辨识
				t = (t+1)%(CRM_256.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CRM_256);}
				
				if(RX_BMS_TAB[WAIT_512_BRM].Rx_status == 1)	{timeout = 0;	Data_256.IdentifyResult = 0xAA;}//辨识成功	清空超时时间
					else if(timeout > 20)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.IdentifyTimeOut |= 0x01;}//接收车辆辨识报文BRM超时5s
				
				if(RX_BMS_TAB[WAIT_1536_BCP].Rx_status == 1)	{t = timeout = 0;	BMS_STA = SEND_2048;}
					else if(timeout > 30)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x01;}//接受1536电池充电参数超时7s(default 5s)
			break;
			
			case SEND_2048://充电机最大输出能力/时间同步
				t = (t+1)%(CML_2048.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CTS_1792);}
				if(t == 2)	BMS_Send(CML_2048);
				if(RX_BMS_TAB[WAIT_2304_BRO].Rx_status == 1)
				{
					if(Data_2304.BMSSta==0xAA)	{t=timeout=timeout1=0;	BMS_STA = SEND_2560;}
						else if(timeout>240)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x04;}//BRO在60s内未准备好
				}else if(timeout > 20)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x04;}//接受BROBMS准备充电就绪超时5s
			break;
			
			case SEND_2560://充电机输出准备就绪
				t = (t+1)%(CRO_2560.Period/BMS_Task_Time);
				if(t == 1)	
				{
					BMS_Send(CRO_2560);
					LED_CHARGE_ON;//充电灯亮起
					ACDC_Set_Vol_Cur(Data_1536.BatPreVolt+55,0);//预充
					if(Data_2560.PrepareOK == 0xAA) timeout++;//充电机准备就绪后才计算超时
					timeout1++;
				}
				if((Module_Status.Output_Vol*10>Data_1536.BatPreVolt)||(timeout1 > 20))	Data_2560.PrepareOK = 0xAA;//充电机准备就绪//调整电源模块输出电压高于电池电压1-10v				
				
				if((RX_BMS_TAB[WAIT_4352_BCS].Rx_status==1)&&(RX_BMS_TAB[WAIT_4096_BCL].Rx_status==1))	
				{
					Close_K1K2;//闭合枪上主继电器K1K2
					t=t1=timeout=timeout1=0;	
					BMS_STA = SEND_4608;
				}
				else if(timeout>20)	{t=timeout=0;	Data_7936.BMSChargingStaTimeOut|=0x05;	BMS_STA=TIME_OUT;}//4096电池充电需求超时5s 4352电池充电总状态报文超5s(共生共死)
			break;
			
			case SEND_4608:/*充电机充电状态(充电阶段)*/
				OUT = 0;//通讯超时重连次数清零：到充电阶段说明所有报文都收到一遍
				t = (t+1)%(CCS_4608.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;timeout1++;	BMS_Send(CCS_4608);}
				
				if((RX_BMS_TAB[WAIT_4096_BCL].Rx_status==0)&&(timeout>5))//接受4096电池充电需求超时1s
				{
					Data_7936.BMSChargingStaTimeOut|=0x04;
					t = timeout = 0;	BMS_STA=TIME_OUT;	
				}
				if((RX_BMS_TAB[WAIT_4352_BCS].Rx_status==0)&&(timeout1>20))//4352电池充电总状态报文超5s
				{
					Data_7936.BMSChargingStaTimeOut|=0x01;
					t = timeout = 0;	BMS_STA=TIME_OUT;
				}
				if(RX_BMS_TAB[WAIT_4096_BCL].Rx_status==1)	{timeout = 0;	RX_BMS_TAB[WAIT_4096_BCL].Rx_status = 0;}//接受到报文清空标记
				if(RX_BMS_TAB[WAIT_4352_BCS].Rx_status==1)	{timeout1= 0;	RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 0;}
				
				if(RX_BMS_TAB[WAIT_4864_BSM].Rx_status==1)
				{
					RX_BMS_TAB[WAIT_4864_BSM].Rx_status = 0;
					if((Data_4864.BatSta==0)&&((Data_4864.BatConnetSta&0x0f)==0))//电池状态位都正常
					{
						if(Data_4864.BatConnetSta&0x10)	ACDC_Set_Vol_Cur(Data_4096.NeedVolt,4000-Data_4096.NeedCurr);//允许充电
							else	ACDC_Set_Vol_Cur(0,0);//充电暂停	
					}else	{t=timeout=0;	BMS_STA = SEND_6656;}//电池异常停止充电
				}

				t1 = (t1+1)%(300/BMS_Task_Time);/*300ms周期更新电源模块输出、检测继电器状态是否正确*/
				if(t1 == (300/BMS_Task_Time)-1)
				{
					Data_4608.OutputVolt = Module_Status.Output_Vol*10;
					Data_4608.OutputCurr = 4000-Module_Status.Output_Cur*10;
					Data_4608.ChargingTime = Bound(ChargTime>>1,600,0);
					if((DI_Ack.GUN!=0xFF)&&(DI_Ack.GUN!=((K_GUN_PORT->IDR>>K_GUN_PIN)&1)))	guzhang = GUN_Relay_Err;//枪上继电器错误
					if((guzhang==GUN_Relay_Err)||(guzhang==KK_Relay_Err))
					{	
						Data_6656.ChargStopChargingReason = Err_Stop;	
						Data_6656.ChargFaultReason |=0x04;//充电机连接器故障
						t = timeout = 0;	
						BMS_STA = SEND_6656;
					}
				}
				
				/*如果采用不中断充电的切换方式，则这里不会进来，因为C板没有置位Suspend*/
				if(Type_Control_Cmd.CMD.Suspend)//C板收到消息后将Type_Control_Cmd.CMD.Suspend置0
				{
					ACDC_Set_Vol_Cur(0,0);//充电暂停->分配电源模块！
					if(Module_Status.Output_Vol < 10)//等待电流<10A 
						{Open_K1K2;	Type_DM.M_Suspend_ACK = 1;}//断开枪上继电器//告知C板 本枪暂停ok了
				}else if(Type_DM.M_Suspend_ACK == 1)	{Close_K1K2;	Type_DM.M_Suspend_ACK = 0;}//闭合枪上继电器 情况暂停应答标志 继续充电
				/**********************************************************************/
				
				if(RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)//收到BMS中止充电
				{
					Data_6656.ChargStopChargingReason = BMS_Stop;//BMS停止
					t = timeout = 0;	
					BMS_STA = STOP;
				}
				
				if((Board_C_Sta == 0)&&(DI_Ack.START==0))//未连接C板通过启停开关开启充电//人工停止-启停开关
				{
					Data_6656.ChargStopChargingReason = Mannul_Stop;
					Type_BMS.Manual = Start_Stop;	
					t = timeout = 0;	
					BMS_STA = SEND_6656;
				}
				if((Board_C_Sta != 0)&&(Type_Control_Cmd.CMD.Start_Stop==false))//人工停止-刷卡/APP停止
				{
					Data_6656.ChargStopChargingReason=Mannul_Stop;	
					if(Type_Control_Cmd.CMD.Type)	Type_BMS.Manual = Card_Stop;	
						else Type_BMS.Manual = App_Stop;
					t = timeout = 0;	
					BMS_STA = SEND_6656;
				}
			break;
			
			case SEND_6656://充电机中止充电
				BMS_Send(CST_6656);
				if(guzhang == 0)
				{
					timeout++;
					if((RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)||(timeout>(2000/BMS_Task_Time)))//收到BMS中止充电BST||发送CTS10*500=5s后
					{
						BMS_Send(CSD_7424);//充电机统计数据报文
						t = timeout = 0;
						BMS_STA = STOP;
					}
				}
				else
				{
					LED_ERR_ON;//故障灯亮起	
					t = timeout = 0;	
					BMS_STA = STOP;
				}
			break;			

			case TIME_OUT://超时重连处理（7936发送3条后重新开始辨识）
				if(OUT < 3)//3次以内超时则重连
				{
					t = (t+1)%(600/BMS_Task_Time);
					if(t == 1)	{ACDC_Set_Vol_Cur(0,0);	BMS_Send(CEM_7936);}//电力输出停止操作//发送充电机错误报文
					if(t == (250/BMS_Task_Time))	BMS_Send(CEM_7936);//发送充电机错误报文
					if(t == (500/BMS_Task_Time))//delay200ms(或者电流5A以下)-保护继电器寿命
					{	
						BMS_Send(CEM_7936);//发送充电机错误报文
						Open_K1K2;//断开枪上主继电器K1K2
						OUT++;//重连次数+1
						BMS_Data_Init();//报文接受标记
						t=timeout=0;	
						BMS_STA = SEND_256;//重新开始辨识
					}					
				}
				else {t=timeout=0;	BMS_STA = STOP;}//3次通讯超时重连后再超时则不再重连 直接结束充电					
			break;
			
			case STOP://结束充电操作后等待再次插抢->BEGIN
				if(once_run)
				{
					LED_CHARGE_OFF;//充电灯熄灭
					t = (t+1)%(600/BMS_Task_Time);
					if(t == 1)	ACDC_Set_Vol_Cur(0,0);//电力输出停止操作
					if(t == (200/BMS_Task_Time))	{Open_K3K4;	Open_K1K2;	once_run = false;}//断开辅助电源K3K4//断开枪上主继电器K1K2//delay200ms(或者电流5A以下)-保护继电器寿命
				}
				else if(Type_Control_Cmd.CMD.Account&&once_run_lock)
				{
					if(t == (200/BMS_Task_Time))	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,1);//打开锁
					if(t == (300/BMS_Task_Time))	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,0);
					if(t == (450/BMS_Task_Time))	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,1);//打开锁
					if(t == (550/BMS_Task_Time))	{GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,0);once_run_lock=false;}
					t++;
				}
				if((once_run_lock==false)&&(AD_DATA.CC > 5.5f))//必须重新拔枪插抢才可以下一次充电
				{
					once_run=once_run_lock = true;	
					t=t1=timeout=timeout1=0;
					OUT = 0;//清空重连次数					
					BMS_Data_Init();//清空本次充电的报文数据
					LED_ERR_OFF;//故障灯熄灭
					BMS_STA = BEGIN;
				}
			break;
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
	guzhang = 0;//故障位清零
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
				RX_BMS_TAB[i].Rx_status = 1;	memcpy(RX_BMS_TAB[i].Data,BMS_RX_1.Data,BMS_RX_1.DLC);
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
	int C = Bound(cur,4000-ACDC_MAX_CUR,0)*100;//0-300A
	ACDC_VolCur_Buffer[0] = V>>24;
	ACDC_VolCur_Buffer[1] = V>>16;
	ACDC_VolCur_Buffer[2] = V>>8;
	ACDC_VolCur_Buffer[3] = V>>0;
	ACDC_VolCur_Buffer[4] = C>>24;
	ACDC_VolCur_Buffer[5] = C>>16;
	ACDC_VolCur_Buffer[6] = C>>8;
	ACDC_VolCur_Buffer[7] = C>>0;
}

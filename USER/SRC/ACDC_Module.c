#include "main.h"
#include "can.h"
#include "acdc_module.h"
       
uint8_t ACDC_VolCur_Buffer[8];
ACDC_Status Module_Status;//电源模块的个数 温度 状态 电压电流		 
Module_Rx_Flag_Bits	Module_Rx_Flag;//电源模块报文接收标记
CanTxMsg TxMsg_ACDC = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//扩展帧 数据帧

enum _ACDC_STA ACDC_STA = Set_Group;
void ACDC_Module_Task(void const *argument)
{
	const unsigned short ACDC_Module_Task_Time = 125U;
	static unsigned char number,module_init_time;
	Module_Rx_Flag.ON_OFF_STA = true;
	while(1)
	{
		ACDC_RxMsg_Deal();//处理模块应答的数据
		
		//if(ACDC_STA < Read_Status)	Type_DM.DErr = No_Module;//故障：无电源模块！
		switch(ACDC_STA)
		{
			case Set_Group:
			{
				if(Module_Rx_Flag.All_num&&(Module_Status.num>0))//读取模块总数成功&&模块数量>0
				{	
					Module_Rx_Flag.All_num = false;	number = 0;	ACDC_STA = Group_Verify;			
				}
				else	if(++module_init_time > 10000/ACDC_Module_Task_Time)//上电10s等待模块硬件初始化完成
				{	
					module_init_time = 10000/ACDC_Module_Task_Time + 1;
					TxMsg_ACDC.ExtId = Total_Number_Read;//读取模块总数
					memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);		
				}
			}break;
			
			case Group_Verify://读取各组模块数量，确认每个组至少有一个模块
			{
				if(Board_Type == 0x0A)
				{
					if(Module_Rx_Flag.A_num)
					{
						Module_Rx_Flag.A_num = false;
						if(Module_Status.numA == 0)	ACDC_STA = Set_Group;//重新开始							
						else 
						{
							Type_DM.MNum = Module_Rx_Flag.A_num;
							ACDC_STA = Read_Status;//至少有一个模块就可以正常工作
						}
					}
					else{TxMsg_ACDC.ExtId = GroupA_Number_Read;	CAN_Transmit(CAN2, &TxMsg_ACDC);}//读取A组模块数量
				}
				else if(Board_Type == 0x0B)//B板
				{
					if(Module_Rx_Flag.B_num)
					{
						Module_Rx_Flag.B_num = false;
						if(Module_Status.numB == 0)	ACDC_STA = Read_Status;//至少有一个模块就可以正常工作
						else
						{
							Type_DM.MNum = Module_Rx_Flag.B_num;
							ACDC_STA = Set_Group;//重新设置组号
						}
					}
					else{TxMsg_ACDC.ExtId = GroupB_Number_Read;	CAN_Transmit(CAN2, &TxMsg_ACDC);}//读取A组模块数量	
				}
			}break;
			
			case Read_Status://读取模块状态
			{	
				if(Board_Type == 0x0A)//A板子负责轮询所有模块状态 温度信息
				{	
					TxMsg_ACDC.ExtId = Single_Module_Sta_Read|(number<<8);//读取模块状态/温度/组号
					if(++number==Module_Status.num)	number = 0;
					memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);
				}
				ACDC_STA =	Read_Vol_Cur;
			}break;
			
			case Read_Vol_Cur:
			{	
				if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Vol_Cur_Read;//读取A组电压电流
				else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Vol_Cur_Read;//读取B组电压电流	
				memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);
				ACDC_STA =	Set_Vol_Cur;
			}break;
			
			case Set_Vol_Cur://设置电压电流 开关机
			{
				if(ACDC_VolCur_Buffer[3] != 0)//电压不为0
				{
					if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = Set_GroupA_Vol_Cur;
					else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = Set_GroupB_Vol_Cur;
					memcpy(TxMsg_ACDC.Data,ACDC_VolCur_Buffer,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);
					if(Module_Rx_Flag.ON_OFF_STA)//开机一次
					{
						Module_Rx_Flag.ON_OFF_STA = false;
						if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Module_ONOFF;
						else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Module_ONOFF;
						memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);//0为开机
					}
				}
				else//电压为0则执行关机
				{
					if(!Module_Rx_Flag.ON_OFF_STA)//开过机才关机一次
					{
						Module_Rx_Flag.ON_OFF_STA = true;
						if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Module_ONOFF;
						else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Module_ONOFF;
						memset(TxMsg_ACDC.Data,0,8);					
						TxMsg_ACDC.Data[0] = 1;	CAN_Transmit(CAN2, &TxMsg_ACDC);//1为关机
					}
				}
				ACDC_STA =	Read_Status;
			}break;
			default:break;
		}
		osDelay(ACDC_Module_Task_Time);
	}
}


sta_ack_type	Module_Sta_Ack_type;
static void ACDC_RxMsg_Deal(void)
{
	if(RX_Flag.ACDC_Rx_Flag)
	{	
		if(ACDC_RX.ExtId == Total_Number_Ack)	
		{
			Module_Rx_Flag.All_num = true;	
			Module_Status.num = ACDC_RX.Data[2];
		}		
		if(Board_Type == 0x0A)
		{			
			if(ACDC_RX.ExtId == GroupA_Number_Ack)	
			{
				Module_Rx_Flag.A_num = true;	
				Module_Status.numA = ACDC_RX.Data[2];
			}
			if((ACDC_RX.ExtId == Total_Vol_Cur_Ack)||(ACDC_RX.ExtId == GroupA_Vol_Cur_Ack))//读取模块输出电压总电流//所有模块|A组
			{
				char *V = (char*)&Module_Status.Output_Vol,*C = (char*)&Module_Status.Output_Cur;
				for(char i = 0;i < 4;i++)	{*V++ = ACDC_RX.Data[3-i];	*C++ = ACDC_RX.Data[7-i];}//IEEE-754单精度浮点要把数据倒过来
			}
			if((ACDC_RX.ExtId >= Single_Module_Sta_Ack)&&(ACDC_RX.ExtId <= Single_Module_Sta_Ack+Module_Status.num))
			{
				memcpy(&Module_Sta_Ack_type.group,&ACDC_RX.Data[2],6);//读取所有模块组号 温度 状态
				Type_DM.MErr2 = Module_Sta_Ack_type.sta2;
				Type_DM.MErr1 = Module_Sta_Ack_type.sta1;
				Type_DM.MErr0 = Module_Sta_Ack_type.sta0;//模块状态位
				if(((Module_Sta_Ack_type.sta2&0x7f)!=0)||((Module_Sta_Ack_type.sta1&0xbe)!=0)||((Module_Sta_Ack_type.sta0&0x11)!=0))//判断模块状态是否正常
				{
					//TxMsg_ACDC.ExtId = 0x029400F0U;TxMsg_ACDC.Data[0] = 1;CAN_Transmit(CAN2, &TxMsg_ACDC);//设置模块0绿灯闪烁
					//TxMsg_ACDC.ExtId = (0x029400F0U|(ADCD_RX.ExtId&0x000f));TxMsg_ACDC.Data[0] = 1;CAN_Transmit(CAN2, &TxMsg_ACDC);//设置对应模块绿灯闪烁,何时取消闪烁？
				}
			}
		}
		else if(Board_Type == 0x0B)
		{
			if(ACDC_RX.ExtId == GroupB_Number_Ack)	
			{
				Module_Rx_Flag.B_num = true;	
				Module_Status.numB = ACDC_RX.Data[2];
			}
			if((ACDC_RX.ExtId == Total_Vol_Cur_Ack)||(ACDC_RX.ExtId == GroupB_Vol_Cur_Ack))//读取模块输出电压总电流//所有模块|B组
			{
				char *V = (char*)&Module_Status.Output_Vol,*C = (char*)&Module_Status.Output_Cur;
				for(char i = 0;i < 4;i++)	{*V++ = ACDC_RX.Data[3-i];	*C++ = ACDC_RX.Data[7-i];}//IEEE-754单精度浮点要把数据倒过来
			}
		}		
		RX_Flag.ACDC_Rx_Flag = false;
	}
}


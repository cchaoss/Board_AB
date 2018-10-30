#include "main.h"
#include "can.h"
#include "acdc_module.h"
       

uint8_t ACDC_VolCur_Buffer[8];
ACDC_Status Module_Status;//电源模块的个数 温度 状态 电压电流		 
Module_Rx_Flag_Bits	Module_Rx_Flag;//电源模块报文接收标记
CanTxMsg TxMsg_ACDC = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//扩展帧 数据帧

unsigned char ACDC_STA = Set_Group;
void ACDC_Module_Task(void const *argument)
{
	const unsigned short ACDC_Module_Task_Time = 110U;
	static unsigned char number,module_init_time;
	Module_Rx_Flag.ON_OFF_STA = true;
	while(1)
	{
		ACDC_RxMsg_Deal();//处理模块应答的数据
		
		switch(ACDC_STA)
		{
			case Set_Group://设置模块分组A-B
			{
				if(Module_Rx_Flag.All_num&&(Module_Status.num>0))//读取模块总数成功&&模块数量>0
				{	
					Module_Rx_Flag.All_num = false;	number = 0;	ACDC_STA = Group_Verify;
//					if(Board_Type == 0x0A)//A板
//					{
//						TxMsg_ACDC.ExtId = Set_Single_Module_Group|(number<<8);
//						TxMsg_ACDC.Data[0] = 0x0A;//0-7设置为A组
//					}
//					if(Board_Type == 0x0B)
//					{
//						TxMsg_ACDC.ExtId = Set_Single_Module_Group|((number+MAX_GROUP_NUM)<<8);//MAX_GROUP_NUM 8
//						TxMsg_ACDC.Data[0] = 0x0B;//8-15设置为B组
//					}		
//					CAN_Transmit(CAN2, &TxMsg_ACDC);
//					if(++number==MAX_GROUP_NUM)	{Module_Rx_Flag.All_num = false;	number = 0;	ACDC_STA = Group_Verify;}					
				}
				else	//if(++module_init_time > 11000/ACDC_Module_Task_Time)//上电11s等待模块硬件初始化完成
				{	
					module_init_time = 11000/ACDC_Module_Task_Time + 1;
					TxMsg_ACDC.ExtId = Total_Number_Read;//读取模块总数
					memset(TxMsg_ACDC.Data,0,8);
					CAN_Transmit(CAN2, &TxMsg_ACDC);		
				}
			}break;
			case Group_Verify://读取各组模块数量，确认每个组至少有一个模块
			{
				if(Board_Type == 0x0A)
				{
					if(Module_Rx_Flag.A_num)
					{
						Module_Rx_Flag.A_num = false;
						if(Module_Status.numA > 0)	ACDC_STA = Read_Status;//至少有一个模块就可以正常工作
							else ACDC_STA = Set_Group;//重新设置组号
//TxMsg_ACDC.ExtId = 0x029401F0U;TxMsg_ACDC.Data[0] = 1;CAN_Transmit(CAN2, &TxMsg_ACDC);//设置模块1绿灯闪烁
					}
					else
					{
						TxMsg_ACDC.ExtId = GroupA_Number_Read;//读取A组模块数量
						CAN_Transmit(CAN2, &TxMsg_ACDC);
					}	
				}
				else if(Board_Type == 0x0B)//B板
				{
					if(Module_Rx_Flag.B_num)
					{
						Module_Rx_Flag.B_num = false;
						if(Module_Status.numB>0)	ACDC_STA = Read_Status;//至少有一个模块就可以正常工作
							else ACDC_STA = Set_Group;//重新设置组号
					}
					else
					{
						TxMsg_ACDC.ExtId = GroupB_Number_Read;//读取A组模块数量
						CAN_Transmit(CAN2, &TxMsg_ACDC);
					}	
				}
			}break;
			case Read_Status://读取模块状态
			{	
				if(Board_Type == 0x0A)	
				{	
					TxMsg_ACDC.ExtId = Single_Module_Sta_Read|(number<<8);//读取模块状态/温度/组号(A组模块地址：0-7)
					if(++number==Module_Status.numA+1)	number = 0;
				}
				else if(Board_Type == 0x0B)	
				{
					TxMsg_ACDC.ExtId = Single_Module_Sta_Read|((number+MAX_GROUP_NUM)<<8);//读取模块状态/温度/组号(B组模块地址：8-15)
					if(++number==Module_Status.numB)	number = 0;
				}
				memset(TxMsg_ACDC.Data,0,8);
				CAN_Transmit(CAN2, &TxMsg_ACDC);	
				ACDC_STA =	Read_Vol_Cur;
				//if(Module_Rx_Flag.Temp_sta == true);//判断模块状态是否正常，做降额关机处理	
			}break;
			case Read_Vol_Cur:
			{	
				if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Vol_Cur_Read;//读取A组电压电流
				else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Vol_Cur_Read;//读取A组电压电流	
				memset(TxMsg_ACDC.Data,0,8);
				CAN_Transmit(CAN2, &TxMsg_ACDC);
				ACDC_STA =	Set_Vol_Cur;
			}break;
			case Set_Vol_Cur://设置电压电流 开关机
			{
				if((ACDC_VolCur_Buffer[3] != 0)&&(ACDC_VolCur_Buffer[7] != 0))
				{
					if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = Set_GroupA_Vol_Cur;
					else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = Set_GroupB_Vol_Cur;
					memcpy(TxMsg_ACDC.Data,ACDC_VolCur_Buffer,8);
					CAN_Transmit(CAN2, &TxMsg_ACDC);
					if(Module_Rx_Flag.ON_OFF_STA)//开机一次
					{
						Module_Rx_Flag.ON_OFF_STA = false;
						if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Module_ONOFF;
						else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Module_ONOFF;
						memset(TxMsg_ACDC.Data,0,8);//0为开机
						CAN_Transmit(CAN2, &TxMsg_ACDC);
					}
				}
				else//电压电流为0则执行关机
				{
					if(!Module_Rx_Flag.ON_OFF_STA)//开过机才关机
					{
						Module_Rx_Flag.ON_OFF_STA = true;
						if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Module_ONOFF;
						else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Module_ONOFF;
						memset(TxMsg_ACDC.Data,0,8);					
						TxMsg_ACDC.Data[0] = 1;	//1为关机
						CAN_Transmit(CAN2, &TxMsg_ACDC);
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
		if(ADCD_RX.ExtId == Total_Number_Ack)	{Module_Rx_Flag.All_num = true;	Module_Status.num = ADCD_RX.Data[2];}		
		if(Board_Type == 0x0A)
		{			
			if(ADCD_RX.ExtId == GroupA_Number_Ack)	{Module_Rx_Flag.A_num = true;	Module_Status.numA = ADCD_RX.Data[2];}
			if((ADCD_RX.ExtId == Total_Vol_Cur_Ack)||(ADCD_RX.ExtId == GroupA_Vol_Cur_Ack))//读取模块输出电压总电流//所有模块|A组
			{
				char *V = (char*)&Module_Status.Output_Vol,*C = (char*)&Module_Status.Output_Cur;
				for(char i = 0;i < 4;i++)//IEEE-754单精度浮点要把数据倒过来
				{
					*V++ = ADCD_RX.Data[3-i];
					*C++ = ADCD_RX.Data[7-i];
				}
			}
			if((ADCD_RX.ExtId >= Single_Module_Sta_Ack)&&(ADCD_RX.ExtId <= Single_Module_Sta_Ack+7))	
				memcpy(&Module_Sta_Ack_type.group,&ADCD_RX.Data[2],6);//读取0-7号模块组号 温度 状态
		}
		else if(Board_Type == 0x0B)
		{
			if(ADCD_RX.ExtId == GroupB_Number_Ack)	{Module_Rx_Flag.B_num = true;	Module_Status.numB = ADCD_RX.Data[2];}
			if((ADCD_RX.ExtId == Total_Vol_Cur_Ack)||(ADCD_RX.ExtId == GroupB_Vol_Cur_Ack))//读取模块输出电压总电流//所有模块|B组
			{
				char *V = (char*)&Module_Status.Output_Vol,*C = (char*)&Module_Status.Output_Cur;
				for(char i = 0;i < 4;i++)//IEEE-754单精度浮点要把数据倒过来
				{
					*V++ = ADCD_RX.Data[3-i];
					*C++ = ADCD_RX.Data[7-i];
				}
			}
			if((ADCD_RX.ExtId >= Single_Module_Sta_Ack+8)&&(ADCD_RX.ExtId <= Single_Module_Sta_Ack+15))	
				memcpy(&Module_Sta_Ack_type.group,&ADCD_RX.Data[2],6);//读取8-15号模块组号 温度 状态	
		}		
		RX_Flag.ACDC_Rx_Flag = false;
	}
}


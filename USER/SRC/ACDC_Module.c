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
	static unsigned char number;
	static bool last_Module_Assign;
	while(1)
	{
		if(ACDC_STA < Read_Status)	Type_DM.DErr |= No_Module;	else Type_DM.DErr &= ~No_Module;/*故障：无电源模块！*/
		ACDC_RxMsg_Deal();//处理模块应答的数据
		switch(ACDC_STA)
		{
			case Set_Group:			
				if(Module_Rx_Flag.All_num&&(Module_Status.num>0))//读取模块总数成功&&模块数量>0
				{	
					Module_Rx_Flag.All_num = false;	number = 0;	
					if(Board_C_Sta == 0)	ACDC_STA = Set_Vol_Cur;//没有连接C板跳过验证A组模块数量步骤
						else ACDC_STA = Group_Verify;			
				}
				else //if(++module_init_time > 9000/ACDC_Module_Task_Time)//上电等待~10s模块硬件初始化完成
				{	
					TxMsg_ACDC.ExtId = Total_Number_Read;//读取模块总数
					memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);		
				}
			break;
			
			case Group_Verify://读取各组模块数量，确认每个组至少有一个模块
				if(Board_Type == 0x0A)
				{
					if(Module_Rx_Flag.A_num)
					{
						Module_Rx_Flag.A_num = false;
						if(Module_Status.numA == 0)	ACDC_STA = Set_Group;//重新开始							
							else {Type_DM.MNum = Module_Status.numA;	ACDC_STA = Set_Vol_Cur;}//至少有一个模块就可以正常工作
					}
					else	TxMsg_ACDC.ExtId = GroupA_Number_Read;//读取A组模块数量
				}
				else//B板
				{
					if(Module_Rx_Flag.B_num)
					{
						Module_Rx_Flag.B_num = false;
						if(Module_Status.numB == 0)	ACDC_STA = Set_Group;//重新开始		
							else {Type_DM.MNum = Module_Status.numB;	ACDC_STA = Set_Vol_Cur;}//至少有一个模块就可以正常工作
					}
					else	TxMsg_ACDC.ExtId = GroupB_Number_Read;//读取B组模块数量	
				}
				CAN_Transmit(CAN2, &TxMsg_ACDC);
			break;
			/*读取模块状态*/
			case Read_Status:
				if(Board_Type == 0x0A)//轮询A+B组所有模块状态 温度信息
				{	
					TxMsg_ACDC.ExtId = Single_Module_Sta_Read|(number<<8);//读取模块状态/温度/组号
					if(++number==Module_Status.num)	number = 0;
				}
				memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);
				ACDC_STA =	Read_Vol_Cur;
			break;
			/*读取电压电流*/
			case Read_Vol_Cur:	
				if(Board_Type == 0x0A)	
				{
					if(Type_Control_Cmd.Module_Assign == 0xAA)	TxMsg_ACDC.ExtId = Total_Vol_Cur_Read;//读取所有模块总电压电流
						else TxMsg_ACDC.ExtId = GroupA_Vol_Cur_Read;//读取A组电压电流
				}
				else if(Board_Type == 0x0B)	
				{
					if(Type_Control_Cmd.Module_Assign == 0xBB)	TxMsg_ACDC.ExtId = Total_Vol_Cur_Read;//读取所有模块总电压电流
						else	TxMsg_ACDC.ExtId = GroupB_Vol_Cur_Read;//读取B组电压电流
				}					
				memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);
				ACDC_STA =	Set_Vol_Cur;
			break;
			/*设置电压电流 开关机*/
			case Set_Vol_Cur:
				if(ACDC_VolCur_Buffer[3] != 0)//电压不为0
				{	/*没有连接C板时 A板可以控制所有模块*/	
					if(Board_Type == 0x0A)
					{
						if(Type_Control_Cmd.Module_Assign == 0xAA)	TxMsg_ACDC.ExtId = Set_Total_Vol_Cur;//设置所有模块输出电压电流
							else TxMsg_ACDC.ExtId = Set_GroupA_Vol_Cur;//设置A组模块输出电压电流
					}
					else if(Board_Type == 0x0B)	
					{
						if(Type_Control_Cmd.Module_Assign == 0xBB)	TxMsg_ACDC.ExtId = Set_Total_Vol_Cur;//设置所有模块输出电压电流
							else TxMsg_ACDC.ExtId = Set_GroupB_Vol_Cur;//设置B组模块输出电压电流
					}
					memcpy(TxMsg_ACDC.Data,ACDC_VolCur_Buffer,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);
					
					if(((Type_Control_Cmd.Module_Assign==0XAA)||(Type_Control_Cmd.Module_Assign==0XBB))&&(last_Module_Assign == 0XAB))	Module_Rx_Flag.ON_OFF_STA = true;\
						last_Module_Assign = Type_Control_Cmd.Module_Assign;//如果由AB->AA|BB则所有模快开机一次
					if(Module_Rx_Flag.ON_OFF_STA)/*关过机才开机一次*/
					{
						Module_Rx_Flag.ON_OFF_STA = false;
						if(Board_Type == 0x0A)	
						{
							if(Type_Control_Cmd.Module_Assign == 0xAA) TxMsg_ACDC.ExtId = Total_Module_ONOFF;//所有模块开机
							 else TxMsg_ACDC.ExtId = GroupA_Module_ONOFF;//A组模块开机
						}
						else if(Board_Type == 0x0B)	
						{
							if(Type_Control_Cmd.Module_Assign == 0xBB) TxMsg_ACDC.ExtId = Total_Module_ONOFF;//所有模块开机
							 else TxMsg_ACDC.ExtId = GroupB_Module_ONOFF;//B组模块开机
						}
						memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);//0为开机
					}
				}
				else//电压为0则执行关机
				{
					if(!Module_Rx_Flag.ON_OFF_STA)/*开过机才关机一次*/
					{
						Module_Rx_Flag.ON_OFF_STA = true;
						if(Board_Type == 0x0A)	
						{
							if(Type_Control_Cmd.Module_Assign == 0xAA) TxMsg_ACDC.ExtId = Total_Module_ONOFF;//所有模块关机(当分配模块暂停关机时，关闭所有模块)
								else TxMsg_ACDC.ExtId = GroupA_Module_ONOFF;//A组模块关机
						}
						else if(Board_Type == 0x0B)	
						{
							if(Type_Control_Cmd.Module_Assign == 0xBB) TxMsg_ACDC.ExtId = Total_Module_ONOFF;//所有模块关机(当分配模块暂停关机时，关闭所有模块)
							 else TxMsg_ACDC.ExtId = GroupB_Module_ONOFF;//B组模块关机
						}
						memset(TxMsg_ACDC.Data,0,8);	TxMsg_ACDC.Data[0] = 1;	CAN_Transmit(CAN2, &TxMsg_ACDC);//1为关机
					}
				}
				ACDC_STA =	Read_Status;
			break;
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
		if(ACDC_RX.ExtId == Total_Number_Ack)	{Module_Rx_Flag.All_num = true;	Module_Status.num = ACDC_RX.Data[2];}
		
		if(Board_Type == 0x0A)
		{			
			if(ACDC_RX.ExtId == GroupA_Number_Ack)	{Module_Rx_Flag.A_num = true;	Module_Status.numA = ACDC_RX.Data[2];}
			
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
//				if(((Module_Sta_Ack_type.sta2&0x7f)!=0)||((Module_Sta_Ack_type.sta1&0xbe)!=0)||((Module_Sta_Ack_type.sta0&0x11)!=0))//判断模块状态不正常
//				{
//					//TxMsg_ACDC.ExtId = 0x029400F0U;TxMsg_ACDC.Data[0] = 1;CAN_Transmit(CAN2, &TxMsg_ACDC);//设置模块0绿灯闪烁
//					//TxMsg_ACDC.ExtId = (0x029400F0U|(ADCD_RX.ExtId&0x000f));TxMsg_ACDC.Data[0] = 1;CAN_Transmit(CAN2, &TxMsg_ACDC);//设置对应模块绿灯闪烁,何时取消闪烁？
//				}
			}
		}
		else if(Board_Type == 0x0B)
		{
			if(ACDC_RX.ExtId == GroupB_Number_Ack)	{Module_Rx_Flag.B_num = true;	Module_Status.numB = ACDC_RX.Data[2];}
			
			if((ACDC_RX.ExtId == Total_Vol_Cur_Ack)||(ACDC_RX.ExtId == GroupB_Vol_Cur_Ack))//读取模块输出电压总电流//所有模块|B组
			{
				char *V = (char*)&Module_Status.Output_Vol,*C = (char*)&Module_Status.Output_Cur;
				for(char i = 0;i < 4;i++)	{*V++ = ACDC_RX.Data[3-i];	*C++ = ACDC_RX.Data[7-i];}//IEEE-754单精度浮点要把数据倒过来
			}
		}		
		RX_Flag.ACDC_Rx_Flag = false;
	}
}


#include "main.h"
#include "can.h"
#include "acdc_module.h"
       

uint8_t ACDC_VolCur_Buffer[8];
ACDC_Status Module_Status;//��Դģ��ĸ��� �¶� ״̬ ��ѹ����		 
Module_Rx_Flag_Bits	Module_Rx_Flag;//��Դģ�鱨�Ľ��ձ��
CanTxMsg TxMsg_ACDC = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//��չ֡ ����֡

unsigned char ACDC_STA = Set_Group;
void ACDC_Module_Task(void const *argument)
{
	const unsigned short ACDC_Module_Task_Time = 110U;
	static unsigned char number,module_init_time;
	Module_Rx_Flag.ON_OFF_STA = true;
	while(1)
	{
		ACDC_RxMsg_Deal();//����ģ��Ӧ�������
		
		switch(ACDC_STA)
		{
			case Set_Group://����ģ�����A-B
			{
				if(Module_Rx_Flag.All_num&&(Module_Status.num>0))//��ȡģ�������ɹ�&&ģ������>0
				{	
					Module_Rx_Flag.All_num = false;	number = 0;	ACDC_STA = Group_Verify;
//					if(Board_Type == 0x0A)//A��
//					{
//						TxMsg_ACDC.ExtId = Set_Single_Module_Group|(number<<8);
//						TxMsg_ACDC.Data[0] = 0x0A;//0-7����ΪA��
//					}
//					if(Board_Type == 0x0B)
//					{
//						TxMsg_ACDC.ExtId = Set_Single_Module_Group|((number+MAX_GROUP_NUM)<<8);//MAX_GROUP_NUM 8
//						TxMsg_ACDC.Data[0] = 0x0B;//8-15����ΪB��
//					}		
//					CAN_Transmit(CAN2, &TxMsg_ACDC);
//					if(++number==MAX_GROUP_NUM)	{Module_Rx_Flag.All_num = false;	number = 0;	ACDC_STA = Group_Verify;}					
				}
				else	//if(++module_init_time > 11000/ACDC_Module_Task_Time)//�ϵ�11s�ȴ�ģ��Ӳ����ʼ�����
				{	
					module_init_time = 11000/ACDC_Module_Task_Time + 1;
					TxMsg_ACDC.ExtId = Total_Number_Read;//��ȡģ������
					memset(TxMsg_ACDC.Data,0,8);
					CAN_Transmit(CAN2, &TxMsg_ACDC);		
				}
			}break;
			case Group_Verify://��ȡ����ģ��������ȷ��ÿ����������һ��ģ��
			{
				if(Board_Type == 0x0A)
				{
					if(Module_Rx_Flag.A_num)
					{
						Module_Rx_Flag.A_num = false;
						if(Module_Status.numA > 0)	ACDC_STA = Read_Status;//������һ��ģ��Ϳ�����������
							else ACDC_STA = Set_Group;//�����������
//TxMsg_ACDC.ExtId = 0x029401F0U;TxMsg_ACDC.Data[0] = 1;CAN_Transmit(CAN2, &TxMsg_ACDC);//����ģ��1�̵���˸
					}
					else
					{
						TxMsg_ACDC.ExtId = GroupA_Number_Read;//��ȡA��ģ������
						CAN_Transmit(CAN2, &TxMsg_ACDC);
					}	
				}
				else if(Board_Type == 0x0B)//B��
				{
					if(Module_Rx_Flag.B_num)
					{
						Module_Rx_Flag.B_num = false;
						if(Module_Status.numB>0)	ACDC_STA = Read_Status;//������һ��ģ��Ϳ�����������
							else ACDC_STA = Set_Group;//�����������
					}
					else
					{
						TxMsg_ACDC.ExtId = GroupB_Number_Read;//��ȡA��ģ������
						CAN_Transmit(CAN2, &TxMsg_ACDC);
					}	
				}
			}break;
			case Read_Status://��ȡģ��״̬
			{	
				if(Board_Type == 0x0A)	
				{	
					TxMsg_ACDC.ExtId = Single_Module_Sta_Read|(number<<8);//��ȡģ��״̬/�¶�/���(A��ģ���ַ��0-7)
					if(++number==Module_Status.numA+1)	number = 0;
				}
				else if(Board_Type == 0x0B)	
				{
					TxMsg_ACDC.ExtId = Single_Module_Sta_Read|((number+MAX_GROUP_NUM)<<8);//��ȡģ��״̬/�¶�/���(B��ģ���ַ��8-15)
					if(++number==Module_Status.numB)	number = 0;
				}
				memset(TxMsg_ACDC.Data,0,8);
				CAN_Transmit(CAN2, &TxMsg_ACDC);	
				ACDC_STA =	Read_Vol_Cur;
				//if(Module_Rx_Flag.Temp_sta == true);//�ж�ģ��״̬�Ƿ�������������ػ�����	
			}break;
			case Read_Vol_Cur:
			{	
				if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Vol_Cur_Read;//��ȡA���ѹ����
				else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Vol_Cur_Read;//��ȡA���ѹ����	
				memset(TxMsg_ACDC.Data,0,8);
				CAN_Transmit(CAN2, &TxMsg_ACDC);
				ACDC_STA =	Set_Vol_Cur;
			}break;
			case Set_Vol_Cur://���õ�ѹ���� ���ػ�
			{
				if((ACDC_VolCur_Buffer[3] != 0)&&(ACDC_VolCur_Buffer[7] != 0))
				{
					if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = Set_GroupA_Vol_Cur;
					else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = Set_GroupB_Vol_Cur;
					memcpy(TxMsg_ACDC.Data,ACDC_VolCur_Buffer,8);
					CAN_Transmit(CAN2, &TxMsg_ACDC);
					if(Module_Rx_Flag.ON_OFF_STA)//����һ��
					{
						Module_Rx_Flag.ON_OFF_STA = false;
						if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Module_ONOFF;
						else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Module_ONOFF;
						memset(TxMsg_ACDC.Data,0,8);//0Ϊ����
						CAN_Transmit(CAN2, &TxMsg_ACDC);
					}
				}
				else//��ѹ����Ϊ0��ִ�йػ�
				{
					if(!Module_Rx_Flag.ON_OFF_STA)//�������Źػ�
					{
						Module_Rx_Flag.ON_OFF_STA = true;
						if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Module_ONOFF;
						else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Module_ONOFF;
						memset(TxMsg_ACDC.Data,0,8);					
						TxMsg_ACDC.Data[0] = 1;	//1Ϊ�ػ�
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
			if((ADCD_RX.ExtId == Total_Vol_Cur_Ack)||(ADCD_RX.ExtId == GroupA_Vol_Cur_Ack))//��ȡģ�������ѹ�ܵ���//����ģ��|A��
			{
				char *V = (char*)&Module_Status.Output_Vol,*C = (char*)&Module_Status.Output_Cur;
				for(char i = 0;i < 4;i++)//IEEE-754�����ȸ���Ҫ�����ݵ�����
				{
					*V++ = ADCD_RX.Data[3-i];
					*C++ = ADCD_RX.Data[7-i];
				}
			}
			if((ADCD_RX.ExtId >= Single_Module_Sta_Ack)&&(ADCD_RX.ExtId <= Single_Module_Sta_Ack+7))	
				memcpy(&Module_Sta_Ack_type.group,&ADCD_RX.Data[2],6);//��ȡ0-7��ģ����� �¶� ״̬
		}
		else if(Board_Type == 0x0B)
		{
			if(ADCD_RX.ExtId == GroupB_Number_Ack)	{Module_Rx_Flag.B_num = true;	Module_Status.numB = ADCD_RX.Data[2];}
			if((ADCD_RX.ExtId == Total_Vol_Cur_Ack)||(ADCD_RX.ExtId == GroupB_Vol_Cur_Ack))//��ȡģ�������ѹ�ܵ���//����ģ��|B��
			{
				char *V = (char*)&Module_Status.Output_Vol,*C = (char*)&Module_Status.Output_Cur;
				for(char i = 0;i < 4;i++)//IEEE-754�����ȸ���Ҫ�����ݵ�����
				{
					*V++ = ADCD_RX.Data[3-i];
					*C++ = ADCD_RX.Data[7-i];
				}
			}
			if((ADCD_RX.ExtId >= Single_Module_Sta_Ack+8)&&(ADCD_RX.ExtId <= Single_Module_Sta_Ack+15))	
				memcpy(&Module_Sta_Ack_type.group,&ADCD_RX.Data[2],6);//��ȡ8-15��ģ����� �¶� ״̬	
		}		
		RX_Flag.ACDC_Rx_Flag = false;
	}
}


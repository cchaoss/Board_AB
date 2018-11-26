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
	const unsigned short ACDC_Module_Task_Time = 125U;
	static unsigned char number,module_init_time;
	Module_Rx_Flag.ON_OFF_STA = true;
	while(1)
	{
		ACDC_RxMsg_Deal();//����ģ��Ӧ�������
		
		//if(ACDC_STA < Read_Status)	Type_DM.DErr = No_Module;//���ϣ��޵�Դģ�飡
		switch(ACDC_STA)
		{
			case Set_Group:
			{
				if(Module_Rx_Flag.All_num&&(Module_Status.num>0))//��ȡģ�������ɹ�&&ģ������>0
				{	
					Module_Rx_Flag.All_num = false;	number = 0;	ACDC_STA = Group_Verify;			
				}
				else	if(++module_init_time > 10000/ACDC_Module_Task_Time)//�ϵ�10s�ȴ�ģ��Ӳ����ʼ�����
				{	
					module_init_time = 10000/ACDC_Module_Task_Time + 1;
					TxMsg_ACDC.ExtId = Total_Number_Read;//��ȡģ������
					memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);		
				}
			}break;
			
			case Group_Verify://��ȡ����ģ��������ȷ��ÿ����������һ��ģ��
			{
				if(Board_Type == 0x0A)
				{
					if(Module_Rx_Flag.A_num)
					{
						Module_Rx_Flag.A_num = false;
						if(Module_Status.numA == 0)	ACDC_STA = Set_Group;//���¿�ʼ							
						else 
						{
							Type_DM.MNum = Module_Rx_Flag.A_num;
							ACDC_STA = Read_Status;//������һ��ģ��Ϳ�����������
						}
					}
					else{TxMsg_ACDC.ExtId = GroupA_Number_Read;	CAN_Transmit(CAN2, &TxMsg_ACDC);}//��ȡA��ģ������
				}
				else if(Board_Type == 0x0B)//B��
				{
					if(Module_Rx_Flag.B_num)
					{
						Module_Rx_Flag.B_num = false;
						if(Module_Status.numB == 0)	ACDC_STA = Read_Status;//������һ��ģ��Ϳ�����������
						else
						{
							Type_DM.MNum = Module_Rx_Flag.B_num;
							ACDC_STA = Set_Group;//�����������
						}
					}
					else{TxMsg_ACDC.ExtId = GroupB_Number_Read;	CAN_Transmit(CAN2, &TxMsg_ACDC);}//��ȡA��ģ������	
				}
			}break;
			
			case Read_Status://��ȡģ��״̬
			{	
				if(Board_Type == 0x0A)//A���Ӹ�����ѯ����ģ��״̬ �¶���Ϣ
				{	
					TxMsg_ACDC.ExtId = Single_Module_Sta_Read|(number<<8);//��ȡģ��״̬/�¶�/���
					if(++number==Module_Status.num)	number = 0;
					memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);
				}
				ACDC_STA =	Read_Vol_Cur;
			}break;
			
			case Read_Vol_Cur:
			{	
				if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Vol_Cur_Read;//��ȡA���ѹ����
				else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Vol_Cur_Read;//��ȡB���ѹ����	
				memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);
				ACDC_STA =	Set_Vol_Cur;
			}break;
			
			case Set_Vol_Cur://���õ�ѹ���� ���ػ�
			{
				if(ACDC_VolCur_Buffer[3] != 0)//��ѹ��Ϊ0
				{
					if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = Set_GroupA_Vol_Cur;
					else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = Set_GroupB_Vol_Cur;
					memcpy(TxMsg_ACDC.Data,ACDC_VolCur_Buffer,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);
					if(Module_Rx_Flag.ON_OFF_STA)//����һ��
					{
						Module_Rx_Flag.ON_OFF_STA = false;
						if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Module_ONOFF;
						else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Module_ONOFF;
						memset(TxMsg_ACDC.Data,0,8);	CAN_Transmit(CAN2, &TxMsg_ACDC);//0Ϊ����
					}
				}
				else//��ѹΪ0��ִ�йػ�
				{
					if(!Module_Rx_Flag.ON_OFF_STA)//�������Źػ�һ��
					{
						Module_Rx_Flag.ON_OFF_STA = true;
						if(Board_Type == 0x0A)	TxMsg_ACDC.ExtId = GroupA_Module_ONOFF;
						else if(Board_Type == 0x0B)	TxMsg_ACDC.ExtId = GroupB_Module_ONOFF;
						memset(TxMsg_ACDC.Data,0,8);					
						TxMsg_ACDC.Data[0] = 1;	CAN_Transmit(CAN2, &TxMsg_ACDC);//1Ϊ�ػ�
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
			if((ACDC_RX.ExtId == Total_Vol_Cur_Ack)||(ACDC_RX.ExtId == GroupA_Vol_Cur_Ack))//��ȡģ�������ѹ�ܵ���//����ģ��|A��
			{
				char *V = (char*)&Module_Status.Output_Vol,*C = (char*)&Module_Status.Output_Cur;
				for(char i = 0;i < 4;i++)	{*V++ = ACDC_RX.Data[3-i];	*C++ = ACDC_RX.Data[7-i];}//IEEE-754�����ȸ���Ҫ�����ݵ�����
			}
			if((ACDC_RX.ExtId >= Single_Module_Sta_Ack)&&(ACDC_RX.ExtId <= Single_Module_Sta_Ack+Module_Status.num))
			{
				memcpy(&Module_Sta_Ack_type.group,&ACDC_RX.Data[2],6);//��ȡ����ģ����� �¶� ״̬
				if(((Module_Sta_Ack_type.sta2&0x7f)!=0)||((Module_Sta_Ack_type.sta1&0xbe)!=0)||((Module_Sta_Ack_type.sta0&0x11)!=0))//�ж�ģ��״̬�Ƿ�����
				{
					Type_DM.MErr2 = Module_Sta_Ack_type.sta2;
					Type_DM.MErr1 = Module_Sta_Ack_type.sta1;
					Type_DM.MErr0 = Module_Sta_Ack_type.sta0;
					//TxMsg_ACDC.ExtId = 0x029400F0U;TxMsg_ACDC.Data[0] = 1;CAN_Transmit(CAN2, &TxMsg_ACDC);//����ģ��0�̵���˸
					//TxMsg_ACDC.ExtId = (0x029400F0U|(ADCD_RX.ExtId&0x000f));TxMsg_ACDC.Data[0] = 1;CAN_Transmit(CAN2, &TxMsg_ACDC);//���ö�Ӧģ���̵���˸,��ʱȡ����˸��
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
			if((ACDC_RX.ExtId == Total_Vol_Cur_Ack)||(ACDC_RX.ExtId == GroupB_Vol_Cur_Ack))//��ȡģ�������ѹ�ܵ���//����ģ��|B��
			{
				char *V = (char*)&Module_Status.Output_Vol,*C = (char*)&Module_Status.Output_Cur;
				for(char i = 0;i < 4;i++)	{*V++ = ACDC_RX.Data[3-i];	*C++ = ACDC_RX.Data[7-i];}//IEEE-754�����ȸ���Ҫ�����ݵ�����
			}
		}		
		RX_Flag.ACDC_Rx_Flag = false;
	}
}


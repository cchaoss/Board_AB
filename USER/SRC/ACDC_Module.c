#include "main.h"
#include "acdc_module.h"
       
CanTxMsg TxMsg2 = {
0,								// ��׼��ʶ�� 
0x100956f4,				// ��չ��ʶ�� 
CAN_Id_Extended,	//��չ֡
CAN_RTR_Data,			//����֡
2,								//���ݳ���0-8
{0x6e,0x0f}};			//Data[8]
			 
			 
void ACDC_Module_Task(void const *argument)
{
	const unsigned short ACDC_Module_Task_Time = 250U;
	
	while(1)
	{
		//CAN_Transmit(CAN2, &TxMsg2);
		
		osDelay(ACDC_Module_Task_Time);
	}
}


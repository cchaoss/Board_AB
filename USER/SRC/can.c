#include "can.h"
#include "bms.h"

#define  BMS_Filter_ID_Mask 0x000056f4U
#define  ACDC_Filter_ID_Mask 0x00000000U

//��ʼ��BMS_CAN��:CAN1
void BMS_Can_Init(void)  
{                                                         
	GPIO_PinConfigure(GPIOA,11,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//RX��������
	GPIO_PinConfigure(GPIOA,12,GPIO_AF_PUSHPULL,GPIO_MODE_OUT50MHZ);//TX�����������
	
	CAN_InitTypeDef	CAN_InitStructure;
	CAN_DeInit(CAN1);//��CANx�Ĵ���ȫ������Ϊȱʡֵ
	CAN_StructInit(&CAN_InitStructure);//CAN register init	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//ʹ��CAN1ʱ��
	
	CAN_InitStructure.CAN_TTCM = DISABLE;//ʱ�䴥��ģʽ
	CAN_InitStructure.CAN_ABOM = DISABLE; //�Զ����߹���
	CAN_InitStructure.CAN_AWUM = DISABLE;//�Զ�����ģʽ
	CAN_InitStructure.CAN_NART = ENABLE;//���Զ��ش�ģʽ
	CAN_InitStructure.CAN_RFLM = DISABLE;//����FIFO����ģʽ
	CAN_InitStructure.CAN_TXFP = DISABLE;//����FIFO���ȼ�
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//�����Բ�ģʽ	CAN_Mode_LoopBack;	//CAN����ģʽ
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;			//����ͬ����Ծ���1��ʱ�䵥λ
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;			//ʱ���1λ13��ʱ�䵥λ		
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;			//ʱ���2Ϊ2��ʱ�䵥λ
	
	//BaudRate=72M/2/CAN_Prescaler/(1+13+2)
	////250Kʱ������
	CAN_InitStructure.CAN_Prescaler = 9;//ʱ�䵥λ����Ϊ60	//72M/2/9/(1+13+2)=0.25 =250K
	//125Kʱ������
	//CAN_InitStructure.CAN_Prescaler = 18;break;//ʱ�䵥λ����Ϊ60 //72M/2/18/(1+13+2)=0.125 =125K
	CAN_Init(CAN1, &CAN_InitStructure);

	//����CAN������
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 0;//ָ��������(0-13)
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//ָ��������Ϊ��ʶ������λģʽ
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//ָ��������λ��Ϊ32λ
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((BMS_Filter_ID_Mask<<3)&0xffff0000)>>16);
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((BMS_Filter_ID_Mask<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);//CAN_ID_EXT	CAN_RTR_DATA
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0xffff;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;//������0������FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE); //FIFO0��Ϣ�Һ��ж�����
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // �����ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // �����ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


//��ʼ������Դģ��_CAN��:CAN2
void ACDC_Module_Can_Init(void)  
{                                                         
	GPIO_PinConfigure(GPIOB,12,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//RX��������
	GPIO_PinConfigure(GPIOB,13,GPIO_AF_PUSHPULL,GPIO_MODE_OUT50MHZ);//TX�����������
	
	CAN_InitTypeDef	CAN_InitStructure;
	CAN_DeInit(CAN2);//��CANx�Ĵ���ȫ������Ϊȱʡֵ
	CAN_StructInit(&CAN_InitStructure);//CAN register init
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);//ʹ��CAN2ʱ��
	
	CAN_InitStructure.CAN_TTCM = DISABLE;//ʱ�䴥��ģʽ
	CAN_InitStructure.CAN_ABOM = DISABLE; //�Զ����߹���
	CAN_InitStructure.CAN_AWUM = DISABLE;//�Զ�����ģʽ
	CAN_InitStructure.CAN_NART = ENABLE;//���Զ��ش�ģʽ
	CAN_InitStructure.CAN_RFLM = DISABLE;//����FIFO����ģʽ
	CAN_InitStructure.CAN_TXFP = DISABLE;//����FIFO���ȼ�
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//�����Բ�ģʽ	CAN_Mode_Silent;	//CAN����ģʽ
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;			//����ͬ����Ծ���1��ʱ�䵥λ
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;			//ʱ���1λ13��ʱ�䵥λ		
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;			//ʱ���2Ϊ2��ʱ�䵥λ
	
	//BaudRate=72M/2/CAN_Prescaler/(1+13+2)
	//250Kʱ������
	CAN_InitStructure.CAN_Prescaler = 9;//ʱ�䵥λ����Ϊ60	//72M/2/9/(1+13+2)=0.25 =250K
	//125Kʱ������
	//CAN_InitStructure.CAN_Prescaler = 18;break;//ʱ�䵥λ����Ϊ60 //72M/2/18/(1+13+2)=0.125 =125K
	CAN_Init(CAN2, &CAN_InitStructure);

	//����CAN������
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 14;//ָ��������14(14-27)
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//ָ��������Ϊ��ʶ������λģʽ
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//ָ��������λ��Ϊ32λ
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((ACDC_Filter_ID_Mask<<3)&0xffff0000)>>16);
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((ACDC_Filter_ID_Mask<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;//������0������FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE); //FIFO0��Ϣ�Һ��ж�����
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // �����ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // �����ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


//����ֵ:0,�ɹ�;1,ʧ��
unsigned char Can_Send_Msg(CAN_TypeDef* CANx, CanTxMsg* TxMessage)
{	
	uint16_t i = 0;          
	uint8_t mbox = CAN_Transmit(CANx, TxMessage);   
	while((CAN_TransmitStatus(CANx, mbox) == CAN_TxStatus_Failed)&&(i<0XFFF))i++;	//�ȴ����ͽ���
	if(i>=0XFFF)return 1;
	return 0;	 
}


/**********************************************�жϴ���*****************************************************/
unsigned char BMS_Recevie_Flag;
CanRxMsg RxMsg1;
//CAN_1FIFO�жϷ�����			    
void CAN1_RX0_IRQHandler(void)
{
	if(CAN_MessagePending(CAN1,CAN_FIFO0) != 0)
	{	
		BMS_Recevie_Flag = 1;
		CAN_Receive(CAN1, 0, &RxMsg1);
	}

}


unsigned short  k2;
CanRxMsg RxMsg2;
//CAN_2FIFO�жϷ�����			    
void CAN2_RX0_IRQHandler(void)
{
	k2++;
  CAN_Receive(CAN2, 0, &RxMsg2);
}
/*******************************************************************************************************/


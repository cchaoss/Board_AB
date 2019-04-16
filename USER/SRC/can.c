#include "can.h"
#include "bms.h"


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
	CAN_InitStructure.CAN_RFLM = ENABLE;//����FIFO����ģʽ
	CAN_InitStructure.CAN_TXFP = DISABLE;//����FIFO���ȼ�
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;			//����ͬ����Ծ���1��ʱ�䵥λ
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;			//ʱ���1λ13��ʱ�䵥λ		
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;			//ʱ���2Ϊ2��ʱ�䵥λ
	
	////250Kʱ������//BaudRate=72M/2/CAN_Prescaler/(1+13+2)
	CAN_InitStructure.CAN_Prescaler = 9;//ʱ�䵥λ����Ϊ60	//72M/2/9/(1+13+2)=0.25 =250K
	//CAN_InitStructure.CAN_Prescaler = 18;break;//ʱ�䵥λ����Ϊ60 //72M/2/18/(1+13+2)=0.125 =125K
	CAN_Init(CAN1, &CAN_InitStructure);

	//����CAN������0 -> FIFO0
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 0;//ָ����������0-13
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;//ָ��������Ϊ�б�ģʽ
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//ָ��������λ��Ϊ32λ
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((0x1CEC56F4U<<3)&0xffff0000)>>16);//�������ID
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((0x1CEC56F4U<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh =(uint16_t)(((0x1CEB56F4U<<3)&0xffff0000)>>16);//�������ID
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = (uint16_t)(((0x1CEB56F4U<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;//������1������FIFO1
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	//����CAN������1 -> FIFO1
	CAN_FilterInitStructure.CAN_FilterNumber = 1;//ָ����������0-13
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//ָ��������Ϊ��ʶ������λģʽ
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//ָ��������λ��Ϊ32λ
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((0x000056f4U<<3)&0xffff0000)>>16);
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((0x000056f4U<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = (uint16_t)(0xffff<<3);
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO1;//������1������FIFO1
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	CAN_ITConfig(CAN1, CAN_IT_FMP0|CAN_IT_FMP1, ENABLE); //FIFO0 FIFO1��Ϣ�Һ��ж�����
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;           
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            
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
	CAN_InitStructure.CAN_RFLM = ENABLE;//����FIFO����ģʽ
	CAN_InitStructure.CAN_TXFP = DISABLE;//����FIFO���ȼ�
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//�����Բ�ģʽ	CAN_Mode_Silent;	//CAN����ģʽ
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;			//����ͬ����Ծ���1��ʱ�䵥λ
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;			//ʱ���1λ13��ʱ�䵥λ		
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;			//ʱ���2Ϊ2��ʱ�䵥λ
	CAN_InitStructure.CAN_Prescaler = 18;//ʱ�䵥λ����Ϊ60 //72M/2/18/(1+13+2)=0.125 =125K
	CAN_Init(CAN2, &CAN_InitStructure);

	//����CAN��������0 ->FIFO0
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 14;//ָ����������14(14-27)
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//ָ��������Ϊ��ʶ������λģʽ
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//ָ��������λ��Ϊ32λ
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((0x0000F000U<<3)&0xffff0000)>>16);//����Ӣ��Դģ��Ӧ���ַF0
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((0x0000F000U<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = (uint16_t)(0xff00U<<3);
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;//������14������FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
		//����CAN������1 -> FIFO1
	CAN_FilterInitStructure.CAN_FilterNumber = 15;//ָ����������0-13
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//ָ��������Ϊ��ʶ������λģʽ
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//ָ��������λ��Ϊ32λ
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((0x0ABC00C0U<<3)&0xffff0000)>>16);//������ݴ����ַǰ3λ�̶�ABC
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((0x0ABC00C0U<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);//AB��ֻ������C������ݽ���
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (uint16_t)(0x0fffU<<3);
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = (uint16_t)(0x00f0U<<3);
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO1;//������15������FIFO1
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	CAN_ITConfig(CAN2, CAN_IT_FMP0|CAN_IT_FMP1, ENABLE); //FIFO0 FIFO1��Ϣ�Һ��ж�����
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;           
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;           
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/************************BMS�жϴ���*****************************/
Can_Rx_FlagBits	RX_Flag;
CanRxMsg BMS_RX_0;
void CAN1_RX0_IRQHandler(void)//������Ͷ��
{		
	CAN_Receive(CAN1, CAN_FIFO0, &BMS_RX_0);
	Multi_Package_Deal();//����BMS�������
}

CanRxMsg BMS_RX_1;
void CAN1_RX1_IRQHandler(void)
{
	CAN_Receive(CAN1, CAN_FIFO1, &BMS_RX_1);
	RX_Flag.BMS_Rx_Flag1 = true;
}

/************************ACDC�жϴ���***************************/
CanRxMsg ACDC_RX;	    
void CAN2_RX0_IRQHandler(void)
{
  CAN_Receive(CAN2, CAN_FIFO0, &ACDC_RX);
	RX_Flag.ACDC_Rx_Flag = true;
}

/************************AB����C��ͨѶ****************************/
CanRxMsg ABC_DATA_RX;
void CAN2_RX1_IRQHandler(void)
{
  CAN_Receive(CAN2, CAN_FIFO1, &ABC_DATA_RX);
	RX_Flag.ABC_Data_Rx_Flag = true;
}

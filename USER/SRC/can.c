#include "can.h"
#include "bms.h"

#define  BMS_Filter_ID_Mask 0x000056f4U
#define  ACDC_Filter_ID_Mask 0x00000000U

//初始化BMS_CAN口:CAN1
void BMS_Can_Init(void)  
{                                                         
	GPIO_PinConfigure(GPIOA,11,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//RX上拉输入
	GPIO_PinConfigure(GPIOA,12,GPIO_AF_PUSHPULL,GPIO_MODE_OUT50MHZ);//TX复用推挽输出
	
	CAN_InitTypeDef	CAN_InitStructure;
	CAN_DeInit(CAN1);//将CANx寄存器全部设置为缺省值
	CAN_StructInit(&CAN_InitStructure);//CAN register init	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时钟
	
	CAN_InitStructure.CAN_TTCM = DISABLE;//时间触发模式
	CAN_InitStructure.CAN_ABOM = DISABLE; //自动离线管理
	CAN_InitStructure.CAN_AWUM = DISABLE;//自动唤醒模式
	CAN_InitStructure.CAN_NART = ENABLE;//非自动重传模式
	CAN_InitStructure.CAN_RFLM = DISABLE;//接受FIFO锁定模式
	CAN_InitStructure.CAN_TXFP = DISABLE;//发送FIFO优先级
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//环回自测模式	CAN_Mode_LoopBack;	//CAN正常模式
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;			//重新同步跳跃宽度1个时间单位
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;			//时间段1位13个时间单位		
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;			//时间段2为2个时间单位
	
	//BaudRate=72M/2/CAN_Prescaler/(1+13+2)
	////250K时的配置
	CAN_InitStructure.CAN_Prescaler = 9;//时间单位长度为60	//72M/2/9/(1+13+2)=0.25 =250K
	//125K时的配置
	//CAN_InitStructure.CAN_Prescaler = 18;break;//时间单位长度为60 //72M/2/18/(1+13+2)=0.125 =125K
	CAN_Init(CAN1, &CAN_InitStructure);

	//配置CAN过滤器
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 0;//指定过滤器(0-13)
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//指定过滤器为标识符屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//指定过滤器位宽为32位
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((BMS_Filter_ID_Mask<<3)&0xffff0000)>>16);
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((BMS_Filter_ID_Mask<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);//CAN_ID_EXT	CAN_RTR_DATA
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0xffff;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;//过滤器0关联到FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE); //FIFO0消息挂号中断允许
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 主优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


//初始化充电电源模块_CAN口:CAN2
void ACDC_Module_Can_Init(void)  
{                                                         
	GPIO_PinConfigure(GPIOB,12,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//RX上拉输入
	GPIO_PinConfigure(GPIOB,13,GPIO_AF_PUSHPULL,GPIO_MODE_OUT50MHZ);//TX复用推挽输出
	
	CAN_InitTypeDef	CAN_InitStructure;
	CAN_DeInit(CAN2);//将CANx寄存器全部设置为缺省值
	CAN_StructInit(&CAN_InitStructure);//CAN register init
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);//使能CAN2时钟
	
	CAN_InitStructure.CAN_TTCM = DISABLE;//时间触发模式
	CAN_InitStructure.CAN_ABOM = DISABLE; //自动离线管理
	CAN_InitStructure.CAN_AWUM = DISABLE;//自动唤醒模式
	CAN_InitStructure.CAN_NART = ENABLE;//非自动重传模式
	CAN_InitStructure.CAN_RFLM = DISABLE;//接受FIFO锁定模式
	CAN_InitStructure.CAN_TXFP = DISABLE;//发送FIFO优先级
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//环回自测模式	CAN_Mode_Silent;	//CAN正常模式
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;			//重新同步跳跃宽度1个时间单位
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;			//时间段1位13个时间单位		
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;			//时间段2为2个时间单位
	
	//BaudRate=72M/2/CAN_Prescaler/(1+13+2)
	//250K时的配置
	CAN_InitStructure.CAN_Prescaler = 9;//时间单位长度为60	//72M/2/9/(1+13+2)=0.25 =250K
	//125K时的配置
	//CAN_InitStructure.CAN_Prescaler = 18;break;//时间单位长度为60 //72M/2/18/(1+13+2)=0.125 =125K
	CAN_Init(CAN2, &CAN_InitStructure);

	//配置CAN过滤器
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 14;//指定过滤器14(14-27)
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//指定过滤器为标识符屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//指定过滤器位宽为32位
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((ACDC_Filter_ID_Mask<<3)&0xffff0000)>>16);
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((ACDC_Filter_ID_Mask<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;//过滤器0关联到FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE); //FIFO0消息挂号中断允许
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 主优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


//返回值:0,成功;1,失败
unsigned char Can_Send_Msg(CAN_TypeDef* CANx, CanTxMsg* TxMessage)
{	
	uint16_t i = 0;          
	uint8_t mbox = CAN_Transmit(CANx, TxMessage);   
	while((CAN_TransmitStatus(CANx, mbox) == CAN_TxStatus_Failed)&&(i<0XFFF))i++;	//等待发送结束
	if(i>=0XFFF)return 1;
	return 0;	 
}


/**********************************************中断处理*****************************************************/
uint8_t buf[8];
volatile unsigned char BMS_Recevie_Flag;
CanRxMsg RxMsg1;
//CAN_1FIFO中断服务函数			    
void CAN1_RX0_IRQHandler(void)
{
	if(CAN_MessagePending(CAN1,CAN_FIFO0) != 0)
	{	
		CAN_Receive(CAN1, CAN_FIFO0, &RxMsg1);
		
			if(RxMsg1.ExtId == 0X1CEC56F4)//BMS请求建立多包发送连接
			{	
				memcpy(buf,RxMsg1.Data,RxMsg1.DLC);
				if(RxMsg1.Data[0] == 0x10)
				{
					memcpy(J1939_Multi_Package,RxMsg1.Data,RxMsg1.DLC);//保存多包发送连接的配置数据					
					TxMsg1.ExtId = 0X1CECF456;			 //应答多包发送请求
					TxMsg1.DLC = 8;																	
					TxMsg1.Data[0] = 0x11;									//应答头	
					TxMsg1.Data[1] = J1939_Multi_Package[3];//可发送包数
					TxMsg1.Data[2] = 1;											//包号
					TxMsg1.Data[3] = TxMsg1.Data[4] = 0xff;//缺省值		
					memcpy(&TxMsg1.Data[5],&J1939_Multi_Package[5],3);//PGN*/							
					Can_Send_Msg(CAN1, &TxMsg1);
				}
				
				BMS_Recevie_Flag = 0;
			}else BMS_Recevie_Flag = 1;
		
	}


}


unsigned short  k2;
CanRxMsg RxMsg2;
//CAN_2FIFO中断服务函数			    
void CAN2_RX0_IRQHandler(void)
{
	k2++;
  CAN_Receive(CAN2, 0, &RxMsg2);
}
/*******************************************************************************************************/


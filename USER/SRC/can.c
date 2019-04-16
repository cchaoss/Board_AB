#include "can.h"
#include "bms.h"


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
	CAN_InitStructure.CAN_RFLM = ENABLE;//接受FIFO锁定模式
	CAN_InitStructure.CAN_TXFP = DISABLE;//发送FIFO优先级
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;			//重新同步跳跃宽度1个时间单位
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;			//时间段1位13个时间单位		
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;			//时间段2为2个时间单位
	
	////250K时的配置//BaudRate=72M/2/CAN_Prescaler/(1+13+2)
	CAN_InitStructure.CAN_Prescaler = 9;//时间单位长度为60	//72M/2/9/(1+13+2)=0.25 =250K
	//CAN_InitStructure.CAN_Prescaler = 18;break;//时间单位长度为60 //72M/2/18/(1+13+2)=0.125 =125K
	CAN_Init(CAN1, &CAN_InitStructure);

	//配置CAN过滤组0 -> FIFO0
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 0;//指定过滤器组0-13
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;//指定过滤器为列表模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//指定过滤器位宽为32位
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((0x1CEC56F4U<<3)&0xffff0000)>>16);//多包请求ID
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((0x1CEC56F4U<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh =(uint16_t)(((0x1CEB56F4U<<3)&0xffff0000)>>16);//多包传输ID
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = (uint16_t)(((0x1CEB56F4U<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;//过滤组1关联到FIFO1
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	//配置CAN过滤组1 -> FIFO1
	CAN_FilterInitStructure.CAN_FilterNumber = 1;//指定过滤器组0-13
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//指定过滤器为标识符屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//指定过滤器位宽为32位
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((0x000056f4U<<3)&0xffff0000)>>16);
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((0x000056f4U<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = (uint16_t)(0xffff<<3);
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO1;//过滤器1关联到FIFO1
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	CAN_ITConfig(CAN1, CAN_IT_FMP0|CAN_IT_FMP1, ENABLE); //FIFO0 FIFO1消息挂号中断允许
	
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
	CAN_InitStructure.CAN_RFLM = ENABLE;//接受FIFO锁定模式
	CAN_InitStructure.CAN_TXFP = DISABLE;//发送FIFO优先级
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//环回自测模式	CAN_Mode_Silent;	//CAN正常模式
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;			//重新同步跳跃宽度1个时间单位
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;			//时间段1位13个时间单位		
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;			//时间段2为2个时间单位
	CAN_InitStructure.CAN_Prescaler = 18;//时间单位长度为60 //72M/2/18/(1+13+2)=0.125 =125K
	CAN_Init(CAN2, &CAN_InitStructure);

	//配置CAN过滤器组0 ->FIFO0
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 14;//指定过滤器组14(14-27)
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//指定过滤器为标识符屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//指定过滤器位宽为32位
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((0x0000F000U<<3)&0xffff0000)>>16);//永联英飞源模块应答地址F0
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((0x0000F000U<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = (uint16_t)(0xff00U<<3);
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;//过滤器14关联到FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
		//配置CAN过滤组1 -> FIFO1
	CAN_FilterInitStructure.CAN_FilterNumber = 15;//指定过滤器组0-13
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//指定过滤器为标识符屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//指定过滤器位宽为32位
	CAN_FilterInitStructure.CAN_FilterIdHigh = (uint16_t)(((0x0ABC00C0U<<3)&0xffff0000)>>16);//板间数据传输地址前3位固定ABC
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t)(((0x0ABC00C0U<<3)|CAN_RTR_DATA|CAN_ID_EXT)&0x0000ffff);//AB板只对来自C板的数据接受
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (uint16_t)(0x0fffU<<3);
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = (uint16_t)(0x00f0U<<3);
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO1;//过滤器15关联到FIFO1
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	CAN_ITConfig(CAN2, CAN_IT_FMP0|CAN_IT_FMP1, ENABLE); //FIFO0 FIFO1消息挂号中断允许
	
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

/************************BMS中断处理*****************************/
Can_Rx_FlagBits	RX_Flag;
CanRxMsg BMS_RX_0;
void CAN1_RX0_IRQHandler(void)//负责接送多包
{		
	CAN_Receive(CAN1, CAN_FIFO0, &BMS_RX_0);
	Multi_Package_Deal();//处理BMS多包传输
}

CanRxMsg BMS_RX_1;
void CAN1_RX1_IRQHandler(void)
{
	CAN_Receive(CAN1, CAN_FIFO1, &BMS_RX_1);
	RX_Flag.BMS_Rx_Flag1 = true;
}

/************************ACDC中断处理***************************/
CanRxMsg ACDC_RX;	    
void CAN2_RX0_IRQHandler(void)
{
  CAN_Receive(CAN2, CAN_FIFO0, &ACDC_RX);
	RX_Flag.ACDC_Rx_Flag = true;
}

/************************AB——C板通讯****************************/
CanRxMsg ABC_DATA_RX;
void CAN2_RX1_IRQHandler(void)
{
  CAN_Receive(CAN2, CAN_FIFO1, &ABC_DATA_RX);
	RX_Flag.ABC_Data_Rx_Flag = true;
}

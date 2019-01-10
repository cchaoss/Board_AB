#include "bsp.h"
#include "main.h"
#include "can.h"
#include "lcd.h"
#include "adc.h"
#include "acdc_module.h"
#include "electric_meter.h"

static void DIDO_init(void)
{
	//LED
	GPIO_PinConfigure(LED_BOARD_PORT,LED_RUN_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ��RUN_LED
	GPIO_PinConfigure(LED_BOARD_PORT,LED3_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ��LED3
	GPIO_PinConfigure(LED_BOARD_PORT,LED4_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ��LED4
	GPIO_PinConfigure(LED_BOARD_PORT,LED5_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ��LED5
	GPIO_PinConfigure(LED_CHARGE_PORT,LED_CHARGE_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ������
	GPIO_PinConfigure(LED_GUZHANG_PORT,LED_GUZHANG_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ�����ϵ�
	//����ź�����DI
	GPIO_PinConfigure(JT_PORT,JT_PIN,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//��ͣ����:��������
	GPIO_PinConfigure(START_PORT,START_PIN,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//��ͣ����
	GPIO_PinConfigure(K_GUN_ACK_PORT,K_GUN_ACK_PIN,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//ǹ�̵�����������
	GPIO_PinConfigure(LOCK_ACK_PORT,LOCK_ACK_PIN,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//����������
	GPIO_PinConfigure(KK_ACK_PORT,KK_ACK_PIN2,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//�м�̵������߷�������
	GPIO_PinConfigure(KK_ACK_PORT,KK_ACK_PIN1,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//�м�̵������߷�������

	//���Ƽ̵���DO
	GPIO_PinConfigure(LOCK_GUN_PORT,LOCK_GUN_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(LOCK_GUN_PORT,LOCK_GUN_PIN2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//ǹ������
	GPIO_PinConfigure(K_GUN_PORT,K_GUN_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//ǹ�ϼ̵������ƿ���
	GPIO_PinConfigure(KK_PORT,KK_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(KK_PORT,KK_PIN2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//�м�̵������ƿ���
	GPIO_PinConfigure(PE_RELAY_PORT,PE_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//�ӵؼ�鿪��
	GPIO_PinConfigure(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��Ե��鿪��
	GPIO_PinConfigure(XIEFANG_RELAY_PORT,XIEFANG_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//й�ż�鿪��
	GPIO_PinConfigure(BMS_POWER_RELAY_PORT,BMS_POWER_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//BMS������Դ����
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//����JTAG���� ����ͨ����
	GPIO_PinConfigure(DIP_SWITCH_PORT1,DIP_SWITCH_PIN1,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//AB��ѡ����������
	GPIO_PinConfigure(DIP_SWITCH_PORT2,DIP_SWITCH_PIN2,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//Ԥ���Ĳ��뿪������
	GPIO_PinConfigure(PE_CHECK_PORT,PE_CHECK_PIN,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//�ӵؼ�������
	
	//�ϵ��������п��ضϿ�
	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN1,0);
	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,1);
	delay_us(100000);//100ms
	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN2,0);//����
	LED_ERR_OFF;LED_CHARGE_OFF;//���/���ϵ�Ϩ��
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,0);//�رսӵؼ��relay
	GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,0);
	GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN2,0);//�رվ�Ե���relay
	GPIO_PinWrite(XIEFANG_RELAY_PORT,XIEFANG_RELAY_PIN,0);//�ر�й�ż��relay
	GPIO_PinWrite(LED_BOARD_PORT,LED5_PIN,0);//ʧ����5Ĭ��Ϩ��
	Open_K1K2;//�Ͽ�ǹ�ϼ̵���
	Open_K3K4;//�رո�����Դ
	Open_KK;//�Ͽ��м�̵���
}

//note:Reload <=4095
static void IWDG_Init(uint8_t Prescaler ,uint16_t Reload)
{
	IWDG->KR = IWDG_WriteAccess_Enable;
	IWDG->PR = Prescaler;
	IWDG->RLR = Reload;
	IWDG->KR = (uint16_t)0xAAAA;
	IWDG->KR = (uint16_t)0xCCCC;
}

unsigned short ChargTime;//���Ʒ�ʱ�䣬������ʹ�� TIM2���м�ʱ��ÿ30S��1
float dianliang;
void Tim2_Init(unsigned short Psc,unsigned short Arr)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM_TimeBaseStructure.TIM_Prescaler= Psc-1;
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period= Arr-1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM2, ENABLE);	
	dianliang = 0;
	ChargTime = 0;
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{	
		TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);  	
		ChargTime++;
		dianliang += Module_Status.Output_Vol * Module_Status.Output_Cur / 120 / 1000;//����ѳ����(kwh��) = ��ѹ*����*ʱ��(Сʱ)/1000
	}
}

DI_Ack_Flags DI_Ack;
DI_Data_Type DI_Filter;
//���ƻ��������˲�������������DI_Filter_Size�����ݾ�һ������Ϊ����
//����ֵ��0�͵�ƽ 1�ߵ�ƽ FF��Ч����
unsigned char DI_Status_Check(_Filter_TYPE*	Filter, unsigned char GPIOX_Status)
{
	Filter->Buffer[Filter->Count] = GPIOX_Status;
	if(++Filter->Count == DI_Filter_Size)
	{
		Filter->Count = 0;
		Filter->Ready = true;
	}
	if(Filter->Ready)
	{
		for(char i = 0;i<DI_Filter_Size-1;i++)
		{
			if(Filter->Buffer[i] != Filter->Buffer[i+1])	
				return 0XFF;//ֻҪ�в�һ�µ�ֵ����Ϊ���ݲ�����
		}
		return Filter->Buffer[0];
	}else return 0XFF;
}

//�ӵؼ��:0���� 1�ӵع���
unsigned char Check_PE(void)
{
	unsigned short PE_SUM;
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,1);//�򿪽ӵؼ��̵���
	for(short i = 0;i<3000;i++)//~600ms
	{
		delay_us(200);
		PE_SUM += GPIO_PinRead(PE_CHECK_PORT,PE_CHECK_PIN);
	}
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,0);//�رսӵؼ��̵���
	return ((3000-PE_SUM) < 300);//��鵽�͵�ƽ����>300
}

//���5%
void delay_us(u32 nTimer)
{
	for(u32 i=0;i<nTimer;i++){
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();//__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();//for���Ҳ��Ҫʱ��
	}
}

void Bsp_init(void)
{
	/*Notes:���ڰ��Ӿ����õ��ⲿ8M����Ҫ�޸������ļ�
		1.system_stm32f10x.c	1036���޸ľ���8821 
		2.stm32F10x.h  122��25M��Ϊ8M								*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//Ƕ�������жϿ�������ѡ���жϷ���
	DIDO_init();//��ʼ��Ӳ��IO
	ADCx_Init();//��ʼ���ڲ�ADC�ɼ�
	LCD_UART_Init(435200);//��ʼ��LCD��Ļ����	244800	870400
	METER_UART_Init(2400);//��ʼ�����485���� 2400 9600
	/*Notes:Ҫ��ʼ��CAN1 ��������ʹ��CAN2�Ľ��ܣ�*/
	BMS_Can_Init();				 //��ʼ����BMSͨѶ��CAN��
	ACDC_Module_Can_Init();//��ʼ�����Դģ��ͨѶ��CAN��

	IWDG_Init(IWDG_Prescaler_64 ,1250);//Tout=(4*2^Prescaler*Reload)/40��λ:ms ������2s���
}

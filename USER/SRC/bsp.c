#include "bsp.h"
#include "main.h"
#include "can.h"
#include "lcd.h"

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
	GPIO_AFConfigure(AFIO_SWJ_JTAG_NO_SW);//����JTAG IO��
	GPIO_PinConfigure(DIP_SWITCH_PORT1,DIP_SWITCH_PIN1,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//AB��ѡ����������
	GPIO_PinConfigure(DIP_SWITCH_PORT2,DIP_SWITCH_PIN2,GPIO_IN_PULL_UP,GPIO_MODE_INPUT);//Ԥ���Ĳ��뿪������
	//���Ƽ̵���DO
	GPIO_PinConfigure(LOCK_GUN_PORT,LOCK_GUN_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(LOCK_GUN_PORT,LOCK_GUN_PIN2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//ǹ������
	GPIO_PinConfigure(K_GUN_PORT,K_GUN_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//ǹ�ϼ̵������ƿ���
	GPIO_PinConfigure(KK_PORT,KK_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(KK_PORT,KK_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//�м�̵������ƿ���
	GPIO_PinConfigure(PE_RELAY_PORT,PE_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//�ӵؼ�鿪��
	GPIO_PinConfigure(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);
	GPIO_PinConfigure(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��Ե��鿪��
	GPIO_PinConfigure(XIEFANG_RELAY_PORT,XIEFANG_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//й�ż�鿪��
	GPIO_PinConfigure(BMS_POWER_RELAY_PORT,BMS_POWER_RELAY_PIN,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//BMS������Դ����
	//�ϵ��������п��ضϿ�
	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN1,0);
	GPIO_PinWrite(LOCK_GUN_PORT,LOCK_GUN_PIN1,0);//����
	GPIO_PinWrite(K_GUN_PORT,K_GUN_PIN,0);//�ر�ǹ�ϼ̵���
	GPIO_PinWrite(KK_PORT,KK_PIN1,0);
	GPIO_PinWrite(KK_PORT,KK_PIN2,0);//�ر��м�̵���
	GPIO_PinWrite(LED_CHARGE_PORT,LED_CHARGE_PIN,0);
	GPIO_PinWrite(LED_GUZHANG_PORT,LED_GUZHANG_PIN,0);//���/���ϵ�Ϩ��
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,0);			//�رսӵؼ��relay
	GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,0);
	GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN2,0);//�رվ�Ե���relay
	GPIO_PinWrite(XIEFANG_RELAY_PORT,XIEFANG_RELAY_PIN,0);//�ر�й�ż��relay
	GPIO_PinWrite(BMS_POWER_RELAY_PORT,BMS_POWER_RELAY_PIN,0);//�رո�����Դ
	/**/
	GPIO_PinConfigure(GPIOD,2,GPIO_OUT_PUSH_PULL,GPIO_MODE_OUT2MHZ);//��ʼ��Ӳ�����Ź�IO
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

void Bsp_init(void)
{
	//Notes:1.system_stm32f10x.c 1036���޸ľ���8821 stm32F10x.h 122��25M��Ϊ8M
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//Ƕ�������жϿ�������ѡ���жϷ���
	DIDO_init();//��ʼ��Ӳ��IO
	BMS_Can_Init();//��ʼ����BMSͨѶ��CAN��
	ACDC_Module_Can_Init();//��ʼ�����Դģ��ͨѶ��CAN��
	
//	LCD_USART2_Config(9600);//��ʼ��LCD��Ļ����
	
	IWDG_Init(IWDG_Prescaler_64 ,1250);//Tout=(4*2^Prescaler*Reload)/40��λ:ms ������2s���
}


DI_Ack_Flags DI_Ack;
DI_Data_Type DI_Filter;
//���ƻ��������˲�������������DI_Filter_Size�����ݾ�һ������Ϊ����
//����ֵ��0�͵�ƽ 1�ߵ�ƽ -1��Ч����
short DI_Status_Check(_Filter_TYPE	Filter, unsigned char GPIOX_Status)
{
	Filter.Buffer[Filter.Count] = GPIOX_Status;
	if(++Filter.Count == DI_Filter_Size)
	{
		Filter.Count = 0;
		Filter.Ready = true;
	}
	if(Filter.Ready)
	{
		for(char i = 0;i<DI_Filter_Size-1;i++)
		{
			if(Filter.Buffer[i] != Filter.Buffer[i+1])	
				return -1;//ֻҪ�в�һ�µ�ֵ����Ϊ���ݲ�����
		}
		if(Filter.Buffer[0]==1)	return 1;
			else return 0;
	}else return -1;
}

//�ӵؼ��
unsigned char Check_PE(void)
{
	unsigned short PE_SUM;
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,1);//�򿪽ӵؼ��̵���
	for(short i = 0;i<3000;i++)//~600ms
	{
		delay_us(200);
		PE_SUM += GPIO_PinRead(PE_RELAY_PORT,PE_RELAY_PIN);
	}
	GPIO_PinWrite(PE_RELAY_PORT,PE_RELAY_PIN,0);//�رսӵؼ��̵���
	if((3000-PE_SUM) > 50)//��鵽�͵�ƽ����50����
		return 0;//�ӵ�����
	else return 1;//�ӵع���
}

//���5%
void delay_us(u32 nTimer)
{
	u32 i=0;
	for(i=0;i<nTimer;i++){
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();//__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();//for���Ҳ��Ҫʱ��
	}
}

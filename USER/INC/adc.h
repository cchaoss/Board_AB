#ifndef __ADC_H
#define __ADC_H

#include "main.h"

/***************ADC1����ͨ�������ţ�����*******************/
#define	ADC_x                ADC1
#define	ADC_APBxClock_FUN    RCC_APB2PeriphClockCmd
#define	ADC_CLK              RCC_APB2Periph_ADC1

#define	AD_VT1_CHANNEL			 ADC_Channel_11//��Ե���VT 1
#define	AD_VT2_CHANNEL       ADC_Channel_12//��Ե���VT 2
#define	AD_CC_CHANNEL        ADC_Channel_13//CC
#define	AD_T1_CHANNEL				 ADC_Channel_1//ǹ�¶�1
#define	AD_T2_CHANNEL        ADC_Channel_2//ǹ�¶�2

//ADC1 ��Ӧ DMA1��ͨ��1��ADC3 ��Ӧ DMA2��ͨ��5��ADC2û��DMA����
#define	ADC_DMA_CLK          RCC_AHBPeriph_DMA1
#define ADC_DMA_CHANNEL      DMA1_Channel1

#define CHANNEL_NUM	 5	//ת��ͨ������
#define CC_K	0.003216f

enum _CCStatus
{
	PlugSta_Unknown = 0,		//δ֪״̬
	PlugSta_Open,						//����״̬
	PlugSta_Close,					//�γ�״̬
};


typedef struct
{
	unsigned char VT_Return;//��Ե�����0��Ե������1��Ե����
	float VT1;//��Ե���VT 1
	float VT2;//��Ե���VT 2
	float CC;//CC
	float T1;//ǹ�¶�1
	float T2;//ǹ�¶�2
}AD_VALUE;
extern AD_VALUE AD_DATA;

#define Insulation_Check_VOL 500 //��Ե����ѹ500v
#define LPF_1_(hz,t,in,out) ((out) += ( 1 / ( 1 + 1 / ( (hz) *3.14159f *(t) ) ) ) *( (in) - (out) ))

void ADCx_Init(void);
void Get_Adc_Status(void);
void Start_Insulation_Check(void);
#endif	

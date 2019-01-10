#ifndef __ADC_H
#define __ADC_H

#include "main.h"

/***************ADC1输入通道（引脚）配置*******************/
#define	ADC_x                ADC1
#define	ADC_APBxClock_FUN    RCC_APB2PeriphClockCmd
#define	ADC_CLK              RCC_APB2Periph_ADC1

#define	AD_VT1_CHANNEL			 ADC_Channel_11//绝缘检查VT 1
#define	AD_VT2_CHANNEL       ADC_Channel_12//绝缘检查VT 2
#define	AD_CC_CHANNEL        ADC_Channel_13//CC
#define	AD_T1_CHANNEL				 ADC_Channel_1//枪温度1
#define	AD_T2_CHANNEL        ADC_Channel_2//枪温度2

//ADC1 对应 DMA1的通道1，ADC3 对应 DMA2的通道5，ADC2没有DMA功能
#define	ADC_DMA_CLK          RCC_AHBPeriph_DMA1
#define ADC_DMA_CHANNEL      DMA1_Channel1

#define CHANNEL_NUM	 5	//转换通道个数
#define CC_K	0.003216f

enum _CCStatus
{
	PlugSta_Unknown = 0,		//未知状态
	PlugSta_Open,						//插入状态
	PlugSta_Close,					//拔出状态
};


typedef struct
{
	unsigned char VT_Return;//绝缘检查结果0绝缘正常，1绝缘错误
	float VT1;//绝缘检查VT 1
	float VT2;//绝缘检查VT 2
	float CC;//CC
	float T1;//枪温度1
	float T2;//枪温度2
}AD_VALUE;
extern AD_VALUE AD_DATA;

#define Insulation_Check_VOL 500 //绝缘检查电压500v
#define LPF_1_(hz,t,in,out) ((out) += ( 1 / ( 1 + 1 / ( (hz) *3.14159f *(t) ) ) ) *( (in) - (out) ))

void ADCx_Init(void);
void Get_Adc_Status(void);
void Start_Insulation_Check(void);
#endif	

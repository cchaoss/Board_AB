#include "adc.h"
#include "main.h"

unsigned short ADC_RAW_DATA[CHANNEL_NUM];//缓存adc各通道的值
//初始化ADC相关的IO口并启动AD采集以及DMA搬运
void ADCx_Init(void)
{
	GPIO_PinConfigure(GPIOC,1,GPIO_IN_ANALOG,GPIO_MODE_INPUT);//绝缘检查VT 1
	GPIO_PinConfigure(GPIOC,2,GPIO_IN_ANALOG,GPIO_MODE_INPUT);//绝缘检查VT 2
	GPIO_PinConfigure(GPIOC,3,GPIO_IN_ANALOG,GPIO_MODE_INPUT);//CC信号
	GPIO_PinConfigure(GPIOA,1,GPIO_IN_ANALOG,GPIO_MODE_INPUT);//枪温度检测-
	GPIO_PinConfigure(GPIOA,2,GPIO_IN_ANALOG,GPIO_MODE_INPUT);//枪温度检测+
	/*配置ADC1-DMA1-Channel1*/
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(ADC_DMA_CLK, ENABLE);
	
	DMA_DeInit(ADC_DMA_CHANNEL);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(ADC_x->DR));
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_RAW_DATA;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = CHANNEL_NUM;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(ADC_DMA_CHANNEL, &DMA_InitStructure);
	DMA_Cmd(ADC_DMA_CHANNEL , ENABLE);
	/*配置ADC*/
	ADC_InitTypeDef ADC_InitStructure;
	ADC_APBxClock_FUN (ADC_CLK, ENABLE);
	
	ADC_DeInit(ADC_x);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = CHANNEL_NUM;
	ADC_Init(ADC_x, &ADC_InitStructure);
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);//9M
												/*ADCx  	通道   	ADC通道的转换顺序  采样时间[(239.5+12.5)/9]us*/
	ADC_RegularChannelConfig(ADC_x, AD_VT1_CHANNEL, 1, ADC_SampleTime_239Cycles5);//绝缘检查VT 1
	ADC_RegularChannelConfig(ADC_x, AD_VT2_CHANNEL, 2, ADC_SampleTime_239Cycles5);//绝缘检查VT 2
	ADC_RegularChannelConfig(ADC_x, AD_CC_CHANNEL,  3, ADC_SampleTime_239Cycles5);//CC
	ADC_RegularChannelConfig(ADC_x, AD_T1_CHANNEL,  4, ADC_SampleTime_239Cycles5);//枪温度检测-
	ADC_RegularChannelConfig(ADC_x, AD_T2_CHANNEL,  5, ADC_SampleTime_239Cycles5);//枪温度检测+

	ADC_DMACmd(ADC_x, ENABLE);
	ADC_Cmd(ADC_x, ENABLE);

	ADC_ResetCalibration(ADC_x);
	while(ADC_GetResetCalibrationStatus(ADC_x));
	ADC_StartCalibration(ADC_x);
	while(ADC_GetCalibrationStatus(ADC_x));
	ADC_SoftwareStartConvCmd(ADC_x, ENABLE);
}

//开始绝缘检查
unsigned char insulation_flag;
void Start_Insulation_Check(void)
{
	insulation_flag = 1;//开启绝缘检查标志
	AD_DATA.VT_Return = 0;//清除绝缘错误
	GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,1);
	GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN2,1);//闭合绝缘检查继电器
}

AD_VALUE AD_DATA;
float VTCalibrate[2];//保存的绝缘检查校准值
void Get_Adc_Status(void)
{
	static char count = 0;
	static bool once = true;
	static unsigned int temp[CHANNEL_NUM] = {0};
	unsigned short gg = 0;
	float VT = 0;
	if(count == 6)
	{	/****************/
		if(once){once = false;VTCalibrate[0] = temp[0]/6;VTCalibrate[1] = temp[1]/6;}//保存绝缘检查的校准值
		if(insulation_flag)
		{
			if(insulation_flag++>1)//确保电压稳定后在检查值（继电器闭合会出现波动）
			{
				AD_DATA.VT1 = temp[0]/6;
				AD_DATA.VT2 = temp[1]/6;
				VT = AD_DATA.VT1 - AD_DATA.VT2;
				if(VT > 0)	
				{
					gg = AD_DATA.VT1 - VTCalibrate[0];
					VT = (AD_DATA.VT2-VTCalibrate[1])/VT*847.5;//0.118为100k
				}
				else
				{
					gg = AD_DATA.VT2 - VTCalibrate[1];
					VT = (AD_DATA.VT1-VTCalibrate[0])/(-VT)*847.5;
				}
				if((gg < 200)||(VT < Insulation_Check_VOL*0.2f))	AD_DATA.VT_Return = 1;//绝缘检查错误！100-500欧/V报警可以充电 这里取200欧/V*500=100k以下报警不能充电
				insulation_flag = 0;//清除绝缘检查标志
				GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,0);
				GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN2,0);//断开绝缘检查继电器
			}
		}/*以上为绝缘检查*/
		AD_DATA.CC	= temp[2]/6*CC_K;//3.3V基准
		AD_DATA.T1 	= temp[3]/6;
		AD_DATA.T2 	= temp[4]/6;
		for(char i=0;i<CHANNEL_NUM;i++)	temp[i] = 0;
		count = 0;
	}
	else
	{
		count++;
		for(char i=0;i<CHANNEL_NUM;i++)	temp[i] += ADC_RAW_DATA[i];
	}
}

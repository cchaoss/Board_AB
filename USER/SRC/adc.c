#include "adc.h"
#include "main.h"

unsigned short ADC_RAW_DATA[CHANNEL_NUM];//����adc��ͨ����ֵ
//��ʼ��ADC��ص�IO�ڲ�����AD�ɼ��Լ�DMA����
void ADCx_Init(void)
{
	GPIO_PinConfigure(GPIOC,1,GPIO_IN_ANALOG,GPIO_MODE_INPUT);//��Ե���VT 1
	GPIO_PinConfigure(GPIOC,2,GPIO_IN_ANALOG,GPIO_MODE_INPUT);//��Ե���VT 2
	GPIO_PinConfigure(GPIOC,3,GPIO_IN_ANALOG,GPIO_MODE_INPUT);//CC�ź�
	GPIO_PinConfigure(GPIOA,1,GPIO_IN_ANALOG,GPIO_MODE_INPUT);//ǹ�¶ȼ��-
	GPIO_PinConfigure(GPIOA,2,GPIO_IN_ANALOG,GPIO_MODE_INPUT);//ǹ�¶ȼ��+
	/*����ADC1-DMA1-Channel1*/
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
	/*����ADC*/
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
												/*ADCx  	ͨ��   	ADCͨ����ת��˳��  ����ʱ��[(239.5+12.5)/9]us*/
	ADC_RegularChannelConfig(ADC_x, AD_VT1_CHANNEL, 1, ADC_SampleTime_239Cycles5);//��Ե���VT 1
	ADC_RegularChannelConfig(ADC_x, AD_VT2_CHANNEL, 2, ADC_SampleTime_239Cycles5);//��Ե���VT 2
	ADC_RegularChannelConfig(ADC_x, AD_CC_CHANNEL,  3, ADC_SampleTime_239Cycles5);//CC
	ADC_RegularChannelConfig(ADC_x, AD_T1_CHANNEL,  4, ADC_SampleTime_239Cycles5);//ǹ�¶ȼ��-
	ADC_RegularChannelConfig(ADC_x, AD_T2_CHANNEL,  5, ADC_SampleTime_239Cycles5);//ǹ�¶ȼ��+

	ADC_DMACmd(ADC_x, ENABLE);
	ADC_Cmd(ADC_x, ENABLE);

	ADC_ResetCalibration(ADC_x);
	while(ADC_GetResetCalibrationStatus(ADC_x));
	ADC_StartCalibration(ADC_x);
	while(ADC_GetCalibrationStatus(ADC_x));
	ADC_SoftwareStartConvCmd(ADC_x, ENABLE);
}

AD_VALUE AD_DATA;
unsigned char insulation_flag;
float VTCalibrate[2];//����ľ�Ե���У׼ֵ
//float t1,t2��g1,g2;
void Get_Adc_Status(void)
{
	static char count = 0;
	static bool once = true;
	static unsigned int temp[CHANNEL_NUM] = {0};
	unsigned short gg = 0;
	float VT = 0;
	if(count == 6)
	{
		if(once)//�����Ե����У׼ֵ
		{
			once = false;
			VTCalibrate[0] = temp[0]/6;
			VTCalibrate[1] = temp[1]/6;
		}
		if(insulation_flag)
		{
			if(insulation_flag++>1)//ȷ����ѹ�ȶ����ڼ��ֵ���̵����պϻ���ֲ�����
			{
				AD_DATA.VT1 = temp[0]/6;
				AD_DATA.VT2 = temp[1]/6;
				VT = AD_DATA.VT1 - AD_DATA.VT2;
				if(VT > 0)	
				{
					gg = AD_DATA.VT1 - VTCalibrate[0];
					VT = (AD_DATA.VT2-VTCalibrate[1])/VT*847.5;//0.118Ϊ100k
				}
				else
				{
					gg = AD_DATA.VT2 - VTCalibrate[1];
					VT = (AD_DATA.VT1-VTCalibrate[0])/(-VT)*847.5;
				}
				if((gg < 200)||(VT < 500))	AD_DATA.VT_Return = 1;//��Ե������
			}
		}
		AD_DATA.CC	= temp[2]/6*CC_K;//3.3V��׼
//		AD_DATA.T1 	= temp[3]/6;
//		AD_DATA.T2 	= temp[4]/6;
		for(char i=0;i<CHANNEL_NUM;i++)	temp[i] = 0;
		count = 0;
	}
	else
	{
		count++;
		for(char i=0;i<CHANNEL_NUM;i++)
			temp[i] += ADC_RAW_DATA[i];
	}
//	g1 = ADC_RAW_DATA[3];
//	g2 = ADC_RAW_DATA[4];
//	LPF_1_(10,0.02f,ADC_RAW_DATA[3],t1);
//	LPF_1_(10,0.02f,ADC_RAW_DATA[4],t2);
}

//��ʼ��Ե���
void Start_Insulation_Check(unsigned char Cmd)
{
	if(Cmd == 1)
	{
		insulation_flag = 1;
		AD_DATA.VT_Return = 0;//�����Ե����
		GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,1);
		GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN2,1);
	}
	else
	{
		GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN1,0);
		GPIO_PinWrite(JUEYUAN_RELAY_PORT,JUYUAN_RELAY_PIN2,0);//�رվ�Ե���̵���
		insulation_flag = 0;
	}
}

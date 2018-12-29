#include "lcd.h"
#include "bms.h"
#include "main.h"
#include "adc.h"
#include "acdc_module.h"
#include "electric_meter.h"

#define LCD_RX_DMA DMA2_Channel3
#define LCD_TX_DMA DMA2_Channel5

#define LCD_RX_SIZE	8//½ÓÊÜ
unsigned char LCD_Rx_Buffer[LCD_RX_SIZE];//½ÓÊÜÊý¾Ý»º´æ

#define LCD_TX_SIZE	105//·¢ËÍ
unsigned char LCD_Tx_Buffer[LCD_TX_SIZE];//·¢ËÍ
Uart_Rx_FlagBits Uart_Flag;//´®¿Ú½ÓÊÜ±ê¼Ç

void LCD_UART_Init(uint32_t bound)
{
	GPIO_PinConfigure(GPIOC,10,GPIO_AF_PUSHPULL,GPIO_MODE_OUT50MHZ);//TX-¸´ÓÃÍÆÍìÊä³ö
	GPIO_PinConfigure(GPIOC,11,GPIO_IN_PULL_DOWN,GPIO_MODE_INPUT);//RX-ÏÂÀ­ÊäÈë
	//³õÊ¼»¯´®¿Ú½á¹¹Ìå
	USART_InitTypeDef USART_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//³õÊ¼»¯´®¿ÚÊ±ÖÓ
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
	//ÉèÖÃ´®¿ÚÖÐ¶ÏÓÅÏÈ¼¶
	NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);//´ò¿ª¿ÕÏÐÖÐ¶Ï,ÓÃÓÚ½ÓÊÕ²»¶¨³¤Êý¾ÝÊ±ÅÐ¶Ï½ÓÊÜÍêÒ»Ö¡Êý¾Ý
	USART_Cmd(UART4, ENABLE);
	//USART_ClearFlag(UART4, USART_FLAG_TC);
	/*ÅäÖÃUART4 µÄDMA TX RX*/
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);//¿ªÆôDMA2Ê±ÖÓ
	//DMA2 Channel 3 triggerd by USART4 Rx event
	DMA_DeInit(DMA2_Channel3);//¸´Î»DMA¿ØÖÆÆ÷µÄÍ¨µÀ
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)LCD_Rx_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//ÍâÉè×÷ÎªÊý¾ÝÀ´Ô´
	DMA_InitStructure.DMA_BufferSize = LCD_RX_SIZE;//»º´æ³¤¶È
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//ÍâÉèµØÖ·²»µÝÔö
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//ÄÚ´æµØÖ·µÝÔö
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//ÍâÉèÊý¾Ý¿í¶È1×Ö½Ú
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//ÄÚ´æÊý¾Ý¿í¶È1×Ö½Ú
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//·ÇÑ­»·Ä£Ê½
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//ÍâÉèÓëÄÚ´æÍ¨Ñ¶£¬¶ø·ÇÄÚ´æµ½ÄÚ´æ
	DMA_Init(DMA2_Channel3,&DMA_InitStructure);
	DMA_Cmd (DMA2_Channel3,ENABLE);//Æô¶¯DMA½ÓÊÜ
	//DMA2 Channel 5 triggerd by USART4 Tx event
	DMA_DeInit(DMA2_Channel5);//¸´Î»DMA¿ØÖÆÆ÷µÄÍ¨µÀ
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)LCD_Tx_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//ÍâÉè×÷Îª´«ËÍÊý¾ÝÄ¿µÄµØ
	DMA_InitStructure.DMA_BufferSize = 0;//·¢ËÍ³¤¶ÈÎª0£¬Ä¬ÈÏ¹Ø±Õ
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//ÍâÉèµØÖ·²»µÝÔö
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//ÄÚ´æµØÖ·µÝÔö
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//ÍâÉèÊý¾Ý¿í¶È1×Ö½Ú
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//ÄÚ´æÊý¾Ý¿í¶È1×Ö½Ú
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//·ÇÑ­»·Ä£Ê½
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//ÍâÉèÓëÄÚ´æÍ¨Ñ¶£¬¶ø·ÇÄÚ´æµ½ÄÚ´æ
	DMA_Init(DMA2_Channel5,	&DMA_InitStructure);
	//DMA_Cmd (DMA2_Channel5,ENABLE);//Ä¬ÈÏ¹Ø±Õ
	USART_DMACmd(UART4, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);//Ê¹ÄÜ´®¿ÚDMA·¢ËÍºÍ½ÓÊÜ
}

//´®¿Ú2ÖÐ¶Ï·þÎñº¯Êý,½ÓÊÕ²»¶¨³¤Êý¾Ý---LCD
void UART4_IRQHandler(void)
{
	uint8_t clear = clear;
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)
	{
		clear = UART4->SR;
		clear = UART4->DR;//ÏÈ¶ÁSRÔÙ¶ÁDR²ÅÄÜÇå³ý
		Uart_Flag.Lcd_Rx_Flag = true;
	}
}

void LCD_DMA_Reset(DMA_Channel_TypeDef* DMAy_Channelx, unsigned char len)
{
	//USART_ClearFlag(UART4, USART_FLAG_TC);
	DMA_Cmd (DMAy_Channelx,DISABLE);//ÒªÏÈ¹Ø±ÕDMA²ÅÄÜÉèÖÃ³¤¶È
	DMAy_Channelx->CNDTR = len;			//ÖØÐÂÉèÖÃ¿ªÊ¼µØÖ·
	DMA_Cmd (DMAy_Channelx,ENABLE); //Æô¶¯DMA·¢ËÍ
}

unsigned char End_of_package[4] = {0xcc,0x33,0xc3,0x3c};//¹Ì¶¨°üÎ²
unsigned char Head_number[7] = {0xAA,0x14,0x02,0xff,0xff,0x00,0x00};//²»ÏÔÊ¾±³¾°É« ÎÞ·ûºÅÊý ÎÞÐ§²»ÏÔÊ¾ ´óÐ¡10*20 
unsigned char Head_string[7] = {0xAA,0x11,0x02,0xff,0xff,0x00,0x00};//²»ÏÔÊ¾±³¾°É« 10*20 ×Ö·ûÑÕÉ« ±³¾°ÑÕÉ«
unsigned char Page[8] = {0xAA,0x22,0x00,0x00,0xCC,0x33,0xC3,0x3C};//ÇÐ»»Ò³ÃæÖ¸Áî
unsigned char Hand[6] = {0xAA,0x00,0xCC,0x33,0xC3,0x3C};//ÎÕÊÖÖ¸Áî
void show_number(unsigned int number,unsigned short x,unsigned short y,unsigned char num1,unsigned char num2)
{
	memcpy(LCD_Tx_Buffer,Head_number,sizeof(Head_number));
	LCD_Tx_Buffer[7] = num1;//ÕûÊýÎ»Êý
	LCD_Tx_Buffer[8] = num2;//Ð¡ÊýÎ»Êý
	LCD_Tx_Buffer[9] = x>>8;
  LCD_Tx_Buffer[10]= x;
	LCD_Tx_Buffer[11]= y>>8;
  LCD_Tx_Buffer[12]= y;		
	LCD_Tx_Buffer[13]= number>>24;
	LCD_Tx_Buffer[14]= number>>16;			
	LCD_Tx_Buffer[15]= number>>8;
	LCD_Tx_Buffer[16]= number;		
	memcpy(&LCD_Tx_Buffer[17],End_of_package,sizeof(End_of_package));
	
	LCD_DMA_Reset(LCD_TX_DMA,21);
	while(DMA_GetCurrDataCounter(LCD_TX_DMA));//Ôô·³
	//while(RESET == USART_GetFlagStatus(UART4, USART_FLAG_TC));//Õâ¸ö±ÈÉÏÃæDMAcounterÒªºÄÊ±³¤Ò»Ð©£¡
}
//ÏÔÊ¾×Ö·û¸úÊý×ÖµÄxy×ø±êÊÇ·´µÄ
void Show_hanzi(unsigned char *hanzi,unsigned char size,unsigned short y,unsigned short x)
{
	memcpy(LCD_Tx_Buffer,Head_string,sizeof(Head_string));
	LCD_Tx_Buffer[7] = x>>8;
  LCD_Tx_Buffer[8] = x;
	LCD_Tx_Buffer[9] = y>>8;
  LCD_Tx_Buffer[10]= y;																									
	for(char i=0;i<(size);i++)	LCD_Tx_Buffer[11+i] = hanzi[i];//ºº×Ö´æ´¢ÇøÓò
	memcpy(&LCD_Tx_Buffer[size+11],End_of_package,sizeof(End_of_package));
	
	LCD_DMA_Reset(LCD_TX_DMA,size+15);
	while(DMA_GetCurrDataCounter(LCD_TX_DMA));
}

void switch_page(unsigned char num)
{
	memcpy(LCD_Tx_Buffer,Page,sizeof(Page));
	LCD_Tx_Buffer[3] = num;
	LCD_DMA_Reset(LCD_TX_DMA,8);
	while(DMA_GetCurrDataCounter(LCD_TX_DMA));
}

void Check_Hand(void)
{
	memcpy(LCD_Tx_Buffer,Hand,sizeof(Hand));
	LCD_DMA_Reset(LCD_TX_DMA,6);//¸´Î»DMA·¢ËÍÊ×µØÖ·
}

												/* P0			 P0_CC		  P1_V			P1_A		  P1_SOC	P1_KW 		 P1_TIME		P2_EXP*/
Display_Position SHOW = {{155,71},{125,289},{122,195},{122,219},{95,125},{122,241},{122,264},{258,58},{7,29,199},{295,50,175}};//ÇÎÆ¤UI
unsigned char Link_ok = 0;//LCDÆÁÁ¬½Ó×´Ì¬1Á¬½Ó 0Î´Á¬½Ó
void LcdShow(void)
{
	if(Uart_Flag.Lcd_Rx_Flag)
	{
		unsigned char LCD_DataLong = LCD_RX_SIZE - DMA_GetCurrDataCounter(LCD_RX_DMA);
		if((LCD_Rx_Buffer[0]==0xAA)&&(LCD_Rx_Buffer[1]==0x00)&&(LCD_Rx_Buffer[2]==0x4F)&&(LCD_Rx_Buffer[3]==0x4B))
			Link_ok = 1;//LCDÉè±¸´æÔÚ
		memset(LCD_Rx_Buffer,0,LCD_DataLong);
		LCD_DMA_Reset(LCD_RX_DMA,LCD_RX_SIZE);//¸´Î»DMA½ÓÊÜÊ×µØÖ·
		Uart_Flag.Lcd_Rx_Flag = false;
	}

	if(Link_ok)
	{
		if(Type_DM.DErr!=0)//×®×Ô¼ì´íÎó(ÕâÀïÊÇÕë¶Ô¼òÒ××®ÏÔÊ¾3¸öÄÚÈÝ)
		{
			switch_page(4);//ÏµÍ³×Ô¼ì½çÃæ
			if(Type_DM.DErr&Geodesic)	Show_hanzi(" Î´½ÓµØÏß",9,SHOW.P0[0],SHOW.P0[1]);
				else if(Type_DM.DErr&No_Module)	Show_hanzi("ÎÞµçÔ´Ä£¿é",10,SHOW.P0[0],SHOW.P0[1]);
					else if(Type_DM.DErr&Disconnect_C)	Show_hanzi("C°åÍ¨Ñ¶¹ÊÕÏ",11,SHOW.P0[0],SHOW.P0[1]);
						else if(Type_DM.DErr&Dc_Table_Err)	Show_hanzi("ÎÞÖ±Á÷µç±í",10,SHOW.P0[0],SHOW.P0[1]);
			if(MeterSta != No_Link)	Show_hanzi("M",1,SHOW.A_M[0],SHOW.A_M[2]);//Á¬ÉÏµç±í
			if(Board_Type == 0X0A)	Show_hanzi("A",1,SHOW.A_M[0],SHOW.A_M[1]);else Show_hanzi("B",1,SHOW.A_M[0],SHOW.A_M[1]);//¼òÒ××®ÒªÊ¹ÓÃA°å£¬ÕâÀïÏÔÊ¾·½±ãÉú²ú±æ±ð£¡
		}
		else//×®×Ô¼ìÍ¨¹ý
		{
			switch(Type_BMS.Step)
			{
				case BEGIN:
				case LOCKED:
				{
					switch_page(1);//´ý»ú½çÃæ
					show_number(Type_VolCur.CC,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);
					if(Type_DM.JiTing == 1)	Show_hanzi("¼±Í£ÒÑ°´ÏÂ£¡",12,SHOW.P0[0],SHOW.P0[1]-8);
						else if((AD_DATA.CC>3)&&(AD_DATA.CC<5))	Show_hanzi("Çë°´ÏÂÆôÍ£½¡",12,SHOW.P0[0],SHOW.P0[1]-10);
							else Show_hanzi("Çë²å³äµçÇ¹",10,SHOW.P0[0],SHOW.P0[1]);
					
					if(MeterSta != No_Link)	Show_hanzi("M",1,SHOW.A_M[0],SHOW.A_M[2]);//Á¬ÉÏµç±í
					if(Board_Type == 0X0A)	Show_hanzi("A",1,SHOW.A_M[0],SHOW.A_M[1]);else Show_hanzi("B",1,SHOW.A_M[0],SHOW.A_M[1]);//¼òÒ××®ÒªÊ¹ÓÃA°å£¬ÕâÀïÏÔÊ¾·½±ãÉú²ú±æ±ð£¡
						
				}break;
				case SEND_9728:	switch_page(1);Show_hanzi("³äµçÎÕÊÖÖÐ",10,SHOW.P0[0],SHOW.P0[1]);show_number(Type_VolCur.CC,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;
				case SEND_256:	switch_page(1);Show_hanzi("³äµç±æÊ¶ÖÐ",10,SHOW.P0[0],SHOW.P0[1]);show_number(Type_VolCur.CC,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;
				case SEND_2048:	switch_page(1);Show_hanzi(" BMS×¼±¸"  ,9 ,SHOW.P0[0],SHOW.P0[1]);show_number(Type_VolCur.CC,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;
				case SEND_2560:	switch_page(1);Show_hanzi("³äµç»ú×¼±¸",10,SHOW.P0[0],SHOW.P0[1]);show_number(Type_VolCur.CC,SHOW.P0_CC[0],SHOW.P0_CC[1],2,1);break;	
				case SEND_4608:	
					switch_page(2);//³äµç½çÃæ
					show_number(Type_VolCur.Soc,SHOW.P1_Soc[0],SHOW.P1_Soc[1],3,0);//SOC
					show_number(Type_VolCur.Vol,SHOW.P1_V[0],SHOW.P1_V[1],4,1);//µçÑ¹
					show_number(Type_VolCur.Cur,SHOW.P1_A[0],SHOW.P1_A[1],4,1);//µçÁ÷ 
					show_number(Type_VolCur.KWh,SHOW.P1_KW[0],SHOW.P1_KW[1],4,1);//µçÁ¿:µç±íÊý¾Ý»òÕß¶¨Ê±Æ÷ÀÛ¼Ó
					show_number(Data_4608.ChargingTime,SHOW.P1_Time[0],SHOW.P1_Time[1],3,0);//³äµçÊ±¼ä
					if((Type_DM.MErr2&0x7f)!=0)	{Show_hanzi("Ä£¿é¸æ¾¯Sta2:",13,SHOW.Sta[0],SHOW.Sta[1]);	show_number(Type_DM.MErr2,SHOW.Sta[2],SHOW.Sta[0],3,0);}//ÅÐ¶ÏÄ£¿é×´Ì¬²»Õý³£
						else if((Type_DM.MErr1&0xbe)!=0)	{Show_hanzi("Ä£¿é¸æ¾¯Sta1:",13,SHOW.Sta[0],SHOW.Sta[1]);	show_number(Type_DM.MErr1,SHOW.Sta[2],SHOW.Sta[0],3,0);}
							else if((Type_DM.MErr0&0x11)!=0)	{Show_hanzi("Ä£¿é¸æ¾¯Sta0:",13,SHOW.Sta[0],SHOW.Sta[1]);	show_number(Type_DM.MErr0,SHOW.Sta[2],SHOW.Sta[0],3,0);}								
					break;
				case SEND_6656:
				case STOP:
					switch_page(3);//³äµçÍê³É½çÃæ
					if(MeterSta != No_Link)	Show_hanzi("M",1,SHOW.A_M[0],SHOW.A_M[2]);//Á¬ÉÏµç±í
					if(Board_Type == 0X0A)	Show_hanzi("A",1,SHOW.A_M[0],SHOW.A_M[1]);else Show_hanzi("B",1,SHOW.A_M[0],SHOW.A_M[1]);//¼òÒ××®ÒªÊ¹ÓÃA°å£¬ÕâÀïÏÔÊ¾·½±ãÉú²ú±æ±ð£
					switch(Type_BMS.Stop_Reason)
					{
						case Time_Out:
							Show_hanzi(" Í¨Ñ¶³¬Ê±",9,SHOW.P0[0],SHOW.P0[1]);
							switch(Type_BMS.time_out)
							{
								case BRM512_Timeout:	Show_hanzi("   BRM512",9,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCP1536_Timeout:	Show_hanzi("  BCP 1536",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BRO2304_Timeout:	Show_hanzi("  BRO 2304",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCS4352_Timeout:	Show_hanzi("  BCS 4352",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BCL4096_Timeout:	Show_hanzi("  BCL 4096",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BST6400_Timeout:	Show_hanzi("  BST 6400",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
						case Err_Stop:
							Show_hanzi(" ¹ÊÕÏÍ£Ö¹",9,SHOW.P0[0],SHOW.P0[1]);
							switch(Type_BMS.DErr)
							{
								case Lock_ERR:			Show_hanzi("  ÎÞ·¨ÉÏËø",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case GUN_Relay_Err:	Show_hanzi("Ç¹ÉÏ¼ÌµçÆ÷¹ÊÕÏ",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-10);break;
								case KK_Relay_Err:	Show_hanzi("ÖÐ¼ä¼ÌµçÆ÷¹ÊÕÏ",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-10);break;
								case Insulation_ERR:Show_hanzi("  ¾øÔµ´íÎó",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Bat_Vol_ERR:		Show_hanzi("µç³ØµçÑ¹²»Æ¥Åä",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case CC_ERR:				Show_hanzi("CCµçÑ¹Òì³£",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
						case BMS_Stop:
							Show_hanzi(" BMSÖÐÖ¹",9,SHOW.P0[0],SHOW.P0[1]);
							switch(Type_BMS.BErr)
							{
								case Soc_Full:			Show_hanzi(" ÒÑ³äÂú£¡£¡",11,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Insulation:		Show_hanzi("  ¾øÔµ¹ÊÕÏ",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case BmsOutNetTemp:	Show_hanzi("Êä³öÁ¬½ÓÆ÷¹ýÎÂ",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-10);break;
								case ChargeNet:			Show_hanzi("³äµçÁ¬½ÓÆ÷¹ÊÕÏ",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-10);break;
								case BatTemp:				Show_hanzi("µç³ØÎÂ¶È¹ý¸ß",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case HighRelay:			Show_hanzi("¸ßÑ¹¼ÌµçÆ÷¹ÊÕÏ",14,SHOW.P2_Exp[0],SHOW.P2_Exp[1]-10);break;
								case Vol_2:					Show_hanzi(" µã2µçÑ¹´íÎó",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case CurOver:				Show_hanzi("  µçÁ÷¹ý´ó",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case CurUnknown:		Show_hanzi("  µçÁ÷²»¿ÉÐÅ",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case VolErr:				Show_hanzi("  µçÑ¹Òì³£",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case VolUnknown:		Show_hanzi("  µçÑ¹²»¿ÉÐÅ",12,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
						case Mannul_Stop:
							Show_hanzi(" ÈË¹¤ÖÐÖ¹",9,SHOW.P0[0],SHOW.P0[1]);
							switch(Type_BMS.Manual)
							{
								case JT_Stop:			Show_hanzi(" ¼±Í£°´ÏÂ£¡",11,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Card_Stop:		Show_hanzi("  Ë¢¿¨Í£Ö¹",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case App_Stop:		Show_hanzi("   AppÍ£Ö¹",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								case Start_Stop:	Show_hanzi("  ÆôÍ£¿ª¹Ø",10,SHOW.P2_Exp[0],SHOW.P2_Exp[1]);break;
								default:break;
							}break;
					}break;
				default:break;
			}
		}
	}else	Check_Hand();
}


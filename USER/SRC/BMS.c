#include "main.h"
#include "bms.h"
#include "can.h"

#pragma pack(1)   //Ç¿ÖÆ1×Ö½Ú¶ÔÆë
/*³äµç×®·¢¸øBMSµÄÊý¾Ý*/
stuPGN9728Type Data_9728 = {0x01,0x01,0x00};
stuPGN256Type	 Data_256  = {0x00,0,};
stuPGN1792Type Data_1792;
stuPGN2048Type Data_2048 = {7500,	2000,	4000-2000};//³äµç»úÊä³öµçÑ¹£º200-750V Êä³öµçÁ÷£º0-200A
stuPGN2560Type Data_2560;
stuPGN4608Type Data_4608 = {0,0,0,0x01};//Ä¬ÈÏÔÊÐí³äµç2015GB
stuPGN6656Type Data_6656;
stuPGN7424Type Data_7424;
stuPGN7936Type Data_7936;
/*BMS·¢¹ýÀ´µÄÊý¾Ý*/
stuPGN9984Type Data_9984;
stuPGN2304Type Data_2304;
stuPGN4096Type Data_4096;
stuPGN512Type	 Data_512;
stuPGN1536Type Data_1536;
stuPGN4352Type Data_4352;
stuPGN4864Type Data_4864;
stuPGN6400Type Data_6400;
stuPGN7168Type Data_7168;
stuPGN7680Type Data_7680;
#pragma pack()

#define RX_BMS_NUM 10							/*½ÓÊÜ±êÖ¾		±¨ÎÄPGN					Êý¾ÝÇø*/
RX_BMS RX_BMS_TAB[RX_BMS_NUM] =	{	{		0,			(9984>>8),			&Data_9984},	//BMSÎÕÊÖ±¨ÎÄ
																	{		0,			(2304>>8),			&Data_2304},	//µç³Ø×¼±¸¾ÍÐ÷±¨ÎÄ
																	{		0,			(4096>>8),			&Data_4096},	//µç³Ø³äµçÐèÇó±¨ÎÄ
																	{		0,			(4864>>8),			&Data_4864},	//¶¯Á¦Ðîµç³Ø×´Ì¬ÐÅÏ¢
																	{		0,			(6400>>8),			&Data_6400},	//BMSÖÐÖ¹³äµç±¨ÎÄ
																	{		0,			(7168>>8),			&Data_7168},	//BMSÍ³¼ÆÊý¾Ý±¨ÎÄ
																	{		0,			(7680>>8),			&Data_7680}, //BMS´íÎó±¨ÎÄ																	
																	{		0,			(512>>8),				&Data_512	},	//BMS³µÁ¾±æÊ¶±¨ÎÄ
																	{		0,			(1536>>8),			&Data_1536},	//¶¯Á¦Ðîµç³Ø³äµç²ÎÊý
																	{		0,			(4352>>8),			&Data_4352}};	//µç³Ø³äµç×Ü×´Ì¬±¨ÎÄ
									/*±¨ÎÄPGN		ÓÅÏÈ¼¶	Ä¿µÄµØÖ·	Ô´µØÖ·	Êý¾Ý³¤¶È		·¢ËÍÖÜÆÚms		Êý¾ÝÇø*/		
TX_BMS CHM_9728 = {(9728>>8),		6,		 0xf4,		 0x56,		 3,					250,			&Data_9728};//³äµç»úÎÕÊÖ±¨ÎÄ
TX_BMS CRM_256 = 	{(256 >>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_256 };//³äµç»ú±æÊ¶±¨ÎÄ
TX_BMS CTS_1792 = {(1792>>8),		6,		 0xf4,		 0x56,	 	 7,					250,			&Data_1792};//³äµç»ú·¢ËÍÊ±¼äÍ¬²½
TX_BMS CML_2048 = {(2048>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_2048};//³äµç»ú×î´óÊä³öÄÜÁ¦
TX_BMS CRO_2560 = {(2560>>8),		4,		 0xf4,		 0x56,		 1,					250,			&Data_2560};//³äµç»úÊä³ö×¼±¸¾ÍÐ÷
TX_BMS CCS_4608 = {(4608>>8),		6,		 0xf4,		 0x56,		 8,					50,				&Data_4608};//³äµç»ú³äµç×´Ì¬
TX_BMS CST_6656 = {(6656>>8),		4,		 0xf4,		 0x56,		 4,					10,				&Data_6656};//³äµç»úÖÐÖ¹³äµç
TX_BMS CSD_7424 = {(7424>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_7424};//³äµç»úÍ³¼ÆÊý¾Ý
TX_BMS CEM_7936 = {(7936>>8),		2,		 0xf4,		 0x56,		 4,					250,			&Data_7936};//³äµç»ú´íÎó±¨ÎÄ

CanTxMsg TxMsg1 =  {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//À©Õ¹Ö¡ Êý¾ÝÖ¡
unsigned char OUT,	BMS_STA = SEND_256;//Ìø¹ýÎÕÊÖÖ±½Ó±æÊ¶
void BMS_Task(void const *argument)
{
	const unsigned char BMS_Task_Time = 10U;//10msÑ­»·
	static unsigned short t,time_out;
	while(1)
	{	//¸ºÔð½ÓËÍ´¦Àíµ¥°üÊý¾Ý
		Single_Package_Deal();
		//BMS·¢ËÍ
		switch(BMS_STA)
		{
			case BEGIN://²åÇÀ×¼±¸
			{
				//CCÐÅºÅÊÇ·ñ3.2v-4.8v
				//ÉÏËø£¬ÉÏËø·´À¡¾ÍÐ÷£¿
				//¸¨ÖúµçÔ´¼ÌµçÆ÷±ÕºÏK3K4
				//¶¼okµÄ»°->{Ïà¹Ø±êÖ¾Î»Çå0£¬BMS_STA = SEND_9728;}//±¨ÎÄ½ÓÊÕ±êÖ¾Çå0
				break;
			}	
			case SEND_9728://³äµçÎÕÊÖ
			{
				t = (t+1)%(CHM_9728.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CHM_9728);}		
				if((RX_BMS_TAB[WAIT_9984_BHM].Rx_status == 1)||(time_out > 20))//ÊÕµ½9984||³¬Ê±5s->GB2015
				{
					//if(k1k2½Ó´¥Æ÷µçÑ¹<10V)
					//if(¾øÔµ¼à²âok£¿)//ÐèÒªµçÑ¹Ä£¿éÊä³öµçÑ¹
					//if(Ð¹·ÅµçÂ·¼ì²éok£¿)
					t = time_out = 0;
					Data_256.IdentifyResult = 0x00;//±æÊ¶×´Ì¬£ºÎ´±æÊ¶
					BMS_STA = SEND_256;
				}
				break;
			}		
			case SEND_256://³äµç±æÊ¶
			{
				t = (t+1)%(CRM_256.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CRM_256);}
				
				if(RX_BMS_TAB[WAIT_512_BRM].Rx_status == 1)	{time_out = 0;	Data_256.IdentifyResult = 0xAA;}//±æÊ¶³É¹¦	
					else if(time_out > 20)	BMS_STA = TIME_OUT;//½ÓÊÕ³µÁ¾±æÊ¶±¨ÎÄBRM³¬Ê±5s
				
				if(RX_BMS_TAB[WAIT_1536_BCP].Rx_status == 1)
				{
					//RX_BMS_TAB[WAIT_1536_BCP].Rx_status = RX_BMS_TAB[WAIT_512_BRM].Rx_status = 0;
					t = time_out = 0;
					BMS_STA = SEND_2048;				
				}else if(time_out > 20)	BMS_STA = TIME_OUT;//½ÓÊÜ1536±¨ÎÄ³¬Ê±5s
				break;
			}		
			case SEND_2048://³äµç»ú×î´óÊä³öÄÜÁ¦/Ê±¼äÍ¬²½
			{
				t = (t+1)%(CML_2048.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CML_2048);	BMS_Send(CTS_1792);}
				
				if(RX_BMS_TAB[WAIT_2304_BRO].Rx_status == 1)
				{
					if(Data_2304.BMSSta==0xAA)
					{
						//RX_BMS_TAB[WAIT_2304_BRO].Rx_status = 0;
						t = time_out = 0;
						Data_2560.PrepareOK = 0x00;//³äµç»ú×¼±¸Î´Íê³É
						BMS_STA = SEND_2560;
					}else if(time_out > 240)	BMS_STA = TIME_OUT;//BRO 60sÄÚÎ´×¼±¸ºÃ
				}else if(time_out >20)	BMS_STA = TIME_OUT;//½ÓÊÜ2304_BRO BMS×¼±¸³äµç¾ÍÐ÷³¬Ê±5s
				break;
			}
			
			case SEND_2560://³äµç»úÊä³ö×¼±¸¾ÍÐ÷
			{	
				t = (t+1)%(CRO_2560.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CRO_2560);}
				//ÅÐ¶ÏÇ¹µçÑ¹Óë±¨ÎÄµç³ØµçÑ¹Ïà²î5%ÒÔÄÚ£¬ÔÚ³äµç»ú×î´ó×îÐ¡µçÑ¹Ö®¼ä
				//µ÷ÕûµçÔ´Ä£¿éÊä³öµçÑ¹¸ßÓÚµç³ØµçÑ¹1-10vºó£¬±ÕºÏ¼ÌµçÆ÷K1K2ºó±ä¸üÎª0xAA
				if(1)	Data_2560.PrepareOK = 0xAA;
				if(RX_BMS_TAB[WAIT_4096_BCL].Rx_status == 1)
				{
					//RX_BMS_TAB[WAIT_4096_BCL].Rx_status = RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 0;
					time_out = 0;				
				}else if(time_out > 4)	BMS_STA = TIME_OUT;//½ÓÊÜ4096µç³Ø³äµçÐèÇó³¬Ê±250*4=1s
				if(RX_BMS_TAB[WAIT_4352_BCS].Rx_status == 1)
				{
					t = time_out = 0;
					BMS_STA = SEND_4608;
				}else if(time_out > 20)	BMS_STA = TIME_OUT;//4352µç³Ø³äµç×Ü×´Ì¬±¨ÎÄ³¬Ê±5s
				break;
			}		
			case SEND_4608://³äµç»ú³äµç×´Ì¬
			{
				t = (t+1)%(CCS_4608.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CCS_4608);
					Data_4608.OutputCurr = Data_4096.NeedCurr+10;//²âÊÔÓÃ
					Data_4608.OutputVolt = Data_4096.NeedVolt-10;//Êµ¼ÊÊ¹ÓÃÄÃACDCÄ£¿éÊý¾Ý
					Data_4608.ChargingTime++;}
				
				if(RX_BMS_TAB[WAIT_4864_BSM].Rx_status == 1)
				{
					RX_BMS_TAB[WAIT_4864_BSM].Rx_status = 0;
					//if(1)//µç³Ø×´Ì¬±êÇ©ÊÇ·ñÈ«²¿Õý³££¿else err
					//if(1)³äµç½ûÖ¹±ê¼ÇON?->³äµçÔÝÍ££¬¼ÌÐø½ÓÊÜÅÐ¶Ï4864BSMµç³Ø×´Ì¬ÊÇ·ñÕý³££	
				}
				//if(1)³äµç½áÊøÌõ¼þÊÇ·ñ³ÉÁ¢£¿³ÉÁ¢·¢ËÍ6656³äµç»úÖÐÖ¹³äµçCTS
				break;
			}
			case SEND_6656://³äµç»úÖÐÖ¹³äµç
			{
				t = (t+1)%(CST_6656.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CST_6656);}

				if((RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)||(time_out > 500))//ÊÕµ½BMSÖÐÖ¹³äµçBST||·¢ËÍCTS10*100*5=5sºó
				{
					//µçÁ¦Êä³öÍ£Ö¹²Ù×÷
					t = time_out = 0;
					BMS_STA = SEND_7424;
				}
				break;
			}			
			case SEND_7424://³äµç»úÍ³¼ÆÊý¾Ý±¨ÎÄ
			{
				t = (t+1)%(CSD_7424.Period/BMS_Task_Time);
				if(t == 1)	{time_out++;	BMS_Send(CSD_7424);}
				//¶Ï¿ªDC¼ÌµçÆ÷K1K2
				//Ð¹·ÅµçÂ·¶¯×÷

				if(RX_BMS_TAB[WAIT_7168_BSD].Rx_status == 1)//ÊÕµ½BMSÍ³¼ÆÊý¾ÝBSD
				{
					//¸¨ÖúµçÔ´¼ÌµçÆ÷¶Ï¿ªK3K4µç×ÓËø½âËø
					t = time_out = 0;
					BMS_STA = BEGIN;
				}//else if(time_out > 40)	BMS_STA = TIME_OUT;//½ÓÊÜBMSÍ³¼ÆÊý¾Ý±¨ÎÄ³¬Ê±10s		
				break;
			}
			case TIME_OUT:
			{
				if(BMS_STA != TIME_OUT)	OUT++;
				if(OUT > 3)//ÖØÁ¬³¬¹ý3´Î²»ÔÙÖØÁ¬
				{
					//µçÁ¦Êä³öÍ£Ö¹²Ù×÷
					if(time_out > 1)
					{
						//¶Ï¿ªK1K2
						//Ð¹·ÅµçÂ·¶¯×÷
						//¸¨ÖúµçÔ´¼ÌµçÆ÷K3K4¶Ï¿ª
						//µç×ÓËø½âËø
						t = time_out = 0;
						//BMS_STA = ;//±ØÐëÖØÐÂ°ÎÇ¹²åÇÀ²Å¿ÉÒÔ¼ÌÐø¿ªÊ¼
					}							
				}
				else 
				{
					t = (t+1)%(CEM_7936.Period/BMS_Task_Time);
					if(t == 1)	{time_out++;	BMS_Send(CEM_7936);}
					//µçÁ¦Êä³öÍ£Ö¹²Ù×÷
					if(time_out > 1)
					{
						//¶Ï¿ªK1K2
						t = time_out = 0;
						BMS_STA = SEND_256;//ÖØÐÂ¿ªÊ¼±æÊ¶
					}					
				}
				break;
			}
			default:break;
		}
		
		osDelay(BMS_Task_Time);
	}
}



static void BMS_Send(TX_BMS Pbuf)
{
	TxMsg1.ExtId = (Pbuf.IFArbition<<26) | (Pbuf.PGN_S<<16) | (Pbuf.PDUSpecific<<8) | (Pbuf.SourceAddress);
	TxMsg1.DLC = Pbuf.DLC;
	memset(TxMsg1.Data,0,sizeof(TxMsg1.Data));
	memcpy(TxMsg1.Data,Pbuf.Data,Pbuf.DLC);
	CAN_Transmit(CAN1, &TxMsg1);
}




static void Single_Package_Deal(void)
{
	if(BMS_Rx_Flag1 == 1)
	{	
		for(char i = WAIT_9984_BHM; i <= WAIT_7680_BEM; i++)
		{
			if((unsigned char)((BMS_RX_1.ExtId&0x00ff0000)>>16) == RX_BMS_TAB[i].PGN_S)//ÌáÈ¡±¨ÎÄPGNºÅ
			{
				RX_BMS_TAB[i].Rx_status = 1;
				memcpy(RX_BMS_TAB[i].Data,BMS_RX_1.Data,BMS_RX_1.DLC);
			}
		}
		BMS_Rx_Flag1 = 0;
	}
}
		

unsigned char *P = NULL;
unsigned char J1939_Multi_Package[8];//0x10,len_L,len_H,°üÊý,0xff,PGN[3]
CanTxMsg TxMsg_Ack  = {0, 0X1CECF456, CAN_Id_Extended, CAN_RTR_Data, 8, {0x11,0x00,1,0xff,0xff,0x00,0x00,0x00}};
CanTxMsg TxMsg_Done = {0, 0X1CECF456, CAN_Id_Extended, CAN_RTR_Data, 8, {0x13,0x00,0,0x00,0xff,0x00,0x00,0x00}};
//´¦ÀíBMS¶à°ü´«Êä	
void Multi_Package_Deal(void)
{
	if((BMS_RX_0.ExtId == 0X1CEC56F4)&&(BMS_RX_0.Data[0] == 0x10))//BMSÇëÇó½¨Á¢¶à°ü·¢ËÍÁ¬½Ó
	{		
		memcpy(&J1939_Multi_Package[1],&BMS_RX_0.Data[1],7);//±£´æ¶à°ü·¢ËÍÁ¬½ÓµÄÅäÖÃÊý¾Ý	
		for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++) \
			if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	P = (unsigned char*)RX_BMS_TAB[i].Data;
		
		TxMsg_Ack.Data[1] = J1939_Multi_Package[3];//¿É·¢ËÍ°üÊý	
		TxMsg_Ack.Data[6] = J1939_Multi_Package[6];//PGN*/							
		CAN_Transmit(CAN1, &TxMsg_Ack);
		if(TxMsg_Ack.Data[6] == 0x11)	CAN_Transmit(CAN1, &TxMsg_Ack);//Ó¦´ð4352µÃ·¢Á½±é£¡ÕæµÄÓÐ¶¾£¡£¡£¡
	}
	else if(BMS_RX_0.ExtId == 0X1CEB56F4)//¶à°üÊý¾Ý´«Êä
	{				
		if(BMS_RX_0.Data[0] == 1)	memcpy(P,&BMS_RX_0.Data[1],7); \
			else if(BMS_RX_0.Data[0] == 2)	memcpy(P+7,&BMS_RX_0.Data[1],7);//ÖØ×é¶à°üÊý¾Ý
				
		if(BMS_RX_0.Data[0] == J1939_Multi_Package[3])//¶à°ü½ÓÊÜÍê³É£¿
		{
			for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++)
				if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	RX_BMS_TAB[i].Rx_status  = 1;				
			TxMsg_Done.Data[1] = J1939_Multi_Package[1];
			TxMsg_Done.Data[3] = J1939_Multi_Package[3];
			TxMsg_Done.Data[6] = J1939_Multi_Package[6];
			CAN_Transmit(CAN1, &TxMsg_Done);//Ó¦´ð¶à°ü½ÓÊÜÍê³É
		}
	}
}

#include "main.h"
#include "bms.h"
#include "can.h"

#pragma pack(1)//Ç¿ÖÆ1×Ö½Ú¶ÔÆë
stuPGN9728Type Data_9728 = {0x01,0x01,0x00};
stuPGN256Type	 Data_256  = {0x00,0,};
stuPGN1792Type Data_1792;
stuPGN2048Type Data_2048 = {ACDC_MAX_VOL,ACDC_MIN_VOL,ACDC_MAX_CUR};//³äµç»úÊä³öµçÑ¹£º200-750V Êä³öµçÁ÷£º0-200A
stuPGN2560Type Data_2560;
stuPGN4608Type Data_4608 = {0,0,0,0x01};//Ä¬ÈÏÔÊĞí³äµç2015GB
stuPGN6656Type Data_6656;
stuPGN7424Type Data_7424;
stuPGN7936Type Data_7936;/*³äµç×®·¢¸øBMSµÄÊı¾İ*/
stuPGN9984Type Data_9984;/*BMS·¢¹ıÀ´µÄÊı¾İ*/
stuPGN2304Type Data_2304;
stuPGN4096Type Data_4096;
stuPGN512Type	 Data_512;
stuPGN1536Type Data_1536;
stuPGN4352Type Data_4352;
stuPGN4864Type Data_4864;
stuPGN6400Type Data_6400;
stuPGN7168Type Data_7168;
stuPGN7680Type Data_7680;
#pragma pack()							/*½ÓÊÜ±êÖ¾		±¨ÎÄPGN					Êı¾İÇø*/
RX_BMS RX_BMS_TAB[10] =	{	{		0,			(9984>>8),			&Data_9984},	//BMSÎÕÊÖ±¨ÎÄ
													{		0,			(2304>>8),			&Data_2304},	//µç³Ø×¼±¸¾ÍĞ÷±¨ÎÄ
													{		0,			(4096>>8),			&Data_4096},	//µç³Ø³äµçĞèÇó±¨ÎÄ
													{		0,			(4864>>8),			&Data_4864},	//¶¯Á¦Ğîµç³Ø×´Ì¬ĞÅÏ¢
													{		0,			(6400>>8),			&Data_6400},	//BMSÖĞÖ¹³äµç±¨ÎÄ
													{		0,			(7168>>8),			&Data_7168},	//BMSÍ³¼ÆÊı¾İ±¨ÎÄ
													{		0,			(7680>>8),			&Data_7680},  //BMS´íÎó±¨ÎÄ																	
													{		0,			(512>>8),				&Data_512	},	//BMS³µÁ¾±æÊ¶±¨ÎÄ
													{		0,			(1536>>8),			&Data_1536},	//¶¯Á¦Ğîµç³Ø³äµç²ÎÊı
													{		0,			(4352>>8),			&Data_4352}};	//µç³Ø³äµç×Ü×´Ì¬±¨ÎÄ
									/*±¨ÎÄPGN		ÓÅÏÈ¼¶	Ä¿µÄµØÖ·	Ô´µØÖ·	Êı¾İ³¤¶È		·¢ËÍÖÜÆÚms		Êı¾İÇø*/		
TX_BMS CHM_9728 = {(9728>>8),		6,		 0xf4,		 0x56,		 3,					250,			&Data_9728};//³äµç»úÎÕÊÖ±¨ÎÄ
TX_BMS CRM_256  =	{(256 >>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_256 };//³äµç»ú±æÊ¶±¨ÎÄ
TX_BMS CTS_1792 = {(1792>>8),		6,		 0xf4,		 0x56,	 	 7,					250,			&Data_1792};//³äµç»ú·¢ËÍÊ±¼äÍ¬²½
TX_BMS CML_2048 = {(2048>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_2048};//³äµç»ú×î´óÊä³öÄÜÁ¦
TX_BMS CRO_2560 = {(2560>>8),		4,		 0xf4,		 0x56,		 1,					250,			&Data_2560};//³äµç»úÊä³ö×¼±¸¾ÍĞ÷
TX_BMS CCS_4608 = {(4608>>8),		6,		 0xf4,		 0x56,		 8,					50,				&Data_4608};//³äµç»ú³äµç×´Ì¬
TX_BMS CST_6656 = {(6656>>8),		4,		 0xf4,		 0x56,		 4,					10,				&Data_6656};//³äµç»úÖĞÖ¹³äµç
TX_BMS CSD_7424 = {(7424>>8),		6,		 0xf4,		 0x56,		 8,					250,			&Data_7424};//³äµç»úÍ³¼ÆÊı¾İ
TX_BMS CEM_7936 = {(7936>>8),		2,		 0xf4,		 0x56,		 4,					250,			&Data_7936};//³äµç»ú´íÎó±¨ÎÄ
float CC = 12;
unsigned char OUT,	BMS_STA = BEGIN,g,gg;
void BMS_Task(void const *argument)
{
	const unsigned char BMS_Task_Time = 10U;//10msÑ­»·
	static unsigned short t,timeout,timeout1;
	static bool once_run = true;
	
	while(1)
	{	
		Single_Package_Deal();//¸ºÔğ½ÓËÍ´¦Àíµ¥°üÊı¾İ
		
		switch(BMS_STA)//BMS·¢ËÍ´¦Àí
		{
			case BEGIN://²åÇÀ×¼±¸
			{	//Ç°Ìá£º³äµç»ú×ÔÉí×´Ì¬Õı³£
				//CCĞÅºÅÊÇ·ñ3.2v-4.8v
				//ÉÏËø£¬ÉÏËø·´À¡¾ÍĞ÷£¿
				//¸¨ÖúµçÔ´¼ÌµçÆ÷±ÕºÏK3K4
				//¶¼okµÄ»°->{Ïà¹Ø±êÖ¾Î»Çå0£¬BMS_STA = SEND_9728;}//±¨ÎÄ½ÓÊÕ±êÖ¾Çå0
				if((CC>3)&&(CC<5))
				{
					BMS_Data_Init();
					once_run = true;
					OUT = 0;
					t = timeout = timeout1 = 0;
					BMS_STA = SEND_9728;
				}
			}break;	
			case SEND_9728://³äµçÎÕÊÖ
			{
				t = (t+1)%(CHM_9728.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CHM_9728);}		
				if((RX_BMS_TAB[WAIT_9984_BHM].Rx_status == 1)||(timeout > 20))//ÊÕµ½9984||³¬Ê±5s->GB2015
				{
					//if(k1k2½Ó´¥Æ÷µçÑ¹<10V)
					//if(¾øÔµ¼à²âok£¿)//ĞèÒªµçÑ¹Ä£¿éÊä³öµçÑ¹
					//if(Ğ¹·ÅµçÂ·¼ì²éok£¿)
					t = timeout = 0;
					BMS_STA = SEND_256;
				}
			}break;	
			case SEND_256://³äµç±æÊ¶
			{
				t = (t+1)%(CRM_256.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CRM_256);}
				
				if(RX_BMS_TAB[WAIT_512_BRM].Rx_status == 1)	{timeout = 0;	Data_256.IdentifyResult = 0xAA;}//±æÊ¶³É¹¦	
					else if(timeout > 20)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.IdentifyTimeOut = 0x01;}//½ÓÊÕ³µÁ¾±æÊ¶±¨ÎÄBRM³¬Ê±5s
				
				if(RX_BMS_TAB[WAIT_1536_BCP].Rx_status == 1)	{t = timeout = 0;	BMS_STA = SEND_2048;}
					else if(timeout > 20)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x01;}//½ÓÊÜ1536µç³Ø³äµç²ÎÊı³¬Ê±5s
			}break;
			case SEND_2048://³äµç»ú×î´óÊä³öÄÜÁ¦/Ê±¼äÍ¬²½
			{
				t = (t+1)%(CML_2048.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CTS_1792);	BMS_Send(CML_2048);}
				
				if(RX_BMS_TAB[WAIT_2304_BRO].Rx_status == 1)
				{
					if(Data_2304.BMSSta==0xAA)	{t = timeout = 0;	BMS_STA = SEND_2560;}
						else if(timeout>240)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x04;}//BROÔÚ60sÄÚÎ´×¼±¸ºÃ
				}else if(timeout>20)	{t=timeout=0;	BMS_STA = TIME_OUT;	Data_7936.ChargingParamTimeOut|=0x04;}//½ÓÊÜBROBMS×¼±¸³äµç¾ÍĞ÷³¬Ê±5s	
			}break;
			case SEND_2560://³äµç»úÊä³ö×¼±¸¾ÍĞ÷
			{
				t = (t+1)%(CRO_2560.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;	BMS_Send(CRO_2560);}
				//ÅĞ¶ÏÇ¹µçÑ¹Óë±¨ÎÄµç³ØµçÑ¹Ïà²î5%ÒÔÄÚ£¬ÔÚ³äµç»ú×î´ó×îĞ¡µçÑ¹Ö®¼ä
				//µ÷ÕûµçÔ´Ä£¿éÊä³öµçÑ¹¸ßÓÚµç³ØµçÑ¹1-10vºó£   5.5v ACDC_Start(float vol,float cur);
				//±ÕºÏ¼ÌµçÆ÷K1K2
				if(1)	Data_2560.PrepareOK = 0xAA;
				if((RX_BMS_TAB[WAIT_4352_BCS].Rx_status==1)&&(RX_BMS_TAB[WAIT_4096_BCL].Rx_status==1))	{t=timeout=0;	BMS_STA = SEND_4608;}
					else if(timeout>20)	{t=timeout=0;	BMS_STA=TIME_OUT;	Data_7936.BMSChargingStaTimeOut|=0x05;}//4096µç³Ø³äµçĞèÇó³¬Ê±5s 4352µç³Ø³äµç×Ü×´Ì¬±¨ÎÄ³¬5s(¹²Éú¹²ËÀ)
			}break;
			case SEND_4608:/*³äµç»ú³äµç×´Ì¬(³äµç½×¶Î)*/
			{
				OUT = 0;//Í¨Ñ¶³¬Ê±ÖØÁ¬´ÎÊıÇåÁã£ºµ½³äµç½×¶ÎËµÃ÷ËùÓĞ±¨ÎÄ¶¼ÊÕµ½Ò»±é
				//if(³äµç»ú¹ÊÕÏ){BMS_Send(CST_6656);	t=timeout=0;	BMS_STA = STOP;}
				
				Data_4608.OutputVolt = Data_4096.NeedVolt-10;//Êµ¼ÊÊ¹ÓÃÄÃACDCÄ£¿éÊı¾İ
				Data_4608.OutputCurr = Data_4096.NeedCurr+10;//²âÊÔÓÃ
				Data_4608.ChargingTime++;
				
				t = (t+1)%(CCS_4608.Period/BMS_Task_Time);
				if(t == 1)	{timeout++;timeout1++;	BMS_Send(CCS_4608);}
				
				if((RX_BMS_TAB[WAIT_4096_BCL].Rx_status==0)&&(timeout>4))	{t=timeout=0;	BMS_STA=TIME_OUT;	Data_7936.BMSChargingStaTimeOut|=0x04;}//½ÓÊÜ4096µç³Ø³äµçĞèÇó³¬Ê±1s
				if((RX_BMS_TAB[WAIT_4352_BCS].Rx_status==0)&&(timeout1>20)){t=timeout=0;	BMS_STA=TIME_OUT;	Data_7936.BMSChargingStaTimeOut|=0x01;}//4352µç³Ø³äµç×Ü×´Ì¬±¨ÎÄ³¬5s
				
				if(RX_BMS_TAB[WAIT_4096_BCL].Rx_status==1)	{timeout = 0;	RX_BMS_TAB[WAIT_4096_BCL].Rx_status = 0;}
				if(RX_BMS_TAB[WAIT_4352_BCS].Rx_status==1)	{timeout1= 0;	RX_BMS_TAB[WAIT_4352_BCS].Rx_status = 0;}
				
				if((Data_4864.BatSta==0)&&((Data_4864.BatConnetSta&0x0f)==0))//µç³Ø×´Ì¬Î»¶¼Õı³£
				{
					if(Data_4864.BatConnetSta&0x10)	
						ACDC_Start(Bound(Data_4096.NeedVolt*0.1f,ACDC_MAX_VOL*0.1f,ACDC_MIN_VOL*0.1f),Bound(400-Data_4096.NeedCurr*0.1f,400-ACDC_MAX_CUR*0.1f,0));//ÔÊĞí³äµçbound´«²ÎÊı×¢ÒâBMSµçÑ¹µçÁ÷¸ñÊ½¸úacdcÄ£¿éÊı¾İ¸ñÊ½µÄÇø±ğ
					else	ACDC_Stop();//½ûÖ¹³äµç->³äµçÔİÍ££¬¼ÌĞø½ÓÊÜÅĞ¶Ï4864BSMµç³Ø×´Ì¬ÊÇ·ñÕı³££		
				}else{t = timeout = 0;	BMS_STA = SEND_6656;}//µç³ØÒì³£Í£Ö¹³äµç

				//¼±Í££¿->{t = timeout = 0;	BMS_STA = SEND_6656;}
				if(RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)	{t = timeout = 0;	BMS_STA = STOP;}//ÊÕµ½BMSÖĞÖ¹³äµç
			}break;
			case SEND_6656://³äµç»úÖĞÖ¹³äµç
			{
				BMS_Send(CST_6656);//10msÖÜÆÚ·¢ËÍ
				timeout++;
				if((RX_BMS_TAB[WAIT_6400_BST].Rx_status == 1)||(timeout > 500))//ÊÕµ½BMSÖĞÖ¹³äµçBST||·¢ËÍCTS10*500=5sºó
				{
					ACDC_Stop();//µçÁ¦Êä³öÍ£Ö¹²Ù×÷
					BMS_Send(CSD_7424);//³äµç»úÍ³¼ÆÊı¾İ±¨ÎÄ
					t = timeout = 0;
					BMS_STA = STOP;
				}
			}break;			
//			case SEND_7424://³äµç»úÍ³¼ÆÊı¾İ±¨ÎÄ
//			{
//				t = (t+1)%(CSD_7424.Period/BMS_Task_Time);
//				if(t == 1)	BMS_Send(CSD_7424);
//				if((t>20)||(0))//delay20*10ms(»òÕßµçÁ÷5AÒÔÏÂ)-±£»¤¼ÌµçÆ÷ÊÙÃü
//				{
//					Charge_Close();
//					t = timeout = 0;
//					BMS_STA = BEGIN;
//				}//Ìø¹ı½ÓÊÜ7168BMSÍ³¼ÆÊı¾İ±¨ÎÄ										
//			}break;
			case TIME_OUT://³¬Ê±ÖØÁ¬´¦Àí
			{
				if(OUT < 3)//3´ÎÒÔÄÚ³¬Ê±ÔòÖØÁ¬
				{
					t = (t+1)%(CEM_7936.Period/BMS_Task_Time);
					if(t == 1)	
					{
						BMS_Send(CEM_7936);//·¢ËÍ³äµç»ú´íÎó±¨ÎÄ
						ACDC_Stop();			 //µçÁ¦Êä³öÍ£Ö¹²Ù×÷
					}	
					if((t>20)||(0))//delay20*10ms(»òÕßµçÁ÷5AÒÔÏÂ)-±£»¤¼ÌµçÆ÷ÊÙÃü
					{	//¶Ï¿ªK1K2
						OUT++;//ÖØÁ¬´ÎÊı+1
						BMS_Data_Init();//Çé¿ö½ÓÊÜ±ê¼Ç
						t = timeout = timeout1 = 0;						
						BMS_STA = SEND_256;//ÖØĞÂ¿ªÊ¼±æÊ¶
					}					
				}
				else {t = timeout = 0;	BMS_STA = STOP;}//3´ÎÍ¨Ñ¶³¬Ê±ÖØÁ¬ºóÔÙ³¬Ê±Ôò²»ÔÙÖØÁ¬ Ö±½Ó½áÊø³äµç					
			}break;
			case STOP://½áÊø³äµç²Ù×÷ºóµÈ´ıÔÙ´Î²åÇÀ->BEGIN
			{
				if(t == 0)	ACDC_Stop();//µçÁ¦Êä³öÍ£Ö¹²Ù×÷
				if((t==20)||(0))	{once_run = false;	Charge_Close();}//delay20*10ms(»òÕßµçÁ÷5AÒÔÏÂ)-±£»¤¼ÌµçÆ÷ÊÙÃü	
				if(once_run)	t++;	else t = 100;
				if(CC > 10)	BMS_STA = BEGIN;//±ØĞëÖØĞÂ°ÎÇ¹²åÇÀ²Å¿ÉÒÔÏÂÒ»´Î³äµç
			}break;
			default:break;
		}	
		osDelay(BMS_Task_Time);
	}
}


static void BMS_Data_Init(void)
{
	for(char j = 0; j<10; j++)	RX_BMS_TAB[j].Rx_status = 0;//Çå³ı±¨ÎÄ½ÓÊÜ±êÖ¾
	Data_256.IdentifyResult = 0x00;//±æÊ¶×´Ì¬£ºÎ´±æÊ¶
	Data_2560.PrepareOK = 0x00;		 //×¼±¸×´Ì¬£º×¼±¸Î´Íê³É
	memset(&Data_1792,0,sizeof(Data_1792));
	memset(&Data_4608,0,sizeof(Data_4608));
	memset(&Data_6656,0,sizeof(Data_6656));
	memset(&Data_7424,0,sizeof(Data_7424));
	memset(&Data_7936,0,sizeof(Data_7936));//TX
	memset(&Data_4864,0,sizeof(Data_4864));//RXµç³Ø×´Ì¬±ê¼ÇÇå¿Õ
}


CanTxMsg TxMsg1 = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//À©Õ¹Ö¡ Êı¾İÖ¡
static void BMS_Send(TX_BMS Pbuf)
{
	TxMsg1.ExtId = (Pbuf.IFArbition<<26) | (Pbuf.PGN_S<<16) | (Pbuf.PDUSpecific<<8) | (Pbuf.SourceAddress);
	TxMsg1.DLC = Pbuf.DLC;
	memset(TxMsg1.Data,0,sizeof(TxMsg1.Data));
	memcpy(TxMsg1.Data,Pbuf.Data,Pbuf.DLC);
	CAN_Transmit(CAN1, &TxMsg1);
}

//´¦Àíµ¥°üÊı¾İ
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
unsigned char J1939_Multi_Package[8];//0x10,len_L,len_H,°üÊı,0xff,PGN[3]
CanTxMsg TxMsg_Ack  = {0, 0X1CECF456, CAN_Id_Extended, CAN_RTR_Data, 8, {0x11,0x00,1,0xff,0xff,0x00,0x00,0x00}};
CanTxMsg TxMsg_Done = {0, 0X1CECF456, CAN_Id_Extended, CAN_RTR_Data, 8, {0x13,0x00,0,0x00,0xff,0x00,0x00,0x00}};
//´¦ÀíBMS¶à°ü´«Êä	
void Multi_Package_Deal(void)
{
	if((BMS_RX_0.ExtId == 0X1CEC56F4)&&(BMS_RX_0.Data[0] == 0x10))//BMSÇëÇó½¨Á¢¶à°ü·¢ËÍÁ¬½Ó
	{		
		memcpy(&J1939_Multi_Package[1],&BMS_RX_0.Data[1],7);//±£´æ¶à°ü·¢ËÍÁ¬½ÓµÄÅäÖÃÊı¾İ	
		for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++) \
			if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	P = (unsigned char*)RX_BMS_TAB[i].Data;
		
		TxMsg_Ack.Data[1] = J1939_Multi_Package[3];//¿É·¢ËÍ°üÊı	
		TxMsg_Ack.Data[6] = J1939_Multi_Package[6];//PGN*/							
		CAN_Transmit(CAN1, &TxMsg_Ack);
		if(TxMsg_Ack.Data[6] == 0x11)	CAN_Transmit(CAN1, &TxMsg_Ack);//Ó¦´ğ4352µÃ·¢Á½±é£¡ÕæµÄÓĞ¶¾£¡£¡£¡
	}
	else if(BMS_RX_0.ExtId == 0X1CEB56F4)//¶à°üÊı¾İ´«Êä
	{				
		if(BMS_RX_0.Data[0] == 1)	memcpy(P,&BMS_RX_0.Data[1],7); \
			else if(BMS_RX_0.Data[0] == 2)	memcpy(P+7,&BMS_RX_0.Data[1],7);//ÖØ×é¶à°üÊı¾İ
				
		if(BMS_RX_0.Data[0] == J1939_Multi_Package[3])//¶à°ü½ÓÊÜÍê³É£¿
		{
			for(char i = WAIT_512_BRM; i < (WAIT_4352_BCS+1); i++)
				if(J1939_Multi_Package[6] == RX_BMS_TAB[i].PGN_S)	RX_BMS_TAB[i].Rx_status  = 1;				
			TxMsg_Done.Data[1] = J1939_Multi_Package[1];
			TxMsg_Done.Data[3] = J1939_Multi_Package[3];
			TxMsg_Done.Data[6] = J1939_Multi_Package[6];
			CAN_Transmit(CAN1, &TxMsg_Done);//Ó¦´ğ¶à°ü½ÓÊÜÍê³É
		}
	}
}



static void ACDC_Start(float vol,	float cur)
{

}
static void ACDC_Stop(void)
{
	//1µçÁ¦Êä³öÍ£Ö¹²Ù×÷¹Ø±ÕÄ£¿éÊä³ö£¬ÉèÖÃµçÑ¹µçÁ÷Îª0
}
static void Charge_Close(void)
{
	//3¶Ï¿ªK1K2
	//4Ğ¹·ÅµçÂ·¶¯×÷
	//5¸¨ÖúµçÔ´¼ÌµçÆ÷K3K4¶Ï¿ª
	//6µç×ÓËø½âËø
}

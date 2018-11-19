#include "main.h"
#include "can.h"
#include "bms.h"
#include "lcd.h"

osThreadId MAIN_ID;//ÈÎÎñID
osThreadId	System_Task_ID;
osThreadId	BMS_Task_ID;
osThreadId	ACDC_Module_Task_ID;

//Ïß³Ì¶¨Òå½á¹¹Ìå:ÈÎÎñÃû¡¢ÓÅÏÈ¼¶¡¢È±Ê¡¡¢ÈÎÎñÕ»¿Õ¼ä£¨byte£©
osThreadDef(System_Task, 	osPriorityHigh, 1, 300);
osThreadDef(BMS_Task, 		osPriorityHigh, 1, 600); 
osThreadDef(ACDC_Module_Task, osPriorityNormal, 1, 200);

#define TIMER1_Delay	50U//ms
osTimerId TIMER1_ID;//Èí¶¨Ê±Æ÷ID
osTimerDef (Timer1, Timer1_Callback);//Èí¶¨Ê±Æ÷½á¹¹Ìå£º¶¨Ê±Æ÷Ãû¡¢¶ÔÓ¦µÄ»Øµ÷º¯Êı


unsigned char Board_Type = 0x0A;//Ä¬ÈÏÎªA°å
int main(void)
{
	Bsp_init();						//³õÊ¼»¯°å¼¶Éè±¸	
	osKernelInitialize();	//³õÊ¼»¯RTXÏµÍ³
	System_Task_ID 			= osThreadCreate(osThread(System_Task), NULL);			//´´½¨Ö÷ÈÎÎñ
	BMS_Task_ID					= osThreadCreate(osThread(BMS_Task), NULL);					//´´½¨Æû³µBMSÍ¨Ñ¶ÈÎÎñ
	ACDC_Module_Task_ID	= osThreadCreate(osThread(ACDC_Module_Task), NULL);	//´´½¨½»Á÷-Ö±Á÷µçÔ´Ä£¿éÍ¨Ñ¶ÈÎÎñ
	osKernelStart();    	//¿ªÊ¼ÈÎÎñ´´½¨ÒÔÉÏÈÎÎñ
	
	TIMER1_ID = osTimerCreate(osTimer(Timer1), osTimerPeriodic, NULL);
	osTimerStart(TIMER1_ID, TIMER1_Delay);	//¿ªÆôÈí¶¨Ê±Æ÷TIMER1
	
	MAIN_ID = osThreadGetId();
	osThreadTerminate(MAIN_ID);	//É¾³ıMAINÈÎÎñ
}


uint16_t kkk;
static void Timer1_Callback(void const *arg)
{	
	kkk = (kkk+1)%(500/TIMER1_Delay);
	if(kkk == 1)
	{
		GPIO_PinWrite(LED_BOARD_PORT,LED_RUN_PIN,!GPIO_PinRead(LED_BOARD_PORT,LED_RUN_PIN));//RUN_LED 500s·´×ªÒ»´Î
		IWDG_ReloadCounter();//ÄÚ²¿¿´ÃÅ¹·Î¹¹·£¡
		GPIO_PinWrite(GPIOD,2,0);GPIO_PinWrite(GPIOD,2,1);//Íâ²¿Ó²¼ş¿ªÃÅ¹·Î¹¹·£
	}
	DI_Ack.JT 	 = DI_Status_Check(DI_Filter.JT,GPIO_PinRead(JT_PORT,JT_PIN));
	DI_Ack.START = DI_Status_Check(DI_Filter.START,GPIO_PinRead(START_PORT,START_PIN));
	DI_Ack.GUN   = DI_Status_Check(DI_Filter.GUN,GPIO_PinRead(K_GUN_ACK_PORT,K_GUN_ACK_PIN));
	DI_Ack.LOCK  = DI_Status_Check(DI_Filter.LOCK,GPIO_PinRead(LOCK_ACK_PORT,LOCK_ACK_PIN));
	DI_Ack.KK_H  = DI_Status_Check(DI_Filter.KK_H,GPIO_PinRead(KK_ACK_PORT,KK_ACK_PIN1));
	DI_Ack.KK_L  = DI_Status_Check(DI_Filter.KK_L,GPIO_PinRead(KK_ACK_PORT,KK_ACK_PIN2));
}	


Bms_Type	Type_BMS;
VolCur_Type	Type_VolCur;
Device_Module_Type Type_DM;
Control_Type	Type_Control_Cmd;
CanTxMsg TxMsg_ABC = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//À©Õ¹Ö¡ Êı¾İÖ¡
unsigned char Board_C_Sta = 0;//0:C°å²»´æÔÚ 1:C°åÍ¨Ñ¶Õı³£ 0xFF:C°åÍ¨Ñ¶³¬Ê±
//´¦ÀíAB°åÊı¾İÉÏ±¨µ½C°å
void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 75U;
	static unsigned char t,STEP;
	if(Check_PE())		Type_DM.DErr = Geodesic;//½ÓµØ¹ÊÕÏ
	if(GPIO_PinRead(DIP_SWITCH_PORT1,DIP_SWITCH_PIN1))	Board_Type  = 0X0A;//¶ÁÈ¡²¦Âë¿ª¹ØµØÖ· È·¶¨A B°å
		else	{Board_Type  = 0X0B;	Type_DM.DErr = 0;}//½ÓµØ¼ì²éÓÉA°å¼ì²éB°åÎŞ½ÓµØ¹ÊÕÏ
	if((DI_Ack.GUN&DI_Ack.KK_H&DI_Ack.KK_H)!=0)	Type_DM.DErr = Relay_Err;//¼ÌµçÆ÷ÎŞ·¨¶Ï¿ª
	Type_DM.JiTing = 0;//Ä¬ÈÏ¼±Í£Î´°´ÏÂ
	while(1)
	{
		if(DI_Ack.JT == 0)	Type_DM.JiTing = 1;//¼±Í£°´Å¥°´ÏÂ
			else if(DI_Ack.JT == 1)Type_DM.JiTing = 0;
		
		ABC_Data_Deal(System_Task_Time);//½âÎöC°åÊı¾İ
		switch(STEP)
		{
			case 0://×®×´Ì¬/Ä£¿é×´Ì¬Ö¡
			{
				if(Type_DM.DErr != 0)	Type_DM.DSta = 2;//×®¹ÊÕÏ:Ä£¿é¸öÊı//µç±íÍ¨Ñ¶´íÎó//ABCÊ§Áª
					else if((BMS_STA == BEGIN)||(BMS_STA == STOP))	Type_DM.DSta = 0;//×®´ı»ú
						else Type_DM.DSta = 1;//×®³äµçÁ÷³ÌÖĞ
				memcpy(TxMsg_ABC.Data,&Type_DM,sizeof(Type_DM));
				STEP = 1;
			}break;
			case 1://BMS×´Ì¬Ö¡
			{
				Type_BMS.Step = BMS_STA;
				if(Data_6656.ChargStopChargingReason==0)	Type_BMS.Stop_Reason = Time_Out;//³¬Ê±ÖØÁ¬³¬¹ı3´ÎÍ£Ö¹
					else Type_BMS.Stop_Reason = Data_6656.ChargStopChargingReason;
				if(Data_7936.IdentifyTimeOut == 0x01)	Type_BMS.time_out = BRM512_Timeout;
					else if(Data_7936.ChargingParamTimeOut&0x01) Type_BMS.time_out = BCP1536_Timeout;
						else if(Data_7936.ChargingParamTimeOut&0x04)	Type_BMS.time_out = BRO2304_Timeout;
							else if(Data_7936.BMSChargingStaTimeOut&0x01) Type_BMS.time_out = BCS4352_Timeout;
								else if(Data_7936.BMSChargingStaTimeOut&0x01) Type_BMS.time_out = BCL4096_Timeout;
				Type_BMS.DErr = guzhang;			
				if(Data_6400.BMSFaultReason&0x03)	Type_BMS.BErr = Insulation;//¾øÔµ¹ÊÕÏ
					else if(Data_6400.BMSFaultReason&0x3c)	Type_BMS.BErr = BmsOutNetTemp;//BMSÔª¼ş/Êä³öÁ¬½ÓÆ÷¹ıÎÂ(2ºÏ1)
						else if(Data_6400.BMSFaultReason&0xc0)	Type_BMS.BErr = ChargeNet;//³äµçÁ¬½ÓÆ÷¹ÊÕÏ
							else if(Data_6400.BMSFaultReason&0x0300)	Type_BMS.BErr = BatTemp;//µç³Ø×éÎÂ¶È¹ı¸ß
								else if(Data_6400.BMSFaultReason&0x0c00)	Type_BMS.BErr = HighRelay;//¸ßÑ¹¼ÌµçÆ÷¹ÊÕÏ
									else if(Data_6400.BMSFaultReason&0x3000)	Type_BMS.BErr = Vol_2;////¼ì²éµã2µçÑ¹¼ì²é¹ÊÕÏ
				if(Data_6400.BMSStopChargingReason&0x01)	Type_BMS.BErr = CurOver;//µçÁ÷¹ı´ó
					else if(Data_6400.BMSStopChargingReason&0x02)	Type_BMS.BErr = CurUnknown;//µçÁ÷²»¿ÉĞÅ
						else if(Data_6400.BMSStopChargingReason&0x04)	Type_BMS.BErr = CurOver;//µçÑ¹Òì³£
							else if(Data_6400.BMSStopChargingReason&0x08)	Type_BMS.BErr = VolUnknown;//µçÑ¹²»¿ÉĞÅ
				if((Data_6400.BMSStopChargingReason&0x05)||(Data_4352.PreSOC>=98))	Type_BMS.BErr = Soc_Full;//³äÂúÍ£Ö¹(´ïµ½ËùĞèSOC)	
//				if(Type_DM.JiTing == 1)	Type_BMS.Manual = JT;//3ÖÖÇé¿öÔÚÇé¿ö·¢Éú´úÂë¶Î¸³ÖµÊÇ·ñºÃĞ©£¿
				STEP = 2;
			}break;
			case 2://µçÑ¹µçÁ÷SOCÖ¡
			{
				Type_VolCur.Vol = Data_4608.OutputVolt;//Ä£¿é²âÁ¿
				Type_VolCur.Cur =	Data_4608.OutputCurr;
				Type_VolCur.Soc = Data_4352.PreSOC;		 //³µ·´À¡
				Type_VolCur.KW  =	305;//30.5kw
				STEP = 0;
			}break;
			default:break;
		}
		
		if(Board_C_Sta == 0)//Î´Á¬½ÓC°å²Å×öÏÔÊ¾(¼òÒ×)
		{
			t = (t+1)%(1000/System_Task_Time);//ÏÔÊ¾ÈÎÎñ1sË¢ĞÂÒ»´Î
			//if(t == 1)	LcdShow();		
		}
		else//Ö»ÒªÁ¬½ÓÁËC°å¾ÍÑ­»··¢ËÍ
		{
			CAN_Transmit(CAN2, &TxMsg_ABC);
			memset(TxMsg_ABC.Data,0,sizeof(TxMsg_ABC.Data));
		}
		
		osDelay(System_Task_Time);
	}
}


unsigned char count;
static void ABC_Data_Deal(unsigned short Task_Time)
{
	if(RX_Flag.ABC_Data_Rx_Flag)
	{
		RX_Flag.ABC_Data_Rx_Flag = false;
		if(ABC_DATA_RX.ExtId==0xC0000ABC)//³äµçÆôÍ£/ÔİÍ££¬Ä£¿é·ÖÅä
		{
			Board_C_Sta = 0x01;//ÓëC°åÍ¨Ñ¶Õı³£
			memcpy(&Type_Control_Cmd,ABC_DATA_RX.Data,sizeof(Type_Control_Cmd));
		}
		if(ABC_DATA_RX.ExtId==0xC1000ABC);//TODO:²ÎÊıÉèÖÃ
		count = 0;
	}else count++;
	if((count > (5000/Task_Time))&&(Board_C_Sta==1))	
	{
		Board_C_Sta = 0xFF;//Í¨Ñ¶¹ÊÕÏ£¨5sÄÚÎ´ÊÕµ½C°åÊı¾İ£©
		Type_DM.DErr = Disconnect_C;
	}
}

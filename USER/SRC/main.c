#include "main.h"
#include "can.h"
#include "bms.h"

osThreadId MAIN_ID;//ÈÎÎñID
osThreadId	System_Task_ID;
osThreadId	BMS_Task_ID;
osThreadId	ACDC_Module_Task_ID;

//Ïß³Ì¶¨Òå½á¹¹Ìå:ÈÎÎñÃû¡¢ÓÅÏÈ¼¶¡¢È±Ê¡¡¢ÈÎÎñÕ»¿Õ¼ä£¨byte£©
osThreadDef(System_Task, 	osPriorityHigh, 1, 300);
osThreadDef(BMS_Task, 		osPriorityHigh, 1, 300); 
osThreadDef(ACDC_Module_Task, osPriorityNormal, 1, 300);

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
		GPIO_PinWrite(LED_RUN_PORT,LED_RUN_PIN,!GPIO_PinRead(LED_RUN_PORT,LED_RUN_PIN));//RUN_LED 500s·´×ªÒ»´Î
		IWDG_ReloadCounter();//ÄÚ²¿¿´ÃÅ¹·Î¹¹·£¡
		GPIO_PinWrite(GPIOD,2,0);GPIO_PinWrite(GPIOD,2,1);//Íâ²¿Ó²¼ş¿ªÃÅ¹·Î¹¹·£
	}
}	


Bms_Type	Type_BMS;
VolCur_Type	Type_VolCur;
Device_Module_Type Type_DM;
Control_Type	Type_Control_Cmd;
CanTxMsg TxMsg_ABC = {0, 0, CAN_Id_Extended, CAN_RTR_Data, 8, {0}};//À©Õ¹Ö¡ Êı¾İÖ¡
unsigned char Board_C_Sta;//0:C°å²»´æÔÚ 1:C°åÍ¨Ñ¶Õı³£ 0xFF:C°åÍ¨Ñ¶³¬Ê±
//´¦ÀíAB°åÊı¾İÉÏ±¨µ½C°å
void System_Task(void const *argument)
{
	const unsigned short System_Task_Time = 75U;
	static unsigned char STEP;
	//Board_Type¼ì²é
	//½ÓµØ¼ì²é		{Type_DM.DSta = 2;//×®¹ÊÕÏ Type_DM.DErr = Geodesic;//½ÓµØ¼ì²éÖ»¶ÔA°å}
	//¶Ï¿ªS1S2S3S4
	//¼ì²éS1S2S3S4×´Ì¬ÊÇ·ñÕı³£{Type_DM.DSta = 2;//×®¹ÊÕÏ Type_DM.DErr = Relay_Err;//¼ÌµçÆ÷×´Ì¬²»Õı³£}
	while(1)
	{
		if(0)	Type_DM.JiTing = 1;//¼ì²é¼±Í£ÊÇ·ñ°´ÏÂ
			else Type_DM.JiTing = 0;
		ABC_Data_Deal();//½âÎöC°åÊı¾İ
		
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
				if((Data_6400.BMSStopChargingReason&0x05)!=0)	Type_BMS.BErr = Soc_Full;//³äÂúÍ£Ö¹(´ïµ½ËùĞèSOC)
				if(Data_6400.BMSFaultReason&0x01)	Type_BMS.BErr = Insulation;
				
				STEP = 2;
			}break;
			case 2://µçÑ¹µçÁ÷SOCÖ¡
			{
				
				STEP = 0;
			}break;
			default:break;
		}
		
		if(Board_C_Sta!=0)	
		{
			CAN_Transmit(CAN2, &TxMsg_ABC);//Ö»ÒªÁ¬½ÓÁËC°å¾ÍÑ­»··¢ËÍ
			memset(TxMsg_ABC.Data,0,sizeof(TxMsg_ABC.Data));
		}
		osDelay(System_Task_Time);
	}
}


unsigned char count;
static void ABC_Data_Deal(void)
{
	if(RX_Flag.ABC_Data_Rx_Flag)
	{
		RX_Flag.ABC_Data_Rx_Flag = false;
		if(ABC_DATA_RX.ExtId==0xC0000ABC)//³äµçÆôÍ£/ÔİÍ££¬Ä£¿é·ÖÅä
		{
			Board_C_Sta = 0x01;//ÓëC°åÍ¨Ñ¶Õı³£
		}
		if(ABC_DATA_RX.ExtId==0xC1000ABC)//²ÎÊıÉèÖÃ
		{}
		count = 0;
	}else count++;
	if((count > 50)&&(Board_C_Sta==1))	Board_C_Sta = 0xFF;//Í¨Ñ¶¹ÊÕÏ£¨5sÄÚÎ´ÊÕµ½C°åÊı¾İ£©
	
	if(Board_C_Sta != 0x01)	Type_DM.DErr = Disconnect_C;//Ã»ÓĞÁ¬ÉÏC°å
}

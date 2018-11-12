#ifndef __BSP_H
#define __BSP_H

/*硬件IO口定义*/
#define BMS_RX_PORT		GPIOA
#define BMS_RX_PIN		11
#define BMS_TX_PORT		GPIOA
#define BMS_TX_PIN		12

#define ACDC_RX_PORT	GPIOB
#define ACDC_RX_PIN		12
#define	ACDC_TX_PORT	GPIOB
#define ACDC_TX_PIN		13

#define LED_RUN_PORT	GPIOC
#define LED_RUN_PIN		15


void Bsp_init(void);//板级硬件初始化

#endif

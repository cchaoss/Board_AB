#ifndef __MAIN_H
#define __MAIN_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"                  // Device header
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "GPIO_STM32F10x.h"


extern unsigned char Board_Type;//A B∞Â∂®“Â

static void Timer1_Callback(void const *arg);
void System_Task(void const *argument);
void BMS_Task(void const *argument);                
void ACDC_Module_Task(void const *argument); 

#endif


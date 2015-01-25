#ifndef __STM_CONFIG_H
#define __STM_CONFIG_H

#include "stm32f4xx.h"

void PB_Config(void);
void LED_Config(void);
void GPIO_Config(void);
void RNG_Config(void);
void Timer2_Config_State0_1(void);
void Timer2_Config_State2(void);
void Timer3_Config(void);

int Get32BitRNGValue(void);

#endif

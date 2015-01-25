#include "stm_config.h"
#include "main.h"

void PB_Config(void) {
	STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
}

void ExtBut_Config(void) {
	/*  Create structures for the GPIO buttons  */
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;	
	NVIC_InitTypeDef NVIC_InitStructure;		

	/*  Enable peripheral clocks  */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);	

	/**  GPIO Initialization  **/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);		
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOD, &GPIO_InitStructure);		
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource3);

	/** EXTI External Interrupt Handler Initialization  **/
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;							
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_Init(&EXTI_InitStructure);

	/**  NVIC Interupt Handler Initialization **/
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}

void Timer2_Config(void) {
	/*  Structures for enabling timer components  */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	/*  Enable peripheral clock  */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

	/*  NVIC Interupt Handler Initialization */
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0X00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	/*  Timer basic setup values  */
	TIM_TimeBaseStructure.TIM_Period=65535;
	TIM_TimeBaseStructure.TIM_Prescaler=9000;	// divides the clock down to 10,000 Hz
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/*  Timer interrupt setup values  */
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse=1000;  // interrupt thrown after 0.1 seconds
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);
	TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

	/* TIM3 set to 0 and enable counter */
	TIM_SetCounter(TIM2, 0x0000);
	TIM_Cmd(TIM2, ENABLE);
}


void Timer3_Config(uint32_t CCR1Val) {
	/*  Structures for enabling timer components  */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	/*  Enable peripheral clock  */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

	/*  NVIC Interupt Handler Initialization */
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0X00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	/*  Timer basic setup values  */
	TIM_TimeBaseStructure.TIM_Period=65535;
	TIM_TimeBaseStructure.TIM_Prescaler=45000;	// divides the clock down to 2000 Hz
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/*  Timer interrupt setup values  */
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse=CCR1Val;  // interrupt thrown after 2 seconds
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);
	TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

	/* TIM3 set to 0 and enable counter */
	TIM_SetCounter(TIM3, 0x0000);
}




void RTC_Config(void) {
	RTC_InitTypeDef  RTC_InitStructure;
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;
	
	RTC_AlarmTypeDef RTC_AlarmStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	PWR_BackupAccessCmd(ENABLE);

	RCC_LSICmd(ENABLE);

	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
	
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	RCC_RTCCLKCmd(ENABLE);
	RTC_WaitForSynchro();

	RTC_InitStructure.RTC_AsynchPrediv 	= 0x7F;
	RTC_InitStructure.RTC_SynchPrediv 	= 0xFF;
	RTC_InitStructure.RTC_HourFormat 	= RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);

	//RTC_TimeStructure.RTC_H12    	= RTC_H12_AM;
	RTC_TimeStructure.RTC_Hours 	= 0x08;
	RTC_TimeStructure.RTC_Minutes 	= 0x00;
	RTC_TimeStructure.RTC_Seconds 	= 0x00;
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

	RTC_DateStructure.RTC_Month = RTC_Month_October;
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Monday;
	RTC_DateStructure.RTC_Date = 0x01;
	RTC_DateStructure.RTC_Year = 0x01;
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);


	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0X00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_All;
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);

	RTC_AlarmSubSecondConfig(RTC_Alarm_A, 0xFF, RTC_AlarmSubSecondMask_All); //RTC_AlarmSubSecondMask_SS14_8);

	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
}

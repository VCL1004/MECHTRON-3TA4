#include "stm_config.h"
#include "main.h"

int Get32BitRNGValue(void) {
	uint32_t random32bit;
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET);
	random32bit = RNG_GetRandomNumber();
	return random32bit;
}

void RNG_Config(void){
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
}

void PB_Config(void) {
	STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
}

void LED_Config(void){
 /* Initialize Leds mounted on STM32F429-Discovery board */
	STM_EVAL_LEDInit(LED3);		//Initialize LED3
	STM_EVAL_LEDInit(LED4);

	STM_EVAL_LEDOff(LED3);			//Turn on LED3
	STM_EVAL_LEDOff(LED4);
}


void GPIO_Config(void){
	
	GPIO_InitTypeDef GPIO_InitStructure;		//GPIO initialization structure is created
	EXTI_InitTypeDef EXTI_InitStructure;		//External interupt handler structure is created
	NVIC_InitTypeDef NVIC_InitStructure;		//Interupt controller structure is created


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);		//To use the GPIO pins, we must enable the gpio peripheral clock. Here, GPIOE means we want the clock enabled for GPIO port E
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);	//This enables the SYSCFG clock. This must be enabled to connect gpio pins to the EXTI interupt handler
	
	/**  GPIO Initialization  **/

	/*	This is where we initialize our gpio structure. The defined option names can be found in stm32f4xx_gpio.h.	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;         //Here we set activate pin 2 (this will enable PE2, pin number 2 on port E, because we enabled the GPIOE clock above
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;      //GPIO pins can be inputs or outputs, for this example and input is used
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    //Connect our pin to a pullup resistor, setting the voltage to vcc.

	GPIO_Init(GPIOE, &GPIO_InitStructure);		//Now that the gpio struct has its values set, we send it off the gpio initialization function, as well as what port the pin is on (GPIOE for port E)
																						//Notice the & sign, we send its *memory address*, not the struct itself
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2); //Here we say that PE2 will be connected to the external interrupt handler. Pins can only be connect to the
																	//EXTI line that shares a number with them. E.x. PE2 must be connected to EXTI_Line2, PC4 to EXTI_Line4

	/** EXTI External Interrupt Handler Initialization  **/
	
	/*	This is for the external interupt handler. Info on each line can be found in stm32f4xx_exti.h	*/
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;							//There are multiple lines for handling external interrupts. Becuase we are using PE2, we must use EXTI_Line2
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//Explicitly set the EXTI mode to handling interrupts
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;	//The line is set to trigger on the rising edge of the clock
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;								//Explicitly enable this external line

	EXTI_Init(&EXTI_InitStructure);			//Similar to the gpio, now that the structure is created, we send it the *memory address* to the EXTI initialization function

	/**  NVIC Interupt Handler Initialization **/
	
	/* NVIC is the nested vector interrupt controller. It handles all interupts, determines priority, and then sends them to the processor. */
	/* All interrupts pass through the NVIC. All external interupts are handled with the EXTI handler, and then pass on to the NVIC	*/
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;								//Tell the NVIC to handle interrupts coming from EXTI_Line2
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;		//This is a hex value from 0x00 to 0x0F, with 0 being the highest priorty.
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;						//Each priority also has a sub priority from 0 to F, these prioirites are used to determine the order in which interrupts are handled
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;									//Explicitly enable this channel on the NVIC

	NVIC_Init(&NVIC_InitStructure);		//Send our structures memory address off to the init function to have it initialized
}

void Timer3_Config(void) {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0X00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;

	NVIC_Init(&NVIC_InitStructure);	

	TIM_TimeBaseStructure.TIM_Period=65535; // need to be larger than CCR1_VAL, has no effect on the Output compare event.
	TIM_TimeBaseStructure.TIM_Prescaler=900;		// divides the clock down to 10000 Hz
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;	//Set the clock divider to 1, has no effect on timing
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	// set the timer to count up continuously from 0
	 
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse=10000;
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);

	TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

	/* TIM3 set to 0 and enable counter */
	TIM_SetCounter(TIM3, 0x0000);
	TIM_Cmd(TIM3, ENABLE);

}

void Timer2_Config_State0_1(void) {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0X00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;

	NVIC_Init(&NVIC_InitStructure);	
	
	TIM_TimeBaseStructure.TIM_Period=65535; // need to be larger than CCR1_VAL, has no effect on the Output compare event.
	TIM_TimeBaseStructure.TIM_Prescaler=0;			
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;	//Set the clock divider to 1, has no effect on timing
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	// set the timer to count up continuously from 0
	 
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse=10000;
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);

	TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

	/* TIM3 set to 0 and enable counter */
	TIM_SetCounter(TIM2, 0x0000);
	TIM_Cmd(TIM2, ENABLE);
}

void Timer2_Config_State2(void) {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0X00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;

	NVIC_Init(&NVIC_InitStructure);	
	
	TIM_TimeBaseStructure.TIM_Period=65535; 		// need to be larger than CCR1_VAL, has no effect on the Output compare event.
	TIM_TimeBaseStructure.TIM_Prescaler=45000;		// divides the clock down to 2000 Hz
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;	//Set the clock divider to 1, has no effect on timing
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;	// set the timer to count up continuously from 0
	 
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse=64000;		//timer will time out and end the test after 32 seconds
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);

	TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

	/* TIM3 set to 0 and enable counter */
	TIM_SetCounter(TIM2, 0x0000);
	TIM_Cmd(TIM2, ENABLE);
}

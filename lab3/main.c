

#include "main.h"
#include "stm_config.h"


uint8_t CurrentState = 0;
uint8_t SetDateState = 0;
uint8_t SetValues[7];


uint16_t VirtAddVarTab[NB_OF_VAR] = {0x5555, 0x6666, 0x7777};
__IO uint16_t eepromstate = 0;
uint8_t But0Flag0 = 0;
uint8_t But0Flag1 = 0;
uint8_t But1Flag0 = 0;
uint8_t But1Flag1 = 0;
uint8_t ToggleFlag = 0;

void State0(void);
void State1(void);
void State2(void);
void State3(void);
void StateInit(void);


int main(void) {
	// initialize
	StateInit();

	State0();

	while(1);
}

/*  State 0 handles the RTC ints only, other than that it does nothing. Update time and wait.  */
void State0(void) {
	CurrentState = 0;
	// RTC interrupts will handle displaying the time, nothing else needs to be done
}

/*  State 1 displays the 2 most recently stored values for X seconds and then returns to state 0  */
void State1(void) {
	CurrentState = 1;

	// Prev2Disp();	//implment this to display the last two times

	TIM_SetCounter(TIM3, 0x0000);
	TIM_Cmd(TIM3, ENABLE);
}

/*  State 2 allows the date and time to be set using 2 external buttons, returns to State 0 when done  */
void State2(void) {
	CurrentState = 2;

	// use EXTI 1,2 ints to change date and time


}

/*  One function to run them all  */
void StateInit(void) {
	uint8_t loop;
	for (loop = 0; loop < 7; loop += 1)
		SetValues[loop] = 0;
	ExtBut_Config();
	PB_Config();
	RTC_Config();
	Timer2_Config();
	Timer3_Config(4000);
	LCD_Init();
	LCD_LayerInit();
	LTDC_Cmd(ENABLE);
	LCD_SetLayer(LCD_FOREGROUND_LAYER);
	LCD_Clear(LCD_COLOR_WHITE);
}




void EXTI0_IRQHandler(void){
	if (CurrentState == 0) {
		uint32_t TimeVal;
		char DateString[32];
		Timer3_Config(60000);
		TIM_SetCounter(TIM3, 0x0000);
		TIM_Cmd(TIM3, ENABLE);
		while (STM_EVAL_PBGetState(BUTTON_USER) == Bit_SET){
			TimeVal = TIM_GetCounter(TIM3);
			if ((TimeVal >= 1000) && ((TimeVal % 1000) == 0)){
				LCD_Clear(LCD_COLOR_WHITE);
				sprintf(DateString, "%i/%i/%i  %i:%i:%i", SetValues[1], SetValues[2], SetValues[3], SetValues[4], SetValues[5], SetValues[6]);
				LCD_DisplayStringLine(LINE(0),  (uint8_t *) DateString);
			}
		}
		if (TimeVal < 1000) {
			// Store time in eeprom
		}

		Timer3_Config(4000);
		TIM_Cmd(TIM3, DISABLE);
		State0();
	}

	EXTI_ClearITPendingBit(USER_BUTTON_EXTI_LINE);
}

void EXTI2_IRQHandler(void) {
	char SetString[32];
	if((EXTI_GetITStatus(EXTI_Line2) != RESET) && ((But0Flag0 + But0Flag1) == 0)){
		But0Flag0 = 1;
		But0Flag1 = 1;
		if (CurrentState == 0) {
			// Go into set the date state here
			State2();
		}
		else if (CurrentState == 2) {
			SetDateState += 1;
			LCD_Clear(LCD_COLOR_WHITE);
			sprintf(SetString, "%i", SetDateState);
			LCD_DisplayStringLine(LINE(0),  (uint8_t *) SetString);
			if (SetDateState >= 7) {
				// run a 'set the date from the values' function here
				SetDateState = 0;
				LCD_Clear(LCD_COLOR_WHITE);
				State0();
			}
		}
  	}
	EXTI_ClearITPendingBit(EXTI_Line2);
}

void EXTI3_IRQHandler(void) {
	if ((EXTI_GetITStatus(EXTI_Line3) != RESET) && ((But1Flag0 + But1Flag0) == 0)){
		But1Flag0 = 1;
		But1Flag1 = 1;
		if (CurrentState == 0) {
			// display last 2 values in state 1
			State1();
		}
		else if (CurrentState == 2) {
			SetValues[SetDateState] += 1;
/*
			SetValues[0] %= 8;
			SetValues[1] %= 13;
			SetValues[2] %= 32;
			SetValues[3] %= 50;
			SetValues[4] %= 13;
			SetValues[5] %= 60;
			SetValues[6] %= 60;
*/
		}
  	}
	EXTI_ClearITPendingBit(EXTI_Line3);
}

void TIM2_IRQHandler(void) {
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

		if (ToggleFlag == 0) {
			But0Flag0 = 0;
			But1Flag0 = 0;
		}
		else {
			But0Flag1 = 0;
			But1Flag1 = 0;
		}
		ToggleFlag += 1;
		ToggleFlag %= 2;
		TIM_SetCounter(TIM2, 0x0000);
	}
}

void TIM3_IRQHandler(void) {
	if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);

		TIM_Cmd(TIM3, DISABLE);
		State0();
	}
}

void RTC_Alarm_IRQHandler(void) {
	if((RTC_GetITStatus(RTC_IT_ALRA) != RESET) && (CurrentState == 0)) {

		//If in state 0, display the time. Else, do nothing

		
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		//EXTI_ClearITPendingBit(EXTI_Line17);

	}
}

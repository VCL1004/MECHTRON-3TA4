/* Includes ------------------------------------------------------------------*/
#include "main.h" 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define COLUMN(x) ((x) * (((sFONT *)LCD_GetFont())->Width))    //see font.h, for defining LINE(X)

/*  For the EEPROM  */
uint16_t VirtAddVarTab[NB_OF_VAR] = {0x5555, 0x6666, 0x7777};
__IO uint16_t eepromstate = 0;

/*  3 flags are used for debouncing the external button  */
unsigned char LEDFlagEven, LEDFlagOdd;
unsigned char FlagToggle = 0;

/*  Keeps track of the current state of the program  */
unsigned char StateSwitch = 0;

/*  Flag for disregarding user button presses after time has been displayed and before the reset  */
unsigned char TryTracker = 0;


void StateInit(void);
void State0to1(void);
void State1to2(void);
void State2to0(void);

void LCD_DisplayString(uint16_t LineNumber, uint16_t ColumnNumber, uint8_t *ptr);
void LCD_DisplayInt(uint16_t LineNumber, uint16_t ColumnNumber, int Number);
void LCD_DisplayFloat(uint16_t LineNumber, uint16_t ColumnNumber, float Number, int DigitAfterDecimalPoint);


int main(void){
	StateInit();

	while(1);		//Functions for switching states and ISR's are all that are used in this code, our main while loop is empty
}

/*  StateInit runs all of the basic configuration needed at the beginning, such as initiallizing the LEDs, RNG, etc...  */
void StateInit(void){
	LED_Config();
	PB_Config();
	RNG_Config();
	LCD_Init();
	LCD_LayerInit();
	LTDC_Cmd(ENABLE);
	LCD_SetLayer(LCD_FOREGROUND_LAYER);
	LCD_Clear(LCD_COLOR_WHITE);
	GPIO_Config();
	Timer3_Config();
	FLASH_Unlock();
	EE_Init();
	EE_WriteVariable(VirtAddVarTab[0], 10000);
	State2to0();	//After initialization, the program is put into state 0
}


/*  This function moves the state from state 0 into state 1. State 1 starts at the first user button press and waits a random time before turning on the LEDs (in the ISR)  */
void State0to1(void) {
	uint32_t intHold;
	uint32_t prescaler;
	
	StateSwitch = 1;	//Keep track of our current state

	TIM_Cmd(TIM2, DISABLE);		//Timer 2 is disabled so that an interrupt does not cause the LEDs to turn back on prematurely

	intHold = Get32BitRNGValue();
	prescaler = (int) ((1.5 + ((double)intHold / 4294967295.))*9000); 	/* This gives a random prescaler. The timer has a constant Pulse value of 10,000
												//0xFFFFFFFF?			 * This prescaler will give us the clock division required to hit 10,000 in a random
																		 * interval between 1.5 and 2.5 seconds
																		 */
	STM_EVAL_LEDOff(LED3);	
	STM_EVAL_LEDOff(LED4);	
	Timer2_Config_State0_1();											// Timer 2 is used for multiple tasks. To do this, its functionality changes between states.
	TIM_PrescalerConfig(TIM2, prescaler, TIM_PSCReloadMode_Immediate);	// After the timer has been properly configured, the prescaler is updated to the randomly
	TIM_SetCounter(TIM2, 0x0000);										// generated value, and then the timer is reset
}

/*  Moves from state 1 to state 2. State 2 is starts when the LEDs come on after the random off time, and measures reaction time  */
void State1to2(void) {

	StateSwitch = 2;
	Timer2_Config_State2();		//Timer 2 is set to its second configuration state. This state counts up at 2000Hz and is used to measure reaction time.
								//The timer will time out after 32 seconds and reset the system
}

/*  Sets the system back to state 0. The program will stay hang in state 2 until the external button is pressed, then the GPIO interrupt will run this state transition  */
void State2to0(void) {
	StateSwitch = 0;	//sets the state tracker and reaction time try tracker back to 0, the defaults for state 0
	TryTracker = 0;
	STM_EVAL_LEDOff(LED3);
	STM_EVAL_LEDOff(LED4);
	Timer2_Config_State0_1();										//Set the timer up with the configuration used for states 1 and 2
	TIM_PrescalerConfig(TIM2, 2250, TIM_PSCReloadMode_Immediate);	//and then change its prescaler to 2250
	TIM_SetCounter(TIM2, 0x0000);
}


void TIM2_IRQHandler(void) {
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

		if (StateSwitch == 0) {
			STM_EVAL_LEDToggle(LED3);
			STM_EVAL_LEDToggle(LED4);
		}
		else if (StateSwitch == 1) {
			if (STM_EVAL_PBGetState(BUTTON_USER) == Bit_SET) {
				LCD_Clear(LCD_COLOR_WHITE);		
				LCD_DisplayStringLine(LINE(0),  (uint8_t *) "No cheating!");
				State2to0();
			}
			else {
				STM_EVAL_LEDOn(LED3);
				STM_EVAL_LEDOn(LED4);
				State1to2();
			}
		}
		else {
			LCD_Clear(LCD_COLOR_WHITE);		
			LCD_DisplayStringLine(LINE(0),  (uint8_t *) "Too slow!");
			State2to0();
		}
	}
	TIM_SetCounter(TIM2, 0x0000);
}

void TIM3_IRQHandler(void) {
	if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);

		if (FlagToggle < 1) {LEDFlagEven = 0;}
		else {LEDFlagOdd = 0;}
		
		FlagToggle += 1;
		FlagToggle %= 2;

		TIM_SetCounter(TIM3, 0x0000);
	}
}

void EXTI0_IRQHandler(void){

	if (StateSwitch == 0) {
		STM_EVAL_LEDOff(LED3);
		STM_EVAL_LEDOff(LED4);
		LCD_Clear(LCD_COLOR_WHITE);
		State0to1();
	}
	else if (StateSwitch == 1) {
		LCD_Clear(LCD_COLOR_WHITE);		
		LCD_DisplayStringLine(LINE(0),  (uint8_t *) "No cheating!");
		State2to0();
	}
	else {
		//record time still needs to be implemented
		if (TryTracker == 0) {
			char timeString[32];
			char highScore[32];
			uint16_t eepromdata;
			uint16_t intReactionTime;

			uint32_t TimerVal = TIM_GetCounter(TIM2);
			float ReactionTime = ((float) TimerVal) / 2000.F;

			//Comapre EEPROM value to current, store if current is better
			intReactionTime = (int)(ReactionTime * 1000);
			eepromstate = EE_ReadVariable(VirtAddVarTab[0], &eepromdata);
			if (eepromdata > intReactionTime) {
				eepromstate = EE_WriteVariable(VirtAddVarTab[0], intReactionTime);
				sprintf(highScore, "Best: %0.3f", ReactionTime);
				LCD_DisplayStringLine(LINE(0), (uint8_t *) "New High Score!");
			}
			else {
				sprintf(highScore, "Best: %0.3f", (float)eepromdata/1000.F);
			}
			sprintf(timeString, "%.3f seconds", ReactionTime);
			LCD_DisplayStringLine(LINE(2), (uint8_t *) highScore);
			LCD_DisplayStringLine(LINE(4), (uint8_t *) timeString);
			TIM_Cmd(TIM2, DISABLE);

			TryTracker = 1;
		}
	}
	EXTI_ClearITPendingBit(USER_BUTTON_EXTI_LINE);
}

void EXTI2_IRQHandler(void){
	TIM_Cmd(TIM3, DISABLE);
	if((EXTI_GetITStatus(EXTI_Line2) != RESET) && ((LEDFlagEven + LEDFlagOdd) == 0)) {
		LEDFlagEven = 1;
		LEDFlagOdd = 1;

		State2to0();
  	}
	EXTI_ClearITPendingBit(EXTI_Line2);
	TIM_Cmd(TIM3, ENABLE);
}

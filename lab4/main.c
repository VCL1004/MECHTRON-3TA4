#include "main.h"

//the following two addresses are useful when using the ADC and DAC in DMA mode

#define ADC_CDR_ADDR 				((uint32_t)(ADC_BASE + 0x08)) //ADC_CDR address as described in section 10.13.17 of reference manual
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MESSAGE1   "ADC conversion w/DMA"
#define MESSAGE1_1 "continuouslyTransfer"
#define MESSAGE2   " ADC Ch13 Conv   "
#define MESSAGE2_1 "    2.4Msps      "
#define MESSAGE5   " Temp = %d C "
#define LINENUM            0x15
#define FONTSIZE         Font12x12
#define ADC3_DR_ADDRESS     ((uint32_t)0x4001224C)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* You can monitor the converted value by adding the variable "uhADC3ConvertedValue"
   to the debugger watch window */
__IO uint16_t uhADC3ConvertedValue = 0;
__IO uint32_t uwADC3ConvertedVoltage = 0;

/**  Flags  **/
unsigned char EXTPBE2 = 0;
unsigned char EXTPBE3 = 0;
unsigned char PBE2F1 = 0;
unsigned char PBE2F2 = 0;
unsigned char PBE3F1 = 0;
unsigned char PBE3F2 = 0;
unsigned char debounceFlag = 0;
unsigned char fanMode = 0;

/**  User Variabls  **/
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t CCR1_Val = 1;
uint32_t currTemp = 0;
uint32_t modeTemp = 0;

//ADC CDR is the common regular (not injected) data register for dual and triple modes
//also defined in stm32f4xx_adc.c as
//#define CDR_ADDRESS 			((uint32_t)0x40012308)
//which results in the same address

void Display(void);
void Display_Init(void);
void ADC_Config(void);
void PWM_Config(void);
void GPIO_PB_Config(void);
void Timer3_Config(void);
void LED_Config(void);
void fanOn(void);
void fanOff(void);

void tempConv(void);
void readDelay(void);

int main(void){
    /**  PushButton Config  **/
    STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);

    /**  Init and Config functions for peripherals  **/
    Display_Init();
    ADC_Config();
    PWM_Config();
    GPIO_PB_Config();
    Timer3_Config();
    LED_Config();

    /**  Start ADC conversion  **/
    ADC_SoftwareStartConv(ADC3);

    readDelay();    // sensor takes time to report accurate values, delays until values are close to normalized
    tempConv();
    modeTemp = currTemp + 3;

    while(1) {
        /**  Convert 12 bit value to an int from 0 to 5000, millivolts when powered off 5V rail  **/
        tempConv();
        Display();

        if (currTemp > modeTemp) {      // fan must go 1 degree over modeTemp to turn on, and 1 below to turn off to avoid power oscillations
            fanOn();
        }
        else if  (currTemp < modeTemp)
            fanOff();
    }
}


void fanOn(void) {
    uint16_t pulseVal = ((currTemp - modeTemp) * 50) + 200;
    if (pulseVal > 699)
        pulseVal = 699;
    TIM_OCInitStructure.TIM_Pulse = pulseVal;

    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
}

void fanOff(void) {
    TIM_OCInitStructure.TIM_Pulse = 0;

    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
}

void readDelay(void) {
    int i;
    for (i = 10000; i > 0; i-= 1);
}

/**  converts the voltage values to temp approximations  **/
void tempConv(void) {
    uwADC3ConvertedVoltage = uhADC3ConvertedValue * 3000 / 0xFFF;
    // currTemp = (uwADC3ConvertedVoltage % 3000) / 30;
    currTemp = uwADC3ConvertedVoltage / 30;
}

/**  Display all relevent information on the screen  **/
void Display(void) {
    uint8_t aTextBuffer[50];
    uint8_t bTextBuffer[50];

    sprintf((char *)aTextBuffer, MESSAGE5, currTemp);
    sprintf((char *)bTextBuffer, "modeTemp = %d C", modeTemp);
    LCD_DisplayStringLine(LCD_LINE_6, (uint8_t *)aTextBuffer);
    LCD_DisplayStringLine(LCD_LINE_8, (uint8_t *)bTextBuffer);
}


void Display_Init(void) {
    /* Initialize the LCD */
    LCD_Init();
    LCD_LayerInit();
    /* Enable the LTDC */
    LTDC_Cmd(ENABLE);

    /* Set LCD Background Layer  */
    LCD_SetLayer(LCD_BACKGROUND_LAYER);

    /* Clear the Background Layer */
    LCD_Clear(LCD_COLOR_WHITE);

    /* Configure the transparency for background */
    LCD_SetTransparency(0);

    /* Set LCD Foreground Layer  */
    LCD_SetLayer(LCD_FOREGROUND_LAYER);

    /* Configure the transparency for foreground */
    LCD_SetTransparency(200);

    /* Clear the Foreground Layer */
    LCD_Clear(LCD_COLOR_WHITE);

    /* Set the LCD Back Color and Text Color*/
    LCD_SetBackColor(LCD_COLOR_BLUE);
    LCD_SetTextColor(LCD_COLOR_WHITE);

    /* Set the LCD Text size */
    LCD_SetFont(&FONTSIZE);

    /* Set the LCD Back Color and Text Color*/
    LCD_SetBackColor(LCD_COLOR_BLUE);
    LCD_SetTextColor(LCD_COLOR_WHITE);

    LCD_DisplayStringLine(LINE(LINENUM), (uint8_t*)MESSAGE1);
    LCD_DisplayStringLine(LINE(LINENUM + 1), (uint8_t*)MESSAGE1_1);
    LCD_DisplayStringLine(LINE(0x17), (uint8_t*)"                               ");

    /* Set the LCD Text size */
    LCD_SetFont(&Font16x24);

    LCD_DisplayStringLine(LCD_LINE_0, (uint8_t*)MESSAGE2);
    LCD_DisplayStringLine(LCD_LINE_1, (uint8_t*)MESSAGE2_1);

    /* Set the LCD Back Color and Text Color*/
    LCD_SetBackColor(LCD_COLOR_WHITE);
    LCD_SetTextColor(LCD_COLOR_BLUE);
}


void ADC_Config(void){
    ADC_InitTypeDef       ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    DMA_InitTypeDef       DMA_InitStructure;
    GPIO_InitTypeDef      GPIO_InitStructure;

    /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

    /* DMA2 Stream0 channel2 configuration **************************************/
    DMA_InitStructure.DMA_Channel = DMA_Channel_2;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC3_DR_ADDRESS;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&uhADC3ConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);
    DMA_Cmd(DMA2_Stream0, ENABLE);

    /* Configure ADC3 Channel13 pin as analog input ******************************/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* ADC Common Init **********************************************************/
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    /* ADC3 Init ****************************************************************/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC3, &ADC_InitStructure);

    /* ADC3 regular channel13 configuration *************************************/
    ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 1, ADC_SampleTime_3Cycles);

    /* Enable DMA request after last transfer (Single-ADC mode) */
    ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

    /* Enable ADC3 DMA */
    ADC_DMACmd(ADC3, ENABLE);

    /* Enable ADC3 */
    ADC_Cmd(ADC3, ENABLE);
}

void PWM_Config(void){
    uint16_t PrescalerValue = (uint16_t) (SystemCoreClock / 21000000) - 1;
    /******************************/
    /**  PWM TIM4 & GPIO Config  **/
    /******************************/
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    /* TIM4 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* GPIOD clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    /*  GPIOD Configuration: TIM4 CH1 (PD12)  */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Connect TIM4 pin to AF2 */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 699;
    TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM4, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM4, ENABLE);

    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);
}

/**  Initializes the GPIO pins for the external buttons  **/
void GPIO_PB_Config(void) {
    /**  Create Init Structures  **/
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /**  Clock Enables  **/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /**  GPIO Initialization  **/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource3);

    /**  EXTI External Interrupt Handler Initialization  **/
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
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


/**  Timer 3 is for the GPIO debounce  **/
void Timer3_Config(void) {
    /**  Create init structures  **/
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /**  Enable peripheral clock  **/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

    /**  Init the interrupt controller values  **/
    NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0X00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /**  Timer basic config values  **/
    TIM_TimeBaseStructure.TIM_Period=65535;
    TIM_TimeBaseStructure.TIM_Prescaler=9000;
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /**  Configure output compare for timer  **/
    TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse=1000;
    TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

    /**  Set timer to 0 and enable  **/
    TIM_SetCounter(TIM3, 0x0000);
    TIM_Cmd(TIM3, ENABLE);
}


void LED_Config(void) {
    STM_EVAL_LEDInit(LED3);
    STM_EVAL_LEDInit(LED4);

    STM_EVAL_LEDOff(LED3);
    STM_EVAL_LEDOff(LED4);
}


/**  Interrupt Handlers  **/

/**  Handler for PB on PE2  **/
void EXTI2_IRQHandler(void){

    if ((EXTI_GetITStatus(EXTI_Line2) != RESET) && (PBE2F1 + PBE2F2 == 0)) {    // PBE2F1 & 2 are debounce flags for push buttons
        PBE2F1 = 1; PBE2F2 = 1;     // set debounce flags high

        modeTemp += 1;
    }

    EXTI_ClearITPendingBit(EXTI_Line2);
}

/**  Handler for PB on PE3  **/
void EXTI3_IRQHandler(void){

    if ((EXTI_GetITStatus(EXTI_Line3) != RESET) && (PBE3F1 + PBE3F2 == 0)) {    // PBE3F1 & 2 are debounce flags for push buttons
        PBE3F1 = 1; PBE3F2 = 1;     // set debounce flags high
        modeTemp -= 1;
        // STM_EVAL_LEDToggle(LED4);    // leftover debug code
    }

    EXTI_ClearITPendingBit(EXTI_Line3);
}

/**  Timer IRQ for debounce  **/
void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);

        if (debounceFlag == 0) {
            PBE2F1 = 0;
            PBE3F1 = 0;
        }
        else {
             PBE2F2 = 0;
             PBE3F2 = 0;
        }

        debounceFlag += 1;
        debounceFlag %= 2;

        TIM_SetCounter(TIM3, 0x0000);
    }
}

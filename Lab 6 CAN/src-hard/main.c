/**
  ******************************************************************************
  * @file    CAN_lab/main.c
  * @author  modified by Keybo Q.
  * @version 
  * @date    Jan 2015
  * @brief   
	* @projectfunction  
  
  ******************************************************************************
 * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h" 


__IO uint8_t UBPressed = 0;
__IO uint8_t getmessage = 0;


/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
/* Private define ------------------------------------------------------------*/
#define COLUMN(x) ((x) * (((sFONT *)LCD_GetFont())->Width))    //see font.h, for defining LINE(X)


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

volatile TestStatus TestRx;


/* Private function prototypes -----------------------------------------------*/
void PB_Config(void);
void LED_Config(void);
//void CAN_Config(void); /*I separate it into CAN_Polling and GPIO_Config*/
void GPIO_Config(void);
void TIM_Config(void);
void NVIC_Config(void);
void LED_Display(uint8_t num);


TestStatus CAN_Polling(void);
//TestStatus CAN_Interrupt(void);
//char display[10];

CanTxMsg TxMessage;
CanRxMsg RxMessage;


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */
	
	//uint32_t i = 0;
  uint8_t TransmitMailbox = 0;
	
//initiate user button
  PB_Config();

	//initiate LEDs and turn them on
  LED_Config();	
	
	GPIO_Config();
	/*Split into CAN_Polling and GPIO_Config*/
	//CAN_Config(); /*********** you need to implement this one yourself ********/
	
	
	TIM_Config();
	
	NVIC_Config();

	
////======================================configure and init LCD  ======================	
////	 /* LCD initiatization */
//  LCD_Init();
////  
////  /* LCD Layer initiatization */
//  LCD_LayerInit();
////    
////  /* Enable the LTDC */
//  LTDC_Cmd(ENABLE);
////  
////  /* Set LCD foreground layer */
//  LCD_SetLayer(LCD_FOREGROUND_LAYER);
////	
//	LCD_Clear(LCD_COLOR_WHITE);
	
	
//main program	

TestRx = CAN_Polling();

TransmitMailbox = CAN_Transmit(CANx, &TxMessage);
//i = 0;
  while((CAN_TransmitStatus(CANx, TransmitMailbox)  !=  CANTXOK)) //&& (i  !=  0xFFFF))
  {
    
		//STM_EVAL_LEDOn(LED4);
	
  }
	//STM_EVAL_LEDOn(LED3);
  while (1){ 
		
		//TransmitMailbox = CAN_Transmit(CANx, &TxMessage);
	
//  i = 0;
//  while((CAN_TransmitStatus(CANx, TransmitMailbox)  !=  CANTXOK) && (i  !=  0xFFFF))
//  {
//    i++;
//		STM_EVAL_LEDOn(LED3);
//	
//  }
//		
		
//		sprintf(display,"%d",111);
//		LCD_DisplayStringLine(LINE(0),(uint8_t *)display);
		
//		uint32_t n = 0; 
//		////STM_EVAL_LEDOn(LED3);
//			if (TestRx !=  FAILED && getmessage == 1)
//			{ /* OK */
//				CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
//				if ((RxMessage.StdId == 0x026)&&(RxMessage.IDE == CAN_ID_STD) && (RxMessage.DLC == 2))
//				{
//					LED_Display(RxMessage.Data[1]);
//					getmessage = 0;
//				} 
//				
//				while((CAN_MessagePending(CANx, CAN_FIFO0) < 1) && (n  !=  0xFFFF))
//				{
//					n++;
//				}
//			//sprintf(display,"%d",RxMessage.Data[1]);
//			//LCD_DisplayStringLine(LINE(0),(uint8_t *)display);
//				
//			}		
			
	}

}

/**
  * @brief  Configure the TIM IRQ Handler.
  * @param  No
  * @retval None
  */


void PB_Config(void)
{
/* Initialize User_Button on STM32F4-Discovery
   * Normally one would need to initialize the EXTI interrupt
   * to handle the 'User' button, however the function already
   * does this.
   */
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
}

void LED_Config(void)
{
 /* Initialize Leds mounted on STM32F429-Discovery board */
  STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED4); 

  /* Turn on LED3, LED4 */
  //STM_EVAL_LEDOn(LED3);
	//STM_EVAL_LEDOn(LED4);
}

void GPIO_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* CAN GPIOs configuration **************************************************/

  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(CAN_GPIO_CLK, ENABLE);

  /* Connect CAN pins to AF9 */
  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_RX_SOURCE, CAN_AF_PORT);
  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_TX_SOURCE, CAN_AF_PORT); 
  
  /* Configure CAN RX and TX pins */
	
  GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN  | CAN_TX_PIN ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //set to alternate function
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; 
  GPIO_Init(CAN_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  Configures the CAN.
  * @param  None
  * @retval None
  */
TestStatus CAN_Polling(void)
{
	CAN_InitTypeDef        CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	
//  uint32_t i = 0;
//  uint8_t TransmitMailbox = 0;
	
//  GPIO_InitTypeDef  GPIO_InitStructure;
//  
//  /* CAN GPIOs configuration **************************************************/

//  /* Enable GPIO clock */
//  RCC_AHB1PeriphClockCmd(CAN_GPIO_CLK, ENABLE);

//  /* Connect CAN pins to AF9 */
//  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_RX_SOURCE, CAN_AF_PORT);
//  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_TX_SOURCE, CAN_AF_PORT); 
//  
//  /* Configure CAN RX and TX pins */
//	
//  GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN  | CAN_TX_PIN ;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //set to alternate function
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; 
//  GPIO_Init(CAN_GPIO_PORT, &GPIO_InitStructure);

  /* CAN configuration ********************************************************/  
 
	/* Enable CAN clock */
  RCC_APB1PeriphClockCmd(CAN_CLK, ENABLE);
  
	
	
  /* CAN register init */
  CAN_DeInit(CANx);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;
  CAN_InitStructure.CAN_ABOM = DISABLE;
  CAN_InitStructure.CAN_AWUM = DISABLE;
  CAN_InitStructure.CAN_NART = DISABLE;
  CAN_InitStructure.CAN_RFLM = DISABLE;
  CAN_InitStructure.CAN_TXFP = DISABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    
  /* CAN Baudrate = 800 KBps (CAN clocked at 45 MHz) */
  CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
  CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
  CAN_InitStructure.CAN_Prescaler = 7; //atmost 1024  45M/LP = 800K
  CAN_Init(CANx, &CAN_InitStructure);

  /* CAN filter init */
#ifdef  USE_CAN1
  CAN_FilterInitStructure.CAN_FilterNumber = 0;
#else /* USE_CAN2 */
  CAN_FilterInitStructure.CAN_FilterNumber = 14;
#endif  /* USE_CAN1 */


  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x1111;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x1111;  
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	
  CAN_FilterInit(&CAN_FilterInitStructure);

 //CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
  /* Transmit Structure preparation */
	TxMessage.StdId = 0x07;
	//TxMessage.ExtId = 0x00;
  TxMessage.RTR = CAN_RTR_DATA;
  TxMessage.IDE = CAN_ID_STD;
  TxMessage.DLC = 1;
  TxMessage.Data[0] = 0x07;
  
	/* Enable FIFO 0 message pending Interrupt */
  CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
	
	
//	TransmitMailbox = CAN_Transmit(CANx, &TxMessage);
//	

//  i = 0;
//  while((CAN_TransmitStatus(CANx, TransmitMailbox)  !=  CANTXOK) && (i  !=  0xFFFF))
//  {
//    i++;
//  }
	// success ? 

//  i = 0;
//  while((CAN_MessagePending(CANx, CAN_FIFO0) < 1) && (i  !=  0xFFFF))
//  {
//    i++;
//  }
// receive ?
	
 

//	/* receive */
//  RxMessage.StdId = 0x00;
//  RxMessage.IDE = CAN_ID_STD;
//  RxMessage.DLC = 0;
//  RxMessage.Data[0] = 0x00;
//  RxMessage.Data[1] = 0x00;
	
  return PASSED; /* Test Passed */
	

}

void TIM_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStructure;
  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	
	TIM_Cmd(TIM6, DISABLE);
	TIM_Cmd(TIM7, DISABLE);
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);	
	TIM_TimeBaseInitStructure.TIM_Prescaler = 45000;
	TIM_TimeBaseInitStructure.TIM_Period = 0x0000;
	
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStructure);
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStructure);
	
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
}

/**
  * @brief  Configures the NVIC for CAN.
  * @param  None
  * @retval None
  */
void NVIC_Config(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;

#ifdef  USE_CAN1 
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
#else  /* USE_CAN2 */
  NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
#endif /* USE_CAN1 */

  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  Initializes the Rx Message.
  * @param  RxMessage: pointer to the message to initialize
  * @retval None
  */
void Init_RxMes(CanRxMsg *RxMessage)
{
  uint8_t i = 0;

  RxMessage->StdId = 0x00;
  RxMessage->ExtId = 0x00;
  RxMessage->IDE = CAN_ID_STD;
  RxMessage->DLC = 0;
  RxMessage->FMI = 0;
  for (i = 0;i < 8;i++)
  {
    RxMessage->Data[i] = 0x00;
  }
}

void LED_Display(uint8_t num)
{
	if ((num & 0x1) == 0x1)
	{
		TIM_SetAutoreload(TIM7, 499);
	}
	else
	{
		TIM_SetAutoreload(TIM7, 124);
	}
	
	TIM_Cmd(TIM7, DISABLE);
	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	if ((num & 0x3)==0x0)
	{
		STM_EVAL_LEDOff(LED4);
	}
	else if	((num & 0x3)==0x3)
	{
		STM_EVAL_LEDOn(LED4);
	}
	else
	{
		TIM_Cmd(TIM7, ENABLE);
	}
	
	if (((num>>2) & 0x01) == 0x1)
	{
		TIM_SetAutoreload(TIM6, 499);
	}
	else
	{
		TIM_SetAutoreload(TIM6, 124);
	}
	
	TIM_Cmd(TIM6, DISABLE);
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	if (((num>>2) & 0x3)==0x0)
	{
		STM_EVAL_LEDOff(LED3);
	}
	else if	(((num>>2) & 0x3)==0x3)
	{
		STM_EVAL_LEDOn(LED3);
	}
	else
	{
		TIM_Cmd(TIM6, ENABLE);
	}
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  while (1)
  {}
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

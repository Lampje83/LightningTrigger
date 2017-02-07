/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */

#include "sh1106.h"
#include "ui.h"
#include "functions.h"
#include "input.h"
#include "triggers.h"
#include "eeprom.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

typedef enum
{
	WELCOME,
	SHOWMODE,
	MENU,
	LIGHTNING_TRIGGER,
	TIMELAPSE,
	EXTERNAL_TRIGGER,
	REMOTE_CONTROL,
	SHOWVOLTAGES,
	SHOWSCOPE,
	SHUTTERDELAY_COUNTER,
	SET_BRIGHTNESS,
	SHOWCLOCK,
} run_state;

volatile uint16_t ADC_Done = 0;		// Aantal conversies in huidige frame
volatile uint16_t ADC_Count = 0;	// Aantal conversies vorige frame

volatile float voltages[8];
volatile float ldr_voltage;

uint16_t ADC_Samples[NUM_SAMPLES];
uint8_t scopedata[SCOPESAMPLES];

volatile uint8_t Dirty = 0;
uint16_t framecount;

// run_state RunState = WELCOME;
uint32_t nextactiontick;
volatile uint32_t lastinputtick;
uint32_t triggertick = 0;
uint32_t untrigtick = 0;
volatile int8_t enccount;
extern uint8_t brightness;
extern uint8_t reversecontact;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);

/* USER CODE BEGIN PFP */

void LoadSettings (void);
void SaveSettings (void);

static void MX_ADC1_Init(void);
static void MX_RTC_Init(void);
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

// inputs testen
switch_state Test_Input (uint8_t value, volatile switch_t *input)
{
	if (value)
	{
		input->locount = 0;
		input->hicount++;
	}
	else
	{
		input->hicount = 0;
		input->locount++;
	}

	if (input->locount >= SWITCH_DEBOUNCE)
	{
		input->state = SW_OFF;
		if (input->locount > SWITCH_VERYLONGPRESS)
			input->locount = SWITCH_DEBOUNCE;   // niet door nul laten gaan

		if (input->locount == SWITCH_DEBOUNCE)
			// dalende flank
			return SW_FALLING;
	}
	else if (input->hicount >= SWITCH_DEBOUNCE)
	{
		if (input->hicount >= SWITCH_LONGPRESS)
		{
			if (input->hicount >= SWITCH_VERYLONGPRESS)
			{
				input->state = SW_VERYLONG_PRESS;
				input->hicount = SWITCH_VERYLONGPRESS;// zorgen dat teller niet overflowt en daardoor naar nul gaat
				return SW_VERYLONG_PRESS;
			}
			else
			{
				input->state = SW_LONG_PRESS;
				return SW_LONG_PRESS;
			}
		}
		else
		{
			input->state = SW_ON;

			if (input->hicount == SWITCH_DEBOUNCE)
				// stijgende flank
				return SW_RISING;
		}
	}

	// niets veranderd, geef huidige status door
	return input->state;
}

// controleer op wijzing schakelaarstatus, maar werk status niet bij
switch_state Input_PeekEvent (volatile switch_t *input)
{
	if (input->state != input->prevstate)
		return input->state;
	else
		return SW_NONE;
}

// controleer op wijziging schakelaarstatus en werk status bij
switch_state Input_GetEvent (volatile switch_t *input)
{
	switch_state value = Input_PeekEvent(input);
	input->prevstate = input->state;
	return value;
}

void EnterDeepSleep (void)
{
	uint32_t i;
	RTC_DateTypeDef date;		// datum

	if (SH1106_HSPI != NULL)
		// wacht totdat er geen data meer naar het scherm gaat
		while (SH1106_HSPI->hdmatx->State == HAL_DMA_STATE_BUSY)
			;

	SH1106_TurnOff();

	SaveSettings ();

	while (Test_Input (HAL_GPIO_ReadPin (ENC_SEL_GPIO_Port, ENC_SEL_Pin),
					&ENCSELsw) != SW_OFF)
	;
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU | PWR_FLAG_SB);
	// Even wachten zodat de controller niet meteen weer ingeschakeld wordt
	for (i = 0; i < 500000; i++)
	{
		// doe iets, zorg dat de compiler deze lus niet wegoptimaliseet
		ENCAsw.state = ENCBsw.state;
	}

	HAL_RTC_GetDate (&hrtc, &date, RTC_FORMAT_BIN);

	// Datum opslaan in backup register
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1,
												(date.Date & 31) |
												((date.Month & 15) << 5) |
												((date.Year & 127) << 9));

	HAL_PWR_EnableWakeUpPin (PWR_WAKEUP_PIN1);
	HAL_PWR_EnterSTANDBYMode ();
	//HAL_PWR_EnterSTOPMode (PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);
}

void LT_SetNewHandler (void* handler)
{
	if (LT_FuncHandlerExit != NULL)
	{
		LT_FuncHandlerExit ();
		// Exit handler wissen
		LT_FuncHandlerExit = NULL;
	}
	LT_FuncHandler = handler;
}

void LT_ShowVoltages (void)
{
	SH1106_Clear ();
	LT_SetNewHandler (&func_showvoltages);

}

void LT_ShowScope (void)
{
	LT_SetNewHandler (&func_showscope);
}

void LT_ShowClock (void)
{
	LT_SetNewHandler (&func_showclock);

	Dirty = 1; // zorg dat scherm getekend wordt
	HAL_RTCEx_SetSecond_IT(&hrtc);	// RTC interrupt inschakelen

//	HAL_PWR_DisableSleepOnExit ();
//	HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI); // Stroom besparen
}

void LT_SetBrightness (void)
{
	LT_SetNewHandler (&func_setbrightness);

	Dirty = 1;
}

void MX_RTC_Init_Without_Time (void)
{
	RTC_DateTypeDef date;
	uint16_t				date_backup = 0;

	/**Initialize RTC Only
	*/
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
	Error_Handler();
  }

  // RTC uitlezen. Als er een dag voorbij gegaan is sinds laatste update, staat deze niet op 1 januari '00

  HAL_RTC_GetDate (&hrtc, &date, RTC_FORMAT_BIN);

  // Controleer of er iets in het backupgeheugen staat
  if (EE_ReadVariable (0x1005, &date_backup))
		date_backup = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);

  if (date_backup != 0)
		{
			// Opgeslagen datum bij klok optellen
			date.Date += (date_backup & 31) - 1;
			date.Month += ((date_backup >> 5) & 15) - 1;
			date.Year += (date_backup >> 9);

			HAL_RTC_SetDate (&hrtc, &date, RTC_FORMAT_BIN);
		}
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
	uint8_t i = 0;

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */

	HAL_Delay (100);
	// HAL_TIM_Base_Start (&htim3);

	if (__HAL_PWR_GET_FLAG(PWR_FLAG_WU))	// wakeup, controleer op lange druk
	{
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU | PWR_FLAG_SB);

		Test_Input (HAL_GPIO_ReadPin (ENC_SEL_GPIO_Port, ENC_SEL_Pin), &ENCSELsw);

		while (ENCSELsw.state == SW_ON)
			;

		if (ENCSELsw.state != SW_LONG_PRESS)
		{
			EnterDeepSleep ();
		}
	}

	// EEPROM emulatie initializeren
	EE_Init ();

	LoadSettings ();

	// Geen tijd instellen, RTC blijft doorlopen tijdens standby
	MX_RTC_Init_Without_Time ();

	SH1106_Init (&hspi1);

	// overige periferie initializeren
	MX_ADC1_Init ();
	HAL_ADC_Start (&hadc1);		// ADC alvast starten zodat calibratie nauwkeurig verloopt

	MX_USB_DEVICE_Init ();

	SH1106_SetBrightness (0);

	HAL_PWR_DisableWakeUpPin (PWR_WAKEUP_PIN1);

	SH1106_TurnOn ();

	LT_ShowStartScreen ();
	SH1106_PaintScreen ();
	HAL_Delay (10);

	HAL_ADCEx_Calibration_Start (&hadc1);

	// fade in
	for (i = 0; i < 32; i++)
	{
		SH1106_SetBrightness ((uint8_t)((i * i) >> 2) * brightness / 256);
		HAL_Delay (10);
	}

	HAL_ADC_Start_DMA (&hadc1, ADC_Samples, NUM_SAMPLES);

	HAL_Delay (1800);
	UI_ShowMenu (&LT_MainMenu);
	LT_SetNewHandler (&func_menu);

	lastinputtick = hrtc.Instance->CNTL;
	HAL_RTCEx_SetSecond_IT(&hrtc);	// RTC interrupt inschakelen

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	i = 0;

	while (1)
	{
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

		LT_FuncHandler ();

		if (enccount != 0)	// draaiknop afhandelen
		{
			if (LT_EncTurnCallback)
			{
				LT_EncTurnCallback (enccount);
				enccount = 0;
			}
		}

		if (Input_PeekEvent (&ENCSELsw))
			if (LT_EncPressCallback)	// drukknop afhandelen
				LT_EncPressCallback (Input_GetEvent (&ENCSELsw));

		// Alleen als we niet al in een menu zitten, brengt een lange druk ons naar het menu
		if (LT_FuncHandler != func_menu)
		{
			if (ENCSELsw.state == SW_LONG_PRESS)
			{
				enccount = 0;
				UI_ShowMenu (&LT_MainMenu);
				LT_SetNewHandler (&func_menu);
				Dirty = 1;
			}
		}

		if (Dirty)
		{
			// memset (disp_buffer + 132*4, 0, 132*4);	// buffer wissen
			SH1106_PaintScreen ();
			Dirty = 0;
		}

	}
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC
                              |RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 3;
  sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* RTC init function */
static void MX_RTC_Init(void)
{

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef DateToUpdate;

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initialize RTC and set the Time and Date 
    */
  sTime.Hours = 0x12;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 0x1;
  DateToUpdate.Year = 0x17;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DISP_DC_GPIO_Port, DISP_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DISP_RES_Pin|CAM_B_Pin|CAM_A_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ENC_SEL_Pin */
  GPIO_InitStruct.Pin = ENC_SEL_Pin | EXT_TRIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(ENC_SEL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DISP_DC_Pin */
  GPIO_InitStruct.Pin = DISP_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DISP_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DISP_RES_Pin */
  GPIO_InitStruct.Pin = DISP_RES_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DISP_RES_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CHG_ACT_Pin CHG_DONE_Pin */
  GPIO_InitStruct.Pin = CHG_ACT_Pin|CHG_DONE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ENC_B_Pin ENC_A_Pin */
  GPIO_InitStruct.Pin = ENC_B_Pin|ENC_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : CAM_B_Pin CAM_A_Pin */
  GPIO_InitStruct.Pin = CAM_B_Pin|CAM_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void HAL_ADC_ConvCpltCallback (ADC_HandleTypeDef* hadc)
{
	if (LT_ADCCompleteCallback != NULL)
	{
		// 2e helft buffer doorgeven aan functie
		LT_ADCCompleteCallback (ADC_Samples + (NUM_SAMPLES >> 1), NUM_SAMPLES >> 1);
	}

	// batterijspanning uitlezen
	batteryvoltage = (float)ADC_Samples[NUM_SAMPLES - 2] * 2.4 / (float)ADC_Samples[NUM_SAMPLES - 3];
	ADC_Done++;
}

void HAL_ADC_ConvHalfCpltCallback (ADC_HandleTypeDef* hadc)
{
	if (LT_ADCCompleteCallback != NULL)
	{
		// 1e helft buffer doorgeven aan functie
		LT_ADCCompleteCallback (ADC_Samples, NUM_SAMPLES >> 1);
	}
}

void HAL_SYSTICK_Callback (void)
{
	switch_state b;
	uint32_t tick = HAL_GetTick();

	// Ingangen testen. Debouncen en controleren op lange druk
	Test_Input (HAL_GPIO_ReadPin (ENC_A_GPIO_Port, ENC_A_Pin), &ENCAsw);
	b = Test_Input (HAL_GPIO_ReadPin (ENC_B_GPIO_Port, ENC_B_Pin), &ENCBsw);
	Test_Input (HAL_GPIO_ReadPin (ENC_SEL_GPIO_Port, ENC_SEL_Pin), &ENCSELsw);

	// Controleren voor zeer lange druk
	if (ENCSELsw.state > SW_LONG_PRESS)
	{
		// in diepe slaap gaan
		EnterDeepSleep ();
	}

	// status encoder bepalen
	if (b == SW_RISING && (ENCAsw.state == SW_ON))
	{
		// stijgende flank
		enccount++;
		if (hrtc.Instance->CNTL > lastinputtick + 30)
			SH1106_SetBrightness (brightness);
		lastinputtick = hrtc.Instance->CNTL;
	}
	else if (b == SW_FALLING && (ENCAsw.state == SW_ON))
	{
		// dalende flank
		enccount--;
		if (hrtc.Instance->CNTL > lastinputtick + 30)
			SH1106_SetBrightness (brightness);
		lastinputtick = hrtc.Instance->CNTL;
	}

	if (ENCSELsw.state != SW_OFF)
	{
		if (hrtc.Instance->CNTL > lastinputtick + 30)
			SH1106_SetBrightness (brightness);
		lastinputtick = hrtc.Instance->CNTL;
	}

	if ((tick % 1000) == 0)
	{
		ADC_Count = ADC_Done;
		ADC_Done = 0;
		// Dirty = 1;
		framecount = SH1106_GetFrameCount ();
	}

	// ontspanknop na bepaalde tijd loslaten

	if ((HAL_GetTick() > triggertick) && untrigtick == 0)
	{
		Output_CamUntrigger ();
		Output_CamDefocus ();
		untrigtick = HAL_GetTick ();
	}

}

/**
  * @brief  Second event callback.
  * @param  hrtc: pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @retval None
  */

uint8_t	MinuteChanged = 0;

void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	RTC_TimeTypeDef time;
	// zorg ervoor dat scherm opnieuw getekend wordt
	Dirty = 1;

	HAL_RTC_GetTime (&hrtc, &time, RTC_FORMAT_BIN);

	if (hrtc->Instance->CNTL > lastinputtick + 30)
		// scherm dimmen
		SH1106_SetBrightness (0);

	if (screenofftime > 0)
	{
		if (hrtc->Instance->CNTL > lastinputtick + screenofftime / 1000)
			// scherm uitschakelen
			func_setscreenoff ();
	}

	if (time.Seconds == 0)
		// UI laten weten dat scherm bijgewerkt moet worden
		MinuteChanged = 1;
}

/*
 * Instellingen:
 *
 * 0 - Helderheid (8-bit)
 *   - Camerapolariteit (1-bit)
 * 1 - Beeld uit (16-bit)
 * 2 - Trigger uit (16-bit)
 * 3+4 - Shutter time (32-bit)
 * 5 - Datum
 */

uint16_t	VirtAddVarTab[NB_OF_VAR] = { 0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005 };

void SaveSettings (void)
{
	uint16_t	registers[NB_OF_VAR];
	uint8_t		index;
	uint16_t	readval;	// opslag om eeprom met nieuwe waarde te vergelijken
	RTC_DateTypeDef	date;

	HAL_FLASH_Unlock ();

	EE_Init ();

	HAL_RTC_GetDate (&hrtc, &date, RTC_FORMAT_BIN);

	// registers vullen
	registers[0] = (brightness << 8) |
								 reversecontact;
	registers[1] = screenofftime / 1000;
	registers[2] = deviceofftime / 60000;
	registers[3] = shuttertime & 65535;
	registers[4] = shuttertime >> 16;
	registers[5] = (date.Date & 31) |
								 ((date.Month & 15) << 5) |
								 ((date.Year & 127) << 9);

	for (index = 0; index < NB_OF_VAR; index++)
	{
		if (!EE_ReadVariable (index + 1000, &readval))
		{
			// variable bestaat, vergelijken
			if (readval == registers[index])
				continue;	// sla write opdracht over
		}
		EE_WriteVariable (index + 0x1000, registers[index]);
	}

	HAL_FLASH_Lock ();
}

void LoadSettings (void)
{
	uint16_t	registers[NB_OF_VAR];
	uint8_t		index;
	uint16_t	validflags = 0;
	RTC_DateTypeDef	date;

	for (index = 0; index < NB_OF_VAR; index++)
	{
		if (EE_ReadVariable (index + 0x1000, &registers[index]) == 0)
			validflags |= 1 << index;	// status van opdracht opslaan
			// het kan zijn, dat de code gewijzigd is, en er nieuwe variabelen zijn
			// die nog niet in het geheugen staan
	}

	if (validflags & 1)
	{
		brightness = registers[0] >> 8;
		reversecontact = registers[0] & 1;
	}
	if (validflags & 2) screenofftime = registers[1] * 1000;
	if (validflags & 4) deviceofftime = registers[2] * 60000;
	if ((validflags & 24) == 24) shuttertime = registers[3] | (registers[4] << 16);

	/* WERKT NIET voordat RTC geïnitialiseerd is, verplaatst naar RTC_Init
	if (validflags & 32)
	{
		// datum instellen
	  date.Date = registers[5] & 31;
	  date.Month = (registers[5] >> 5) & 15;
	  date.Year = registers[5] >> 9;

	  HAL_RTC_SetDate (&hrtc, &date, RTC_FORMAT_BIN);
	}
	*/
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
	/* User can add his own implementation to report the HAL error return state */
	while (1)
	{
	}
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

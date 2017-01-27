/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

#include "../../Drivers/SH1106/sh1106.h"
#include "ui.h"
#include "functions.h"
#include "input.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

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
	SHOW_VOLTAGES,
	SHUTTERDELAY_COUNTER,
	SHOWCLOCK
} run_state;

#define NUM_SAMPLES			128

volatile uint16_t ADC_Done = 0, ADC_Count = 0;

volatile float voltages[8];
volatile float ldr_voltage;

uint16_t values[NUM_SAMPLES];

volatile uint8_t Dirty = 0;
uint16_t framecount;

run_state RunState = WELCOME;
uint32_t nextactiontick;

volatile int8_t enccount;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
switch_state
Test_Input (uint8_t value, switch_t *input);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

// inputs testen
switch_state
Test_Input (uint8_t value, switch_t *input)
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
		input->state = OFF;
		if (input->locount > SWITCH_VERYLONGPRESS)
			input->locount = SWITCH_DEBOUNCE;   // niet door nul laten gaan

		if (input->locount == SWITCH_DEBOUNCE)
			// dalende flank
			return FALLING;
	}
	else if (input->hicount >= SWITCH_DEBOUNCE)
	{
		if (input->hicount >= SWITCH_LONGPRESS)
		{
			if (input->hicount >= SWITCH_VERYLONGPRESS)
			{
				input->state = VERYLONG_PRESS;
				input->hicount = SWITCH_VERYLONGPRESS;// zorgen dat teller niet overflowt en daardoor naar nul gaat
				return VERYLONG_PRESS;
			}
			else
			{
				input->state = LONG_PRESS;
				return LONG_PRESS;
			}
		}
		else
		{
			input->state = ON;

			if (input->hicount == SWITCH_DEBOUNCE)
				// stijgende flank
				return RISING;
		}
	}

	// niets veranderd, geef huidige status door
	return input->state;
}

void
EnterDeepSleep (void)
{
	uint32_t i;

	if (SH1106_HSPI != NULL)
		// wacht totdat er geen data meer naar het scherm gaat
		while (SH1106_HSPI->hdmatx->State == HAL_DMA_STATE_BUSY)
			;

	HAL_GPIO_WritePin (SH1106_DC, GPIO_PIN_RESET);
	SH1106_WriteByte (0xAE); /*display off*/

	while (Test_Input (HAL_GPIO_ReadPin (ENC_SEL_GPIO_Port, ENC_SEL_Pin),
						&ENCSELsw) != OFF)
		;
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU | PWR_FLAG_SB);
	// Even wachten zodat de controller niet meteen weer ingeschakeld wordt
	for (i = 0; i < 5000000; i++)
	{
		// doe iets, zorg dat de compiler deze lus niet wegoptimaliseet
		ENCAsw.state = ENCBsw.state;
	}
	// Werkt niet, pulldown nodig op PA0
	HAL_PWR_EnableWakeUpPin (PWR_WAKEUP_PIN1);
	HAL_PWR_EnterSTANDBYMode ();
	//HAL_PWR_EnterSTOPMode (PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);
}

void
LT_ShowVoltages (void)
{
	SH1106_Clear ();
	RunState = SHOW_VOLTAGES;
}

void LT_ShowClock (void)
{
	RunState = SHOWCLOCK;
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
	char text[16];
	int32_t value = 0;
	uint8_t i = 0;
	uint8_t menuselected = 0;

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
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */

	HAL_Delay (100);
	// HAL_TIM_Base_Start (&htim3);

	if (__HAL_PWR_GET_FLAG(PWR_FLAG_WU))	// wakeup, controleer op lange druk
	{
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU | PWR_FLAG_SB);

		Test_Input (HAL_GPIO_ReadPin (ENC_SEL_GPIO_Port, ENC_SEL_Pin), &ENCSELsw);

		while (ENCSELsw.state == ON)
			;

		if (ENCSELsw.state != LONG_PRESS)
		{
			EnterDeepSleep ();
		}
	}

	if (hrtc.State == HAL_RTC_STATE_RESET)
		MX_RTC_Init ();

	SH1106_Init (&hspi1);

	// overige periferie initializeren
	MX_ADC1_Init ();
	MX_USB_DEVICE_Init ();

	//HAL_ADC_Start (&hadc1);
	SH1106_SetBrightness (0);

	HAL_PWR_DisableWakeUpPin (PWR_WAKEUP_PIN1);

	// Dit doen, anders is de eerste regel garbage, om onduidelijke reden
	SH1106_PaintScreen ();

	LT_ShowStartScreen ();
	SH1106_PaintScreen ();

	HAL_ADCEx_Calibration_Start (&hadc1);

	HAL_Delay (2000);

	UI_ShowMenu (&LT_MainMenu);
	RunState = MENU;

	HAL_ADC_Start (&hadc1);

	HAL_ADC_Start_DMA (&hadc1, values, NUM_SAMPLES);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	while (1)
	{
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		switch (RunState)
		{
			case MENU:
				func_menu ();
				break;
			case SHOW_VOLTAGES:
				func_showvoltages ();
				break;
			case SHOWCLOCK:
				func_showclock ();
			default:
				break;
		}

		// Alleen als we niet al in een menu zitten, brengt een lange druk ons naar het menu
		if (RunState != MENU)
		{
			if (ENCSELsw.state == LONG_PRESS)
			{
				enccount = 0;
				UI_ShowMenu (&LT_MainMenu);
				RunState = MENU;

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

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

void
HAL_ADC_ConvCpltCallback (ADC_HandleTypeDef* hadc)
{
	char text[16];
	uint32_t value[2];
	uint16_t i;

	value[0] = 0;
	value[1] = 0;
	for (i = 0; i < NUM_SAMPLES; i += 2)
	{
		value[0] += values[i];
		value[1] += values[i + 1];
	}
	voltages[0] = value[0] / (value[1] / 1.2);

	if (voltages[0] > voltages[1])
		voltages[1] = voltages[0];

	ADC_Done++;
}

void
HAL_SYSTICK_Callback (void)
{
	switch_state a, b;
	uint32_t i;

	// Ingangen testen. Debouncen en controleren op lange druk
	a = Test_Input (HAL_GPIO_ReadPin (ENC_A_GPIO_Port, ENC_A_Pin), &ENCAsw);
	b = Test_Input (HAL_GPIO_ReadPin (ENC_B_GPIO_Port, ENC_B_Pin), &ENCBsw);
	Test_Input (HAL_GPIO_ReadPin (ENC_SEL_GPIO_Port, ENC_SEL_Pin), &ENCSELsw);

	HAL_GPIO_WritePin (CAM_A_GPIO_Port, CAM_A_Pin, ENCSELsw.state > OFF);
	HAL_GPIO_WritePin (CAM_B_GPIO_Port, CAM_B_Pin, ENCSELsw.state > ON);

	if (ENCSELsw.state > LONG_PRESS)
	{
		// in diepe slaap gaan
		EnterDeepSleep ();
	}

	// status encoder bepalen
	if (b == RISING && (ENCAsw.state == ON))
	{
		// stijgende flank
		enccount++;
	}
	else if (b == FALLING && (ENCAsw.state == ON))
	{
		// dalende flank
		enccount--;
	}

	if ((HAL_GetTick () % 1000) == 0)
	{
		ADC_Count = ADC_Done;
		ADC_Done = 0;
		Dirty = 1;
		framecount = SH1106_GetFrameCount ();
	}
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

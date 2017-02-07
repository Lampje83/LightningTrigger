/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

#include "stm32f1xx.h"
#include "input.h"

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define ENC_SEL_Pin GPIO_PIN_0
#define ENC_SEL_GPIO_Port GPIOA
#define LDR_Pin GPIO_PIN_1
#define LDR_GPIO_Port GPIOA
#define EXT_TRIG_Pin GPIO_PIN_2
#define EXT_TRIG_GPIO_Port GPIOA
#define BATV_Pin GPIO_PIN_3
#define BATV_GPIO_Port GPIOA
#define DISP_NSS_Pin GPIO_PIN_4
#define DISP_NSS_GPIO_Port GPIOA
#define DISP_SCK_Pin GPIO_PIN_5
#define DISP_SCK_GPIO_Port GPIOA
#define DISP_DC_Pin GPIO_PIN_6
#define DISP_DC_GPIO_Port GPIOA
#define DISP_MOSI_Pin GPIO_PIN_7
#define DISP_MOSI_GPIO_Port GPIOA
#define DISP_RES_Pin GPIO_PIN_0
#define DISP_RES_GPIO_Port GPIOB
#define CHG_ACT_Pin GPIO_PIN_1
#define CHG_ACT_GPIO_Port GPIOB
#define CHG_DONE_Pin GPIO_PIN_2
#define CHG_DONE_GPIO_Port GPIOB
#define ENC_B_Pin GPIO_PIN_4
#define ENC_B_GPIO_Port GPIOB
#define ENC_A_Pin GPIO_PIN_5
#define ENC_A_GPIO_Port GPIOB
#define CAM_B_Pin GPIO_PIN_6
#define CAM_B_GPIO_Port GPIOB
#define CAM_A_Pin GPIO_PIN_7
#define CAM_A_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define NUM_SAMPLES			512
#define SCOPESAMPLES		1024

#define POWER_HIGH_VOLTAGE	3.2			// max batterijspanning
#define POWER_LOW_VOLTAGE		2.2			// min batterijspanning

/* Private variables ---------------------------------------------------------*/

extern volatile uint16_t scopecount;
volatile float						batteryvoltage;

/* Private function prototypes -----------------------------------------------*/

void MX_RTC_Init_Without_Time (void);
void LT_SetNewHandler (void* handler);

void (*LT_ADCCompleteCallback)(uint16_t *, uint16_t);		// function pointer voor ADC complete
																												// pointer naar data en lengte opgeven
void (*LT_RTCAlarmCallback)(void);						// Handler voor RTC event (timelapse etc)
void (*LT_FuncHandler)(void);									// Algemene handler, beeldopbouw etc
void (*LT_FuncHandlerExit)(void);							// Functie om handler af te sluiten
void (*LT_EncTurnCallback)(int8_t);					// Handler voor draai encoder
void (*LT_EncPressCallback)(switch_state state);		// Handler voor drukknop

/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

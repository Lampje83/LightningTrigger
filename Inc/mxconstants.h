/**
  ******************************************************************************
  * File Name          : mxconstants.h
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MXCONSTANT_H
#define __MXCONSTANT_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define ENC_SEL_Pin GPIO_PIN_0
#define ENC_SEL_GPIO_Port GPIOA
#define LDR_Pin GPIO_PIN_1
#define LDR_GPIO_Port GPIOA
#define BATV_Pin GPIO_PIN_2
#define BATV_GPIO_Port GPIOA
#define EXT_TRIG_Pin GPIO_PIN_3
#define EXT_TRIG_GPIO_Port GPIOA
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

/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MXCONSTANT_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

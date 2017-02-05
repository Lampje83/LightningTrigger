/*
 * triggers.c
 *
 *  Created on: 4 feb. 2017
 *      Author: Erik
 */

#include "triggers.h"

extern volatile float		voltages[];
extern uint8_t	scopedata[];
extern uint32_t	triggertick;

/*
const trigger_mode	TrigMode_Lightning = { &lightning_entry, &lightning_handler, &lightning_exit };
const trigger_mode	TrigMode_ExtTrig = { &exttrig_entry, &exttrig_handler, &exttrig_exit };
*/

/*
 * Alle triggers stoppen
 */

void	Trig_StopAllTriggers (void)
{
	LT_ADCCompleteCallback = NULL;		// ADC callback stoppen
	// TODO: RTC (alarm) callback stoppen

	// Camera vrijgeven
	HAL_GPIO_WritePin(CAM_A_GPIO_Port, CAM_A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CAM_B_GPIO_Port, CAM_B_Pin, GPIO_PIN_RESET);

	// menu bijwerken
	if (LT_FuncHandler == func_menu)
	{
		UI_DrawMenu (NULL);
	}
}

/*
 * Bliksem Trigger
 */

extern ui_screen LT_LightningTrigScreen;
extern ui_screen LT_LightningTrigDefocusScreen;
extern GUI_CONST_STORAGE GUI_BITMAP bmBliksemSmall;

uint8_t	Trig_ADCFirstRun = 1;

// Trigger entry
void Trig_StartLightningTrigger (void)
{
	SH1106_Clear ();
	UI_DrawScreen (&LT_LightningTrigScreen);
	LT_ADCCompleteCallback = &Trig_LightningADCCallback;

	Trig_ADCFirstRun = 1;		// niet vergelijken in eerste run

	Dirty = 1;

	LT_SetNewHandler (&Trig_DoLightning);
}

// ADC Callback voor bliskemtrigger. Hier wordt de camera getriggerd
void Trig_LightningADCCallback (uint16_t *samples, uint16_t length)
{

	uint32_t value[5];
	uint16_t i;
	static float	minvoltage[2] = { 9, 9 };
	static float	maxvoltage[2] = { 0, 0 };

	value[0] = 0;
	value[1] = 0;
	value[2] = 0;
	value[3] = 0;
	value[4] = -1;

	minvoltage[1] = minvoltage [0];
	maxvoltage[1] = maxvoltage [0];

	voltages[1] = 0;

	for (i = 0; i < length; i += 3)
	{
		value[0] += samples[i];
		value[1] += samples[i + 1];
		value[2] += samples[i + 2];
		if (samples[i] > value[3]) value[3] = samples[i];
		if (samples[i] < value[4]) value[4] = samples[i];
	}

	voltages[0] = value[0] / (value[1] / 1.2);

	maxvoltage[0] = value[3] * (length / 3) / (value[1] / 1.2);
	minvoltage[0] = value[4] * (length / 3) / (value[1] / 1.2);
	voltages[1] = maxvoltage[0];
	voltages[2] = minvoltage[0];
	voltages[3] = value[2] / (value[1] / 2.4); // spanningsdeler, factor 2

	if (!Trig_ADCFirstRun)
	{
		if ((voltages[0] - minvoltage[1]) > ((maxvoltage[1] - minvoltage[1]) * 2))
		{
				// trigger!
			HAL_GPIO_WritePin(CAM_A_GPIO_Port, CAM_B_Pin, GPIO_PIN_SET);
			triggertick = HAL_GetTick ();
			if (scopecount >= SCOPESAMPLES)	// scope niet hertriggeren bij nieuwe flank tijdens sampleduur
				scopecount = 0;
		}
	}
	else
		Trig_ADCFirstRun = 0;

	if (scopecount < SCOPESAMPLES)
	{
		scopedata[scopecount] = (voltages[0] / voltages[3]) * 255;
		scopecount++;
	}
}

// UI handler voor bliksemtrigger


void Trig_DoLightning (void)
{
	static uint8_t	triggerdrawn = 0;

	if (enccount > 0)
	{
		// focus
		HAL_GPIO_WritePin(CAM_A_GPIO_Port, CAM_A_Pin, GPIO_PIN_SET);
		SH1106_Clear ();
		triggerdrawn = 0;
		UI_DrawScreen (&LT_LightningTrigDefocusScreen);
		Dirty = 1;
		enccount = 0;
	}
	else if (enccount < 0)
	{
		// defocus
		HAL_GPIO_WritePin(CAM_A_GPIO_Port, CAM_A_Pin, GPIO_PIN_RESET);
		SH1106_Clear ();
		triggerdrawn = 0;
		UI_DrawScreen (&LT_LightningTrigScreen);
		Dirty = 1;
		enccount = 0;
	}

	if (HAL_GetTick() - triggertick < 2000)
	{
		if (!triggerdrawn)
		{
			SH1106_DrawBitmap (52, 26, bmBliksemSmall.YSize, bmBliksemSmall.XSize, DM_NORMAL, bmBliksemSmall.pData);
			Dirty = 1;
			triggerdrawn = 1;
		}
	}
	else if (triggerdrawn)
	{
		SH1106_FillBox (52, 26, bmBliksemSmall.YSize, bmBliksemSmall.XSize, DM_INVERT);
		triggerdrawn = 0;
		Dirty = 1;
	}

}

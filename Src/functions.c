/*
 * functions.c
 *
 *  Created on: 27 jan. 2017
 *      Author: Erik
 */

/*
 * subroutines voor de diverse functies
 */

#include "functions.h"

extern uint32_t triggertick;

// Menu handler
extern uint8_t MinuteChanged;

void func_menu (void)
{
	// niets doen!

	if (!LT_FuncHandlerExit)
		LT_FuncHandlerExit = &func_menuexit;

	if (MinuteChanged)
	{
		UI_DrawMenu (NULL);
		MinuteChanged = 0;
	}
}

void func_menuexit (void)
{
	LT_EncTurnCallback = NULL;
	LT_EncPressCallback = NULL;
}

extern ui_menu LT_SettingsMenu;
extern void func_menu (void);

/*
 *	Parameters
 */

char *msToText (uint32_t milliseconds)
{
	static char text[6];

	if (milliseconds < 1000)	// korter dan 1 sec, milliseconden weergeven
		sprintf (text, "%lums", milliseconds);
	else if (milliseconds < 60000)	// korter dan 60 sec, seconden weergeven
		sprintf (text, "%us", (uint8_t)(milliseconds * 0.001));
	else if (milliseconds < 3600000)	// korter dan 1 uur, minuten weergeven
		sprintf (text, "%lum", milliseconds / 60000);
	else if ((milliseconds % 3600000) == 0) // precies een uur
		sprintf (text, "%luu", milliseconds / 3600000);
	else	// uren en minuten weergeven
		sprintf (text, "%luu%lum", milliseconds / 3600000, (milliseconds % 3600000) / 60000);

	return text;
}

char *secToText (uint32_t seconds)
{
	static char text[6];

	if (seconds < 1)	// korter dan 1 sec, Niet dus
		sprintf (text, "Nooit");
	else if (seconds < 60)	// korter dan 60 sec, seconden weergeven
		sprintf (text, "%lus", seconds);
	else if (seconds < 3600)	// korter dan 1 uur, minuten weergeven
		sprintf (text, "%lum", seconds / 60);
	else if (seconds < 86400) // uren en minuten weergeven
	{
		if ((seconds % 3600) == 0) // precies een uur
			sprintf (text, "%luu", seconds / 3600);
		else
			sprintf (text, "%luu%lum", seconds / 3600, (seconds % 3600) / 60);
	}
	else if ((seconds % 86400) == 0) // precies een dag
		sprintf (text, "%lud", seconds / 86400);
	else
		sprintf (text, "%lud%luu", seconds / 86400, (seconds % 86400) / 3600);

	return text;
}

uint32_t stepMilliSeconds (uint32_t value, int8_t steps)
{
	if (steps > 0)
	{
		if (value < 20)
			value += steps;
		else if (value < 150)
			value += 5 * steps;
		else if (value < 500)
			value += 10 * steps;
		else if (value < 1000)
			value += 100 * steps;
		else if (value < 10000)
			value += 1000 * steps;
		else if (value < 60000)
			value += 5000 * steps;
		else if (value < 1800000)
			value += 60000 * steps;
		else if (value < 7200000)
			value += 300000 * steps;
		else if (value < 86400000)
			value += 3600000 * steps;
		else
			value += 21600000 * steps;
	}
	if (steps < 0)
	{
		if (value <= 20)
			value += steps;
		else if (value <= 150)
			value += 5 * steps;
		else if (value <= 500)
			value += 10 * steps;
		else if (value <= 1000)
			value += 100 * steps;
		else if (value <= 10000)
			value += 1000 * steps;
		else if (value <= 60000)
			value += 5000 * steps;
		else if (value <= 1800000)
			value += 60000 * steps;
		else if (value <= 7200000)
			value += 300000 * steps;
		else if (value <= 86400000)
			value += 3600000 * steps;
		else
			value += 21600000 * steps;
	}

	return value;
}

// Helderheid

uint8_t	brightness = 128;

char *func_getbrightness (void)
{
	static char	text[5];

	sprintf (text, "%u%%", (uint8_t)(brightness / 2.55));

	return text;
}

void func_setbrightness (int8_t	steps)
{
if (steps != 0)
	{
		brightness += steps * 5;
		// enccount = 0;
		SH1106_SetBrightness (brightness);
		UI_DrawMenu (&LT_SettingsMenu);
		Dirty = 1;
	}
}

// Sluitertijd

uint32_t	shuttertime = 1;

char *func_getshuttertime (void)
{
	return msToText (shuttertime);
}

void func_setshuttertime (int8_t steps)
{
	shuttertime = stepMilliSeconds (shuttertime, steps);

	if (shuttertime > (1 << 31) || shuttertime == 0)
		shuttertime = 1;

	if (shuttertime > 60000 * 5)	// 5 minuten max
		shuttertime = 60000 * 5;
}

// Beeld uit

uint32_t screenofftime = 60000;

char *func_getscreenofftime (void)
{
	return secToText ((uint32_t)(screenofftime * 0.001));
}

void func_setscreenofftime (int8_t steps)
{
	if (screenofftime == 0 && steps < 0)
		return;

	screenofftime = stepMilliSeconds (screenofftime, steps);
	if (screenofftime < 10000)
	{
		if (steps > 0)
				screenofftime = 10000;
		else
				screenofftime = 0;
	}

	if (screenofftime > 600000)
		screenofftime = 600000;
}

// Trigger uit

uint32_t deviceofftime = 1800000; // 30 min

char *func_getdeviceofftime (void)
{
	return secToText ((uint32_t)(deviceofftime * 0.001));
}

void func_setdeviceofftime (int8_t steps)
{
	if (deviceofftime == 0 && steps < 0)
		return;

	deviceofftime = stepMilliSeconds (deviceofftime, steps);

	if (deviceofftime < 60000)
	{
		if (steps > 0)
				deviceofftime = 60000;
		else
				deviceofftime = 0;
	}

	if (deviceofftime > 14 * 86400000)
		deviceofftime = 0;

	if (deviceofftime > 7 * 86400000)
		deviceofftime = 7 * 86400000;
}

uint8_t	screenoff = 0;
void (*LT_OldEncTurnCallback)(int8_t);					// Handler voor draai encoder
void (*LT_OldEncPressCallback)(switch_state state);		// Handler voor drukknop

// scherm inschakelen en oude functies herstellen
// parameter is dummy zodat draai-callback veilig vervangen kan worden
void func_setscreenon (int8_t dummy)
{
	// scherm inschakelen
	SH1106_TurnOn ();
	SH1106_SetBrightness (brightness);
	screenoff = 0;

	LT_EncTurnCallback = LT_OldEncTurnCallback;
	LT_EncPressCallback = LT_OldEncPressCallback;
}

void func_setscreenoff (void)
{
	if (screenoff)
		return;

	LT_OldEncTurnCallback = LT_EncTurnCallback;
	LT_OldEncPressCallback = LT_EncPressCallback;

	LT_EncTurnCallback = &func_setscreenon;
	LT_EncPressCallback = &func_setscreenon;
	SH1106_TurnOff ();
	screenoff = 1;
}

/*
 *  camera bediening
 */

uint8_t		reversecontact = 0;

char *func_getcontact (void)
{
	if (!reversecontact)
		return "A>B";
	else
		return "B>A";
}

void func_setcontact (int8_t steps)
{
	if (steps & 1) reversecontact = 1 - reversecontact;
}

void Output_CamFocus (void)
{
	if (reversecontact)
		HAL_GPIO_WritePin(CAM_B_GPIO_Port, CAM_B_Pin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(CAM_A_GPIO_Port, CAM_A_Pin, GPIO_PIN_SET);

	// UI opnieuw tekenen indien in menu
	MinuteChanged = 1;
}

void Output_CamDefocus (void)
{
	if (reversecontact)
		HAL_GPIO_WritePin(CAM_B_GPIO_Port, CAM_B_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(CAM_A_GPIO_Port, CAM_A_Pin, GPIO_PIN_RESET);

	// UI opnieuw tekenen indien in menu
	MinuteChanged = 1;
}

void Output_CamTrigger (void)
{
	if (reversecontact)
		HAL_GPIO_WritePin(CAM_A_GPIO_Port, CAM_A_Pin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(CAM_B_GPIO_Port, CAM_B_Pin, GPIO_PIN_SET);

	// UI opnieuw tekenen indien in menu
	MinuteChanged = 1;
}

void Output_CamUntrigger (void)
{
	if (reversecontact)
		HAL_GPIO_WritePin(CAM_A_GPIO_Port, CAM_A_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(CAM_B_GPIO_Port, CAM_B_Pin, GPIO_PIN_RESET);

	// UI opnieuw tekenen indien in menu
	MinuteChanged = 1;
}

GPIO_PinState CAM_FocusSwitch (void)
{
	if (reversecontact)
		return HAL_GPIO_ReadPin (CAM_B_GPIO_Port, CAM_B_Pin);
	else
		return HAL_GPIO_ReadPin (CAM_A_GPIO_Port, CAM_A_Pin);
}

GPIO_PinState CAM_TriggerSwitch (void)
{
	if (reversecontact)
		return HAL_GPIO_ReadPin (CAM_A_GPIO_Port, CAM_A_Pin);
	else
		return HAL_GPIO_ReadPin (CAM_B_GPIO_Port, CAM_B_Pin);
}

void func_CamFocusSwitch (int8_t steps)
{
	if (steps < 0)
		Output_CamDefocus ();
	else if (steps > 0)
		Output_CamFocus ();
}

void func_CamTriggerSwitch (switch_state state)
{
	if (state == SW_ON)
		Output_CamTrigger ();
	else
		Output_CamUntrigger ();
}

extern uint32_t untrigtick;

void func_TriggerCamera (uint32_t shuttime)
{
	Output_CamTrigger ();
	untrigtick = 0;

	if (!shuttime)
		triggertick = HAL_GetTick () + shuttertime;
	else
		triggertick = HAL_GetTick () + shuttime;
}

// Spanningen handler
void func_showvoltages ()
{
	char		text[22];
	int8_t	value;
	uint8_t	i;

	if (value < enccount)	// oplopend
	{
		memset (disp_buffer, 0, 80);
		SH1106_DrawStringBold ("Rechtsom", 0, 0, DM_NORMAL,
								disp_buffer);
		Dirty = 1;
	}
	else if (value > enccount)
	{
		memset (disp_buffer, 0, 80);
		SH1106_DrawStringBold ("Linksom", 0, 0, DM_NORMAL,
								disp_buffer);
		Dirty = 1;
	}
	value = enccount;

	sprintf (text, "%i", enccount);
	memset (disp_buffer + 90, 0, 42);
	SH1106_DrawString (text, 90, 0, DM_FAST, disp_buffer);

	sprintf (text, "FPS: %i", framecount);
	memset (disp_buffer + 206, 0, 58);
	SH1106_DrawString (text, 74, 8, DM_FAST, disp_buffer);

	sprintf (text, "SPS: %i", ADC_Count);
	memset (disp_buffer + 338, 0, 58);
	SH1106_DrawString (text, 74, 16, DM_FAST, disp_buffer);

	for (i = 0; i < 5; i++)
	{
		sprintf (text, "%1.3f V", voltages[i]);
		SH1106_DrawString (text, 0, i * 8 + 24, DM_FAST, disp_buffer);
	}

	switch_state encstate = Input_GetEvent(&ENCSELsw);

	if (encstate == SW_ON)	// encoder ingedrukt
	{
		memset (disp_buffer, 0, 80);
		SH1106_DrawString ("Drukknop", 0, 0, DM_NORMAL, disp_buffer);
		Dirty = 1;
	}
	else if (encstate == SW_VERYLONG_PRESS)
	{
		memset (disp_buffer, 0, 80);
		SH1106_DrawString ("Nog langer!", 0, 0, DM_NORMAL,
							disp_buffer);
		Dirty = 1;
	}
}

extern uint8_t scopedata[];

void func_showscope (void)
{
	uint8_t	x;

	SH1106_Clear ();
	UI_DrawText ((ui_textitem[]) {{ TEXT, "Scope", 66, 0, DM_NORMAL, TOP }}, 1);

	for (x = 0; x < 128; x++)
	{
		SH1106_SetPixel (x, 63 - (scopedata[x] >> 3), DM_NORMAL);
	}
	Dirty = 1;
}

// ----- Cameravertraging timer

static uint32_t	starttick = 0;

void func_DelayEncTurn (int8_t steps)
{
	func_CamFocusSwitch (steps);

	if (steps > 0)
	{
		SH1106_Clear ();
		Output_CamFocus ();
		SH1106_DrawString ("Druk voor foto", 20, 0, DM_FAST, NULL);
		Dirty = 1;
	}
	else if (steps < 0)
	{
		SH1106_Clear ();
		Output_CamDefocus ();
		SH1106_DrawString ("Rechtsom voor focus", 7, 0, DM_FAST, NULL);
		Dirty = 1;
	}
}

void func_DelayTimer (void)
{
	uint32_t currenttick;
	char	timertext[8];

	if (ENCSELsw.state == SW_ON && starttick == 0)
	{
		if (CAM_FocusSwitch () == GPIO_PIN_SET )
		{
			// start teller
			SH1106_Clear ();
			starttick = HAL_GetTick();
			func_TriggerCamera (1);
		}
	}

	currenttick = HAL_GetTick();
	if (currenttick - starttick < 1000)
	{
		sprintf (timertext, "%4u ms", currenttick - starttick);
		SH1106_DrawString (timertext, 40, 0, DM_FAST, NULL);
		Dirty = 1;
	}
	else if (starttick != 0)
	{
		Output_CamDefocus ();
		SH1106_Clear ();
		SH1106_DrawString ("Rechtsom voor focus", 7, 0, DM_FAST, NULL);
		Dirty = 1;
		starttick = 0;
	}
}

void func_EndDelayTimer (void)
{
	SH1106_SetDisplayHeight (64);		// Normaal
	SH1106_SetDisplayOffset (0);
	SH1106_SetRefreshRate (5, 0);	// fOSC

	// Camera vrijgeven
	HAL_GPIO_WritePin(CAM_A_GPIO_Port, CAM_A_Pin, GPIO_PIN_RESET);
}

void func_StartDelayTimer (void)
{
	// Stel beeldscherm in op 1 regel
	SH1106_SetDisplayHeight (8);
	SH1106_SetDisplayOffset (32);
	SH1106_SetRefreshRate (14, 0);	// fOSC + 45%

	SH1106_Clear ();
	SH1106_DrawString ("Rechtsom voor focus", 7, 0, DM_FAST, NULL);

	Dirty = 1;

	LT_SetNewHandler (&func_DelayTimer);
	LT_EncTurnCallback = &func_DelayEncTurn;
	LT_EncPressCallback = NULL;
	LT_FuncHandlerExit = &func_EndDelayTimer;
}

static GPIO_PinState focusstate = GPIO_PIN_RESET;
static GPIO_PinState triggerstate = GPIO_PIN_RESET;

extern ui_screen	*LT_ManualTrigScreen;
extern ui_screen	*LT_ManualTrigDefocusScreen;

void func_manualtrigger (void)
{
	if (CAM_FocusSwitch () != focusstate)
	{
		focusstate = CAM_FocusSwitch ();

		SH1106_Clear ();
		if (focusstate == GPIO_PIN_SET)
			UI_DrawScreen (&LT_ManualTrigDefocusScreen);
		else
			UI_DrawScreen (&LT_ManualTrigScreen);

		UI_DrawStatusBar ();

		Dirty = 1;
	}
	else if (CAM_TriggerSwitch () != triggerstate)
	{
		triggerstate = CAM_TriggerSwitch();
		UI_DrawStatusBar ();
		Dirty = 1;
	}
}

void func_EndManualTrigger (void)
{
	Output_CamUntrigger();
	Output_CamDefocus();
}

void func_StartManualTrigger (void)
{
	SH1106_Clear ();
	UI_DrawScreen (&LT_ManualTrigScreen);
	UI_DrawStatusBar ();

	Dirty = 1;

	focusstate = GPIO_PIN_RESET;

	LT_SetNewHandler (&func_manualtrigger);
	LT_EncTurnCallback = &func_CamFocusSwitch;
	LT_EncPressCallback = &func_CamTriggerSwitch;
	LT_FuncHandlerExit = &func_EndManualTrigger;
}

uint8_t RTC_IsLeapYear(uint16_t nYear)
{
  if((nYear % 4) != 0)
  {
    return 0;
  }

  if((nYear % 100) != 0)
  {
    return 1;
  }

  if((nYear % 400) == 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

extern uint32_t lastinputtick;

// Klok handler
void func_showclock (void)
{
	char text[22];
	RTC_TimeTypeDef		time;				// struct om tijd in op te slaan
	RTC_DateTypeDef		date;				// struct om datum in op te slaan
	static int8_t		editpos = 0;
	static uint8_t		editmode = 0;
	uint8_t						maxdate;			// laatste dag van de maand
	drawmode					clr;
	static uint32_t					tickoffset;

	SH1106_Clear ();

	// TODO: Ervoor zorgen dat de selector pas zichtbaar is als er aan de knop gedraaid wordt.
	// Ook de selector na bepaalde tijd weer laten verdwijnen

	UI_DrawText ((ui_textitem[]) {{ TEXT, "Klok", 66, 0, DM_NORMAL, TOP }}, 1);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	sprintf (text, "%02u-%02u-%4u", date.Date, date.Month, 2000 + date.Year);
	UI_DrawText ((ui_textitem[]) {{ TEXT, text, 66, 16, DM_NORMAL, TOP }}, 0);

	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	sprintf (text, "%02u:%02u:%02u", time.Hours, time.Minutes, time.Seconds);
	UI_DrawText ((ui_textitem[]) {{ TEXT, text, 66, 30, DM_NORMAL, TOP }}, 0);

	UI_DrawText ((ui_textitem[]) {{ TEXT, "Terug", 66, 44, DM_NORMAL, TOP }}, 0);

	UI_DrawStatusBar ();

	if (enccount != 0 && !editmode)
	{
		editpos = (editpos + enccount) % 7;
		if (editpos < 0) editpos += 7;
		enccount = 0;
		Dirty = 1;
	}

	if (Input_GetEvent(&ENCSELsw) == SW_ON)
	{
		if (editpos == 6)
		{
			// Terug
			enccount = 0;
			UI_ShowMenu (&LT_MainMenu);
			LT_SetNewHandler (&func_menu);
			Dirty = 1;
		}
		else
			editmode = 1 - editmode;
	}

	if (editmode && ((HAL_GetTick() - tickoffset) % 600) >= 300)
	{
		clr = DM_NORMAL;
	}
	else
	{
		clr = DM_XOR;
	}

	if (editmode)
		if (((HAL_GetTick() - tickoffset) % 100) == 0)
			Dirty = 1;

	switch (editpos)
	{
		case 0:		// dagen
			SH1106_FillBox (35, 15, 13, 9, clr);
			break;
		case 1:		// maanden
			SH1106_FillBox (53, 15, 13, 9, clr);
			break;
		case 2:		// jaren
			SH1106_FillBox (71, 15, 25, 9, clr);
			break;
		case 3:		// uren
			SH1106_FillBox (41, 29, 13, 9, clr);
			break;
		case 4:		// minuten
			SH1106_FillBox (59, 29, 13, 9, clr);
			break;
		case 5:		// seconden
			SH1106_FillBox (77, 29, 13, 9, clr);
			break;
		case 6:		// terug
			SH1106_FillBox (0, 43, 128, 9, clr);
			break;
		default:
			break;
	}

	if (enccount != 0 && editmode)
	{
		switch (editpos)
		{
			case 5:	// seconden
				time.Seconds += enccount;
				if (time.Seconds > 195) time.Seconds += 60;
				if (time.Seconds > 59) time.Seconds -= 60;
				break;
			case 4:	// minuten
				time.Minutes += enccount;
				if (time.Minutes > 195)	time.Minutes += 60;
				if (time.Minutes > 59) time.Minutes -= 60;
				break;
			case 3:	// uren
				time.Hours += enccount;
				if (time.Hours > 195)	time.Hours += 24;
				if (time.Hours > 23) time.Hours -= 24;
				break;
			case 0:
				date.Date += enccount;

				switch (date.Month)
				{
					case 2:
						if (RTC_IsLeapYear(date.Year))
							maxdate = 29;
						else
							maxdate = 28;
						break;
					case 4:
					case 6:
					case 9:
					case 11:
						maxdate = 30;
						break;
					default:
						maxdate = 31;
						break;
				}

				if (date.Date < 1 || date.Date > 224) date.Date += maxdate;
				if (date.Date > maxdate) date.Date -= maxdate;
				break;
			case 1:
				date.Month += enccount;
				if (date.Month < 1 || date.Month > 224) date.Month += 12;
				if (date.Month > 12) date.Month -= 12;
				break;
			case 2:
				date.Year += enccount;
				break;
		}

		if (editpos < 3)
			HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
		else
			HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);


		enccount = 0;
		tickoffset = HAL_GetTick ();
		// bijwerken zodat beeld niet meteen uitschakelt
		lastinputtick = hrtc.Instance->CNTL;
		Dirty = 1;
	}

	// Het uiteindelijke weergeven van het scherm gebeurt door interrupt van de RTC
}

// Klok instellen
void func_setclock (void)
{
	char text[22];
	RTC_TimeTypeDef		time;				// struct om tijd in op te slaan
	RTC_DateTypeDef		date;				// struct om datum in op te slaan
	SH1106_Clear ();

	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);

	sprintf (text, "%02u-%02u-%4u", date.Date, date.Month, 0x2000 + date.Year);
	sprintf (text, "%02u:%02u:%02u", time.Hours, time.Minutes, time.Seconds);

}

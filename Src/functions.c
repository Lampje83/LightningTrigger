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

static uint8_t	menuselected = 0;			// bijhouden of drukknop losgelaten is

static uint8_t	brightness = 0;
static uint8_t	vcomdeselect = 0;
static uint8_t	predischarge = 0x22;
static uint8_t	dcvoltage = 3;

char *func_getbrightness (void)
{
	static char	text[5];

	sprintf (text, "%u%%", (uint8_t)(brightness / 2.55));

	return text;
}

// Menu handler
void func_menu ()
{
	if (enccount != 0)
	{
		UI_ScrollMenu (enccount);
		enccount = 0;
		UI_DrawMenu (NULL);
		Dirty = 1;
	}

	if (Input_GetEvent(&ENCSELsw) == SW_ON)
	{
		// selectie gemaakt
		UI_SelectMenu ();
	}
}

extern RunState;
extern ui_menu LT_SettingsMenu;

void func_setbrightness ()
{
	if (enccount != 0)
	{
		brightness += enccount;
		enccount = 0;
		SH1106_SetBrightness (brightness);
		UI_DrawMenu (&LT_SettingsMenu);
		Dirty = 1;
	}

	if (Input_GetEvent(&ENCSELsw) == SW_ON)
		// selectie gemaakt, terug naar menu
		RunState = 2;
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
		SH1106_DrawStringBold ("Rechtsom", 0, 0, NORMAL,
								disp_buffer);
		Dirty = 1;
	}
	else if (value > enccount)
	{
		memset (disp_buffer, 0, 80);
		SH1106_DrawStringBold ("Linksom", 0, 0, NORMAL,
								disp_buffer);
		Dirty = 1;
	}
	value = enccount;

	sprintf (text, "%i", enccount);
	memset (disp_buffer + 90, 0, 42);
	SH1106_DrawString (text, 90, 0, FAST, disp_buffer);

	sprintf (text, "FPS: %i", framecount);
	memset (disp_buffer + 206, 0, 58);
	SH1106_DrawString (text, 74, 8, FAST, disp_buffer);

	sprintf (text, "SPS: %i", ADC_Count);
	memset (disp_buffer + 338, 0, 58);
	SH1106_DrawString (text, 74, 16, FAST, disp_buffer);

	for (i = 0; i < 4; i++)
	{
		sprintf (text, "%1.3f V", voltages[i]);
		SH1106_DrawString (text, 0, i * 8 + 32, FAST, disp_buffer);
	}

	switch_state encstate = Input_GetEvent(&ENCSELsw);

	if (encstate == SW_ON)	// encoder ingedrukt
	{
		memset (disp_buffer, 0, 80);
		SH1106_DrawString ("Drukknop", 0, 0, NORMAL, disp_buffer);
		Dirty = 1;
	}
	else if (encstate == SW_VERYLONG_PRESS)
	{
		memset (disp_buffer, 0, 80);
		SH1106_DrawString ("Nog langer!", 0, 0, NORMAL,
							disp_buffer);
		Dirty = 1;
	}
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

	UI_DrawText ((ui_textitem[]) { TEXT, "Klok", 66, 0, NORMAL, TOP }, 1);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	sprintf (text, "%02u-%02u-%4u", date.Date, date.Month, 2000 + date.Year);
	UI_DrawText ((ui_textitem[1]){ TEXT, text, 66, 16, NORMAL, TOP }, 0);

	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	sprintf (text, "%02u:%02u:%02u", time.Hours, time.Minutes, time.Seconds);
	UI_DrawText ((ui_textitem[1]){ TEXT, text, 66, 44, NORMAL, TOP }, 0);

	if (enccount != 0 && !editmode)
	{
		editpos = (editpos - enccount) % 6;
		if (editpos < 0) editpos += 6;
		enccount = 0;
		Dirty = 1;
	}

	if (Input_GetEvent(&ENCSELsw) == SW_ON)
		editmode = 1 - editmode;

	if (editmode && ((HAL_GetTick() - tickoffset) % 600) >= 300)
	{
		clr = NORMAL;
	}
	else
	{
		clr = XOR;
	}

	if (editmode)
		if (((HAL_GetTick() - tickoffset) % 100) == 0)
			Dirty = 1;

	switch (editpos)
	{
		case 0:		// seconden
			SH1106_FillBox (77, 43, 13, 9, clr);
			break;
		case 1:		// minuten
			SH1106_FillBox (59, 43, 13, 9, clr);
			break;
		case 2:		// uren
			SH1106_FillBox (41, 43, 13, 9, clr);
			break;
		case 3:		// dagen
			SH1106_FillBox (35, 15, 13, 9, clr);
			break;
		case 4:		// maanden
			SH1106_FillBox (53, 15, 13, 9, clr);
			break;
		case 5:		// jaren
			SH1106_FillBox (71, 15, 25, 9, clr);
			break;
		default:
			break;
	}

	if (enccount != 0 && editmode)
	{
		switch (editpos)
		{
			case 0:	// seconden
				time.Seconds += enccount;
				if (time.Seconds > 195) time.Seconds += 60;
				if (time.Seconds > 59) time.Seconds -= 60;
				break;
			case 1:
				time.Minutes += enccount;
				if (time.Minutes > 195)	time.Minutes += 60;
				if (time.Minutes > 59) time.Minutes -= 60;
				break;
			case 2:
				time.Hours += enccount;
				if (time.Hours > 195)	time.Hours += 24;
				if (time.Hours > 23) time.Hours -= 24;
				break;
			case 3:
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
			case 4:
				date.Month += enccount;
				if (date.Month < 1 || date.Month > 224) date.Month += 12;
				if (date.Month > 12) date.Month -= 12;
				break;
			case 5:
				date.Year += enccount;
				break;
		}

		if (editpos >= 3)
			HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
		else
			HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);

		enccount = 0;
		tickoffset = HAL_GetTick ();
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

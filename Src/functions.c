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
	if (ENCSELsw.state == ON && menuselected == 0)
	{
		// selectie gemaakt
		UI_SelectMenu ();
		menuselected = 1;	// schakel vlag in zodat er opnieuw gedrukt moet worden voor een volgend menu
	}
	else if (ENCSELsw.state == OFF)
	{
		menuselected = 0;
	}
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

	if (ENCSELsw.state == ON)	// encoder ingedrukt
	{
		memset (disp_buffer, 0, 80);
		SH1106_DrawString ("Drukknop", 0, 0, NORMAL, disp_buffer);
		Dirty = 1;
	}
	else if (ENCSELsw.state == VERYLONG_PRESS)
	{
		memset (disp_buffer, 0, 80);
		SH1106_DrawString ("Nog langer!", 0, 0, NORMAL,
							disp_buffer);
		Dirty = 1;
	}
}

// Klok handler
void func_showclock (void)
{
	char text[22];
	RTC_TimeTypeDef		time;				// struct om tijd in op te slaan
	RTC_DateTypeDef		date;				// struct om datum in op te slaan
	SH1106_Clear ();

	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BCD);
	sprintf (text, "%02x-%02x-%4x", date.Date, date.Month, 0x2000 + date.Year);
	UI_DrawText ((ui_textitem[1]){ TEXT, text, 66, 12, NORMAL, TOP }, 0);

	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BCD);
	sprintf (text, "%02x:%02x:%02x", time.Hours, time.Minutes, time.Seconds);
	UI_DrawText ((ui_textitem[1]){ TEXT, text, 66, 40, NORMAL, TOP }, 0);

	if (HAL_GetTick() % 1000 == 0)
		Dirty = 1;

}

// Klok instellen
void func_setclock (void)
{

}

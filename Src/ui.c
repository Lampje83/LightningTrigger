/*
 * ui.c
 *
 *  Created on: 24 jan. 2017
 *      Author: Erik
 */

#include "ui.h"
#include "GUI.h"

extern void LT_ShowVoltages (void);
extern void LT_ShowClock (void);
extern void LT_SetBrightness (void);
extern void LT_ShowScope (void);

uint8_t		menupos = 0;		// Eerste menuitem in beeld
uint8_t		selecteditem = 0;	// Geselecteerd item
ui_menu		*activemenu;

ui_menuitem LT_MainMenuItems[5] = {
		{ "Modus" },
		{ }
};

const ui_menu	LT_MainMenu;

const ui_menu LT_DebugMenu = {
		"Diagnose",
		5,
		(ui_menuitem[]) {
			{ "Shutter delay test", &LT_SetNewHandler, NULL, &func_StartDelayTimer },
			{ "Toon spanningen", &LT_ShowVoltages },
			{ "Toon klokfrequentie" },
			{ "Toon triggergrafiek", &LT_ShowScope },
			{ "Terug", &UI_ShowMenu, NULL, &LT_MainMenu }
		}
};

const ui_menu	LT_SettingsMenu;

const ui_menu LT_CameraMenu = {
		"Camera",
		3,
		(ui_menuitem[]) {
			{ "Focus/Sluiter" },
			{ "Sluitertijd" },
			{ "Terug", &UI_ShowMenu, NULL, &LT_SettingsMenu }
		}
};

extern uint8_t	brightness;

const ui_menu	LT_SettingsMenu = {
	"Instellingen",
	6,
	(ui_menuitem[]) {
		{ "Helderheid", &LT_SetBrightness, &func_getbrightness, NULL },
		{ "Tijd en datum", &LT_ShowClock },
		{ "Beeld uit na ..." },
		{ "Uitschakelen na ..." },
		{ "Camera", &UI_ShowMenu, NULL, &LT_CameraMenu },
		{ "Terug", &UI_ShowMenu, NULL, &LT_MainMenu }
	}
};

const ui_menu LT_ModeMenu = {
		"Modus",
		7,
		(ui_menuitem[]) {
			{ "Bliksem", &Trig_StartLightningTrigger },
			{ "Timelapse" },
			{ "Tijdstip" },
			{ "Afstandsbediening" },
			{ "Ext. trigger" },
			{ "Stoppen" },
			{ "Terug", &UI_ShowMenu, NULL, &LT_MainMenu }
		}
};

const ui_menu	LT_MainMenu = {
	"Hoofdmenu",
	4,
	(ui_menuitem[]) {
		{ "Modus", &UI_ShowMenu, NULL, &LT_ModeMenu },
		{ "Instellingen", &UI_ShowMenu, NULL, &LT_SettingsMenu },
		{ "Diagnose", &UI_ShowMenu, NULL, &LT_DebugMenu },
		{ "Uitschakelen", &EnterDeepSleep }
	}
};

extern GUI_CONST_STORAGE GUI_BITMAP bmBliksem;

const ui_screen LT_StartScreen = {
	5,
	(void *[]){
	(ui_textitem[1]) { BOLDTEXT, "BliksemTrigger", 64, 0, DM_NORMAL, TOP },
	(ui_textitem[1]) { TEXT, "Versie 2.0 alpha", 81, 16, DM_FAST, TOP },
	(ui_textitem[1]) { TEXT, "(c) 2017", 81, 32, DM_FAST, TOP },
	(ui_textitem[1]) { TEXT, "Erik van Beilen", 81, 48, DM_FAST, TOP },
	(ui_bitmapitem[1]) { BITMAP, 0, 16, 32, 48, DM_NORMAL, TOPLEFT, &bmBliksem.pData }
	}
};

const char*	FocusHelpText[2] =		{ "Draai rechtsom", "voor focus" };
const char* DefocusHelpText[2] =	{ "Draai linksom", "voor defocus" };

uint8_t *LT_TriggerBitmap = NULL;

const ui_screen LT_LightningTrigScreen = {
		1,
		(void *[]) {
			(ui_textitem[1]) { BOLDTEXT, "Bliksem", 64, 0, DM_NORMAL, TOP },
		}
};

extern volatile uint8_t Dirty;

uint8_t	UI_XAlign (uint8_t x, uint8_t width, ui_align align)
{
	switch (align)
	{
	case TOPLEFT:
	case LEFT:
	case BOTTOMLEFT:
		return x;
		break;
	case TOP:
	case CENTER:
	case BOTTOM:
		return x - (width >> 1);
		break;
	case TOPRIGHT:
	case RIGHT:
	case BOTTOMRIGHT:
		return x - width - 1;
		break;
	}

	// shut up compiler
	return x;
}

uint8_t	UI_YAlign (uint8_t y, uint8_t height, ui_align align)
{
	switch (align)
	{
	case TOPLEFT:
	case TOP:
	case TOPRIGHT:
		return y;
		break;
	case LEFT:
	case CENTER:
	case RIGHT:
		return y - (height >> 1);
		break;
	case BOTTOMLEFT:
	case BOTTOM:
	case BOTTOMRIGHT:
		return y - height - 1;
		break;
	}

	// shut up compiler
	return y;
}

void UI_DrawText (ui_textitem *text, uint8_t bold)
{
	if (!bold)
	{
		SH1106_DrawString (
				text->text,
				UI_XAlign (text->x, strlen(text->text) * 6, text->align),
				UI_YAlign (text->y, 6, text->align),
				text->mode, disp_buffer);
	}
	else
	{
		SH1106_DrawStringBold (
				text->text,
				UI_XAlign (text->x, strlen(text->text) * 7, text->align),
				UI_YAlign (text->y, 6, text->align),
				text->mode, disp_buffer);
	}
}

void UI_DrawBitmap (ui_bitmapitem *bitmap)
{
	SH1106_DrawBitmap (UI_XAlign (bitmap->x, bitmap->width, bitmap->align),
					   UI_YAlign (bitmap->y, bitmap->height, bitmap->align),
					   bitmap->width, bitmap->height, bitmap->mode, bitmap->data + 6);

}

void UI_DrawScreen (ui_screen *screen)
{
	uint8_t	i;		// item index
	ui_textitem	*text;
	ui_bitmapitem *bitmap;
	ui_screenitemtype	*type;

	for (i = 0; i < screen->length; i++)
	{
		type = (ui_screenitemtype*)screen->items[i];
		switch (*type)
		{
			case TEXT:
				text = screen->items[i];
				UI_DrawText (text, 0);
				break;

			case BOLDTEXT:
				text = screen->items[i];
				UI_DrawText (text, 1);
				break;
			case BITMAP:
				bitmap = screen->items[i];
				UI_DrawBitmap (bitmap);
				break;
			default:
				break;
		}
	}
}

extern GUI_CONST_STORAGE GUI_BITMAP bmBattery;
extern GUI_CONST_STORAGE GUI_BITMAP bmLightning;

void UI_DrawMenu (ui_menu *menu)
{
	uint8_t		i;
	uint8_t		firstitem, lastitem;
	drawmode	dm;
	char		filltext[22];
	char		*vartext;
	RTC_TimeTypeDef	time;

	SH1106_Clear ();

	HAL_RTC_GetTime (&hrtc, &time, RTC_FORMAT_BIN);

	if (!menu) menu = activemenu;

	UI_DrawText ((ui_textitem[1]){{ TEXT, menu->title, 66, 0, DM_NORMAL, TOP }}, 1);

	firstitem = menupos;
	lastitem = menu->itemcount;

	// niet meer dan 6 items in menu weergeven
	if (lastitem - firstitem > MAX_MENUITEMS)
		lastitem = firstitem + MAX_MENUITEMS;


	for (i = firstitem; i < lastitem; i++)
	{
		strcpy (filltext, menu->items[i].name);

		if (menu->items[i].value)	// menuitem heeft callback om waarde weer te geven
		{
			filltext[strlen(filltext) + 1] = 0x0;	// spatie toevoegen
			filltext[strlen(filltext)] = 0x20;	// spatie toevoegen
			// strcpy (vartext, menu->items[i].value());
			vartext = menu->items[i].value();
			strcpy (filltext + strlen(filltext), vartext);
		}
		if (selecteditem == i)
		{
			dm = DM_INVERSE;
			memset (filltext + strlen(filltext), 0x20, 21 - strlen(filltext));
			filltext[21] = 0x0;
			//SH1106_DrawString (filltext, strlen(filltext) * 6, (i - firstitem) * 9 + 8, dm, disp_buffer);
		}
		else
			dm = DM_NORMAL;

		SH1106_DrawString (filltext, 0, (i - firstitem) * 9 + 8, dm, disp_buffer);
	}

	// Triggermodus weergeven
 if (LT_ADCCompleteCallback == &Trig_LightningADCCallback)
		SH1106_DrawBitmap (70, 56, bmLightning.YSize, bmLightning.XSize, DM_REPLACE, bmLightning.pData);

	// Klok tekenen
	sprintf (filltext, "%02u:%02u", time.Hours, time.Minutes);
	SH1106_DrawString (filltext, 96, 56, DM_REPLACE, disp_buffer);

	// Batterij tekenen
	SH1106_DrawBitmap (78, 56, bmBattery.YSize, bmBattery.XSize, DM_REPLACE, bmBattery.pData);

	// Batterijbalk tekenen
	i = (uint8_t)(((batteryvoltage - 1.9) / 1.4) * 12);
	if (i >= 128) i = 0;
	if (i > 12) i = 12;
	if (i > 0)
		SH1106_FillBox (80, 58, i, 3, DM_NORMAL);

	Dirty = 1;
}

extern RTC_HandleTypeDef hrtc;

void UI_ShowMenu (ui_menu *menu)
{
    activemenu = menu;
    selecteditem = 0;

  	HAL_RTCEx_DeactivateSecond(&hrtc);	// RTC interrupt uitschakelen

  	UI_ScrollMenu (0);	// zorg ervoor dat selectie in beeld is
    UI_DrawMenu (menu);
}

void UI_ScrollMenu (int8_t steps)
{
	 uint8_t itemcount = activemenu->itemcount;

	 selecteditem += steps;

	 // indien minder dan nul, cursor naar onder brengen

	 while (selecteditem >= 200) selecteditem += itemcount;
	 while (selecteditem >= itemcount) selecteditem -= itemcount;

	 if (selecteditem < menupos) menupos = selecteditem;
	 if (selecteditem > menupos + MAX_MENUITEMS - 1) menupos = selecteditem - (MAX_MENUITEMS - 1);
}

void UI_SelectMenu (void)
{
	if (activemenu->items[selecteditem].callback)
	    activemenu->items[selecteditem].callback (activemenu->items[selecteditem].param);
}

void LT_ShowStartScreen (void)
{
	SH1106_Clear ();
	UI_DrawScreen (&LT_StartScreen);
}

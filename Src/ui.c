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

uint8_t		menupos = 0;			// Eerste menuitem in beeld
uint8_t		selecteditem = 0;	// Geselecteerd item
ui_menu		*activemenu;			// pointer naar menu wat wordt weergegeven
uint8_t		editmode = 0;			// vlag voor parameter bewerken

ui_menuitem LT_MainMenuItems[5] = {
		{ "Modus" },
		{ }
};

const ui_menu	LT_MainMenu;

const ui_menu LT_DebugMenu = {
		"Diagnose",
		5,
		(ui_menuitem[]) {
			{ "Shutter delay test", &LT_SetNewHandler, &func_StartDelayTimer },
			{ "Toon spanningen", &LT_ShowVoltages },
			{ "Toon klokfrequentie" },
			{ "Toon triggergrafiek", &LT_ShowScope },
			{ "Terug", &UI_ShowMenu, &LT_MainMenu }
		}
};

const ui_menu	LT_SettingsMenu;

const ui_menuparam	P_Shuttertime = { &func_setshuttertime, &func_getshuttertime };

const ui_menu LT_CameraMenu = {
		"Camera",
		3,
		(ui_menuitem[]) {
			{ "Focus/Sluiter" },
			{ "Sluitertijd", NULL, NULL, &P_Shuttertime },
			{ "Terug", &UI_ShowMenu, &LT_SettingsMenu }
		}
};

// parameters
const ui_menuparam	P_Brightness = { &func_setbrightness, &func_getbrightness };
const ui_menuparam	P_DisplayOff = { &func_setscreenofftime, &func_getscreenofftime };
const ui_menuparam	P_DeviceOff = { &func_setdeviceofftime, &func_getdeviceofftime };

const ui_menu	LT_SettingsMenu = {
	"Instellingen",
	6,
	(ui_menuitem[]) {
		{ "Helderheid", NULL, NULL, &P_Brightness },
		{ "Tijd en datum", &LT_ShowClock },
		{ "Beeld uit na", NULL, NULL, &P_DisplayOff },
		{ "Uitschakelen na", NULL, NULL, &P_DeviceOff },
		{ "Camera", &UI_ShowMenu, &LT_CameraMenu },
		{ "Terug", &UI_ShowMenu, &LT_MainMenu }
	}
};

const ui_menu LT_ModeMenu = {
		"Modus",
		7,
		(ui_menuitem[]) {
			{ "Bliksem", &Trig_StartLightningTrigger },
			{ "Timelapse" },
			{ "Tijdstip" },
			{ "Afstandsbediening", &func_StartManualTrigger },
			{ "Ext. trigger" },
			{ "Stoppen", &Trig_StopAllTriggers },
			{ "Terug", &UI_ShowMenu, &LT_MainMenu }
		}
};

const ui_menu	LT_MainMenu = {
	"Hoofdmenu",
	4,
	(ui_menuitem[]) {
		{ "Modus", &UI_ShowMenu, &LT_ModeMenu },
		{ "Instellingen", &UI_ShowMenu,  &LT_SettingsMenu },
		{ "Diagnose", &UI_ShowMenu, &LT_DebugMenu },
		{ "Uitschakelen", &EnterDeepSleep }
	}
};

const ui_menu LT_LightningMenu = {
		"Bliksem instellen",
		4,
		(ui_menuitem[]) {
			{ "Schaal" },		// ruisbandbreedte (%) / gem. spanning (%) / absolute spanning (%)
			{ "Gevoeligheid" },
			{ "Flanktijd" },
			{ "Terug" }
		}
};

const ui_menu LT_TimelapseMenu = {
		"Timelapse instellen",
		4,
		(ui_menuitem[]) {
			{ "Aantal foto's" },		// ruisbandbreedte / gem. spanning / absolute spanning
			{ "Interval" },
			{ "FPS" },
			{ "Filmduur" },
			{ "Tijdspanne" },
			{ "Start na" },
			{ "Terug" }
		}
};

const ui_menu LT_ExtTrigMenu = {
		"Timelapse instellen",
		4,
		(ui_menuitem[]) {
			{ "Modus" },		// niveau / flank
			{ "Polariteit" },	// positief / negatief
			{ "Flanktijd" },
			{ "Hysteresis" },
			{ "Terug" }
		}
};

extern GUI_CONST_STORAGE GUI_BITMAP bmBliksemSmall;

const ui_screen LT_StartScreen = {
	5,
	(void *[]){
	(ui_textitem[1]) { BOLDTEXT, "BliksemTrigger", 64, 0, DM_NORMAL, TOP },
	(ui_textitem[1]) { TEXT, "Versie 2.0", 64, 16, DM_FAST, TOP },
	(ui_textitem[1]) { TEXT, "(c) 2017", 64, 32, DM_FAST, TOP },
	(ui_textitem[1]) { TEXT, "Erik van Beilen", 64, 48, DM_FAST, TOP },
	(ui_bitmapitem[1]) { BITMAP, 0, 18, 16, 24,
											 DM_NORMAL, TOPLEFT, &bmBliksemSmall.pData }
	}
};

ui_screen LT_LightningTrigScreen = {
		3,
		(void *[]) {
			(ui_textitem[1]) {{ BOLDTEXT, "Bliksem", 64, 0, DM_NORMAL, TOP }},
			(ui_textitem[1]) {{ TEXT, "Draai rechtsom", 64, 8, DM_NORMAL, TOP }},
			(ui_textitem[1]) {{ TEXT, "voor focus", 64, 16, DM_NORMAL, TOP }}
		}
};

ui_screen LT_LightningTrigDefocusScreen = {
		3,
		(void *[]) {
			(ui_textitem[1]) {{ BOLDTEXT, "Bliksem", 64, 0, DM_NORMAL, TOP }},
			(ui_textitem[1]) {{ TEXT, "Draai linksom", 64, 8, DM_NORMAL, TOP }},
			(ui_textitem[1]) {{ TEXT, "voor vrijgeven", 64, 16, DM_NORMAL, TOP }}
		}
};

ui_screen LT_ManualTrigScreen = {
		4,
		(void *[]) {
			(ui_textitem[1]) {{ BOLDTEXT, "Handbediening", 64, 0, DM_NORMAL, TOP }},
			(ui_textitem[1]) {{ TEXT, "Draai rechtsom", 64, 8, DM_NORMAL, TOP }},
			(ui_textitem[1]) {{ TEXT, "voor focus", 64, 16, DM_NORMAL, TOP }},
			(ui_textitem[1]) {{ TEXT, "Druk voor ontspanknop", 64, 24, DM_NORMAL, TOP }}
		}
};

ui_screen LT_ManualTrigDefocusScreen = {
		4,
		(void *[]) {
			(ui_textitem[1]) {{ BOLDTEXT, "Handbediening", 64, 0, DM_NORMAL, TOP }},
			(ui_textitem[1]) {{ TEXT, "Draai linksom", 64, 8, DM_NORMAL, TOP }},
			(ui_textitem[1]) {{ TEXT, "voor vrijgeven", 64, 16, DM_NORMAL, TOP }},
			(ui_textitem[1]) {{ TEXT, "Druk voor ontspanknop", 64, 24, DM_NORMAL, TOP }}
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
extern GUI_CONST_STORAGE GUI_BITMAP bmCamera;
extern GUI_CONST_STORAGE GUI_BITMAP bmFillCamera;

void UI_DrawStatusBar (void)
{
	uint8_t i;
	char		text[22];
	uint8_t nexticonpos = 76;

	RTC_TimeTypeDef	time;

	HAL_RTC_GetTime (&hrtc, &time, RTC_FORMAT_BIN);

	memset (disp_buffer + XSIZE * 7, 0, XSIZE);

	// Triggermodus weergeven
	if (LT_ADCCompleteCallback == &Trig_LightningADCCallback)
	{
		nexticonpos -= bmLightning.YSize + 2;
		SH1106_DrawBitmap (nexticonpos, 56, bmLightning.YSize, bmLightning.XSize, DM_REPLACE, bmLightning.pData);
	}

	// Camerastatus weergeven
	if (CAM_TriggerSwitch () == GPIO_PIN_SET)
	{
		nexticonpos -= bmFillCamera.YSize + 2;
		SH1106_DrawBitmap (nexticonpos, 56, bmFillCamera.YSize, bmFillCamera.XSize, DM_REPLACE, bmFillCamera.pData);
	}
	else if (CAM_FocusSwitch () == GPIO_PIN_SET)
	{
		nexticonpos -= bmCamera.YSize + 2;
		SH1106_DrawBitmap (nexticonpos, 56, bmCamera.YSize, bmCamera.XSize, DM_REPLACE, bmCamera.pData);
	}

	// Klok tekenen
	sprintf (text, "%02u:%02u", time.Hours, time.Minutes);
	SH1106_DrawString (text, 96, 56, DM_REPLACE, disp_buffer);

	// Batterij tekenen
	SH1106_DrawBitmap (76, 56, bmBattery.YSize, bmBattery.XSize, DM_REPLACE, bmBattery.pData);

	// Batterijbalk tekenen
	i = (uint8_t)(((batteryvoltage - 1.9) / 1.4) * 12);
	if (i >= 128) i = 0;
	if (i > 12) i = 12;
	if (i > 0)
		SH1106_FillBox (78, 58, i, 3, DM_NORMAL);

}

void UI_DrawMenu (ui_menu *menu)
{
	uint8_t		i;
	uint8_t		firstitem, lastitem;
	drawmode	dm;
	char		filltext[22];
	char		*vartext;

	SH1106_Clear ();

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

		if (menu->items[i].param != NULL)	// menuitem heeft callback om waarde weer te geven
		{
			filltext[strlen(filltext) + 1] = 0x0;	// spatie toevoegen
			filltext[strlen(filltext)] = 0x20;	// spatie toevoegen
			// strcpy (vartext, menu->items[i].value());
			vartext = menu->items[i].param->value();
			memset (filltext + strlen(filltext), 0x20, 21 - strlen(filltext)); // vullen met spaties
			strcpy (filltext + 21 - strlen(vartext), vartext);
		}
		if (selecteditem == i)
		{
			dm = DM_INVERSE;
			memset (filltext + strlen(filltext), 0x20, 21 - strlen(filltext)); // vullen met spaties
			filltext[21] = 0x0;
			//SH1106_DrawString (filltext, strlen(filltext) * 6, (i - firstitem) * 9 + 8, dm, disp_buffer);
		}
		else
			dm = DM_NORMAL;

		if (!editmode || dm == DM_INVERSE)
			SH1106_DrawString (filltext, 0, (i - firstitem) * 9 + 8, dm, disp_buffer);
	}

	UI_DrawStatusBar ();
	Dirty = 1;
}

//extern RTC_HandleTypeDef hrtc;

void UI_ShowMenu (ui_menu *menu)
{
    activemenu = menu;
    selecteditem = 0;

  	// HAL_RTCEx_DeactivateSecond(&hrtc);	// RTC interrupt uitschakelen

  	UI_ScrollMenu (0);	// zorg ervoor dat selectie in beeld is
    UI_DrawMenu (menu);

    LT_EncTurnCallback = &UI_ScrollMenu;
    LT_EncPressCallback = &UI_SelectMenu;
}

void UI_ScrollMenu (int8_t steps)
{
	 uint8_t itemcount = activemenu->itemcount;

	 if (editmode)
	 {
		 activemenu->items[selecteditem].param->edit (steps);

		 UI_DrawMenu (NULL);
		 Dirty = 1;
		 return;
	 }

	 selecteditem += steps;

	 // indien minder dan nul, cursor naar onder brengen

	 while (selecteditem >= 200) selecteditem += itemcount;
	 while (selecteditem >= itemcount) selecteditem -= itemcount;

	 if (selecteditem < menupos) menupos = selecteditem;
	 if (selecteditem > menupos + MAX_MENUITEMS - 1) menupos = selecteditem - (MAX_MENUITEMS - 1);

	UI_DrawMenu (NULL);
	Dirty = 1;

}

void UI_SelectMenu (switch_state event)
{
	if (event != SW_ON)
		// alleen op ON event reageren
		return;

	if (editmode)
	{
		// edit modus verlaten
		editmode = 0;
		UI_DrawMenu (NULL);
		return;
	}

	if (activemenu->items[selecteditem].callback)
	{
			// menuitem heeft callback
	    activemenu->items[selecteditem].callback (activemenu->items[selecteditem].callbackparam);
	}
	else if (activemenu->items[selecteditem].param)
	{
		// menuitem heeft parameter
		if (activemenu->items[selecteditem].param->edit)
		{
			// parameter heeft callback
			editmode = 1;
			UI_DrawMenu (NULL);
		}
	}
}

void LT_ShowStartScreen (void)
{
	SH1106_Clear ();
	UI_DrawScreen (&LT_StartScreen);
}

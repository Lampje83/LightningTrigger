/*
 * ui.c
 *
 *  Created on: 24 jan. 2017
 *      Author: Erik
 */

#include "ui.h"

extern void LT_ShowVoltages (void);

uint8_t		menupos = 0;		// Eerste menuitem in beeld
uint8_t		selecteditem = 0;	// Geselecteerd item
ui_menu		*activemenu;

ui_menuitem LT_MainMenuItems[5] = {
		{ "Modus" },
		{ }
};

const ui_menu	LT_MainMenu = {
	"Hoofdmenu",
	8,
	(ui_menuitem[]) {
		{ "Modus" },
		{ "Instellingen" },
		{ "Toon spanningen", &LT_ShowVoltages },
		{ "Shutter delay test" },
		{ "Toon klok" },
		{ "Toon klokfrequentie" },
		{ "Overige informatie" },
		{ "Uitschakelen", &EnterDeepSleep }
	}
};

const ui_screen LT_StartScreen = {
	4,
	(void *[4]){
	(ui_textitem[1]) { BOLDTEXT, "BliksemTrigger", 67, 8, NORMAL, TOP },
	(ui_textitem[1]) { TEXT, "Versie 2.0 alpha", 67, 20, FAST, TOP },
	(ui_textitem[1]) { TEXT, "(c) 2017", 67, 32, FAST, TOP },
	(ui_textitem[1]) { TEXT, "Erik van Beilen", 67, 44, FAST, TOP }
	}
};

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

void UI_DrawScreen (ui_screen *screen)
{
	uint8_t	i;		// item index
	ui_textitem	*text;
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

			default:
				break;
		}
	}
}

void UI_DrawMenu (ui_menu *menu)
{
	uint8_t		i;
	uint8_t		firstitem, lastitem;
	drawmode	dm;
	char		filltext[22];

	SH1106_Clear ();

	activemenu = menu;

	UI_DrawText ((ui_textitem[1]){ TEXT, menu->title, 66, 0, NORMAL, TOP }, 1);

	firstitem = menupos;
	lastitem = menu->itemcount;
	if (lastitem - firstitem > 6)
		lastitem = firstitem + 6;

	for (i = firstitem; i < lastitem; i++)
	{
		if (selecteditem == i)
		{
			dm = INVERSE;
			memset (filltext, 0x20, 22 - strlen(menu->items[i].name));
			filltext[22 - strlen(menu->items[i].name)] = 0x0;
			SH1106_DrawString (filltext, strlen(menu->items[i].name) * 6, (i - firstitem) * 9 + 8, dm, disp_buffer);
		}
		else
			dm = NORMAL;

		SH1106_DrawString (menu->items[i].name, 0, (i - firstitem) * 9 + 8, dm, disp_buffer);
	}
}

void UI_ScrollMenu (int8_t steps)
{
	 uint8_t itemcount = activemenu->itemcount;

	 selecteditem += steps;

	 // indien minder dan nul, cursor naar onder brengen

	 while (selecteditem >= 200) selecteditem += itemcount;
	 while (selecteditem >= itemcount) selecteditem -= itemcount;

	 if (selecteditem < menupos) menupos = selecteditem;
	 if (selecteditem > menupos + 5) menupos = selecteditem - 5;
}

void UI_SelectMenu (void)
{
	if (activemenu->items[selecteditem].callback)
		activemenu->items[selecteditem].callback ();
}

void LT_ShowStartScreen (void)
{
	SH1106_Clear ();
	UI_DrawScreen (&LT_StartScreen);
}

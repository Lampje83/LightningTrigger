/*
 * ui.h
 *
 *  Created on: 24 jan. 2017
 *      Author: Erik
 */

#ifndef UI_H_
#define UI_H_

#include "../../Drivers/SH1106/sh1106.h"

/* ALGEMENE DECLARES */

// --- Menu structs ---

typedef struct
{
	char	name[20];
	void	(*callback)(void*);	// callback naar functie wanneer dit item geselecteerd wordt
	char	**value;		// function callback naar waardebepaling
	void	*param;			// parameter voor callback;
} ui_menuitem;

typedef struct
{
	char		title[20];		// titel van menu
	uint8_t		itemcount;		// antal menu-items
	ui_menuitem	*items;		// lijst van menu-items
} ui_menu;

// --- Scherm structs ---

typedef enum
{
	TEXT,
	BOLDTEXT,
	LINE,
	BOX,
	CIRCLE,
	BITMAP
} ui_screenitemtype;

typedef enum
{
	TOPLEFT,
	TOP,
	TOPRIGHT,
	LEFT,
	CENTER,
	RIGHT,
	BOTTOMLEFT,
	BOTTOM,
	BOTTOMRIGHT
} ui_align;

typedef struct
{
	ui_screenitemtype	type;		// TEXT of BOLDTEXT
	char				*text;
	uint8_t				x;
	uint8_t				y;
	drawmode			mode;
	ui_align			align;
} ui_textitem;

typedef struct
{
	ui_screenitemtype	type;		// LINE
	uint8_t				x1;
	uint8_t				y1;
	uint8_t				x2;
	uint8_t				y2;
	drawmode			color;
} ui_lineitem;

typedef struct
{
	ui_screenitemtype	type;		// BOX
	uint8_t				x;
	uint8_t				y;
	uint8_t				width;
	uint8_t				height;
	drawmode			edgecolor;
	drawmode			fillcolor;
} ui_boxitem;

typedef struct
{
	ui_screenitemtype	type;		// CIRCLE
	uint8_t				x;
	uint8_t				y;
	uint8_t				radius;
	drawmode			edgecolor;
	drawmode			fillcolor;
} ui_circleitem;

typedef struct
{
	ui_screenitemtype	type;		// BITMAP
	uint8_t				x;
	uint8_t				y;
	uint8_t				width;
	uint8_t				height;
	drawmode			mode;
	ui_align			align;
	char				*data;
} ui_bitmapitem;

typedef	struct
{
	uint8_t		length;
	void		**items;
} ui_screen;


void UI_DrawMenu (ui_menu *menu);
void UI_ShowMenu (ui_menu *menu);
void UI_ScrollMenu (int8_t steps);
void UI_SelectMenu (void);

void UI_DrawScreen (ui_screen *screen);


/* APP-SPECIFIEKE DECLARES */

const ui_menu	LT_MainMenu;
uint8_t	ScreenBrightness;

extern void EnterDeepSleep (void);

void LT_ShowStartScreen (void);

#endif /* UI_H_ */

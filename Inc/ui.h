/*
 * ui.h
 *
 *  Created on: 24 jan. 2017
 *      Author: Erik
 */

#ifndef UI_H_
#define UI_H_

#include "sh1106.h"
#include "string.h"
#include "functions.h"
#include "triggers.h"

/* ALGEMENE DECLARES */

#define MAX_MENUITEMS 5

// --- Menu structs ---

typedef struct
{
	void	(*edit)(int8_t);	// callback voor waardeverandering
													// aantal encoderstappen dienen doorgegeven te worden
	char	*(*value)(void);	// function callback voor waardeweergave
} ui_menuparam;

typedef struct
{
	char	name[20];
	void	(*callback)(void*);	// callback naar functie wanneer dit item geselecteerd wordt
	//char	*(*value)(void);		// function callback naar waardebepaling
	void	*callbackparam;							// parameter voor callback;
	ui_menuparam	*param;			// parameter die bewerkt kan worden
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

void UI_DrawText (ui_textitem *text, uint8_t bold);
void UI_DrawMenu (ui_menu *menu);
void UI_DrawStatusBar (void);
void UI_ShowMenu (ui_menu *menu);
void UI_ScrollMenu (int8_t steps);
void UI_SelectMenu (switch_state event);

void UI_DrawScreen (ui_screen *screen);

/* APP-SPECIFIEKE DECLARES */

const ui_menu	LT_MainMenu;
extern ui_screen LT_LightningTrigScreen;

uint8_t	ScreenBrightness;

extern void EnterDeepSleep (void);

void LT_ShowStartScreen (void);

#endif /* UI_H_ */

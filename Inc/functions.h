/*
 * functions.h
 *
 *  Created on: 27 jan. 2017
 *      Author: Erik
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "stm32f1xx.h"
#include "ui.h"
#include "input.h"

void func_menu (void);						// subroutine voor menu afhandeling
void func_setbrightness (void);		// helderheid instellen
void func_showvoltages (void);		// subroutine om spanningen af te lezen
void func_showclock (void);				// subroutine om klok weer te geven
void func_showscope (void);				// triggerdata weergeven

char *func_getbrightness (void);	// functie om ingestelde helderheid in tekst om te zetten

extern volatile uint8_t Dirty;
extern volatile uint16_t ADC_Done, ADC_Count;

extern volatile float voltages[];
extern uint16_t framecount;

extern uint8_t	disp_buffer[];

extern RTC_HandleTypeDef hrtc;

#endif /* FUNCTIONS_H_ */

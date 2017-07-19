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
void func_menuexit (void);				// subroutine voor verlaten menu
void func_showvoltages (void);		// subroutine om spanningen af te lezen
void func_showclock (void);				// subroutine om klok weer te geven
void func_showscope (void);				// triggerdata weergeven
void func_StartDelayTimer (void);			// camera delay test starten
void func_StartManualTrigger (void);	// handbediening starten

char *func_getbrightness (void);	// functie om ingestelde helderheid in tekst om te zetten
void func_setbrightness (int8_t steps);		// helderheid instellen
extern void func_setscreenoff (void);

// camera contacten

void Output_CamTrigger (void);
void Output_CamUntrigger (void);
void func_CamFocusSwitch (int8_t steps);
void func_TriggerCamera (uint32_t shuttime);
extern GPIO_PinState CAM_FocusSwitch (void);
extern GPIO_PinState CAM_TriggerSwitch (void);

// sluitertijd
extern uint32_t	shuttertime;
char *func_getshuttertime (void);
void func_setshuttertime (int8_t steps);

// Display uit
extern uint32_t screenofftime;
extern char *func_getscreenofftime (void);
extern void func_setscreenofftime (int8_t steps);

// Apparaat uit
extern uint32_t deviceofftime;
extern char *func_getdeviceofftime (void);
extern void func_setdeviceofftime (int8_t steps);

// Cameracontact
extern char *func_getcontact (void);
extern void func_setcontact (int8_t steps);

extern volatile uint8_t Dirty;
extern volatile uint16_t ADC_Done, ADC_Count;

extern volatile float voltages[];
extern uint16_t framecount;

extern uint8_t	disp_buffer[];

extern RTC_HandleTypeDef hrtc;

#endif /* FUNCTIONS_H_ */

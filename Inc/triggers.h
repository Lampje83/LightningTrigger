/*
 * triggers.h
 *
 *  Created on: 3 feb. 2017
 *      Author: Erik
 */

#ifndef TRIGGERS_H_
#define TRIGGERS_H_

#include "stm32f1xx.h"
#include "main.h"
#include "ui.h"

typedef struct
{
	void	(*entry);				// functie om deze trigger te starten
	void	(*handler);			// deze wordt elke keer aangeroepen door hoofdlus
	void	(*exit);				// trigger afsluiten
} trigger_mode;

void	lightning_entry (void);
void	lightning_handler (void);
void	lightning_exit (void);

void	exttrig_entry (void);
void	exttrig_handler (void);
void	exttrig_exit (void);

void	Trig_StartLightningTrigger (void);
void	Trig_LightningADCCallback (uint16_t *samples, uint16_t length);
void	Trig_DoLightning (void);

#endif /* TRIGGERS_H_ */

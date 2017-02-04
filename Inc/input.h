/*
 * input.h
 *
 *  Created on: 27 jan. 2017
 *      Author: Erik
 */

#ifndef INPUT_H_
#define INPUT_H_

typedef enum
{
	SW_NONE,
	SW_OFF,
	SW_ON,
	SW_LONG_PRESS,
	SW_VERYLONG_PRESS,
	SW_RISING,
	SW_FALLING
} switch_state;

typedef struct
{
	switch_state state;
	switch_state prevstate;
	uint16_t locount;
	uint16_t hicount;
} switch_t;

#define SWITCH_DEBOUNCE		3		// ms
#define SWITCH_LONGPRESS	1000	// ms
#define SWITCH_VERYLONGPRESS	3000 // ms

volatile switch_t ENCAsw, ENCBsw, ENCSELsw;
extern volatile int8_t enccount;

switch_state Test_Input (uint8_t value, volatile switch_t *input);
switch_state Input_PeekEvent (volatile switch_t *input);
switch_state Input_GetEvent (volatile switch_t *input);

#endif /* INPUT_H_ */

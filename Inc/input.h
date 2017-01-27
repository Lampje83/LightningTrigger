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
	OFF, ON, LONG_PRESS, VERYLONG_PRESS, RISING, FALLING
} switch_state;

typedef struct
{
	switch_state state;
	uint16_t locount;
	uint16_t hicount;
} switch_t;

#define SWITCH_DEBOUNCE		3		// ms
#define SWITCH_LONGPRESS	1000	// ms
#define SWITCH_VERYLONGPRESS	3000 // ms

volatile switch_t ENCAsw, ENCBsw, ENCSELsw;
extern volatile int8_t enccount;

#endif /* INPUT_H_ */

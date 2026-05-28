/*
 * File:   inputs.h
 * Author: Jorik Wittevrongel
 *
 * Created on December 30 2023
 */

#ifndef ACTIONS_H
#define	ACTIONS_H

#include <stdbool.h>
#include <stdint.h>

#include "IO/inputs.h"

typedef enum {
    nop,
	toggle,
    on,
    off
} Action;

typedef struct {
	Action action;
	Action delayAction;
	uint32_t delay; 				//delay in seconds (mapped as high/low 16-bit words over Modbus)
	uint8_t pwm;
	uint8_t id;
	uint8_t output;      			//current domoriks devices only have a max of 6 outputs increase if needed
	uint8_t send;		   			//1 = output, 2 feature, 3 send extraEvent;
	uint8_t extraEventId;
} EventAction;

typedef struct {
	EventAction singlePress;
	EventAction doublePress;
	EventAction longPress;
	EventAction switchOn;
	EventAction switchOff;
}InputAction;

#define EXTRA_ACTION_PER_INPUT 10

extern InputAction inputActions[INPUTS_SIZE];
extern EventAction extraActions[INPUTS_SIZE * EXTRA_ACTION_PER_INPUT];

uint8_t copyEventAction(EventAction* src, EventAction* desc);

#endif	/* INPUTS_H */

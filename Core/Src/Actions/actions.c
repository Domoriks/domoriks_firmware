/*
 * File:   inputs.c
 * Author: Jorik Wittevrongel
 *
 * Created on December 20 2022, 17:40
 */

#include "device_config.h"
#include "Actions/actions.h"
#include "main.h"

/*
typedef struct {
	Action action;
	Action delayAction;
	uint32_t delay;
	uint8_t pwm;
	uint8_t id;
	uint8_t output;      //current domoriks devices only have a max of 6 outputs increase if needed
	uint8_t send;		   //1 = output, 2 feature, 3 send extraEvent;
	uint8_t extraEventId;
} EventAction;
*/

const EventAction noAction = {nop, nop, 0, 100, 0, 0, 0, 0};
//{toggle, nop, 0, 100, 64, 6, 0, 0}
const EventAction input1_singlePress = noAction; //{toggle, nop, 0, 100, 64, 6, 0, 1};
const EventAction input2_singlePress = noAction;
const EventAction input3_singlePress = noAction;
const EventAction input4_singlePress = noAction;
const EventAction input5_singlePress = noAction;
const EventAction input6_singlePress = noAction;

const EventAction input1_doublePress = noAction;//{on, nop, 0, 100, (DEVICE_ID == 10) ? 20 : 10, 1, 0, 0};
const EventAction input2_doublePress = noAction;
const EventAction input3_doublePress = noAction;
const EventAction input4_doublePress = noAction;
const EventAction input5_doublePress = noAction;
const EventAction input6_doublePress = noAction;

const EventAction input1_longPress = noAction;//{nop, nop, 0, 100, (DEVICE_ID == 10) ? 20 : 10, 1, 0, 0};
const EventAction input2_longPress = noAction;
const EventAction input3_longPress = noAction;
const EventAction input4_longPress = noAction;
const EventAction input5_longPress = noAction;
const EventAction input6_longPress = noAction;

const EventAction input1_on = noAction;
const EventAction input2_on = noAction;
const EventAction input3_on = noAction;
const EventAction input4_on = noAction;
const EventAction input5_on = noAction;
const EventAction input6_on = noAction;

const EventAction input1_off = noAction;
const EventAction input2_off = noAction;
const EventAction input3_off = noAction;
const EventAction input4_off = noAction;
const EventAction input5_off = noAction;
const EventAction input6_off = noAction;

const EventAction extraAction1 = noAction;
const EventAction extraAction2 = noAction;
const EventAction extraAction3 = noAction;
const EventAction extraAction4 = noAction;
const EventAction extraAction5 = noAction;
const EventAction extraAction6 = noAction;
const EventAction extraAction7 = noAction;
const EventAction extraAction8 = noAction;
const EventAction extraAction9 = noAction;
const EventAction extraAction10 = noAction;
const EventAction extraAction11 = noAction;
const EventAction extraAction12 = noAction;
const EventAction extraAction13 = noAction;
const EventAction extraAction14 = noAction;
const EventAction extraAction15 = noAction;
const EventAction extraAction16 = noAction;
const EventAction extraAction17 = noAction;
const EventAction extraAction18 = noAction;
const EventAction extraAction19 = noAction;
const EventAction extraAction20 = noAction;
const EventAction extraAction21 = noAction;
const EventAction extraAction22 = noAction;
const EventAction extraAction23 = noAction;
const EventAction extraAction24 = noAction;

InputAction inputActions[INPUTS_SIZE] = {
	{input1_singlePress, input1_doublePress, input1_longPress, input1_on, input1_off},
	{input2_singlePress, input2_doublePress, input2_longPress, input2_on, input2_off},
	{input3_singlePress, input3_doublePress, input3_longPress, input3_on, input3_off},
	{input4_singlePress, input4_doublePress, input4_longPress, input4_on, input4_off},
	{input5_singlePress, input5_doublePress, input5_longPress, input5_on, input5_off},
	{input6_singlePress, input6_doublePress, input6_longPress, input6_on, input6_off}
};

EventAction extraActions[(INPUTS_SIZE * EXTRA_ACTION_PER_INPUT)] = {
		extraAction1,
		extraAction2,
		extraAction3,
		extraAction4,
		extraAction5,
		extraAction6,
		extraAction7,
		extraAction8,
		extraAction9,
		extraAction10,
		extraAction11,
		extraAction12,
		extraAction13,
		extraAction14,
		extraAction15,
		extraAction16,
		extraAction17,
		extraAction18,
		extraAction19,
		extraAction20,
		extraAction21,
		extraAction22,
		extraAction23,
		extraAction24
};

uint8_t copyEventAction(EventAction* src, EventAction* desc) {
	desc->action = src->action;
	desc->delayAction = src->delayAction;
	desc->delay = src->delay;
	desc->pwm = src->pwm;
	desc->id = src->id;
	desc->output = src->output;       //current domoriks devices only have a max of 6 outputs increase if needed
	desc->send = src->send;		   //1 = output, 2 feature, 3 send extraEvent;
	desc->extraEventId = src->extraEventId;
	if (desc->extraEventId == 0)
		desc->extraEventId = 255; //flag to reset via flash
	return 0;
}

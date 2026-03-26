/*
 * File:   input_handler.c
 * Author: Jorik Wittevrongel
 *
 * Created on October 14 2023
 */

#include "main.h"

#include "timer.h"

#include "Actions/actions.h"

#include "IO/inputs.h"
#include "IO/outputs.h"
#include "IO/input_handler.h"


uint32_t timer_debounce;
uint8_t timed_doublepress_index = 255;
uint32_t timer_doublePress;
uint8_t timed_longpress_index = 255;
uint32_t timer_longPress;
uint8_t skip_next_release = false;
uint8_t state_a0 = false;
uint32_t timer_send_modbus;

void input_handler(){
    //BUTTONS
    for (int i = 0; i < INPUTS_SIZE; i++) {
      if (!TIMER_ELAPSED_MS(timer_debounce, 50) && inputs[i].param.button_type == type_pushbutton) {
        inputs[i].param.changed = false;						//ignore changes for 50ms (pushbutton debounce)
      }
      if (skip_next_release && inputs[i].param.changed) {	    //if there is a release after long press
        timer_debounce = TIMER_SET(); 							//reset debounce timer
        inputs[i].param.changed = false;						//set change false to skip
        skip_next_release = false;								//only skip one release
      }

      //Pushbutton
      //Single press
      if (inputs[i].param.button_type == type_pushbutton &&		//check if there is a pushbutton on the input
        inputs[i].param.changed &&								//is the value update
        !inputs[i].param.value) {								//is the button released

        //Single press ACTION
        //outputs[i].param.value = !outputs[i].param.value;		//toggle same output (for DEBUG)
    	inputActions[i].singlePress.send = true;

        //Double press
        if (TIMER_ELAPSED_MS(timer_doublePress, 500)) {			//if timer is older the
          timer_doublePress = TIMER_SET();						//reset timer
          timed_doublepress_index = i;					    	//register that this input is currently timed
        }
        else {													//if pressed again within 500ms of previous press
          if (timed_doublepress_index == i) {					//check if this input is timed

            //Double press ACTION
        	inputActions[i].doublePress.send = true;
        	inputActions[i].singlePress.send = false;
          }
        }
      }

      //Long press
      if (inputs[i].param.button_type == type_pushbutton &&		//check if there is a pushbutton on the input
        inputs[i].param.changed &&								//is the value update
        inputs[i].param.value) {								//is button pressed?

        timer_longPress = TIMER_SET();							//start timer
        timed_longpress_index = i;								//register that this input is currently timed
      }
      if (inputs[i].param.button_type == type_pushbutton &&		//check if there is a pushbutton on the input
        inputs[i].param.value &&								//is the button preset down
        timed_longpress_index == i &&							//check if timer is running for this input
		!skip_next_release &&									//Release first before triggering again (without single press action trigger)
		TIMER_ELAPSED_MS(timer_longPress, 1500)) {				//long press timer if elapsed & button is still pressed -> long press

        //Long press ACTION
    	inputActions[i].longPress.send = true;
        skip_next_release = true;								//next release action must be ignored after long press
      }

      //Switch
      if (inputs[i].param.button_type == type_switch &&			//check if there is a switch on the input
        inputs[i].param.changed) {								//is the value update

        //switch changed ACTION
    	inputActions[i].singlePress.send = true;

        if (inputs[i].param.value) {
          //switch on ACTION
          inputActions[i].switchOn.send = true;
        }
        else {
          //switch off ACTION
          inputActions[i].switchOff.send = true;
        }
      }

      if (inputs[i].param.changed) {
        timer_debounce = TIMER_SET(); 							//reset debounce timer
        inputs[i].param.changed = false;						//set changed flag to false (input actions handled)
      }
    }
}


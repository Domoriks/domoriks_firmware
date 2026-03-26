/*
 * File:   outputs.c
 * Author: Jorik Wittevrongel
 *
 * Created on December 20 2022, 19:02
 */

#include "main.h"
#include "timer.h"
#include "IO/outputs.h"

uint8_t init_do(OutputParam *param) {
	//Not needed done by generated code
	return 0;
}

uint8_t delay_do(OutputParam *param){
	if (param->startTimer == 0)
		param->startTimer = TIMER_SET();
	if (TIMER_ELAPSED_S(param->startTimer, param->delay)) {
		param->value = param->delay_value;
		param->delay = 0;
		param->startTimer = 0;
	}
	return 0;
}

uint8_t write_do(OutputParam *param) {
	if (param->delay != 0)
		delay_do(param);
	HAL_GPIO_WritePin((GPIO_TypeDef *)param->port, param->pin, param->value == 0 ? param->invert : !param->invert);
	return 0;
}

uint8_t init_pwm(OutputParam *param) {
	//Not needed done by generated code
	return 1; //TODO PWM
}

uint8_t delay_pwm(OutputParam *param) {
	return 1; //TODO PWM
}

uint8_t write_pwm(OutputParam *param) {
	return 1; //TODO PWM
}


//Output outputs[OUTPUTS_SIZE] = {};
Output outputs[OUTPUTS_SIZE] = {
//		{{pin,                      port, value,invert,min,max,delay,delayValue,startTimer},init_func, update_func}
		{{L1_Pin,(const int)L1_GPIO_Port,     0,  	 1,  0,  1,    0,         0,         0}, &init_do, &write_do},
		{{L2_Pin,(const int)L2_GPIO_Port,     0,  	 1,	 0,  1,    0,         0,         0}, &init_do, &write_do},
		{{L3_Pin,(const int)L3_GPIO_Port,     0,  	 0,	 0,  1,    0,         0,         0}, &init_do, &write_do},
//		{{pin,                      port, value,invert,min,max,delay,delayValue,startTimer},init_func, update_func}
		{{L4_Pin,(const int)L4_GPIO_Port,     0,  	 0,	 0,  1,    0,         0,         0}, &init_do, &write_do},
		{{L5_Pin,(const int)L5_GPIO_Port,     0,  	 0,	 0,  1,    0,         0,         0}, &init_do, &write_do},
		{{L6_Pin,(const int)L6_GPIO_Port,     0,  	 0,	 0,  1,    0,         0,         0}, &init_do, &write_do}
//		{{pin,                      port, value,invert,min,max,delay,delayValue,startTimer},init_func, update_func}

////		{{pin,                      port, value,invert,min,max,delay,delayValue,startTimer},init_func, update_func}
//		{{L6_Pin,(const int)L6_GPIO_Port,     0,  	 0,  0,  1,    0,         0,         0}, &init_do, &write_do},
//		{{L5_Pin,(const int)L5_GPIO_Port,     0,  	 0,	 0,  1,    0,         0,         0}, &init_do, &write_do},
//		{{L4_Pin,(const int)L4_GPIO_Port,     0,  	 0,	 0,  1,    0,         0,         0}, &init_do, &write_do},
////		{{pin,                      port, value,invert,min,max,delay,delayValue,startTimer},init_func, update_func}
//		{{L3_Pin,(const int)L3_GPIO_Port,     0,  	 0,	 0,  1,    0,         0,         0}, &init_do, &write_do},
//		{{L2_Pin,(const int)L2_GPIO_Port,     0,  	 1,	 0,  1,    0,         0,         0}, &init_do, &write_do},
//		{{L1_Pin,(const int)L1_GPIO_Port,     0,  	 1,	 0,  1,    0,         0,         0}, &init_do, &write_do}
////		{{pin,                      port, value,invert,min,max,delay,delayValue,startTimer},init_func, update_func}
};

uint8_t update_outputs() {
    uint8_t nok = 0;
    Output *ptr = outputs;
    for (int i = 0; i < OUTPUTS_SIZE; i++) {
	        if (!ptr->updateFunction(&ptr->param))
            nok = 1;
        ptr++;
    }
    return nok;
}

/*
 * File:   inputs.c
 * Author: Jorik Wittevrongel
 *
 * Created on December 20 2022, 17:40
 */

#include "main.h"
#include "IO/inputs.h"

uint8_t init_di(InputParam *param) {
	//Not needed done by generated code
	return 0;
}

uint8_t read_di(InputParam *param) {
	uint8_t temp = param->value;
	param->value = (uint8_t) HAL_GPIO_ReadPin((GPIO_TypeDef *)param->port, param->pin);
	if (param->value != temp)
		param->changed = 1;
	return 0;
}

uint8_t read_inv_di(InputParam *param) {
	uint8_t temp = param->value;
	param->value = (uint8_t) (HAL_GPIO_ReadPin((GPIO_TypeDef *)param->port, param->pin) ? 0 : 1);
	if (param->value != temp)
		param->changed = 1;
	return 0;
}

Input inputs[INPUTS_SIZE] = {
//		{{pin,    port,                  type   value,min,max,changed} init_func, update_func}
		{{B1_Pin,(const int)B1_GPIO_Port,type_pushbutton,0,0,1,0}, &init_di, &read_inv_di},
		{{B2_Pin,(const int)B2_GPIO_Port,type_pushbutton,0,0,1,0}, &init_di, &read_inv_di},
		{{B3_Pin,(const int)B3_GPIO_Port,type_pushbutton,0,0,1,0}, &init_di, &read_inv_di},
//		{{pin,    port,                  type   value,min,max,updated} init_func, update_func}
		{{B4_Pin,(const int)B4_GPIO_Port,type_pushbutton,0,0,1,0}, &init_di, &read_inv_di},
		{{B5_Pin,(const int)B5_GPIO_Port,type_pushbutton,0,0,1,0}, &init_di, &read_inv_di},
		{{B6_Pin,(const int)B6_GPIO_Port,type_pushbutton,0,0,1,0}, &init_di, &read_inv_di}
//		{{pin,    port,                  type   value,min,max,updated} init_func, update_func}
};

uint8_t update_inputs() {
    uint8_t nok = 0;
    Input *ptr = inputs;
    for (int i = 0; i < INPUTS_SIZE; i++) {
        if (!ptr->updateFunction(&ptr->param))
            nok = 1;
        ptr++;
    }
    return nok;
}

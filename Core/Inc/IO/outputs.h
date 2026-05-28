/*
 * File:   outputs.h
 * Author: Jorik Wittevrongel
 *
 * Created on December 20 2022
 */

#ifndef OUTPUTS_H
#define	OUTPUTS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {             //Combine with Inputparam in IO param
	uint16_t pin;
  	const int port;
  	uint8_t value;
  	uint8_t invert;
  	uint8_t min;
  	uint8_t max;
  	uint16_t delay;				//max 18.2 hours, in s
  	uint8_t delay_value;		//0, 1 (or pwm value)
  	uint32_t startTimer;
} OutputParam;

typedef uint8_t (*pfnOutputInit)(OutputParam*);
typedef uint8_t (*pfnOutputUpdate)(OutputParam*);

typedef struct {
	OutputParam param;
	pfnOutputInit initFunction;
	pfnOutputUpdate updateFunction;
} Output;

#define OUTPUTS_SIZE 6
extern Output outputs[OUTPUTS_SIZE];

uint8_t update_outputs( void );

#endif	/* OUTPUTS_H */

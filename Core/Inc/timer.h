/* 
 * File:   timer.h
 * Author: Jorik Wittevrongel
 *
 * Created on May 11, 2022
 */

#ifndef TIMER_H
#define	TIMER_H

#include <stdint.h>
#include <stdbool.h>

#include "stm32g0xx_hal.h"

extern volatile uint32_t GLOBAL_TIMER;
#define TICK_INTERVAL_TIME 10
#define GLOBAL_TIMER_TICK() (GLOBAL_TIMER += TICK_INTERVAL_TIME)
#define TIMER_SET() (GLOBAL_TIMER)
#define TIMER_ELAPSED_MS(t, ms) ((((uint32_t)(GLOBAL_TIMER) - (uint32_t)(t)) > (uint32_t)(ms)) ? (true) : (false))
#define TIMER_ELAPSED_S(t, s)   ((((uint32_t)(GLOBAL_TIMER) - (uint32_t)(t)) > ((uint32_t)(s) * 1000)) ? (true) : (false))

/* TIM16-based microsecond timer helpers (configured in CubeMX). */
void timer_us_init(void);
uint32_t timer_us_now(void);
void timer_delay_us(uint32_t us);

#endif	/* TIMER_H */

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

extern uint32_t GLOBAL_TIMER;
#define TICK_INTERVAL_TIME 10
#define GLOBAL_TIMER_TICK() (GLOBAL_TIMER += TICK_INTERVAL_TIME)
#define TIMER_SET() (GLOBAL_TIMER)
#define TIMER_ELAPSED_MS(t, ms) ((((uint32_t)(GLOBAL_TIMER) - (uint32_t)(t)) > (uint32_t)(ms)) ? (true) : (false))
#define TIMER_ELAPSED_S(t, s)   ((((uint32_t)(GLOBAL_TIMER) - (uint32_t)(t)) > ((uint32_t)(s) * 1000)) ? (true) : (false))

#endif	/* TIMER_H */

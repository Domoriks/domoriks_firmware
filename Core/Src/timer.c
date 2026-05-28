/* 
 * File:   timer.h
 * Author: Jorik Wittevrongel
 *
 * Created on May 11, 2022
 */

#include <stdint.h>
#include <stdbool.h>
#include "timer.h"

volatile uint32_t GLOBAL_TIMER = 0;

void timer_us_init(void)
{
#if defined(TIM16)
	/* TIM16 is configured by CubeMX; ensure counter is running. */
	if ((TIM16->CR1 & TIM_CR1_CEN) == 0U) {
		SET_BIT(TIM16->CR1, TIM_CR1_CEN);
	}
#endif
}

uint32_t timer_us_now(void)
{
#if defined(TIM16)
	return (uint32_t)TIM16->CNT;
#else
	return 0U;
#endif
}

void timer_delay_us(uint32_t us)
{
	uint32_t start = timer_us_now();
	while ((uint32_t)(timer_us_now() - start) < us) {
		__NOP();
	}
}

#ifndef TIMING_US_H
#define TIMING_US_H

#include <stdint.h>

/* Initialize a hardware timer to provide microsecond timestamps. */
void timing_us_init(void);

/* Return current microsecond timestamp (rollover-safe as 32-bit). */
uint32_t timing_us_now(void);

/* Busy-wait microsecond delay (helper). */
void timing_us_delay_us(uint32_t us);

#endif // TIMING_US_H

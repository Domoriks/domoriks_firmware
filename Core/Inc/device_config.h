/*
 * File: device_config.h
 * Author: Jorik Wittevrongel
 *
 * Created on October 5 2023
 */

#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

//#include "IO/inputs.h"
//#include "IO/outputs.h"

//#define DEVICE_ID 10

#define BAUDRATE 115200U

/* Character timing (for Modbus RTU framing). Using 8N1 by default: 1 start + 8 data + 1 stop = 10 bits */
#define UART_BITS_PER_CHAR 10U

/* microseconds per UART character */
#define UART_BYTE_TIME_US() ((UART_BITS_PER_CHAR * 1000000UL) / (BAUDRATE))
/* rounded-up milliseconds per byte */
#define UART_BYTE_TIME_MS() (((UART_BYTE_TIME_US()) + 999UL) / 1000UL)

/* Modbus RTU silent intervals */
#define MODBUS_T1_5_US()   ((UART_BYTE_TIME_US() * 15UL) / 10UL)
#define MODBUS_T3_5_US()   ((UART_BYTE_TIME_US() * 35UL) / 10UL)

//#define HOLD_REGS_SIZE (OUTPUTS_SIZE + (INPUTS_SIZE * 10)) //
//#define INPUTS_REGS_SIZE 1 //not used I think

#endif //DEVICE_CONFIG_H

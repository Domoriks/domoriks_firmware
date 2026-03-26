/*
 * File:   modbus.h
 * Author: Jorik Wittevrongel
 *
 * Created on 30 December 2023
 */

/*
 * The modbus implentations is designed to write directly to the Output/Actions Structs
 * This way is is possible to make the program as compact as possible
 * Maybe I can implement a pointers to the structs to make it easier to program / read
 */

#ifndef MODBUS_H
#define MODBUS_H

#include <stdbool.h>
#include <stdint.h>

#include "Modbus/modbusm.h"

extern uint8_t wait_for_response;
extern ModbusMessage send_message;

void modbus();

#endif /* MODBUS_H */

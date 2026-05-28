/*
 * File:   modbus_ascii.h
 * Author: Jorik Wittevrongel
 */

#ifndef MODBUS_ASCII_H
#define MODBUS_ASCII_H

#include <stddef.h>
#include <stdint.h>

#include "Modbus/modbusm.h"

uint8_t calculate_lrc(const uint8_t* data, size_t length);
uint8_t decode_modbus_ascii(char* message, ModbusMessage* decoded);

#endif /* MODBUS_ASCII_H */

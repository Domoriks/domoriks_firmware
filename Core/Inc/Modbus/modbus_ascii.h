/*
 * File:   modbus_decode.h
 * Author: Jorik Wittevrongel
 *
 * Created on December 20 2022
 */

#ifndef MODBUS_ASCII_H
#define MODBUS_ASCII_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "modbusm.h"

#define MODBUS_ASCII_START_BYTE 0x3A  // ASCII ':'
#define MODBUS_ASCII_END_BYTE 0x0D  // ASCII CR
#define MODBUS_ASCII_LRC_BYTE 0x2A  // ASCII '*'

uint8_t encode_modbus_ascii(char* encoded, size_t* length, ModbusMessage* message);
uint8_t decode_modbus_ascii(char* message, ModbusMessage* decoded);
//uint8_t print_modbus_ascii(ModbusMessage* message);

#endif /* MODBUS_ASCII_H */

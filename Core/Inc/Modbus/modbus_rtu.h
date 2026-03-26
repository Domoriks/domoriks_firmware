/*
 * File:   modbus_rtu.h
 * Author: Jorik Wittevrongel
 *
 * Created on October 5 2023
 */

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "modbusm.h"

uint8_t encode_modbus_rtu(uint8_t* encoded, size_t* length, ModbusMessage* message);
uint8_t decode_modbus_rtu(uint8_t* message, size_t length, ModbusMessage* decoded);
//uint8_t print_modbus_rtu(ModbusMessage* message);

#endif /* MODBUS_RTU_H */

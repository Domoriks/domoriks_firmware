/*
 * File:   modbus_rtu.h
 * Author: Jorik Wittevrongel
 *
 * Created on October 5 2023
 */

#ifndef MODBUS_MESSAGE_H
#define MODBUS_MESSAGE_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

 // Structure to hold decoded Modbus message
typedef struct {
    uint8_t slave_address;
    uint8_t function_code;
    uint8_t data[100];
    uint8_t data_length;
} ModbusMessage;

#endif /* MODBUS_MESSAGE_H */

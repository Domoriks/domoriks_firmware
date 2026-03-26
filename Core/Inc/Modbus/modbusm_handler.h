/*
 * File:   modbus_function_handler.h
 * Author: Jorik Wittevrongel
 *
 * Created on October 5 2023
 */
#ifndef MODBUSM_HANDLER_H
#define MODBUSM_HANDLER_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "modbusm.h"
#include "device_config.h"
#include "IO/outputs.h"

extern uint8_t* modbusCoils;
extern uint8_t* modbusInputs;
extern uint16_t* modbusHReg;
extern uint16_t* modbusIReg;

//ERROR
#define HANDLED_OK              0x00
#define ID_MISMATCH             0x01
#define INVALID_DATA_LENGHT     0x02
#define INVALID_COIL_VALUE      0x03
#define INVALID_FUNCTION        0x10
#define NOT_IMPLEMENTED         0xFF

//ERRORS SYNC
#define SYNC_OK                 0x00

//ERRORS
#define ACTION_OK				0x00
#define WRONG_ACTION_TYPE		0x01
#define WRONG_ACTION_INPUTNMBR  0x02

//MESSAGE FUNCTIONS
#define READ_COILS              0x01 
#define READ_DISC_INPUTS        0x02
#define READ_HOLD_REGS          0x03
#define READ_INPUT_REGS         0x04
#define WRITE_SINGLE_COIL       0x05
#define WRITE_SINGLE_REG        0x06
#define READ_EXCEPTION_STATUS   0x07
#define DIAGNOSE_SERIAL         0x08
#define COMM_EVENT_COUNT        0x0B
#define WRITE_MULTI_COILS       0x0F
#define WRITE_MULTI_REGS        0x10
#define REPORT_SERVER_ID        0x11
#define READ_DEVICE_ID          0x14
#define MASK_WRITE_REG          0x16
#define RW_MULTI_REGS           0x17
#define READ_DEVICE_ID_         0x2B

uint8_t modbusm_handle(ModbusMessage* message);
uint8_t modbus_set_outputs(void);
uint8_t modbus_get_outputs(void);
uint8_t modbus_parse_register(void);
uint8_t modbus_parse_action_update(void);

#endif /* MODBUS_FUNCTION_H */

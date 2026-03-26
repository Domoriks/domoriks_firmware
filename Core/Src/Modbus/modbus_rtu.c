/*
 * File:   modbus_rtu.c
 * Author: Jorik Wittevrongel
 *
 * Created on October 5 2023
 */

#include "Modbus/modbus_rtu.h"
#include "main.h"

uint16_t calculate_crc(const uint8_t* data, size_t length) {  //it is possible to make this faster with lookup tables
	uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        for (uint8_t j = 8; j > 0; j--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
                crc >>= 1;
        }
    }
    return (uint16_t)(crc << 8) | (uint16_t)(crc >> 8);
}

uint8_t decode_modbus_rtu(uint8_t* message, size_t length, ModbusMessage* decoded) {
	//check length slave_addr, fucn_code, 1 data, 2 crc min.
	if (length <= 5)
		return 255; //Error

	if (message[0] == 0)
		return 255; //Error

	if (message[0] != MODBUS_ID && message[0] != 250) {
		return message[0]; //Error
	}

	//check CRC
    uint16_t crc_calc = calculate_crc(message, length - 2); //length with crc
    uint16_t crc_recv = (uint16_t)(message[length - 2] << 8) | (uint16_t)(message[length - 1]);
    if (crc_recv != crc_calc)
        return 1; //Error

    //Parse message
    decoded->slave_address = message[0];
    decoded->function_code = message[1];

    for (uint8_t i = 2; i < (length - 2); i++) { //i=2 exculdes slave_addr and func ; - 2 exculde CRC 
        decoded->data[i - 2] = message[i];
        decoded->data_length = (uint8_t)(i - 1);
    }

    //Return
    return 0;  // Success
}

uint8_t encode_modbus_rtu(uint8_t* encoded, size_t* length, ModbusMessage* message) {

	encoded[0] = message->slave_address;
    encoded[1] = message->function_code;

    *length = 4; //address + fnction_code + crc

    for (uint8_t i = 2; i < (message->data_length + 2); i++) {
        encoded[i] = message->data[i - 2];
        *length = *length + 1;
    }

    //calc CRC
    uint16_t crc_calc = calculate_crc(encoded, *length - 2); //length with crc
    encoded[*length - 1] = crc_calc;
    encoded[*length - 2] = crc_calc >> 8;

    //Return
    return 0;  // Success
}

//MOVE to seperate lib/file. no outputs here
//uint8_t print_modbus_rtu(ModbusMessage* message) {
//    uint8_t encoded[100];
//    size_t length;
//
//    encode_modbus_rtu(encoded, &length, message);
//    for (int i = 0; i < length; i++)
//        printf("0x%02X, ", encoded[i]);
//    printf("\n");
//
//    return 0;
//}

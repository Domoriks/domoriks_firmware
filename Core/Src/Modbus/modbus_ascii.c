/*
 * File:   modbus_decode.c
 * Author: Jorik Wittevrongel
 *
 * Created on December 20 2022
 */

#include "Modbus/modbus_ascii.h"

uint8_t calculate_lrc(const uint8_t* data, size_t length)
{
    uint8_t lrc = 0;
    for (size_t i = 0; i < length; i++)
    {
        lrc += data[i];
    }
    return (uint8_t)-lrc;
}

// Decode Modbus ASCII message
uint8_t decode_modbus_ascii(char* message, ModbusMessage* decoded) {
    // Check for start and end bytes
    if (message[0] != ':' || message[strlen(message) - 1] != '\r') {
        return -1;  // Invalid message
    }

    // Convert message from ASCII hex to binary without start and stop
    size_t length = (strlen(message) / 2) - 1;
    uint8_t bin_message[length];

    for (uint8_t i = 0; i < length; i++)
    {
        sscanf(message + 1 + (i * 2), "%2x", (unsigned int *)&bin_message[i]); // + 1 excludes start (:)
    }

    //check LRC
    if (bin_message[length - 1] != calculate_lrc(bin_message, length - 1)) {
        return 1; //Error
    }

    //Parse message
    decoded->slave_address = bin_message[0];
    decoded->function_code = bin_message[1];

    for (uint8_t i = 2; i < (length - 1); i++) { //i=2 exculdes slave_addr and func ; - 1exculde LRC 
        decoded->data[i - 2] = bin_message[i];
        decoded->data_length = (uint8_t)(i - 1);
    }

    //Return
    return 0;  // Success
}

uint8_t encode_modbus_ascii(char* encoded, size_t* lenght, ModbusMessage* message) {
    int i = 0;
    encoded[i++] = ':';
    sprintf(&encoded[i++], "%02X", message->slave_address);
    i++;
    sprintf(&encoded[i++], "%02X", message->function_code);
    i++;
    for (int j = 0; j < message->data_length; j++) {
        sprintf(&encoded[i++], "%02X", message->data[j]);
        i++;
    }
    sprintf(&encoded[i++], "%02X", calculate_lrc((uint8_t*)message, message->data_length + 2));
    i++;
    encoded[i++] = '\r';
    encoded[i++] = '\n';
    *lenght = 1;
    encoded[i] = 0x00;

    return 0;
}

//MOVE to seperate lib/file. no outputs here
//uint8_t print_modbus_ascii(ModbusMessage* message) {
//    char encoded[100];
//    size_t length = 0;
//    if (encode_modbus_ascii(encoded, &length, message) == 0)
//    	printf("%s", encoded);
//    return 0;
//}

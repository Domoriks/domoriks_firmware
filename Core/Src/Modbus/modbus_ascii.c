/*
 * File:   modbus_decode.c
 * Author: Jorik Wittevrongel
 *
 * Created on December 20 2022
 */

#include "Modbus/modbus_ascii.h"
#include <ctype.h>

uint8_t calculate_lrc(const uint8_t* data, size_t length)
{
    uint8_t lrc = 0;
    for (size_t i = 0; i < length; i++)
    {
        lrc += data[i];
    }
    return (uint8_t)-lrc;
}

/* Maximum binary message bytes we support for ASCII conversion. This covers
 * address(1) + function(1) + data(100) + lrc(1) = 103 (safe margin to 128).
 */
#define MODBUS_ASCII_MAX_BIN 128

static int hex_val(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

// Decode Modbus ASCII message
uint8_t decode_modbus_ascii(char* message, ModbusMessage* decoded) {
    if (!message || !decoded) return 2;

    size_t msg_len = strlen(message);
    if (msg_len < 7) return 2; // too short to be valid (":" + 3 bytes -> 6 hex chars + CR)

    if (message[0] != ':') return 2;

    // Accept both CRLF and lone CR terminations
    size_t end = 0;
    if (msg_len >= 2 && message[msg_len - 2] == '\r' && message[msg_len - 1] == '\n') {
        end = msg_len - 2; // exclude CRLF
    } else if (message[msg_len - 1] == '\r') {
        end = msg_len - 1; // exclude CR
    } else {
        return 2; // invalid terminator
    }

    size_t payload_chars = end - 1; // exclude starting ':'
    if ((payload_chars % 2) != 0) return 2; // must be even number of hex chars

    size_t bin_len = payload_chars / 2;
    if (bin_len < 3 || bin_len > MODBUS_ASCII_MAX_BIN) return 2; // need at least addr+func+lrc

    uint8_t bin_message[MODBUS_ASCII_MAX_BIN];

    // Parse ASCII hex pairs into binary
    for (size_t i = 0; i < bin_len; i++) {
        char hi = message[1 + (i * 2)];
        char lo = message[1 + (i * 2) + 1];
        int hi_val = hex_val(hi);
        int lo_val = hex_val(lo);
        if (hi_val < 0 || lo_val < 0) return 2; // invalid hex
        bin_message[i] = (uint8_t)((hi_val << 4) | lo_val);
    }

    // Validate LRC
    if (bin_message[bin_len - 1] != calculate_lrc(bin_message, bin_len - 1)) {
        return 1; // LRC error
    }

    // Fill decoded structure
    decoded->slave_address = bin_message[0];
    decoded->function_code = bin_message[1];
    size_t data_len = bin_len - 3; // minus addr, func, lrc
    if (data_len > sizeof(decoded->data)) return 2;
    decoded->data_length = (uint8_t)data_len;
    if (data_len > 0) {
        memcpy(decoded->data, &bin_message[2], data_len);
    }

    return 0;  // Success
}

uint8_t encode_modbus_ascii(char* encoded, size_t* length, ModbusMessage* message) {
    if (!encoded || !length || !message) return 1;
    if (message->data_length > sizeof(message->data)) return 1;

    const char hex[] = "0123456789ABCDEF";
    size_t idx = 0;

    encoded[idx++] = ':';

    // Helper to append a byte as two hex chars
    auto_append_byte:
    ;

    // Append slave address
    encoded[idx++] = hex[(message->slave_address >> 4) & 0x0F];
    encoded[idx++] = hex[message->slave_address & 0x0F];

    // Append function code
    encoded[idx++] = hex[(message->function_code >> 4) & 0x0F];
    encoded[idx++] = hex[message->function_code & 0x0F];

    // Append data bytes
    for (uint8_t j = 0; j < message->data_length; j++) {
        uint8_t b = message->data[j];
        encoded[idx++] = hex[(b >> 4) & 0x0F];
        encoded[idx++] = hex[b & 0x0F];
    }

    // Compute LRC over address + function + data
    uint8_t lrc = 0;
    lrc += message->slave_address;
    lrc += message->function_code;
    for (uint8_t j = 0; j < message->data_length; j++) {
        lrc += message->data[j];
    }
    lrc = (uint8_t)-lrc;

    // Append LRC
    encoded[idx++] = hex[(lrc >> 4) & 0x0F];
    encoded[idx++] = hex[lrc & 0x0F];

    // Terminate with CRLF
    encoded[idx++] = '\r';
    encoded[idx++] = '\n';
    encoded[idx] = '\0';

    *length = idx;
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

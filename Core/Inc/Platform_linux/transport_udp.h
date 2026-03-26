/*
 * File:   transport_udp.h
 * Author: Jorik Wittevrongel
 *
 * UDP transport layer for Modbus simulation
 * Abstracts the communication interface - allows same code to run on hardware (UART) or simulation (UDP)
 */

#ifndef TRANSPORT_UDP_H
#define TRANSPORT_UDP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set the node ID for filtering (optional)
 * 
 * @param node_id The node ID (1-255)
 */
void transport_set_node_id(uint8_t node_id);

/**
 * @brief Initialize the UDP transport layer
 * 
 * Sets up UDP socket, joins multicast group, configures non-blocking mode
 * Must be called before transport_send() or transport_poll()
 */
void transport_init(void);

/**
 * @brief Send data over UDP transport
 * 
 * @param data Pointer to data buffer to send
 * @param len Length of data in bytes
 */
void transport_send(uint8_t* data, uint16_t len);

/**
 * @brief Poll for incoming data on UDP transport
 * 
 * Non-blocking. Updates uart_rxBuffer, rxDataLen, and new_rxdata if data received.
 * Should be called regularly in main loop.
 */
void transport_poll(void);

#ifdef __cplusplus
}
#endif

#endif /* TRANSPORT_UDP_H */
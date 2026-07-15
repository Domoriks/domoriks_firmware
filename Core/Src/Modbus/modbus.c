/*
 * File:   modbus.c
 * Author: Jorik Wittevrongel
 *
 * Created on 30 December 2023
 */

/*
 * The modbus implementations is designed to write directly to the Output/Actions Structs
 * This way it is possible to make the main program as compact as possible
 * Maybe I can implement pointers to the structs to make it easier to program / read
 */

#include "main.h"
#include "timer.h"

#include "device_config.h"

#include "Actions/actions.h"
#include "IO/outputs.h"

#include "Modbus/modbus.h"
#include "Modbus/modbus_rtu.h"
#include "Modbus/modbusm.h"
#include "Modbus/modbusm_handler.h"

#include "Flash/flash.h"

#include <stdio.h>
#include <stdlib.h>

uint8_t uart_txBuffer[UART_BUFFER_SIZE] = {0};
uint16_t txDataLen = 0;

ModbusMessage recieved_message;
size_t length;

uint8_t waiting4response = 0;
uint32_t timer_wait4response = 0;

uint32_t resend_timer;
uint8_t resend_count = 0;

ModbusMessage send_message;

void recieve() {
	memset(uart_txBuffer, 0, UART_BUFFER_SIZE);
	if (decode_modbus_rtu(uart_rxBuffer, rxDataLen, &recieved_message) == 0) {
		//handle message
		if (modbusm_handle(&recieved_message) != 1) {
			//send reply
			if (recieved_message.slave_address != 250) {
				length = 0;
				encode_modbus_rtu(uart_txBuffer, &length, &recieved_message);
				txDataLen = length;
				/* Start interrupt-driven transmit and set RS485 driver to TX.
				 * The TX-complete callback will switch RS485 back to RX. */
				HAL_GPIO_WritePin(W_RS485_GPIO_Port, W_RS485_Pin, GPIO_PIN_SET);
				HAL_UART_Transmit_IT(&huart1, uart_txBuffer, txDataLen);
			}
		}
	} else {
		//message not modbus rtu;
	}
	//reset for new message
	new_rxdata = 0;
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rxBuffer, UART_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT); // Disable half-transfer interrupt if not needed
	memset(uart_rxBuffer, 0, UART_BUFFER_SIZE);
	rxDataLen = 0;
}

void send(){
	for (int i = 0; i < INPUTS_SIZE; i++) {
		EventAction* event = NULL;

		if (inputActions[i].singlePress.send) {
			event = &inputActions[i].singlePress;
		} else if (inputActions[i].doublePress.send) {
			event = &inputActions[i].doublePress;
		} else if (inputActions[i].longPress.send) {
			event = &inputActions[i].longPress;
		} else if (inputActions[i].switchOn.send) {
			event = &inputActions[i].switchOn;
		} else if (inputActions[i].switchOff.send) {
			event = &inputActions[i].switchOff;
		}

		if (event != NULL) {
			if (event->send == 1) {
				if (event->delayAction != nop && event->delay != 0){
					event->send = 2;
				} else if (event->extraEventId != 0){
					event->send = 3;
				} else {
					event->send = 0;
				}

				uint16_t coil_data = 0x1111; //invalid data
				// Handle the action
				if (event->action == toggle) {
					coil_data = 0x5555;				//Domoriks only less data on the bus + faster execution
				} else if (event->action == on) {
					coil_data = 0xFF00;
				} else if (event->action == off) {
					coil_data = 0x0000;
				} else {

				}

				if (event->action != nop) {
					uint16_t coil_address = event->output;
					// Construct and send a Modbus message to the active address
					send_message.slave_address = event->id;
					send_message.function_code = WRITE_SINGLE_COIL;
					send_message.data_length = 4; // Data length for writing a single coil
					// Set the coil address and value based on your requirements
					send_message.data[0] = (coil_address >> 8) & 0xFF; // High byte of coil address
					send_message.data[1] = coil_address & 0xFF; // Low byte of coil address
					send_message.data[2] = (coil_data >> 8) & 0xFF;
					send_message.data[3] = coil_data & 0xFF;

					// Set RS485 in write mode and transmit the message
					encode_modbus_rtu(uart_txBuffer, &length, &send_message);
					txDataLen = length;
					HAL_GPIO_WritePin(W_RS485_GPIO_Port, W_RS485_Pin, GPIO_PIN_SET);
					HAL_UART_Transmit_IT(&huart1, uart_txBuffer, txDataLen);
					
					resend_timer = TIMER_SET();
					waiting4response = 1;
					return;
				}
			} else if (event->send == 2){
				//send delay action
				send_message.slave_address = event->id;
				send_message.function_code = WRITE_MULTI_REGS;
				uint16_t starting_address = 0x0000;
				send_message.data[0] = (starting_address >> 8) & 0xFF;
				send_message.data[1] = starting_address & 0xFF;
				uint16_t quantity_of_registers = 4;
				send_message.data[2] = (quantity_of_registers >> 8) & 0xFF;
				send_message.data[3] = quantity_of_registers & 0xFF;
				send_message.data[4] = 8; // Number of data bytes

				uint8_t coil_address = event->output;
				uint16_t coil_data = 0x1111; //invalid data
				if (event->delayAction == toggle) {
					coil_data = 0x5555;				//Domoriks only less data on the bus + faster execution
				} else if (event->delayAction == on) {
					coil_data = 0xFF00;
				} else if (event->delayAction == off) {
					coil_data = 0x0000;
				}
				send_message.data[5] = 0x00;
				send_message.data[6] = coil_address;

				send_message.data[7] = (coil_data >> 8) & 0xFF;
				send_message.data[8] = coil_data & 0xFF;

				send_message.data[9] = (event->delay >> 8) & 0xFF;
				send_message.data[10] = event->delay & 0xFF;

				send_message.data[11] = event->pwm; // Brightness
				send_message.data[12] = 0; // Brightness

				send_message.data_length = 13; // Data length (address + quantity + byte count + data)

				// Set RS485 in write mode and transmit the message
				encode_modbus_rtu(uart_txBuffer, &length, &send_message);
				txDataLen = length;
				HAL_GPIO_WritePin(W_RS485_GPIO_Port, W_RS485_Pin, GPIO_PIN_SET);
				HAL_UART_Transmit_IT(&huart1, uart_txBuffer, txDataLen);

				resend_timer = TIMER_SET();
				waiting4response = 1;

				if (event->extraEventId != 0){
					event->send = 3;
				} else {
					event->send = 0;
				}
				return;
			} else if (event->send == 3) {
				//send extra action
				if (event->extraEventId == 255) {
					Flash_ReadInputActions(inputActions); //restore original/start action
					event->send = 0;
				} else {
					copyEventAction(&extraActions[event->extraEventId-1], event);
					if (event->extraEventId == 0) {
						event->extraEventId = 255; // runtime sentinel to restore original chain target
					}
					event->send = 1;
				}
				return;
			} else {
				event->send = 0;
				return;
			}
		}
	}
}


void response() {
	if (new_rxdata) {
		/*
		 * Master-mode reply correlation.
		 * A slave's reply echoes the TARGET's slave_address + function_code,
		 * which decode_modbus_rtu() rejects because it filters on MODBUS_ID.
		 * Match the raw frame against our pending send_message and, on a
		 * CRC-valid echo, clear waiting4response so the state machine can
		 * advance immediately instead of burning the full resend timeout.
		 */
		if (waiting4response && rxDataLen > 4 &&
			uart_rxBuffer[0] == send_message.slave_address &&
			uart_rxBuffer[1] == send_message.function_code) {
			uint16_t crc_calc = calculate_crc(uart_rxBuffer, rxDataLen - 2);
			uint16_t crc_recv = (uint16_t)(uart_rxBuffer[rxDataLen - 2] << 8) | (uint16_t)(uart_rxBuffer[rxDataLen - 1]);
			if (crc_recv == crc_calc) {
				waiting4response = 0;
				resend_count = 0;
			}
		} else if (decode_modbus_rtu(uart_rxBuffer, rxDataLen, &recieved_message) == 0) {
			if (modbusm_handle(&recieved_message) != 1) {
				//send reply
				if (recieved_message.slave_address != 250) {
					length = 0;
					encode_modbus_rtu(uart_txBuffer, &length, &recieved_message);
					txDataLen = length;
					HAL_GPIO_WritePin(W_RS485_GPIO_Port, W_RS485_Pin, GPIO_PIN_SET);
					HAL_UART_Transmit_IT(&huart1, uart_txBuffer, txDataLen);
				}
			}
		}
		//reset for new message
		new_rxdata = 0;
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rxBuffer, UART_BUFFER_SIZE);
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT); // Disable half-transfer interrupt if not needed
		memset(uart_rxBuffer, 0, UART_BUFFER_SIZE);
		rxDataLen = 0;
	}

	if (TIMER_ELAPSED_MS(resend_timer, 50) && waiting4response) {  //resend
		// Resend logic commented out - keeping as per original
		// HAL_GPIO_WritePin(W_RS485_GPIO_Port, W_RS485_Pin, GPIO_PIN_SET);
		// HAL_UART_Transmit(&huart1, uart_txBuffer, txDataLen, 2 * length * UART_BYTE_TIME_MS());
		// HAL_GPIO_WritePin(W_RS485_GPIO_Port, W_RS485_Pin, GPIO_PIN_RESET);
		resend_timer = TIMER_SET();
		waiting4response = 1;
		resend_count++;
	}

	if (resend_count == 5) {
		waiting4response = 0;
		resend_count = 0;
	}
}

void modbus() {
	if (waiting4response)
	{
		response();
	}
	else if (new_rxdata)
	{
		recieve();
	}
	else
	{
		send();
	}
}
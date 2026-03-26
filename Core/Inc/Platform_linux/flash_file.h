/*
 * flash_file.h
 *
 * Linux simulation header for embedded flash interface
 *
 * Author: Adapted from Jorik Wittevrongel
 */

#ifndef FLASH_FILE_H
#define FLASH_FILE_H

#include <stdint.h>

#ifdef SIMULATION

// Status definitions
#define HAL_OK    0
#define HAL_ERROR 1
typedef int HAL_StatusTypeDef;

// Flash addresses and sizes
#define FLASH_INPUTS_ADDR        0x0000
#define FLASH_INPUTS_SIZE        128
#define FLASH_INPUTACTIONS_ADDR  0x0080
#define FLASH_INPUTACTIONS_SIZE  128
#define FLASH_EXTRAACTIONS_ADDR  0x0100
#define FLASH_EXTRAACTIONS_SIZE  128
#define FLASH_OUTPUTS_ADDR       0x0180
#define FLASH_OUTPUTS_SIZE       128

// Data type definitions
typedef struct { uint8_t data[16]; } Input;
typedef struct { uint8_t data[16]; } InputAction;
typedef struct { uint8_t data[16]; } EventAction;
typedef struct { uint8_t data[16]; } Output;

// Node selection
void flash_set_node(uint8_t nodeId);

// Flash operations
HAL_StatusTypeDef Flash_Erase(uint32_t addr, uint32_t size);
HAL_StatusTypeDef Flash_Write(uint32_t addr, void *data, uint32_t size);
HAL_StatusTypeDef Flash_Read(uint32_t addr, void *data, uint32_t size);

// Wrapper functions for structured data
HAL_StatusTypeDef Flash_WriteInputs(Input *inputs);
HAL_StatusTypeDef Flash_ReadInputs(Input *inputs);

HAL_StatusTypeDef Flash_WriteInputActions(InputAction *inputActions);
HAL_StatusTypeDef Flash_ReadInputActions(InputAction *inputActions);

HAL_StatusTypeDef Flash_WriteExtraActions(EventAction *extraActions);
HAL_StatusTypeDef Flash_ReadExtraActions(EventAction *extraActions);

HAL_StatusTypeDef Flash_WriteOutputs(Output *outputs);
HAL_StatusTypeDef Flash_ReadOutputs(Output *outputs);

#endif // SIMULATION

#endif // FLASH_FILE_H

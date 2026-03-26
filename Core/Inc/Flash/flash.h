/*
 * flash.h
 *
 *  Created on: Jul 16, 2024
 *      Author: Jorik Wittevrongel
 */

#ifndef INC_FLASH_FLASH_H_
#define INC_FLASH_FLASH_H_

#include "device_config.h"
#include "Actions/actions.h"
#include "IO/inputs.h"
#include "IO/outputs.h"
#include "stm32g0xx_hal.h"
#include <string.h>
#include <assert.h>

#define USERDATA_ORIGIN     0x08007000
#define USERDATA_LENGTH     0x800 - 1

// Calculate start addresses based on USERDATA_ORIGIN
#define FLASH_INPUTS_ADDR           USERDATA_ORIGIN
#define FLASH_INPUTS_SIZE           (sizeof(Input) * INPUTS_SIZE)

#define FLASH_INPUTACTIONS_ADDR     (FLASH_INPUTS_ADDR + FLASH_INPUTS_SIZE)
#define FLASH_INPUTACTIONS_SIZE     (sizeof(InputAction) * INPUTS_SIZE)

#define FLASH_EXTRAACTIONS_ADDR     (FLASH_INPUTACTIONS_ADDR + FLASH_INPUTACTIONS_SIZE)
#define FLASH_EXTRAACTIONS_SIZE     (sizeof(EventAction) * INPUTS_SIZE * EXTRA_ACTION_PER_INPUT)

#define FLASH_OUTPUTS_ADDR          (FLASH_EXTRAACTIONS_ADDR + FLASH_EXTRAACTIONS_SIZE)
#define FLASH_OUTPUTS_SIZE          (sizeof(Output) * OUTPUTS_SIZE)

#define TOTAL_SIZE_REQUIRED         (FLASH_INPUTS_SIZE + FLASH_INPUTACTIONS_SIZE + FLASH_EXTRAACTIONS_SIZE + FLASH_OUTPUTS_SIZE)

static_assert(TOTAL_SIZE_REQUIRED <= USERDATA_LENGTH, "Total size of inputs, inputActions, and extraActions exceeds USERDATA_LENGTH!");

HAL_StatusTypeDef Flash_Erase(uint32_t addr, uint32_t size);
HAL_StatusTypeDef Flash_WriteInputs(Input *inputs);
HAL_StatusTypeDef Flash_ReadInputs(Input *inputs);
HAL_StatusTypeDef Flash_WriteInputActions(InputAction *inputActions);
HAL_StatusTypeDef Flash_ReadInputActions(InputAction *inputActions);
HAL_StatusTypeDef Flash_WriteExtraActions(EventAction *extraActions);
HAL_StatusTypeDef Flash_ReadExtraActions(EventAction *extraActions);
HAL_StatusTypeDef Flash_WriteOutputs(Output *outputs);
HAL_StatusTypeDef Flash_ReadOutputs(Output *outputs);

#endif /* INC_FLASH_FLASH_H_ */

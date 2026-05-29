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

typedef struct {
	uint8_t button_type;
	uint8_t value;
	uint8_t min;
	uint8_t max;
	uint8_t changed;
} FlashInputRecord;

typedef struct {
	uint8_t value;
	uint8_t invert;
	uint8_t min;
	uint8_t max;
	uint16_t delay;
	uint8_t delay_value;
	uint32_t startTimer;
} FlashOutputRecord;

#define USERDATA_ORIGIN     0x08007000
#define USERDATA_SIZE       0x800U
#define USERDATA_LENGTH     (USERDATA_SIZE - 1U)

#define FLASH_ALIGN_UP_8(x) (((uint32_t)(x) + 7U) & ~((uint32_t)7U))

// Calculate start addresses based on USERDATA_ORIGIN
#define FLASH_INPUTS_ADDR           USERDATA_ORIGIN
#define FLASH_INPUTS_SIZE           (sizeof(FlashInputRecord) * INPUTS_SIZE)

#define FLASH_INPUTACTIONS_ADDR     FLASH_ALIGN_UP_8(FLASH_INPUTS_ADDR + FLASH_INPUTS_SIZE)
#define FLASH_INPUTACTIONS_SIZE     (sizeof(InputAction) * INPUTS_SIZE)

#define FLASH_EXTRAACTIONS_ADDR     FLASH_ALIGN_UP_8(FLASH_INPUTACTIONS_ADDR + FLASH_INPUTACTIONS_SIZE)
#define FLASH_EXTRAACTIONS_SIZE     (sizeof(EventAction) * INPUTS_SIZE * EXTRA_ACTION_PER_INPUT)

#define FLASH_OUTPUTS_ADDR          FLASH_ALIGN_UP_8(FLASH_EXTRAACTIONS_ADDR + FLASH_EXTRAACTIONS_SIZE)
#define FLASH_OUTPUTS_SIZE          (sizeof(FlashOutputRecord) * OUTPUTS_SIZE)

#define TOTAL_SIZE_REQUIRED         ((FLASH_OUTPUTS_ADDR + FLASH_OUTPUTS_SIZE) - USERDATA_ORIGIN)

static_assert(TOTAL_SIZE_REQUIRED <= USERDATA_SIZE, "Total size of persisted data exceeds USERDATA flash page size!");

HAL_StatusTypeDef Flash_Erase(uint32_t addr, uint32_t size);
HAL_StatusTypeDef Flash_WriteInputs(Input *inputs);
HAL_StatusTypeDef Flash_ReadInputs(Input *inputs);
HAL_StatusTypeDef Flash_WriteInputActions(InputAction *inputActions);
HAL_StatusTypeDef Flash_ReadInputActions(InputAction *inputActions);
HAL_StatusTypeDef Flash_WriteExtraActions(EventAction *extraActions);
HAL_StatusTypeDef Flash_ReadExtraActions(EventAction *extraActions);
HAL_StatusTypeDef Flash_WriteOutputs(Output *outputs);
HAL_StatusTypeDef Flash_ReadOutputs(Output *outputs);
HAL_StatusTypeDef Flash_WriteModbusId(uint8_t modbusId);
HAL_StatusTypeDef Flash_ReadModbusId(uint8_t *modbusId);

#endif /* INC_FLASH_FLASH_H_ */

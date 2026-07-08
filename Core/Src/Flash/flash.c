/*
 * flash.c
 *
 *  Created on: Jul 16, 2024
 *      Author: Jorik Wittevrongel
 */

#include "Flash/flash.h"
#include "main.h"
#include <stdbool.h>
#define FLASH_DOUBLEWORD_SIZE  sizeof(uint64_t)
#define FLASH_DOUBLEWORD_MASK  (~((uint32_t)FLASH_DOUBLEWORD_SIZE - 1UL))

static bool Flash_IsErasedValue(const void *valuePtr, size_t valueSize)
{
    const uint8_t *bytes = (const uint8_t *)valuePtr;
    for (size_t i = 0; i < valueSize; i++) {
        if (bytes[i] != 0xFFU) {
            return false;
        }
    }
    return true;
}

static bool Flash_IsValidAction(Action action)
{
    return (action >= nop) && (action <= off);
}

static uint32_t Flash_GetAlignedDoubleWordAddress(uint32_t addr)
{
    return addr & FLASH_DOUBLEWORD_MASK;
}

static uint32_t Flash_GetPageAddress(uint32_t addr)
{
    return addr - (addr % FLASH_PAGE_SIZE);
}

static void Flash_CopyEventAction(EventAction *dst, const EventAction *src)
{
    dst->action = src->action;
    dst->delayAction = src->delayAction;
    dst->delay = src->delay;
    dst->pwm = src->pwm;
    dst->id = src->id;
    dst->output = src->output;
    dst->send = src->send;
    dst->extraEventId = src->extraEventId;
}

static void Flash_RestoreEventAction(EventAction *dst, const EventAction *src)
{
    if (!Flash_IsErasedValue(&src->action, sizeof(src->action)) && Flash_IsValidAction(src->action)) {
        dst->action = src->action;
    }
    if (!Flash_IsErasedValue(&src->delayAction, sizeof(src->delayAction)) && Flash_IsValidAction(src->delayAction)) {
        dst->delayAction = src->delayAction;
    }
    if (!Flash_IsErasedValue(&src->delay, sizeof(src->delay))) {
        dst->delay = src->delay;
    }
    if (!Flash_IsErasedValue(&src->pwm, sizeof(src->pwm))) {
        dst->pwm = src->pwm;
    }
    if (!Flash_IsErasedValue(&src->id, sizeof(src->id))) {
        dst->id = src->id;
    }
    if (!Flash_IsErasedValue(&src->output, sizeof(src->output))) {
        dst->output = src->output;
    }
    if (!Flash_IsErasedValue(&src->send, sizeof(src->send))) {
        dst->send = src->send;
    }
    if (!Flash_IsErasedValue(&src->extraEventId, sizeof(src->extraEventId))) {
        dst->extraEventId = src->extraEventId;
    }
}

static uint32_t GetPage(uint32_t Addr)
{
    return (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
}

HAL_StatusTypeDef Flash_Erase(uint32_t addr, uint32_t size) {
    HAL_StatusTypeDef status = HAL_OK;

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t firstPage = GetPage(addr);
    uint32_t nbOfPages = GetPage(addr + size) - firstPage + 1;
    uint32_t pageError = 0;

    /* Fill EraseInit structure */
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Page        = firstPage;
    EraseInitStruct.NbPages     = nbOfPages;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return HAL_ERROR; // Handle error
    }

    HAL_FLASH_Lock();
    return status;
}

// Generic function to write data to Flash in chunks
HAL_StatusTypeDef Flash_Write(uint32_t addr, void *data, uint32_t size) {
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t remainingSize = size;
    uint8_t *pData = (uint8_t *)data;

    if (pData == NULL) {
        return HAL_ERROR;
    }

    /* nothing to do */
    if (remainingSize == 0) {
        return HAL_OK;
    }

    /* Address must be double-word (64-bit) aligned for FLASH_TYPEPROGRAM_DOUBLEWORD */
    if ((addr % sizeof(uint64_t)) != 0) {
        return HAL_ERROR;
    }

    HAL_FLASH_Unlock();

    const size_t DW_SIZE = sizeof(uint64_t);

    while (remainingSize > 0) {
        /* Default all bytes to 0xFF (erased state) and overwrite with actual data bytes */
        uint64_t data64 = 0xFFFFFFFFFFFFFFFFULL;
        size_t writeBytes = (remainingSize < DW_SIZE) ? remainingSize : DW_SIZE;

        /* Prepare 64-bit data from pData (little-endian order for STM32) */
        for (size_t i = 0; i < writeBytes; i++) {
            /* clear this byte then set it to the provided value */
            data64 &= ~(((uint64_t)0xFF) << (i * 8));
            data64 |= ((uint64_t)pData[i]) << (i * 8);
        }

        /* Program double word (64 bits) at a time; flash address advances by DW_SIZE */
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, data64);
        if (status != HAL_OK) {
            break;
        }

        addr += DW_SIZE;
        pData += writeBytes; /* advance source by actual bytes consumed */
        remainingSize -= (uint32_t)writeBytes;
    }

    HAL_FLASH_Lock();
    return status;
}


// Function to read data from flash
HAL_StatusTypeDef Flash_Read(uint32_t addr, void *data, uint32_t size) {
    HAL_StatusTypeDef status = HAL_OK;
    if (data == NULL) {
        return HAL_ERROR;
    }
    memcpy(data, (void *)addr, size);
    return status;
}

HAL_StatusTypeDef Flash_WriteModbusId(uint8_t modbusId)
{
    if (modbusId < 1U || modbusId > 247U) {
        return HAL_ERROR;
    }

    uint8_t current_id = *(uint8_t *)MODBUS_ID_ADDRESS;
    if (current_id == modbusId) {
        return HAL_OK;
    }

    uint32_t pageAddr = Flash_GetPageAddress(MODBUS_ID_ADDRESS);
    if (Flash_Erase(pageAddr, FLASH_PAGE_SIZE - 1U) != HAL_OK) {
        return HAL_ERROR;
    }

    uint32_t writeAddr = Flash_GetAlignedDoubleWordAddress(MODBUS_ID_ADDRESS);
    uint32_t byteOffset = MODBUS_ID_ADDRESS - writeAddr;
    uint64_t data64 = 0xFFFFFFFFFFFFFFFFULL;

    data64 &= ~(((uint64_t)0xFFU) << (byteOffset * 8U));
    data64 |= ((uint64_t)modbusId) << (byteOffset * 8U);

    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, writeAddr, data64);
    HAL_FLASH_Lock();

    return status;
}

HAL_StatusTypeDef Flash_ReadModbusId(uint8_t *modbusId)
{
    if (modbusId == NULL) {
        return HAL_ERROR;
    }

    *modbusId = *(uint8_t *)MODBUS_ID_ADDRESS;
    return HAL_OK;
}

// Write and read functions for inputs
HAL_StatusTypeDef Flash_WriteInputs(Input *inputs) {
    FlashInputRecord records[INPUTS_SIZE];

    for (uint32_t i = 0; i < INPUTS_SIZE; i++) {
        records[i].button_type = inputs[i].param.button_type;
        records[i].value = inputs[i].param.value;
        records[i].min = inputs[i].param.min;
        records[i].max = inputs[i].param.max;
        records[i].changed = inputs[i].param.changed;
    }

    return Flash_Write(FLASH_INPUTS_ADDR, (void *)records, FLASH_INPUTS_SIZE);
}

HAL_StatusTypeDef Flash_ReadInputs(Input *inputs) {
    FlashInputRecord records[INPUTS_SIZE];
    HAL_StatusTypeDef status = Flash_Read(FLASH_INPUTS_ADDR, (void *)records, FLASH_INPUTS_SIZE);
    if (status != HAL_OK) {
        return status;
    }

    for (uint32_t i = 0; i < INPUTS_SIZE; i++) {
        /* Keep runtime pin/port/callback bindings; restore only mutable state. */
        if (records[i].button_type != 0xFFU) {
            inputs[i].param.button_type = records[i].button_type;
        }
        if (records[i].value != 0xFFU) {
            inputs[i].param.value = records[i].value;
        }
        if (records[i].min != 0xFFU) {
            inputs[i].param.min = records[i].min;
        }
        if (records[i].max != 0xFFU) {
            inputs[i].param.max = records[i].max;
        }
        if (records[i].changed != 0xFFU) {
            inputs[i].param.changed = records[i].changed;
        }
    }

    return HAL_OK;
}

// Write and read functions for inputActions
HAL_StatusTypeDef Flash_WriteInputActions(InputAction *inputActions) {
    InputAction records[INPUTS_SIZE];

    memset(records, 0xFF, sizeof(records));

    for (uint32_t i = 0; i < INPUTS_SIZE; i++) {
        Flash_CopyEventAction(&records[i].singlePress, &inputActions[i].singlePress);
        Flash_CopyEventAction(&records[i].doublePress, &inputActions[i].doublePress);
        Flash_CopyEventAction(&records[i].longPress, &inputActions[i].longPress);
        Flash_CopyEventAction(&records[i].switchOn, &inputActions[i].switchOn);
        Flash_CopyEventAction(&records[i].switchOff, &inputActions[i].switchOff);
    }

    return Flash_Write(FLASH_INPUTACTIONS_ADDR, (void *)records, FLASH_INPUTACTIONS_SIZE);
}

HAL_StatusTypeDef Flash_ReadInputActions(InputAction *inputActions) {
    InputAction records[INPUTS_SIZE];
    HAL_StatusTypeDef status = Flash_Read(FLASH_INPUTACTIONS_ADDR, (void *)records, FLASH_INPUTACTIONS_SIZE);
    if (status != HAL_OK) {
        return status;
    }

    for (uint32_t i = 0; i < INPUTS_SIZE; i++) {
        Flash_RestoreEventAction(&inputActions[i].singlePress, &records[i].singlePress);
        Flash_RestoreEventAction(&inputActions[i].doublePress, &records[i].doublePress);
        Flash_RestoreEventAction(&inputActions[i].longPress, &records[i].longPress);
        Flash_RestoreEventAction(&inputActions[i].switchOn, &records[i].switchOn);
        Flash_RestoreEventAction(&inputActions[i].switchOff, &records[i].switchOff);
    }

    return HAL_OK;
}

// Write and read functions for extraActions
HAL_StatusTypeDef Flash_WriteExtraActions(EventAction *extraActions) {
    EventAction records[INPUTS_SIZE * EXTRA_ACTION_PER_INPUT];

    memset(records, 0xFF, sizeof(records));

    for (uint32_t i = 0; i < (INPUTS_SIZE * EXTRA_ACTION_PER_INPUT); i++) {
        Flash_CopyEventAction(&records[i], &extraActions[i]);
    }

    return Flash_Write(FLASH_EXTRAACTIONS_ADDR, (void *)records, FLASH_EXTRAACTIONS_SIZE);
}

HAL_StatusTypeDef Flash_ReadExtraActions(EventAction *extraActions) {
    EventAction records[INPUTS_SIZE * EXTRA_ACTION_PER_INPUT];
    HAL_StatusTypeDef status = Flash_Read(FLASH_EXTRAACTIONS_ADDR, (void *)records, FLASH_EXTRAACTIONS_SIZE);
    if (status != HAL_OK) {
        return status;
    }

    for (uint32_t i = 0; i < (INPUTS_SIZE * EXTRA_ACTION_PER_INPUT); i++) {
        Flash_RestoreEventAction(&extraActions[i], &records[i]);
    }

    return HAL_OK;
}

// Write and read functions for outputs
HAL_StatusTypeDef Flash_WriteOutputs(Output *outputs) {
    FlashOutputRecord records[OUTPUTS_SIZE];

    for (uint32_t i = 0; i < OUTPUTS_SIZE; i++) {
        records[i].value = outputs[i].param.value;
        records[i].invert = outputs[i].param.invert;
        records[i].min = outputs[i].param.min;
        records[i].max = outputs[i].param.max;
        records[i].delay = outputs[i].param.delay;
        records[i].delay_value = outputs[i].param.delay_value;
        records[i].startTimer = outputs[i].param.startTimer;
    }

    return Flash_Write(FLASH_OUTPUTS_ADDR, (void *)records, FLASH_OUTPUTS_SIZE);
}

HAL_StatusTypeDef Flash_ReadOutputs(Output *outputs) {
    FlashOutputRecord records[OUTPUTS_SIZE];
    HAL_StatusTypeDef status = Flash_Read(FLASH_OUTPUTS_ADDR, (void *)records, FLASH_OUTPUTS_SIZE);
    if (status != HAL_OK) {
        return status;
    }

    for (uint32_t i = 0; i < OUTPUTS_SIZE; i++) {
        if (records[i].value != 0xFFU) {
            outputs[i].param.value = records[i].value;
        }
        if (records[i].invert != 0xFFU) {
            outputs[i].param.invert = records[i].invert;
        }
        if (records[i].min != 0xFFU) {
            outputs[i].param.min = records[i].min;
        }
        if (records[i].max != 0xFFU) {
            outputs[i].param.max = records[i].max;
        }
        if (records[i].delay != 0xFFFFU) {
            outputs[i].param.delay = records[i].delay;
        }
        if (records[i].delay_value != 0xFFU) {
            outputs[i].param.delay_value = records[i].delay_value;
        }
        if (records[i].startTimer != 0xFFFFFFFFUL) {
            outputs[i].param.startTimer = records[i].startTimer;
        }
    }

    return HAL_OK;
}

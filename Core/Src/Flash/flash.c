/*
 * flash.c
 *
 *  Created on: Jul 16, 2024
 *      Author: Jorik Wittevrongel
 */

#ifndef SIMULATION

#include "Flash/flash.h"

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
    memcpy(data, (void *)addr, size);
    return status;
}

// Write and read functions for inputs
HAL_StatusTypeDef Flash_WriteInputs(Input *inputs) {
    return Flash_Write(FLASH_INPUTS_ADDR, (void *)inputs, FLASH_INPUTS_SIZE);
}

HAL_StatusTypeDef Flash_ReadInputs(Input *inputs) {
    return Flash_Read(FLASH_INPUTS_ADDR, (void *)inputs, FLASH_INPUTS_SIZE);
}

// Write and read functions for inputActions
HAL_StatusTypeDef Flash_WriteInputActions(InputAction *inputActions) {
    return Flash_Write(FLASH_INPUTACTIONS_ADDR, (void *)inputActions, FLASH_INPUTACTIONS_SIZE);
}

HAL_StatusTypeDef Flash_ReadInputActions(InputAction *inputActions) {
    return Flash_Read(FLASH_INPUTACTIONS_ADDR, (void *)inputActions, FLASH_INPUTACTIONS_SIZE);
}

// Write and read functions for extraActions
HAL_StatusTypeDef Flash_WriteExtraActions(EventAction *extraActions) {
    return Flash_Write(FLASH_EXTRAACTIONS_ADDR, (void *)extraActions, FLASH_EXTRAACTIONS_SIZE);
}

HAL_StatusTypeDef Flash_ReadExtraActions(EventAction *extraActions) {
    return Flash_Read(FLASH_EXTRAACTIONS_ADDR, (void *)extraActions, FLASH_EXTRAACTIONS_SIZE);
}

// Write and read functions for outputs
HAL_StatusTypeDef Flash_WriteOutputs(Output *outputs) {
    return Flash_Write(FLASH_OUTPUTS_ADDR, (void *)outputs, FLASH_OUTPUTS_SIZE);
}

HAL_StatusTypeDef Flash_ReadOutputs(Output *outputs) {
    return Flash_Read(FLASH_OUTPUTS_ADDR, (void *)outputs, FLASH_OUTPUTS_SIZE);
}

#endif
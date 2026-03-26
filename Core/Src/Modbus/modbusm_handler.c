/*
 * File:   modbusm_handler.c
 * Author: Jorik Wittevrongel
 *
 * Created on October 5 2023
 */

#include "Modbus/modbusm_handler.h"
#include "Actions/actions.h"
#include "Flash/flash.h"
#include "IO/outputs.h"
#include "timer.h"
#include "main.h"
 /*

make functionhandler template with switch case struct for all modbus functions

01 (0x01) Read Coils
02 (0x02) Read Discrete Inputs
03 (0x03) Read Holding Registers
04 (0x04) Read Input Registers
05 (0x05) Write Single Coil
06 (0x06) Write Single Register
*08 (0x08) Diagnostics (Serial Line only)
*11 (0x0B) Get Comm Event Counter (Serial Line only)
15 (0x0F) Write Multiple Coils
16 (0x10) Write Multiple Registers
*17 (0x11) Report Server ID (Serial Line only)
*22 (0x16) Mask Write Register
*23 (0x17) Read/Write Multiple Registers
*43 / 14 (0x2B / 0x0E) Read Device Identification

 */

uint8_t mbCoilsArray[1] = {0x00};
uint8_t mbInputsArray[1];
uint16_t mbHRegArray[50];
uint16_t mbIRegArray[1];

uint8_t* modbusCoils = mbCoilsArray;
uint8_t* modbusInputs = mbInputsArray;
uint16_t* modbusHReg = mbHRegArray;
uint16_t* modbusIReg = mbIRegArray;

uint8_t new_delay_action = 0;
uint8_t new_action_update = 0;

uint8_t modbusm_handle(ModbusMessage* message) {
    //check if device ID matches //done in rtu decode
//    if (message->slave_address != DEVICE_ID) {
//        message = NULL;
//        return ID_MISMATCH;
//    }

    //handle message
    switch (message->function_code) {
    case READ_COILS:
        //parse
        uint16_t first_coil = 0;
        uint16_t amount_coils = 0;
        if (message->data_length == 4) {
            first_coil = (message->data[0] << 8 | message->data[1]);
            amount_coils = (message->data[2] << 8 | message->data[3]);
        }
        else {
            message = NULL;
            return INVALID_DATA_LENGHT;
        }

        //reply
        message->slave_address = MODBUS_ID;
        message->function_code = READ_COILS;
        message->data_length = amount_coils % 8 ? 2 + (amount_coils / 8) : 1 + (amount_coils / 8);
        message->data[0] = message->data_length - 1;
        for (int i = 1; i < message->data_length; i++) {
            uint8_t currentB = *(modbusCoils + ((first_coil / 8) + (i - 1)));
            uint8_t nextB = *(modbusCoils + ((first_coil / 8) + (i)));
            *(message->data + i) = (nextB << (8 - (first_coil % 8)) | currentB >> (first_coil % 8));
        }
        break;
    case READ_DISC_INPUTS:
        //parse
        uint16_t first_input = 0;
        uint16_t amount_inputs = 0;
        if (message->data_length == 4) {
            first_input = (message->data[0] << 8 | message->data[1]);
            amount_inputs = (message->data[2] << 8 | message->data[3]);
        }
        else {
            message = NULL;
            return INVALID_DATA_LENGHT;
        }

        //reply
        message->slave_address = MODBUS_ID;
        message->function_code = READ_DISC_INPUTS;
        message->data_length = amount_inputs % 8 ? 2 + (amount_inputs / 8) : 1 + (amount_inputs / 8);
        *message->data = message->data_length - 1;
        for (int i = 1; i < message->data_length; i++) {
            uint8_t currentB = *(modbusInputs + ((first_input / 8) + (i - 1)));
            uint8_t nextB = *(modbusInputs + ((first_input / 8) + (i)));
            *(message->data + i) = ( ((nextB << 8) - (first_input % 8)) | (currentB >> (first_input % 8)) );
        }
        break;
    case READ_HOLD_REGS:
        //parse
        uint16_t first_hreg = 0;
        uint16_t amount_hregs = 0;
        if (message->data_length == 4) {
            first_hreg = (message->data[0] << 8 | message->data[1]);
            amount_hregs = (message->data[2] << 8 | message->data[3]);
        }
        else {
            message = NULL;
            return INVALID_DATA_LENGHT;
        }

        //reply
        message->slave_address = MODBUS_ID;
        message->function_code = READ_HOLD_REGS;
        message->data_length = 1 + (amount_hregs * 2);
        *message->data = message->data_length - 1;
        for (int i = 1; i < message->data_length; i++) {
            *(message->data + (i * 2)) = *(modbusHReg + i - 1 + first_hreg) >> 8;
            *(message->data + (i * 2) - 1) = *(modbusHReg + i - 1 + first_hreg);
        }
        break;
    case READ_INPUT_REGS:
        //parse
        uint16_t first_ireg = 0;
        uint16_t amount_iregs = 0;
        if (message->data_length == 4) {
            first_ireg = (message->data[0] << 8 | message->data[1]);
            amount_iregs = (message->data[2] << 8 | message->data[3]);
        }
        else {
            //invalid length
            return 2;
        }
        //reply
        message->slave_address = MODBUS_ID;
        message->function_code = READ_INPUT_REGS;
        message->data_length = 1 + (amount_iregs * 2);
        *message->data = message->data_length - 1;
        for (int i = 1; i < message->data_length; i++) {
            *(message->data + (i * 2)) = *(modbusIReg + i - 1 + first_ireg) >> 8;
            *(message->data + (i * 2) - 1) = *(modbusIReg + i - 1 + first_ireg);
        }
        break;
    case WRITE_SINGLE_COIL:
        //parse
        uint16_t coil_adress = 0;
        uint16_t value = 0;
        if (message->data_length == 4) {
            coil_adress = (message->data[0] << 8 | message->data[1]);
            value = (message->data[2] << 8 | message->data[3]);
        }
        else {
            return INVALID_DATA_LENGHT;
        }
        //ADD MAX VALUE ERROR

        //write coil;
        //printf("%02X, %02X\n", coil_adress, value);
        if (value == 0xFF00) {
            modbusCoils[coil_adress / 8] |= (1 << (coil_adress % 8));
        }
        else if (value == 0x0000) {
            modbusCoils[coil_adress / 8] &= ~(1 << (coil_adress % 8));
        }
        else if (value == 0x5555) {  // Unofficial toggle operation domoriks only
            modbusCoils[coil_adress / 8] ^= (1 << (coil_adress % 8));
        }
        else {
            message = NULL;
            return INVALID_COIL_VALUE;
        }
        message = message; //echo message
        break;
    case WRITE_SINGLE_REG:      //   <- Tis only apply for holding regs
        //parse
        uint16_t reg_adress = 0;
        if (message->data_length == 4) {
            reg_adress = (message->data[0] << 8 | message->data[1]);
        }
        else {
            return INVALID_DATA_LENGHT;
        }
        //write regs
        modbusHReg[reg_adress] = message->data[2] << 8 | message->data[3];
        //reply
        message = message; //echo message
        break;

    case WRITE_MULTI_REGS: //   <- Tis only apply for holding regs
    	//TODO test WRITE_MULTI_REGS
        // Parse the request message
        uint16_t start_address = (message->data[0] << 8) | message->data[1];
        uint16_t num_registers = (message->data[2] << 8) | message->data[3];
        uint8_t byte_count = message->data[4];
        if (message->data_length != 5 + byte_count || byte_count != num_registers * 2) {
            return INVALID_DATA_LENGHT;
        }
        // Write the values to the holding registers
        for (uint8_t i = 0; i < num_registers; i++) {
            uint16_t value = (message->data[5 + i * 2] << 8) | message->data[6 + i * 2];
            mbHRegArray[start_address + i] = value;
        }
        new_delay_action = 1;
        // Prepare the response message
        message->data_length = 4;
        message->data[0] = (start_address >> 8) & 0xFF;
        message->data[1] = start_address & 0xFF;
        message->data[2] = (num_registers >> 8) & 0xFF;
        message->data[3] = num_registers & 0xFF;
        break;
    case READ_EXCEPTION_STATUS:
    case DIAGNOSE_SERIAL:
    case COMM_EVENT_COUNT:
    case WRITE_MULTI_COILS:
    case REPORT_SERVER_ID:
    case READ_DEVICE_ID:
    case MASK_WRITE_REG:
    case RW_MULTI_REGS:
    case READ_DEVICE_ID_:
        return NOT_IMPLEMENTED;
        break;
    default:
        return INVALID_FUNCTION;
    }

    return HANDLED_OK;
}

uint8_t modbus_get_outputs() {
    for (int i = 0; i < OUTPUTS_SIZE; i++) {
        uint8_t byte_index = i / 8;
        uint8_t bit_index  = i % 8;
        modbusCoils[byte_index] = (modbusCoils[byte_index] & ~(1 << bit_index)) | (outputs[i].param.value << bit_index);
    }
    return SYNC_OK;
}

uint8_t modbus_set_outputs() {
    for (int i = 0; i < OUTPUTS_SIZE; i++) {
        uint8_t byte_index = i / 8;
        uint8_t bit_index  = i % 8;
        outputs[i].param.value = (modbusCoils[byte_index] >> bit_index) & 1;
    }
    return SYNC_OK;
}

uint8_t modbus_parse_register(){
	if (new_delay_action) {
		uint16_t coil_i = mbHRegArray[0];
		uint16_t coil_data = mbHRegArray[1];
		uint16_t delay = mbHRegArray[2];
		uint8_t pwm = (mbHRegArray[3] >> 8) & 0xFF;
                
		if (coil_data == 0x5555) {
			outputs[coil_i].param.delay_value = !outputs[coil_i].param.value;
		} else if (coil_data == 0xFF00) {
			outputs[coil_i].param.delay_value = 1;
		} else if (coil_data == 0x0000) {
			outputs[coil_i].param.delay_value = 0;
		} else {
			outputs[coil_i].param.value = outputs[coil_i].param.value; //nop
		}
		outputs[coil_i].param.delay = delay;
		if (delay != 0)
			outputs[coil_i].param.startTimer = TIMER_SET();

		new_delay_action = 0;
	}
	return SYNC_OK;
}

uint8_t modbus_parse_action_update(){
	if (new_action_update) {
		uint8_t inputNumber = (mbHRegArray[0] >> 8 & 0xFF);
		uint8_t actionType = (mbHRegArray[0] & 0xFF);      //single, double, long, switchon, switchoff, extra

		EventAction newEventAction;

		newEventAction.action = (mbHRegArray[1] >> 8 & 0xFF);
		newEventAction.delayAction = (mbHRegArray[1] & 0xFF);
		newEventAction.delay = (uint32_t)(((mbHRegArray[2] >> 8 & 0xFF) << 16) | (mbHRegArray[2] & 0xFF));
		newEventAction.pwm = (mbHRegArray[3] >> 8 & 0xFF);
		newEventAction.id = (mbHRegArray[3] & 0xFF);
		newEventAction.output = (mbHRegArray[4] >> 8 & 0xFF);
		newEventAction.send = (mbHRegArray[4] & 0xFF);
		newEventAction.extraEventId = (mbHRegArray[5] >> 8 & 0xFF);
		uint8_t save = (mbHRegArray[5] & 0xFF);


		EventAction* oldEventAction;

		if (inputNumber < (EXTRA_ACTION_PER_INPUT * INPUTS_SIZE) &&
			actionType == 6)  {
			oldEventAction = &extraActions[inputNumber];
		}
		else if (inputNumber < INPUTS_SIZE) {
			switch (actionType) {
			case 1:
				oldEventAction = &inputActions[inputNumber].singlePress;
				break;
			case 2:
				oldEventAction = &inputActions[inputNumber].doublePress;
				break;
			case 3:
				oldEventAction = &inputActions[inputNumber].longPress;
				break;
			case 4:
				oldEventAction = &inputActions[inputNumber].switchOn;
				break;
			case 5:
				oldEventAction = &inputActions[inputNumber].switchOff;
				break;
			default:
				return WRONG_ACTION_TYPE;
			}
		}
		else {
			return WRONG_ACTION_INPUTNMBR;
		}

		copyEventAction(&newEventAction, oldEventAction);


		if (save) {
		    Flash_Erase(USERDATA_ORIGIN, USERDATA_LENGTH);
			Flash_WriteInputs(inputs);
			Flash_WriteInputActions(inputActions);
			Flash_WriteExtraActions(extraActions);
			Flash_WriteOutputs(outputs);
		}
	}
	return ACTION_OK;
}

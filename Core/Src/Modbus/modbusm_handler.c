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

/*
 * Holding Register Address Map
 * ---------------------------------------------------------------------------
 * 0x0000-0x0031  Raw mbHRegArray (50 regs, legacy read/write)
 *   0x0031 is used as Modbus ID register (valid range 1..247).
 *
 * OUTPUT region   BASE=0x0100  stride=REGS_PER_OUTPUT(5)
 *   addr = OUTPUT_REG_BASE + output_index * REGS_PER_OUTPUT
 *   [+0] pin                   uint16_t
 *   [+1] value   | invert      value       << 8 | invert
 *   [+2] min     | max         min         << 8 | max
 *   [+3] delay                 uint16_t
 *   [+4] delay_value           delay_value << 8 | 0x00
 *   (port and startTimer omitted – const / runtime)
 *
 * INPUT region    BASE=0x0200  stride=REGS_PER_INPUT(4)
 *   addr = INPUT_REG_BASE + input_index * REGS_PER_INPUT
 *   [+0] pin                   uint16_t
 *   [+1] button_type | value   button_type << 8 | value
 *   [+2] min         | max     min         << 8 | max
 *   [+3] changed               changed     << 8 | 0x00
 *   (port omitted – const)
 *
 * ACTION region   BASE=0x0300  stride=REGS_PER_ACTION(7)
 *   addr = ACTION_REG_BASE
 *        + input_index * (ACTION_TYPES_PER_INPUT * REGS_PER_ACTION)
 *        + (actionType-1) * REGS_PER_ACTION
 *   actionType: 1=singlePress 2=doublePress 3=longPress 4=switchOn 5=switchOff
 *   [+0] input_index | actionType   input_index << 8 | actionType
 *   [+1] action      | delayAction  action      << 8 | delayAction
 *   [+2] delay high word            (delay >> 16) & 0xFFFF
 *   [+3] delay low  word            (delay      ) & 0xFFFF
 *   [+4] pwm | id                   pwm         << 8 | id
 *   [+5] output | send              output      << 8 | send
 *   [+6] extraEventId               extraEventId<< 8 | 0x00
 *
 * EXTRA ACTION region  BASE=0x0500  stride=REGS_PER_ACTION(7)
 *   addr = EXTRA_ACTION_REG_BASE + extra_index * REGS_PER_ACTION
 *   Same 7-register layout; actionType in [+0] is always 6.
 * ---------------------------------------------------------------------------
 */

#define OUTPUT_REG_BASE           0x0100u
#define INPUT_REG_BASE            0x0200u
#define ACTION_REG_BASE           0x0300u
#define EXTRA_ACTION_REG_BASE     0x0500u

#define REGS_PER_OUTPUT           5u
#define REGS_PER_INPUT            4u
#define REGS_PER_ACTION           7u
#define ACTION_TYPES_PER_INPUT    5u

#define MODBUS_ID_HREG_ADDR       0x0031u
#define MODBUS_ID_MIN             1u
#define MODBUS_ID_MAX             247u


uint8_t  mbCoilsArray[1] = {0x00};
uint8_t  mbInputsArray[1];
uint16_t mbHRegArray[50];
uint16_t mbIRegArray[1];

uint8_t*  modbusCoils  = mbCoilsArray;
uint8_t*  modbusInputs = mbInputsArray;
uint16_t* modbusHReg   = mbHRegArray;
uint16_t* modbusIReg   = mbIRegArray;

uint8_t new_delay_action  = 0;
uint8_t new_action_update = 0;
static uint16_t pending_action_regs[REGS_PER_ACTION] = {0};
static uint8_t pending_action_valid = 0;

static uint8_t update_modbus_id(uint16_t raw_value)
{
    if (raw_value < MODBUS_ID_MIN || raw_value > MODBUS_ID_MAX) {
        return INVALID_DATA_LENGHT;
    }

    uint8_t new_id = (uint8_t)raw_value;
    if (new_id != MODBUS_ID) {
        if (Flash_WriteModbusId(new_id) != HAL_OK) {
            return MODBUS_ID_SAVE_FAILED;
        }
        MODBUS_ID = new_id;
    }

    mbHRegArray[MODBUS_ID_HREG_ADDR] = MODBUS_ID;
    return HANDLED_OK;
}


/* ---------------------------------------------------------------------------
 * populate_output_registers
 * Fills mbHRegArray[0..4] from outputs[output_index].param.
 * Symmetric with modbus_parse_register() write encoding.
 * --------------------------------------------------------------------------- */
static uint8_t populate_output_registers(uint16_t output_index)
{
    if (output_index >= OUTPUTS_SIZE)
        return INVALID_DATA_LENGHT;

    const OutputParam* p = &outputs[output_index].param;

    mbHRegArray[0] = p->pin;
    mbHRegArray[1] = ((uint16_t)p->value << 8) | p->invert;
    mbHRegArray[2] = ((uint16_t)p->min << 8) | p->max;
    mbHRegArray[3] = p->delay;
    mbHRegArray[4] = ((uint16_t)p->delay_value << 8) | 0x00u;

    return 0;
}


/* ---------------------------------------------------------------------------
 * populate_input_registers
 * Fills mbHRegArray[0..3] from inputs[input_index].param.
 * --------------------------------------------------------------------------- */
static uint8_t populate_input_registers(uint16_t input_index)
{
    if (input_index >= INPUTS_SIZE)
        return INVALID_DATA_LENGHT;

    const InputParam* p = &inputs[input_index].param;

    mbHRegArray[0] = p->pin;
    mbHRegArray[1] = ((uint16_t)p->button_type << 8) | p->value;
    mbHRegArray[2] = ((uint16_t)p->min << 8) | p->max;
    mbHRegArray[3] = ((uint16_t)p->changed << 8) | 0x00u;

    return 0;
}

/* ---------------------------------------------------------------------------
 * populate_action_registers
 * Fills mbHRegArray[0..6] from the requested EventAction.
 * actionType 1-5 → regular input action; 6 → extra action (input_index used
 * as the flat extra-action index).
 * Symmetric with modbus_parse_action_update() write encoding.
 * --------------------------------------------------------------------------- */
static uint8_t populate_action_registers(uint8_t input_index, uint8_t actionType)
{
    EventAction* ea = NULL;

    //fetch action
    if (actionType == 6) {
        if (input_index >= (EXTRA_ACTION_PER_INPUT * INPUTS_SIZE))
            return WRONG_ACTION_INPUTNMBR;
        ea = &extraActions[input_index];
    }
    else if (input_index < INPUTS_SIZE) {
        switch (actionType) {
            case 1: ea = &inputActions[input_index].singlePress; break;
            case 2: ea = &inputActions[input_index].doublePress; break;
            case 3: ea = &inputActions[input_index].longPress;   break;
            case 4: ea = &inputActions[input_index].switchOn;    break;
            case 5: ea = &inputActions[input_index].switchOff;   break;
            default: return WRONG_ACTION_TYPE;
        }
    }
    else {
        return WRONG_ACTION_INPUTNMBR;
    }

    //build register values
    mbHRegArray[0] = ((uint16_t)input_index << 8) | actionType;
    mbHRegArray[1] = ((uint16_t)ea->action << 8) | ea->delayAction;
    mbHRegArray[2] = (uint16_t)((ea->delay >> 16) & 0xFFFFu);
    mbHRegArray[3] = (uint16_t)( ea->delay & 0xFFFFu);
    mbHRegArray[4] = ((uint16_t)ea->pwm << 8) | ea->id;
    mbHRegArray[5] = ((uint16_t)ea->output << 8) | ea->send;
    mbHRegArray[6] = ((uint16_t)ea->extraEventId  << 8) | 0x00u;

    return 0;
}


/* ---------------------------------------------------------------------------
 * modbusm_handle
 * --------------------------------------------------------------------------- */
uint8_t modbusm_handle(ModbusMessage *message) {
//check if device ID matches //done in rtu decode
//    if (message->slave_address != DEVICE_ID) {
//        message = NULL;
//        return ID_MISMATCH;
//    }

    //handle message
    switch (message->function_code) {
    case READ_COILS:
    {
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
    }
    case READ_DISC_INPUTS:
    {
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
    }
    case READ_HOLD_REGS:
    {
        uint16_t first_hreg = 0;
        uint16_t amount_hregs = 0;
        if (message->data_length == 4) {
            first_hreg = (message->data[0] << 8 | message->data[1]);
            amount_hregs = (message->data[2] << 8 | message->data[3]);
        } else {
            message = NULL;
            return INVALID_DATA_LENGHT;
        }

        /* ── live-data region decode ─────────────────────────────────────── */
        uint16_t output_region_end = OUTPUT_REG_BASE + (uint16_t)(OUTPUTS_SIZE * REGS_PER_OUTPUT);
        uint16_t input_region_end = INPUT_REG_BASE + (uint16_t)(INPUTS_SIZE * REGS_PER_INPUT);
        uint16_t action_region_end = ACTION_REG_BASE + (uint16_t)(INPUTS_SIZE * ACTION_TYPES_PER_INPUT * REGS_PER_ACTION);
        uint16_t extra_region_end = EXTRA_ACTION_REG_BASE + (uint16_t)(EXTRA_ACTION_PER_INPUT * INPUTS_SIZE * REGS_PER_ACTION);

        if (first_hreg >= OUTPUT_REG_BASE && first_hreg < output_region_end)
        {
            uint16_t offset       = first_hreg - OUTPUT_REG_BASE;
            uint16_t output_index = offset / REGS_PER_OUTPUT;
            uint8_t  err = populate_output_registers(output_index);
            if (err)
                return err;
            first_hreg = 0;
        }
        else if (first_hreg >= INPUT_REG_BASE && first_hreg < input_region_end)
        {
            uint16_t offset      = first_hreg - INPUT_REG_BASE;
            uint16_t input_index = offset / REGS_PER_INPUT;
            uint8_t  err = populate_input_registers(input_index);
            if (err)
                return err;
            first_hreg = 0;
        }
        else if (first_hreg >= ACTION_REG_BASE && first_hreg < action_region_end)
        {
            uint16_t offset = first_hreg - ACTION_REG_BASE;
            uint8_t input_index = (uint8_t)(offset / (ACTION_TYPES_PER_INPUT * REGS_PER_ACTION));
            uint8_t actionType = (uint8_t)((offset % (ACTION_TYPES_PER_INPUT * REGS_PER_ACTION)) / REGS_PER_ACTION) + 1u;
            uint8_t err = populate_action_registers(input_index, actionType);
            if (err)
                return err;
            first_hreg = 0;
        }
        else if (first_hreg >= EXTRA_ACTION_REG_BASE && first_hreg < extra_region_end)
        {
            uint16_t offset     = first_hreg - EXTRA_ACTION_REG_BASE;
            uint8_t  extraIndex = (uint8_t)(offset / REGS_PER_ACTION);
            uint8_t  err = populate_action_registers(extraIndex, 6u);
            if (err)
                return err;
            first_hreg = 0;
        }
        else {
            /* raw mbHRegArray region */
            uint16_t mbHRegCount = (uint16_t)(sizeof(mbHRegArray) / sizeof(mbHRegArray[0]));
            if ((uint32_t)first_hreg + (uint32_t)amount_hregs > (uint32_t)mbHRegCount) {
                return INVALID_DATA_LENGHT;
            }
        }

        /* Keep raw register mirror in sync with runtime ID. */
        mbHRegArray[MODBUS_ID_HREG_ADDR] = MODBUS_ID;

        /* ── build response ─────────────────────────────────────────────── */
        message->slave_address = MODBUS_ID;
        message->function_code = READ_HOLD_REGS;
        message->data_length = 1 + (amount_hregs * 2);
        *message->data = message->data_length - 1;
        for (int i = 0; i < amount_hregs; i++) {
            message->data[1 + i * 2]     = (modbusHReg[first_hreg + i] >> 8) & 0xFF;
            message->data[1 + i * 2 + 1] =  modbusHReg[first_hreg + i] & 0xFF;
        }
        break;
    }
    case READ_INPUT_REGS:
    {
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
        uint16_t mbIRegCount = (uint16_t)(sizeof(mbIRegArray) / sizeof(mbIRegArray[0]));
        if ((uint32_t)first_ireg + (uint32_t)amount_iregs > (uint32_t)mbIRegCount) {
            return INVALID_DATA_LENGHT;
        }
        //reply
        message->slave_address = MODBUS_ID;
        message->function_code = READ_INPUT_REGS;
        message->data_length = 1 + (amount_iregs * 2);
        *message->data = message->data_length - 1;
        for (int i = 0; i < amount_iregs; i++) {
            message->data[1 + i * 2]     = (modbusIReg[first_ireg + i] >> 8) & 0xFF;
            message->data[1 + i * 2 + 1] =  modbusIReg[first_ireg + i] & 0xFF;
        }
        break;
    }
    case WRITE_SINGLE_COIL:
    {
        uint16_t coil_adress = 0;
        uint16_t value = 0;
        if (message->data_length == 4) {
            coil_adress = (message->data[0] << 8 | message->data[1]);
            value = (message->data[2] << 8 | message->data[3]);
        }
        else {
            return INVALID_DATA_LENGHT;
        }
        //ADD MAX VALUE ERROR ?

        //write coil;
        //printf("%02X, %02X\n", coil_adress, value);
        if (value == 0xFF00) {
            modbusCoils[coil_adress / 8] |= (1 << (coil_adress % 8));
        }
        else if (value == 0x0000) {
            modbusCoils[coil_adress / 8] &= ~(1 << (coil_adress % 8));
        }
        else if (value == 0x5555) {  // Unofficial toggle – domoriks only
            modbusCoils[coil_adress / 8] ^= (1 << (coil_adress % 8));
        }
        else {
            message = NULL;
            return INVALID_COIL_VALUE;
        }
        message = message; //echo message
        break;
    }
    case WRITE_SINGLE_REG:      //   <- Tis only apply for holding regs
    {
        //parse
        uint16_t reg_adress = 0;
        if (message->data_length == 4) {
            reg_adress = (message->data[0] << 8 | message->data[1]);
        }
        else {
            return INVALID_DATA_LENGHT;
        }
        uint16_t mbHRegCount = (uint16_t)(sizeof(mbHRegArray) / sizeof(mbHRegArray[0]));
        if (reg_adress >= mbHRegCount) {
            return INVALID_DATA_LENGHT;
        }

        uint16_t value = (uint16_t)((message->data[2] << 8) | message->data[3]);
        if (reg_adress == MODBUS_ID_HREG_ADDR) {
            uint8_t status = update_modbus_id(value);
            if (status != HANDLED_OK) {
                return status;
            }
        }
        //write regs
        modbusHReg[reg_adress] = value;
        //reply
        message = message; //echo message
        break;
    }

    case WRITE_MULTI_REGS:
      {
        // Parse the request message
        uint16_t start_address = (message->data[0] << 8) | message->data[1];
        uint16_t num_registers = (message->data[2] << 8) | message->data[3];
        uint8_t byte_count = message->data[4];
        if (message->data_length != 5 + byte_count || byte_count != num_registers * 2) {
            return INVALID_DATA_LENGHT;
        }

        // Decode payload registers for action path.
        uint16_t incoming_regs[REGS_PER_ACTION] = {0};
        uint8_t copy_count = (num_registers < REGS_PER_ACTION) ? (uint8_t)num_registers : REGS_PER_ACTION;
        for (uint8_t i = 0; i < copy_count; i++) {
            incoming_regs[i] = (message->data[5 + i * 2] << 8) | message->data[6 + i * 2];
        }

        // Detect action writes either by legacy start=0 + reg0 selector,
        // or by mapped action/extra-action register offsets (same as read map).
        uint8_t is_action_write = 0;
        uint8_t mapped_input_index = 0;
        uint8_t mapped_action_type = 0;

        if (num_registers == REGS_PER_ACTION) {
            uint16_t action_region_end = ACTION_REG_BASE + (uint16_t)(INPUTS_SIZE * ACTION_TYPES_PER_INPUT * REGS_PER_ACTION);
            uint16_t extra_region_end = EXTRA_ACTION_REG_BASE + (uint16_t)(EXTRA_ACTION_PER_INPUT * INPUTS_SIZE * REGS_PER_ACTION);

            if (start_address == 0u) {
                is_action_write = 1;
                mapped_input_index = (incoming_regs[0] >> 8) & 0xFF;
                mapped_action_type = incoming_regs[0] & 0xFF;
            }
            else if (start_address >= ACTION_REG_BASE && start_address < action_region_end) {
                uint16_t offset = start_address - ACTION_REG_BASE;
                if ((offset % REGS_PER_ACTION) != 0u) {
                    return INVALID_DATA_LENGHT;
                }
                is_action_write = 1;
                mapped_input_index = (uint8_t)(offset / (ACTION_TYPES_PER_INPUT * REGS_PER_ACTION));
                mapped_action_type = (uint8_t)((offset % (ACTION_TYPES_PER_INPUT * REGS_PER_ACTION)) / REGS_PER_ACTION) + 1u;
            }
            else if (start_address >= EXTRA_ACTION_REG_BASE && start_address < extra_region_end) {
                uint16_t offset = start_address - EXTRA_ACTION_REG_BASE;
                if ((offset % REGS_PER_ACTION) != 0u) {
                    return INVALID_DATA_LENGHT;
                }
                is_action_write = 1;
                mapped_input_index = (uint8_t)(offset / REGS_PER_ACTION);
                mapped_action_type = 6u;
            }
        }

        if (is_action_write) {
            // Capture action write atomically for parser in main loop.
            for (uint8_t i = 0; i < REGS_PER_ACTION; i++) {
                pending_action_regs[i] = incoming_regs[i];
            }
            // Force selector from address map when using offset mode.
            pending_action_regs[0] = ((uint16_t)mapped_input_index << 8) | mapped_action_type;
            pending_action_valid = 1;
            new_action_update = 1;
        } else {
            // Generic holding-register write path (legacy delay action flow).
            uint16_t mbHRegCount = (uint16_t)(sizeof(mbHRegArray) / sizeof(mbHRegArray[0]));
            if ((uint32_t)start_address + (uint32_t)num_registers > (uint32_t)mbHRegCount) {
                return INVALID_DATA_LENGHT;
            }

            for (uint16_t i = 0; i < num_registers; i++) {
                mbHRegArray[start_address + i] = (uint16_t)((message->data[5 + i * 2] << 8) | message->data[6 + i * 2]);
            }

            if (start_address <= MODBUS_ID_HREG_ADDR
                && (uint32_t)start_address + (uint32_t)num_registers > MODBUS_ID_HREG_ADDR) {
                uint16_t idx = (uint16_t)(MODBUS_ID_HREG_ADDR - start_address);
                uint16_t id_value = mbHRegArray[start_address + idx];
                uint8_t status = update_modbus_id(id_value);
                if (status != HANDLED_OK) {
                    return status;
                }
            }

            new_delay_action = 1;
        }

        // Prepare the response message
        message->data_length = 4;
        message->data[0] = (start_address >> 8) & 0xFF;
        message->data[1] = start_address & 0xFF;
        message->data[2] = (num_registers >> 8) & 0xFF;
        message->data[3] = num_registers & 0xFF;
        break;
    }
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

    default:
        return INVALID_FUNCTION;
    }

    return HANDLED_OK;
}


/* ---------------------------------------------------------------------------
 * modbus_get_outputs
 * Copy live output values into the coil array so READ_COILS reflects reality.
 * --------------------------------------------------------------------------- */
uint8_t modbus_get_outputs(void)
{
    for (int i = 0; i < OUTPUTS_SIZE; i++) {
        uint8_t byte_index = i / 8;
        uint8_t bit_index  = i % 8;
        modbusCoils[byte_index] = (modbusCoils[byte_index] & ~(1 << bit_index))
                                | (outputs[i].param.value << bit_index);
    }
    return SYNC_OK;
}


/* ---------------------------------------------------------------------------
 * modbus_set_outputs
 * Push coil array values back into live outputs after a coil write.
 * --------------------------------------------------------------------------- */
uint8_t modbus_set_outputs(void)
{
    for (int i = 0; i < OUTPUTS_SIZE; i++) {
        uint8_t byte_index = i / 8;
        uint8_t bit_index  = i % 8;
        outputs[i].param.value = (modbusCoils[byte_index] >> bit_index) & 1;
    }
    return SYNC_OK;
}


/* ---------------------------------------------------------------------------
 * modbus_parse_register
 * Called from the main loop when new_delay_action is set by WRITE_MULTI_REGS.
 * Reads mbHRegArray[0..3] and applies a (optionally delayed) output command.
 *
 * Register layout (write side – matches populate_output_registers read side):
 *   [0] output index
 *   [1] coil_data   0xFF00=ON  0x0000=OFF  0x5555=toggle
 *   [2] delay       ms
 *   [3] pwm         high byte
 * --------------------------------------------------------------------------- */
uint8_t modbus_parse_register(void)
{
    if (new_delay_action) {
        uint16_t coil_i    = mbHRegArray[0];
        uint16_t coil_data = mbHRegArray[1];
        uint16_t delay     = mbHRegArray[2];

        if (coil_data == 0x5555) {
            outputs[coil_i].param.delay_value = !outputs[coil_i].param.value;
        } else if (coil_data == 0xFF00) {
            outputs[coil_i].param.delay_value = 1;
        } else if (coil_data == 0x0000) {
            outputs[coil_i].param.delay_value = 0;
        } else {
			outputs[coil_i].param.delay_value = outputs[coil_i].param.value; //nop
        }

        outputs[coil_i].param.delay = delay;
        if (delay != 0)
            outputs[coil_i].param.startTimer = TIMER_SET();

        new_delay_action = 0;
    }
    return SYNC_OK;
}


/* ---------------------------------------------------------------------------
 * modbus_parse_action_update
 * Called from the main loop when new_action_update is set.
 * Reads mbHRegArray[0..5] and updates the corresponding EventAction.
 *
 * Register layout (write side – matches populate_action_registers read side):
 *   [0] inputNumber << 8 | actionType
 *   [1] action      << 8 | delayAction
 *   [2] delay high word  (bits 31:16)
 *   [3] delay low  word  (bits 15:0)
 *   [4] pwm         << 8 | id
 *   [5] output      << 8 | send
 *   [6] extraEventId<< 8 | save
 * --------------------------------------------------------------------------- */
uint8_t modbus_parse_action_update(void) {
  if (new_action_update) {
    const uint16_t *regs = pending_action_valid ? pending_action_regs : mbHRegArray;

        //parse new action from mbHRegArray
    uint8_t inputNumber = (regs[0] >> 8) & 0xFF;
    uint8_t actionType  =  regs[0] & 0xFF;      //single, double, long, switchon, switchoff, extra

        EventAction newEventAction;
    newEventAction.action = (regs[1] >> 8) & 0xFF;
    newEventAction.delayAction  =  regs[1] & 0xFF;
    newEventAction.delay = ((uint32_t)regs[2] << 16) |  (uint32_t)regs[3];
    newEventAction.pwm = (regs[4] >> 8) & 0xFF;
    newEventAction.id =  regs[4] & 0xFF;
    newEventAction.output = (regs[5] >> 8) & 0xFF;
    newEventAction.send =  regs[5] & 0xFF;
    newEventAction.extraEventId = (regs[6] >> 8) & 0xFF;
    uint8_t save =  regs[6] & 0xFF;

        //find target EventAction to update
        EventAction* oldEventAction;
        if (actionType == 6) {
            if (inputNumber >= (EXTRA_ACTION_PER_INPUT * INPUTS_SIZE)) {
                pending_action_valid = 0;
                new_action_update = 0;
                return WRONG_ACTION_INPUTNMBR;
            }
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
                pending_action_valid = 0;
                new_action_update = 0;
				return WRONG_ACTION_TYPE;
            }
        }
        else {
            pending_action_valid = 0;
            new_action_update = 0;
            return WRONG_ACTION_INPUTNMBR;
        }

        //update action
        copyEventAction(&newEventAction, oldEventAction);

        if (save) {
            HAL_StatusTypeDef flash_status = HAL_OK;
            if (Flash_Erase(USERDATA_ORIGIN, USERDATA_LENGTH) != HAL_OK) {
                flash_status = HAL_ERROR;
            }
            if (flash_status == HAL_OK && Flash_WriteInputs(inputs) != HAL_OK) {
                flash_status = HAL_ERROR;
            }
            if (flash_status == HAL_OK && Flash_WriteInputActions(inputActions) != HAL_OK) {
                flash_status = HAL_ERROR;
            }
            if (flash_status == HAL_OK && Flash_WriteExtraActions(extraActions) != HAL_OK) {
                flash_status = HAL_ERROR;
            }
            if (flash_status == HAL_OK && Flash_WriteOutputs(outputs) != HAL_OK) {
                flash_status = HAL_ERROR;
            }
            if (flash_status != HAL_OK) {
                pending_action_valid = 0;
                new_action_update = 0;
                return ACTION_SAVE_FAILED;
            }
        }

        pending_action_valid = 0;
        new_action_update = 0;
    }
    return ACTION_OK;
}

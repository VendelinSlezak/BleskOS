/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* includes */
#include <libc/string.h>
#include <kernel/firmware/acpi/aml/aml_definition.h>
#include <kernel/firmware/acpi/acpi.h>
#include <kernel/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>

/* local variables */
aml_block_t *global_aml_namespace;
aml_block_t *global_aml_namespace_actual_block;

aml_block_t *zero_block;
aml_block_t *actual_block;
void *aml_code_ptr;
uint32_t new_block_state;

uint32_t *aml_state_stack;
uint32_t aml_state_stack_pointer;

/* functions */
void initialize_aml_interpreter(void) {
    global_aml_namespace = NULL;
    global_aml_namespace_actual_block = NULL;

    aml_state_stack = kalloc(sizeof(uint32_t) * SIZE_OF_AML_STATE_STACK);
}

void dump_aml_tree(aml_block_t *aml_block, int depth) {
    aml_block_t **stack = kalloc(sizeof(aml_block_t *) * SIZE_OF_AML_STATE_STACK);
    stack[depth] = aml_block;

    while(depth >= 0) {
        aml_block = stack[depth];
        if(aml_block == NULL) {
            depth--;
            continue;
        }

        log("\n");
        for(uint32_t i = 0; i < depth; i++) {
            log("  ");
        }
        log("%x: %s ", aml_block->end, aml_state_names[aml_block->type]);
        switch(aml_block->type) {
            case b_PkgLength_t: {
                aml_block_PkgLength_t *PkgLength_block = (aml_block_PkgLength_t *) aml_block;
                log("length: %d", PkgLength_block->length);
                break;
            }
            case b_NamedField_t:
            case b_NameString_t: {
                aml_block_NameString_t *NameString_block = (aml_block_NameString_t *) aml_block;
                uint8_t *string = kalloc(NameString_block->length_of_string + 1);
                memcpy(string, NameString_block->string, NameString_block->length_of_string);
                log("name: %s", string);
                kfree(string);
                break;
            }
            case b_RegionSpace_t: {
                aml_block_RegionSpace_t *RegionSpace_block = (aml_block_RegionSpace_t *) aml_block;
                switch(RegionSpace_block->region_space) {
                    case 0x00: log("SystemMemory"); break;
                    case 0x01: log("SystemIO"); break;
                    case 0x02: log("PCI_Config"); break;
                    case 0x03: log("EmbeddedController"); break;
                    case 0x04: log("SMBus"); break;
                    case 0x05: log("System CMOS"); break;
                    case 0x06: log("PciBarTarget"); break;
                    case 0x07: log("IPMI"); break;
                    case 0x08: log("GeneralPurposeIO"); break;
                    case 0x09: log("GenericSerialBus"); break;
                    case 0x0A: log("PCC"); break;
                    default: {
                        log("OEM defined 0x%02x", RegionSpace_block->region_space);
                        break;
                    }
                }
                break;
            }
            case b_OneOp_t:
            case b_WordConst_t: {
                aml_block_NumConst_t *NumConst_block = (aml_block_NumConst_t *) aml_block;
                log("value: 0x%x%x", (uint32_t)(NumConst_block->value >> 32), (uint32_t)(NumConst_block->value & 0xFFFFFFFF));
                break;
            }
            case b_FieldFlags_t: {
                aml_block_FieldFlags_t *FieldFlags_block = (aml_block_FieldFlags_t *) aml_block;
                switch(FieldFlags_block->AccessType) {
                    case 0x00: log("AnyAcc"); break;
                    case 0x01: log("ByteAcc"); break;
                    case 0x02: log("WordAcc"); break;
                    case 0x03: log("DWordAcc"); break;
                    case 0x04: log("QWordAcc"); break;
                    case 0x05: log("BufferAcc"); break;
                    default: log("Reserved"); break;
                }
                log(" ");
                switch(FieldFlags_block->LockRule) {
                    case 0x00: log("NoLock"); break;
                    case 0x01: log("Lock"); break;
                }
                log(" ");
                switch(FieldFlags_block->UpdateRule) {
                    case 0x00: log("Preserve"); break;
                    case 0x01: log("WriteAsOnes"); break;
                    case 0x02: log("WriteAsZeros"); break;
                    case 0x03: log("Reserved"); break;
                }
                break;
            }
            case b_MethodFlags_t: {
                aml_block_MethodFlags_t *MethodFlags_block = (aml_block_MethodFlags_t *) aml_block;
                log("ArgCount: %d", MethodFlags_block->ArgCount);
                log(" ");
                switch(MethodFlags_block->SerializeFlag) {
                    case 0x00: log("NotSerialized"); break;
                    case 0x01: log("Serialized"); break;
                }
                log("SyncLevel: %d", MethodFlags_block->SyncLevel);
                break;
            }
        }

        if(aml_block->right != NULL) {
            stack[depth] = aml_block->down;
            stack[++depth] = aml_block->right;
        }
        else {
            stack[depth] = aml_block->down;
        }
    }

    kfree(stack);
}

void dump_aml_block(aml_block_t *aml_block) {
    log("\n%x:", aml_block);
    log("\n LEFT: %x", aml_block->left);
    log("\n RIGHT: %x", aml_block->right);
    log("\n DOWN: %x", aml_block->down);
    log("\n START: %x", aml_block->start);
    log("\n END: %x", aml_block->end);
    log("\n TYPE: %s", aml_state_names[aml_block->type]);
}

void free_part_of_aml_tree(aml_block_t *aml_block) {
    aml_block_t **stack = kalloc(sizeof(aml_block_t *) * SIZE_OF_AML_STATE_STACK);
    int depth = 0;
    stack[depth] = aml_block;

    while(depth >= 0) {
        aml_block_t *aml_block = stack[depth];
        // TODO: type specific free
        if(aml_block->right != NULL) {
            stack[++depth] = aml_block->right;
            aml_block->right = NULL;
        }
        else if(aml_block->down != NULL) {
            stack[depth] = aml_block->down;
            kfree(aml_block);
        }
        else {
            depth--;
            kfree(aml_block);
        }
    }

    kfree(stack);
}

void free_aml_tree(aml_block_t *aml_block) {
    kfree(zero_block);
    free_part_of_aml_tree(aml_block);
}

void *find_aml_block_downward(aml_block_t *aml_block, uint32_t type) {
    if(aml_block == NULL) {
        return NULL;
    }
    while(aml_block != NULL) {
        if(aml_block->type == type) {
            return aml_block;
        }
        aml_block = aml_block->down;
    }
    return NULL;
}

void process_acpi_aml_table(void *aml_table) {
    uint32_t aml_table_length = ((acpi_table_header_t *) aml_table)->length;
    if(aml_table_length <= (sizeof(acpi_table_header_t) + 1)) {
        log("\n[AML] Error: DSDT table is too small");
        return;
    }
    log("\n[AML] Processing DSDT table of length %d", aml_table_length);

    aml_block_t *aml_code_block = process_aml_code(aml_table + sizeof(acpi_table_header_t), aml_table_length - sizeof(acpi_table_header_t));
    if(aml_code_block == NULL) {
        log("\n[AML] Error during AML code processing");
        return;
    }
    if(global_aml_namespace == NULL) {
        global_aml_namespace = aml_code_block;
        global_aml_namespace_actual_block = actual_block;
    }
    else {
        global_aml_namespace_actual_block->down = aml_code_block;
        global_aml_namespace_actual_block = aml_code_block;
    }

    dump_aml_tree(global_aml_namespace, 0);
}

aml_block_t *process_aml_code(void *aml_code, uint32_t aml_code_length) {
    if(aml_code_length == 0) {
        return NULL;
    }

    zero_block = kalloc(sizeof(aml_block_t));
    zero_block->start = aml_code;
    zero_block->end = aml_code + aml_code_length - 1;

    aml_block_t *first_block = kalloc(sizeof(aml_block_t));
    actual_block = first_block;
    actual_block->left = zero_block;
    actual_block->start = aml_code;
    actual_block->end = aml_code + aml_code_length - 1;
    actual_block->type = b_table_t;
    aml_code_ptr = actual_block->start;
    new_block_state = MOVE_RIGHT;

    zero_block->right = first_block;

    aml_state_stack_pointer = SIZE_OF_AML_STATE_STACK - 1;
    aml_state_stack[aml_state_stack_pointer] = b_TermList_t;

    while(aml_state_stack_pointer < SIZE_OF_AML_STATE_STACK) {
        // load state
        uint32_t state = aml_state_stack[aml_state_stack_pointer];
        aml_rule_t *aml_rule = convert_state_to_rule[state];
        log("\n[AML] Processing state");
        for(int i = aml_state_stack_pointer; i < SIZE_OF_AML_STATE_STACK; i++) {
            log(" %s", aml_state_names[aml_state_stack[i]]);
        }

        // check if condition of this state is met
        if(aml_rule->condition == CONDITION_BYTE) {
            if(very_unlikely(aml_code_ptr > actual_block->left->end)) {
                log("\n[AML] ERROR: Condition not met, end of AML block reached");
                free_aml_tree(first_block);
                return NULL;
            }
            if(very_unlikely(*(uint8_t *) aml_code_ptr != aml_rule->condition_value)) {
                log("\n[AML] ERROR: Condition not met, %02x != %02x", *(uint8_t *) aml_code_ptr, aml_rule->condition_value);
                free_aml_tree(first_block);
                return NULL;
            }
        }
        else if(aml_rule->condition == CONDITION_EXT_BYTE) {
            if(very_unlikely((aml_code_ptr + 1) > actual_block->left->end)) {
                log("\n[AML] ERROR: Condition not met, end of AML block reached");
                free_aml_tree(first_block);
                return NULL;
            }
            uint8_t *ptr = aml_code_ptr;
            if(very_unlikely(ptr[0] != 0x5B || ptr[1] != aml_rule->condition_value)) {
                log("\n[AML] ERROR: Condition not met, %02x %02x != %02x", ptr[0], ptr[1], aml_rule->condition_value);
                free_aml_tree(first_block);
                return NULL;
            }
        }
        else if(aml_rule->condition == CONDITION_LEADNAMECHAR) {
            if(very_unlikely(aml_code_ptr > actual_block->left->end)) {
                log("\n[AML] ERROR: Condition not met, end of AML block reached");
                free_aml_tree(first_block);
                return NULL;
            }
            uint8_t character = *(uint8_t *) aml_code_ptr;
            if(very_unlikely((character < 'A' && character > 'Z') && character != '_')) {
                log("\n[AML] ERROR: Condition not met, character %02x is not a letter or underscore", character);
                free_aml_tree(first_block);
                return NULL;
            }
        }

        // create new block for this state
        if(aml_rule->create_new_block == true) {
            aml_block_t *new_block = kalloc(aml_rule->size_of_new_block);
            new_block->start = aml_code_ptr;
            new_block->end = actual_block->left->end;
            new_block->type = state;
            if(new_block_state == MOVE_RIGHT) {
                actual_block->right = new_block;
                new_block->left = actual_block;
            }
            else if(new_block_state == STAY) {
                actual_block->down = new_block;
                new_block->left = actual_block->left;
            }
            actual_block = new_block;
            new_block_state = aml_rule->new_block_state_after;
        }

        // process state
        if(aml_rule->process_state != NULL) {
            if(aml_rule->what_to_do_after == MOVE_LEFT) {
                if(aml_rule->process_state(actual_block->left) == ERROR) {
                    dump_aml_tree(first_block, 0);
                    free_aml_tree(first_block);
                    log("\n[AML] ERROR: Processing state failed");
                    return NULL;
                }
            }
            else if(aml_rule->process_state(actual_block) == ERROR) {
                dump_aml_tree(first_block, 0);
                free_aml_tree(first_block);
                log("\n[AML] ERROR: Processing state failed");
                return NULL;
            }
        }

        // count number of states in state rule
        int number_of_states = 0;
        if(aml_rule->states != NULL) {
            while(aml_rule->states[number_of_states] != 0) {
                number_of_states++;
            }  
        }

        // update state stack
        if(aml_rule->what_to_do_after == ADD_STATES) {
            if(very_unlikely(aml_state_stack_pointer < number_of_states)) {
                log("\n[AML] ERROR: Stack overflow");
                free_aml_tree(first_block);
                return NULL;
            }

            // replace actual state with given states
            for(int i = 0; i < number_of_states; i++) {
                // log("\nAdding state %d", aml_rule->states[number_of_states - i - 1]);
                aml_state_stack[aml_state_stack_pointer--] = aml_rule->states[number_of_states - i - 1];
            }
            aml_state_stack_pointer++; // move pointer to last state
        }
        else if(aml_rule->what_to_do_after == ADD_MULTIPLE_STATES) {
            if(very_unlikely(number_of_states > SIZE_OF_AML_MULTIPLE_STATES_STACK)) {
                log("\n[AML] ERROR: Stack overflow above, change definition");
                free_aml_tree(first_block);
                return NULL;
            }

            // find first state condition that is met
            int state_found = false;
            int multiple_choices_ptr = 0;
            while(aml_rule->states[multiple_choices_ptr] != 0) {
                uint32_t state = aml_rule->states[multiple_choices_ptr++];
                // log("\n[AML] Processing choice state %s", aml_state_names[state]);
                if(state == b_Nothing_t) {
                    log("\n[AML] Nothing, moving on to next state, %x", aml_code_ptr);
                    state_found = true;
                    aml_state_stack_pointer++; // move to next state
                    break;
                }
                aml_rule_t *aml_rule = convert_state_to_rule[state];
                if(aml_rule->condition == CONDITION_BYTE) {
                    if(very_unlikely(aml_code_ptr > actual_block->left->end)) {
                        continue; // we are at the end of block, so condition is not met
                    }
                    if(*(uint8_t *) aml_code_ptr == aml_rule->condition_value) {
                        aml_state_stack[aml_state_stack_pointer] = state; // replace actual state with new state
                        state_found = true;
                        break;
                    }
                    else {
                        continue;
                    }
                }
                else if(aml_rule->condition == CONDITION_EXT_BYTE) {
                    if(very_unlikely((aml_code_ptr + 1) > actual_block->left->end)) {
                        continue; // we are at the end of block, so condition is not met
                    }
                    uint8_t *ptr = aml_code_ptr;
                    if(ptr[0] == 0x5B && ptr[1] == aml_rule->condition_value) {
                        aml_state_stack[aml_state_stack_pointer] = state; // replace actual state with new state
                        state_found = true;
                        break;
                    }
                    else {
                        continue;
                    }
                }
                else if(aml_rule->condition == CONDITION_LEADNAMECHAR) {
                    if(very_unlikely(aml_code_ptr > actual_block->left->end)) {
                        continue; // we are at the end of block, so condition is not met
                    }
                    uint8_t character = *(uint8_t *) aml_code_ptr;
                    if((character >= 'A' && character <= 'Z') || character == '_') {
                        aml_state_stack[aml_state_stack_pointer] = state; // replace actual state with new state
                        state_found = true;
                        break;
                    }
                    else {
                        continue;
                    }
                }
                else if(aml_rule->condition == CONDITION_NOTHING && aml_code_ptr <= actual_block->left->end) { // if there is still some code to process, then this state follows
                    aml_state_stack[aml_state_stack_pointer] = state; // replace actual state with new state
                    state_found = true;
                    break;
                }
            }

            // none of possible states from this state was found
            if(very_unlikely(state_found == false)) {
                uint8_t *ptr = aml_code_ptr;
                log("\n[AML] ERROR: No state matched, 0x%02x", ptr[0]);
                if(*(uint8_t *) aml_code_ptr == 0x5B) {
                    log(" 0x%02x", ptr[1]);
                }
                dump_aml_tree(first_block, 0);
                free_aml_tree(first_block);
                return NULL;
            }
        }
        else if(aml_rule->what_to_do_after == MOVE_LEFT) {
            if(new_block_state == STAY) {
                actual_block = actual_block->left;
            }
            new_block_state = STAY;
            aml_state_stack_pointer++;
        }
        else if(aml_rule->what_to_do_after == NOTHING) {
            aml_state_stack_pointer++;
        }
    }

    log("\n[AML] Processing succesfully finished");
    kfree(zero_block);
    first_block->left = NULL;
    return first_block;
}

uint32_t aml_PkgLength(void *aml_block) {
    aml_block_PkgLength_t *block = (aml_block_PkgLength_t *) aml_block;
    uint8_t *ptr = aml_code_ptr;
    uint8_t following_bytes = (ptr[0] >> 6);
    if((aml_code_ptr + following_bytes) > block->header.left->end) {
        log("\n[AML] PkgLength ERROR: Section overflow");
        block->header.type = b_Error_t;
        return ERROR;
    }
    uint32_t length = 0;
    switch(following_bytes) {
        case 0: {
            length = ptr[0] & 0x3F;
            break;
        }
        case 1: {
            length = (ptr[0] & 0x0F) + (ptr[1] << 4);
            break;
        }
        case 2: {
            length = (ptr[0] & 0x0F) + (ptr[1] << 4) + (ptr[2] << 12);
            break;
        }
        case 3: {
            length = (ptr[0] & 0x0F) + (ptr[1] << 4) + (ptr[2] << 12) + (ptr[3] << 20);
            break;
        }
    }

    if(length == 0) {
        log("\n[AML] ERROR: PkgLength is zero");
        return ERROR;
    }
    log("\n[AML] PkgLength is %d", length);

    block->length = length;

    // set end in all blocks of blocks in space
    void *end = aml_code_ptr + length - 1;
    block->header.left->end = end;
    aml_block_t *block_ptr = block->header.left->right;
    while(block_ptr != NULL) {
        block_ptr->end = end;
        block_ptr = block_ptr->down;
    }

    aml_code_ptr += 1 + following_bytes;

    return SUCCESS;
}

uint32_t aml_RootChar(void *aml_block) {
    log("\n[AML] RootChar");
    aml_block_NameString_t *block = (aml_block_NameString_t *) aml_block;
    block->string = krealloc(block->string, block->length_of_string + 1);
    block->string[block->length_of_string++] = '\\';
    aml_code_ptr++;
    return SUCCESS;
}

uint32_t aml_PrefixPath(void *aml_block) {
    log("\n[AML] PrefixPath");
    aml_block_NameString_t *block = (aml_block_NameString_t *) aml_block;
    block->string = krealloc(block->string, block->length_of_string + 1);
    block->string[block->length_of_string++] = '^';
    aml_code_ptr++;
    return SUCCESS;
}

uint32_t aml_NameSeg(void *aml_block) {
    log("\n[AML] NameSeg");
    aml_block_NameString_t *block = (aml_block_NameString_t *) aml_block;
    if((aml_code_ptr + 4) > block->header.left->end) {
        log("\n[AML] ERROR: Section overflow");
        return ERROR;
    }
    uint8_t *ptr = aml_code_ptr;
    for(int i = 1; i <= 4; i++) {
        if((ptr[i] < '0' && ptr[i] > '9') && (ptr[i] < 'A' && ptr[i]> 'Z') && (ptr[i] != '_')) {
            log("\n[AML] ERROR: Invalid character in NameSeg");
            return ERROR;
        }
    }
    block->string = krealloc(block->string, block->length_of_string + 4);
    log(" ");
    for(int i = 0; i < 4; i++) {
        block->string[block->length_of_string++] = ptr[i];
        log("%c", ptr[i]);
    }
    aml_code_ptr += 4;
    return SUCCESS;
}

uint32_t aml_MultiNamePath(void *aml_block) {
    log("\n[AML] MultiNamePath");
    aml_code_ptr++;
    aml_block_NameString_t *block = (aml_block_NameString_t *) aml_block;
    if(aml_code_ptr > block->header.left->end) {
        log("\n[AML] ERROR: Section overflow");
        return ERROR;
    }
    uint8_t *ptr = aml_code_ptr;
    uint8_t SegCount = *ptr;
    aml_code_ptr++;
    if((aml_code_ptr + (SegCount * 4)) > block->header.left->end) {
        log("\n[AML] ERROR: Section overflow");
        return ERROR;
    }
    ptr++;
    for(int i = 0; i < SegCount; i++, ptr += 4) {
        if((ptr[0] < 'A' && ptr[0] > 'Z') && (ptr[0] != '_')) {
            log("\n[AML] ERROR: Invalid character in NameSeg");
            return ERROR;
        }
        for(int j = 1; j < 4; j++) {
            if((ptr[j] < '0' && ptr[j] > '9') && (ptr[j] < 'A' && ptr[j] > 'Z') && (ptr[j] != '_')) {
                log("\n[AML] ERROR: Invalid character in NameSeg");
                return ERROR;
            }
        }
    }
    block->string = krealloc(block->string, block->length_of_string + (SegCount * 4));
    ptr = aml_code_ptr;
    for(int i = 0; i < (SegCount * 4); i++) {
        block->string[block->length_of_string++] = ptr[i];
    }
    aml_code_ptr += (SegCount * 4);
    return SUCCESS;
}

uint32_t aml_RegionSpace(void *aml_block) {
    log("\n[AML] RegionSpace");
    aml_block_RegionSpace_t *block = (aml_block_RegionSpace_t *) aml_block;
    if(aml_code_ptr > block->header.left->end) {
        log("\n[AML] ERROR: Section overflow");
        return ERROR;
    }
    uint8_t region_space = *(uint8_t *) aml_code_ptr;
    if(region_space > 0x0A && region_space < 0x80) {
        log("\n[AML] ERROR: Invalid region space");
        return ERROR;
    }
    block->region_space = region_space;
    aml_code_ptr++;
    return SUCCESS;
}

uint32_t aml_WordConst(void *aml_block) {
    log("\n[AML] WordConst");
    aml_block_NumConst_t *block = (aml_block_NumConst_t *) aml_block;
    if((aml_code_ptr + 2) > block->header.left->end) {
        log("\n[AML] WordConst ERROR: Section overflow");
        return ERROR;
    }
    aml_code_ptr++;
    block->value = *(uint16_t *) aml_code_ptr;
    aml_code_ptr += 2;
    return SUCCESS;
}

uint32_t aml_OneOp(void *aml_block) {
    log("\n[AML] OneOp");
    aml_block_NumConst_t *block = (aml_block_NumConst_t *) aml_block;
    block->value = 1;
    aml_code_ptr++;
    return SUCCESS;
}

uint32_t aml_skip_zone(void *aml_block) {
    aml_block_t *block = (aml_block_t *) aml_block;
    aml_block_PkgLength_t *PkgLength_block = (aml_block_PkgLength_t *) find_aml_block_downward(block->right, b_PkgLength_t);
    aml_code_ptr = PkgLength_block->header.end + 1;
    return SUCCESS;
}

uint32_t aml_skip_opcode(void *aml_block) {
    aml_code_ptr++;
    return SUCCESS;
}

uint32_t aml_skip_ext_opcode(void *aml_block) {
    aml_code_ptr += 2;
    return SUCCESS;
}

uint32_t aml_FieldFlags(void *aml_block) {
    log("\n[AML] FieldFlags");
    aml_block_FieldFlags_t *block = (aml_block_FieldFlags_t *) aml_block;
    if(aml_code_ptr > block->header.left->end) {
        log("\n[AML] ERROR: Section overflow");
        return ERROR;
    }
    uint8_t field_flags = *(uint8_t *) aml_code_ptr;
    if((field_flags >> 7) != 0) {
        log("\n[AML] ERROR: Invalid field flags");
        return ERROR;
    }
    block->AccessType = field_flags & 0x0F;
    block->LockRule = (field_flags >> 4) & 0x1;
    block->UpdateRule = (field_flags >> 5) & 0x3;
    aml_code_ptr++;
    return SUCCESS;
}

uint32_t aml_debugpoint(void *aml_block) {
    log("\n[AML] DebugPoint");
    return SUCCESS;
}

uint32_t aml_MethodFlags(void *aml_block) {
    log("\n[AML] MethodFlags");
    aml_block_MethodFlags_t *block = (aml_block_MethodFlags_t *) aml_block;
    if(aml_code_ptr > block->header.left->end) {
        log("\n[AML] MethodFlags ERROR: Section overflow");
        return ERROR;
    }
    uint8_t method_flags = *(uint8_t *) aml_code_ptr;
    block->ArgCount = method_flags & 0x03;
    block->SerializeFlag = (method_flags >> 2) & 0x1;
    block->SyncLevel = (method_flags >> 3) & 0x1F;
    aml_code_ptr++;
    return SUCCESS;
}
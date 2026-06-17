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
#include <kernel/memory/memory_allocators.h>
#include <kernel/hardware/groups/logging/logging.h>

/* local variables */
uint8_t *aml_stack = NULL;
uint8_t *aml_stack_pointer = NULL;
uint8_t *aml_stack_end = NULL;

aml_interpreter_operation_stack_entry_t *aml_interpreter_operation_stack = NULL;
aml_interpreter_operation_stack_entry_t *aml_interpreter_operation_stack_first_entry = NULL;
uint32_t aml_interpreter_operation_stack_number_of_free_entries = 0;
aml_interpreter_operation_stack_entry_t **aml_interpreter_operation_stack_free_entries = NULL;

uint64_t aml_return_value = 0;

/* functions */
void aml_log_start(uint32_t depth) {
    log("\n");
    for(uint32_t i = 0; i < depth; i++) {
        log(" ");
    }
}

void dump_aml_code(uint8_t *ptr, uint32_t size) {
    uint32_t depth = 0;

    log("\n[AML] Dumping AML code of size %d bytes", size);

    // initialize stack
    aml_stack = kalloc(sizeof(uint64_t) * AML_STACK_SIZE);
    aml_stack_pointer = aml_stack;
    aml_stack_end = aml_stack + (sizeof(uint64_t) * AML_STACK_SIZE);

    // initialize operation stack
    aml_interpreter_operation_stack = kalloc(sizeof(aml_interpreter_operation_stack_entry_t) * AML_INTERPRETER_STACK_SIZE);
    aml_interpreter_operation_stack_free_entries = kalloc(sizeof(uint32_t) * AML_INTERPRETER_STACK_SIZE);
    for(uint32_t i = 0; i < AML_INTERPRETER_STACK_SIZE; i++) {
        aml_interpreter_operation_stack_free_entries[i] = &aml_interpreter_operation_stack[i];
    }
    aml_interpreter_operation_stack_number_of_free_entries = AML_INTERPRETER_STACK_SIZE;

    // AML code starts with TermList
    uint8_t **end_of_this_section = reserve_space_on_aml_stack(sizeof(uint8_t *));
    *end_of_this_section = (ptr + size);
    aml_ios_push(AML_TermList_Start, end_of_this_section, NULL);

    // process AML code
    while(aml_interpreter_operation_stack_first_entry != NULL) {
        // log("\nOperation: %d", aml_interpreter_operation_stack_first_entry->operation);
        switch(aml_interpreter_operation_stack_first_entry->operation) {
            // AML_DefScope = AML_ScopeOp AML_PkgLength AML_NameString AML_TermList_Start AML_Scope_End
            case AML_DefScope: {
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                aml_log_start(depth);
                log("[AML] DefScope");
                aml_scope_stack_t *scopeop_stack = reserve_space_on_aml_stack(sizeof(aml_scope_stack_t));
                aml_ios_push(AML_Scope_End, scopeop_stack, NULL);
                aml_ios_push(AML_TermList_Start, &scopeop_stack->end_of_section, NULL);
                aml_ios_push(AML_NameString, &scopeop_stack->namestring, end_of_this_section);
                aml_ios_push(AML_PkgLength, &scopeop_stack->end_of_section, end_of_this_section);
                aml_ios_push(AML_ScopeOp, NULL, end_of_this_section);
                break;
            }
            // AML_ScopeOp equals 0x10
            case AML_ScopeOp: {
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 1) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                if(*ptr != 0x10) {
                    log("\n[AML] ERROR: Expected ScopeOp, got 0x%02x", *ptr);
                    return;
                }
                ptr++;
                break;
            }
            case AML_Scope_End: {
                aml_scope_stack_t *scopeop_stack_entry = aml_interpreter_operation_stack_first_entry->stack;
                aml_ios_remove();
                remove_space_from_aml_stack(sizeof(aml_scope_stack_t));
                // TODO: implement fully
                break;
            }

            // AML_DefOpRegion = AML_OpRegionOp AML_NameString AML_RegionSpace AML_RegionOffset AML_RegionLen AML_OpRegion_End
            case AML_DefOpRegion: {
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                aml_log_start(depth);
                log("[AML] DefOpRegion");
                aml_opregion_stack_t *opregion_stack = reserve_space_on_aml_stack(sizeof(aml_opregion_stack_t));
                aml_ios_push(AML_OpRegion_End, opregion_stack, NULL);
                aml_ios_push(AML_RegionLen, &opregion_stack->region_len, end_of_this_section);
                aml_ios_push(AML_RegionOffset, &opregion_stack->region_offset, end_of_this_section);
                aml_ios_push(AML_RegionSpace, &opregion_stack->region_space, end_of_this_section);
                aml_ios_push(AML_NameString, &opregion_stack->namestring, end_of_this_section);
                aml_ios_push(AML_OpRegionOp, NULL, end_of_this_section);
                break;
            }
            // AML_OpRegionOp equals 0x5B 0x80
            case AML_OpRegionOp: {
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 2) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                if(ptr[0] != 0x5B || ptr[1] != 0x80) {
                    log("\n[AML] ERROR: Expected OpRegionOp, got 0x%02x 0x%02x", ptr[0], ptr[1]);
                    return;
                }
                ptr += 2;
                break;
            }
            case AML_RegionSpace: {
                uint8_t *region_space = aml_interpreter_operation_stack_first_entry->stack;
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 1) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                uint8_t data = *ptr;
                *region_space = data;
                log(" RegionSpace: ");
                switch(data) {
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
                        if(data >= 0x80) {
                            log("OEM defined 0x%02x", data);
                        }
                        else {
                            log("Unknown type 0x%02x", data);
                        }
                        break;
                    }
                }
                ptr++;
                break;
            }
            case AML_RegionOffset: {
                void *save_integer = aml_interpreter_operation_stack_first_entry->stack;
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                log(" RegionOffset: ");
                aml_ios_push(AML_RegionOffset_ProcessReturnedValue, save_integer, NULL);
                aml_ios_push(AML_TermArg, NULL, end_of_this_section);
                break;
            }
            case AML_RegionLen: {
                void *save_integer = aml_interpreter_operation_stack_first_entry->stack;
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                log(" RegionLen: ");
                aml_ios_push(AML_RegionLen_ProcessReturnedValue, save_integer, NULL);
                aml_ios_push(AML_TermArg, NULL, end_of_this_section);
                break;
            }
            case AML_RegionOffset_ProcessReturnedValue: 
            case AML_RegionLen_ProcessReturnedValue: {
                uint64_t *save_integer = aml_interpreter_operation_stack_first_entry->stack;
                aml_ios_remove();
                *save_integer = aml_return_value;
                log("0x%x%x", (uint32_t)(aml_return_value >> 32), (uint32_t)(aml_return_value));
                break;
            }
            case AML_OpRegion_End: {
                aml_ios_remove();
                remove_space_from_aml_stack(sizeof(aml_opregion_stack_t));
                break;
            }

            // AML_PkgLength = range 1 - 4 bytes, contains length of section
            case AML_PkgLength: {
                uint8_t **end_of_section_variable = aml_interpreter_operation_stack_first_entry->stack;
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 1) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                uint8_t following_bytes = (ptr[0] >> 6);
                if((ptr + 1 + following_bytes) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
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
                if(length < (following_bytes + 1)) {
                    log("\n[AML] ERROR: PkgLength does not include itself");
                    return;
                }
                if(end_of_section_variable != NULL) {
                    *end_of_section_variable = ptr + length;
                }
                ptr += 1 + following_bytes;
                log(" PkgLength: %d", length);
                break;  
            }

            // AML_NameString = '\' AML_NamePath AML_NameString_End | AML_PrefixPath AML_NamePath AML_NameString_End
            case AML_NameString: {
                aml_namestring_t *namestring = aml_interpreter_operation_stack_first_entry->stack;
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                aml_ios_push(AML_NameString_End, namestring, NULL);
                if((ptr + 1) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                uint8_t starting_character = *ptr;
                switch(starting_character) {
                    case '\\': {
                        log("\nAML_RootChar");
                        namestring->number_of_characters++;
                        namestring->name = krealloc(namestring->name, namestring->number_of_characters + 1);
                        namestring->name[namestring->number_of_characters - 1] = '\\';
                        namestring->name[namestring->number_of_characters] = '\0';
                        ptr++; // skip RootChar
                        aml_ios_push(AML_NamePath, namestring, end_of_this_section);
                        break;
                    }
                    case '^': {
                        aml_ios_push(AML_NamePath, namestring, end_of_this_section);
                        aml_ios_push(AML_PrefixPath, namestring, end_of_this_section);
                        break;
                    }
                    default: {
                        aml_ios_push(AML_NamePath, namestring, end_of_this_section);
                        break;
                    }
                }
                break;
            }
            // AML_PrefixPath = nothing | '^' AML_PrefixPath 
            case AML_PrefixPath: {
                aml_namestring_t *namestring = aml_interpreter_operation_stack_first_entry->stack;
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 1) > end_of_this_section) {
                    log("\n[AML] WARNING: Section ended with PrefixPath");
                    break;
                }
                uint8_t character = *ptr;
                if(character == '^') {
                    namestring->number_of_characters++;
                    namestring->name = krealloc(namestring->name, namestring->number_of_characters + 1);
                    namestring->name[namestring->number_of_characters - 1] = '^';
                    namestring->name[namestring->number_of_characters] = '\0';
                    aml_ios_push(AML_PrefixPath, namestring, end_of_this_section);
                }
                break;
            }
            // AML_NamePath = '\0' | AML_DualNamePath | AML_MultiNamePath | AML_NameSeg
            case AML_NamePath: {
                log("\nAML_NamePath");
                aml_namestring_t *namestring = aml_interpreter_operation_stack_first_entry->stack;
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 1) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                uint8_t character = *ptr;
                switch(character) {
                    case 0x00: {
                        log("\nAML_NullName");
                        namestring->name = krealloc(namestring->name, namestring->number_of_characters + 1);
                        namestring->name[namestring->number_of_characters] = '\0';
                        ptr++; // skip NullName
                        break;
                    }
                    case 0x2E: {
                        aml_ios_push(AML_DualNamePath, namestring, end_of_this_section);
                        break;
                    }
                    case 0x2F: {
                        aml_ios_push(AML_MultiNamePath, namestring, end_of_this_section);
                        break;
                    }
                    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
                    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
                    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                    case 'Y': case 'Z': case '_': {
                        aml_ios_push(AML_NameSeg, namestring, end_of_this_section);
                        break;
                    }
                    default: {
                        log("\n[AML] ERROR: Invalid NamePath start");
                        return;
                    }
                }
                break;
            }
            // AML_DualNamePath = DualNamePrefix AML_NameSeg AML_NameSeg
            case AML_DualNamePath: {
                log("\nAML_DualNamePath");
                aml_namestring_t *namestring = aml_interpreter_operation_stack_first_entry->stack;
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 1) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow %x %x", ptr, end_of_this_section);
                    return;
                }
                ptr++; // skip DualNamePrefix
                aml_ios_push(AML_NameSeg, namestring, end_of_this_section);
                aml_ios_push(AML_NameSeg, namestring, end_of_this_section);
                break;
            }
            // AML_MultiNamePath = MultiNamePrefix SegCount AML_NameSeg(SegCount)
            case AML_MultiNamePath: {
                log("\nAML_MultiNamePath");
                aml_namestring_t *namestring = aml_interpreter_operation_stack_first_entry->stack;
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 2) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                ptr++; // skip MultiNamePrefix
                uint32_t number_of_namesegs = *ptr;
                ptr++; // skip SegCount
                for(uint32_t i = 0; i < number_of_namesegs; i++) {
                    aml_ios_push(AML_NameSeg, namestring, end_of_this_section);
                }
                break;
            }
            // AML_NameSeg = LeadNameChar NameChar NameChar NameChar
            case AML_NameSeg: {
                log("\nAML_NameSeg");
                aml_namestring_t *namestring = aml_interpreter_operation_stack_first_entry->stack;
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 4) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }

                // make space for NameSeg
                namestring->number_of_characters += 4;
                namestring->name = krealloc(namestring->name, namestring->number_of_characters + 1);
                namestring->name[namestring->number_of_characters] = '\0';

                // copy LeadNameChar
                switch(*ptr) {
                    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
                    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
                    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                    case 'Y': case 'Z': case '_': {
                        namestring->name[namestring->number_of_characters - 4] = *ptr;
                        break;
                    }
                    default: {
                        log("\n[AML] ERROR: Invalid NameSeg LeadNameChar");
                        return;
                    }
                }
                ptr++;

                // copy three NameChars
                for(int i = 3; i >= 1; i--) {
                    switch(*ptr) {
                        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
                        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
                        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                        case 'Y': case 'Z': case '_': case '0': case '1': case '2':
                        case '3': case '4': case '5': case '6': case '7': case '8':
                        case '9': {
                            namestring->name[namestring->number_of_characters - i] = *ptr;
                            break;
                        }
                        default: {
                            log("\n[AML] ERROR: Invalid NameSeg NameChar");
                            return;
                        }
                    }
                    ptr++;
                }
                break;
            }
            case AML_NameString_End: {
                aml_namestring_t *namestring = aml_interpreter_operation_stack_first_entry->stack;
                aml_ios_remove();
                log(" NameString: %s", namestring->name);
                break;
            }

            case AML_TermList_Start: {
                // log("\nAML_TermList_Start");
                uint8_t **end_of_termlist_section = aml_interpreter_operation_stack_first_entry->stack;
                aml_ios_remove();
                aml_ios_push(AML_TermList, NULL, *end_of_termlist_section);
                break;
            }
            case AML_TermList: {
                // log("\nAML_TermList");
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if(ptr < end_of_this_section) {
                    aml_ios_push(AML_TermList, NULL, end_of_this_section);
                    aml_ios_push(AML_TermObj, NULL, end_of_this_section);
                }
                break;
            }
            case AML_TermObj: {
                // log("\nAML_TermObj");
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 1) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                uint8_t opcode = ptr[0];
                switch(opcode) {
                    // Object
                        // NameSpaceModifierObj
                            // DefAlias
                            case 0x06: // AliasOp
                            // DefName
                            case 0x08: // NameOp
                            // DefScope
                            case 0x10: { // ScopeOp
                                aml_ios_push(AML_DefScope, NULL, end_of_this_section);
                                break;
                            }
                        // NamedObj
                            // DefCreateBitField
                            case 0x8D: // CreateBitFieldOp
                            // DefCreateByteField
                            case 0x8C: // CreateByteFieldOp
                            // DefCreateWordField
                            case 0x8B: // CreateWordFieldOp
                            // DefCreateDWordField
                            case 0x8A: // CreateDWordFieldOp
                            // DefCreateQWordField
                            case 0x8F: // CreateQWordFieldOp
                            // DefExternal
                            case 0x15: // ExternalOp
                    // StatementOpcode
                        // DefBreak
                        case 0xA5: // BreakOp
                        // DefBreakPoint
                        case 0xCC: // BreakPointOp
                        // DefContinue
                        case 0x9F: // ContinueOp
                        // DefIfElse
                        case 0xA0: // IfOp
                        // DefNoop
                        case 0xA3: // NoopOp
                        // DefNotify
                        case 0x86: // NotifyOp
                        // DefReturn
                        case 0xA4: // ReturnOp
                        // DefWhile
                        case 0xA2: // WhileOp
                    // ExpressionOpcode
                        // DefAdd
                        case 0x72: // AddOp
                        // DefAnd
                        case 0x7B: // AndOp
                        // DefBuffer
                        case 0x11: // BufferOp
                        // DefConcat
                        case 0x73: // ConcatOp
                        // DefConcatRes
                        case 0x84: // ConcatResOp
                        // DefCopyObject
                        case 0x9D: // CopyObjectOp
                        // DefDecrement
                        case 0x76: // DecrementOp
                        // DefDerefOf
                        case 0x83: // DerefOfOp
                        // DefDivide
                        case 0x78: // DivideOp
                        // DefFindSetLeftBit
                        case 0x81: // FindSetLeftBitOp
                        // DefFindSetRightBit
                        case 0x82: // FindSetRightBitOp
                        // DefIncrement
                        case 0x75: // IncrementOp
                        // DefIndex
                        case 0x88: // IndexOp
                        // DefLAnd
                        case 0x90: // LAndOp
                        // DefLEqual
                        case 0x93: // LEqualOp
                        // DefLGreater
                        case 0x94: // LGreaterOp
                        // DefLGreaterEqual
                        // DefLLessEqual
                        // DefLNot
                        // DefLNotEqual
                        case 0x92: // LNotOp
                        // DefLLess
                        case 0x95: // LLessOp
                        // DefMid
                        case 0x9E: // MidOp
                        // DefLOr
                        case 0x91: // LOrOp
                        // DefMatch
                        case 0x89: // MatchOp
                        // DefMod
                        case 0x85: // ModOp
                        // DefMultiply
                        case 0x77: // MultiplyOp
                        // DefNAnd
                        case 0x7C: // NAndOp
                        // DefNOr
                        case 0x7E: // NOrOp
                        // DefNot
                        case 0x80: // NotOp
                        // DefObjectType
                        case 0x8E: // ObjectTypeOp
                        // DefOr
                        case 0x7D: // OrOp
                        // DefPackage
                        case 0x12: // PackageOp
                        // DefVarPackage
                        case 0x13: // VarPackageOp
                        // DefRefOf
                        case 0x71: // RefOfOp
                        // DefShiftLeft
                        case 0x79: // ShiftLeftOp
                        // DefShiftRight
                        case 0x7A: // ShiftRightOp
                        // DefSizeOf
                        case 0x87: // SizeOfOp
                        // DefStore
                        case 0x70: // StoreOp
                        // DefSubtract
                        case 0x74: // SubtractOp
                        // DefToBuffer
                        case 0x96: // ToBufferOp
                        // DefToDecimalString
                        case 0x97: // ToDecimalStringOp
                        // DefToHexString
                        case 0x98: // ToHexStringOp
                        // DefToInteger
                        case 0x99: // ToIntegerOp
                        // DefToString
                        case 0x9C: // ToStringOp
                        // DefXor
                        case 0x7F: // XorOp
                    // ExtOpPrefix
                    case 0x5B: {
                        if((ptr + 2) > end_of_this_section) {
                            log("\n[AML] ERROR: Section overflow");
                            return;
                        }
                        uint8_t extended_opcode = ptr[1];
                        switch(extended_opcode) {
                            // Object
                                // NamedObj
                                    // DefBankField
                                    case 0x87: // BankFieldOp
                                    // DefCreateField
                                    case 0x13: // CreateFieldOp
                                    // DefDataRegion
                                    case 0x88: // DataRegionOp
                                    // DefOpRegion
                                    case 0x80: { // OpRegionOp
                                        aml_ios_push(AML_DefOpRegion, NULL, end_of_this_section);
                                        break;
                                    }
                                    // DefField
                                    case 0x81: { // FieldOp This is not directly in specification, but it is used here
                                        aml_ios_push(AML_DefField, NULL, end_of_this_section);
                                        break;
                                    }
                                    // DefPowerRes
                                    case 0x84: // PowerResOp
                                    // DefThermalZone
                                    case 0x85: // ThermalZoneOp
                                    // DefProcessorOp
                                    case 0x83: // ProcessorOp
                            // StatementOpcode
                                // DefFatal
                                case 0x32: // FatalOp
                                // DefRelease
                                case 0x27: // ReleaseOp
                                // DefReset
                                case 0x26: // ResetOp
                                // DefSignal
                                case 0x24: // SignalOp
                                // DefSleep
                                case 0x22: // SleepOp
                                // DefStall
                                case 0x21: // StallOp
                            // ExpressionOpcode
                                // DefAcquire
                                case 0x23: // AcquireOp
                                // DefCondRefOf
                                case 0x12: // CondRefOfOp
                                // DefFromBCD
                                case 0x28: // FromBCDOp
                                // DefLoadTable
                                case 0x1F: // LoadTableOp
                                // DefTimer
                                case 0x33: // TimerOp
                                // DefToBCD
                                case 0x29: // ToBCDOp
                                // DefWait
                                case 0x25: // WaitOp
                            
                            default:
                            log("\n[AML] ERROR: Unimplemented extended opcode 0x%02x", extended_opcode);
                            return;
                        }
                        break;
                    }

                    log("\n[AML] ERROR: Unimplemented opcode 0x%02x", opcode);
                    return;

                    // MethodInvocation
                    default: {
                        aml_ios_push(AML_MethodInvocation, NULL, end_of_this_section);
                        break;
                    }
                }
                break;
            }

            case AML_TermArg: {
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 1) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                uint8_t opcode = ptr[0];
                switch(opcode) {
                    // ExpressionOpcode
                        // DefAdd
                        case 0x72: // AddOp
                        // DefAnd
                        case 0x7B: // AndOp
                        // DefBuffer
                        case 0x11: // BufferOp
                        // DefConcat
                        case 0x73: // ConcatOp
                        // DefConcatRes
                        case 0x84: // ConcatResOp
                        // DefCopyObject
                        case 0x9D: // CopyObjectOp
                        // DefDecrement
                        case 0x76: // DecrementOp
                        // DefDerefOf
                        case 0x83: // DerefOfOp
                        // DefDivide
                        case 0x78: // DivideOp
                        // DefFindSetLeftBit
                        case 0x81: // FindSetLeftBitOp
                        // DefFindSetRightBit
                        case 0x82: // FindSetRightBitOp
                        // DefIncrement
                        case 0x75: // IncrementOp
                        // DefIndex
                        case 0x88: // IndexOp
                        // DefLAnd
                        case 0x90: // LAndOp
                        // DefLEqual
                        case 0x93: // LEqualOp
                        // DefLGreater
                        case 0x94: // LGreaterOp
                        // DefLGreaterEqual
                        // DefLLessEqual
                        // DefLNot
                        // DefLNotEqual
                        case 0x92: // LNotOp
                        // DefLLess
                        case 0x95: // LLessOp
                        // DefMid
                        case 0x9E: // MidOp
                        // DefLOr
                        case 0x91: // LOrOp
                        // DefMatch
                        case 0x89: // MatchOp
                        // DefMod
                        case 0x85: // ModOp
                        // DefMultiply
                        case 0x77: // MultiplyOp
                        // DefNAnd
                        case 0x7C: // NAndOp
                        // DefNOr
                        case 0x7E: // NOrOp
                        // DefNot
                        case 0x80: // NotOp
                        // DefObjectType
                        case 0x8E: // ObjectTypeOp
                        // DefOr
                        case 0x7D: // OrOp
                        // DefPackage
                        case 0x12: // PackageOp
                        // DefVarPackage
                        case 0x13: // VarPackageOp
                        // DefRefOf
                        case 0x71: // RefOfOp
                        // DefShiftLeft
                        case 0x79: // ShiftLeftOp
                        // DefShiftRight
                        case 0x7A: // ShiftRightOp
                        // DefSizeOf
                        case 0x87: // SizeOfOp
                        // DefStore
                        case 0x70: // StoreOp
                        // DefSubtract
                        case 0x74: // SubtractOp
                        // DefToBuffer
                        case 0x96: // ToBufferOp
                        // DefToDecimalString
                        case 0x97: // ToDecimalStringOp
                        // DefToHexString
                        case 0x98: // ToHexStringOp
                        // DefToInteger
                        case 0x99: // ToIntegerOp
                        // DefToString
                        case 0x9C: // ToStringOp
                        // DefXor
                        case 0x7F: // XorOp
                    // ExtOpPrefix
                    case 0x5B: {
                        if((ptr + 2) > end_of_this_section) {
                            log("\n[AML] ERROR: Section overflow");
                            return;
                        }
                        uint8_t extended_opcode = ptr[1];
                        switch(extended_opcode) {
                            // ExpressionOpcode
                                // DefAcquire
                                case 0x23: // AcquireOp
                                // DefCondRefOf
                                case 0x12: // CondRefOfOp
                                // DefFromBCD
                                case 0x28: // FromBCDOp
                                // DefLoadTable
                                case 0x1F: // LoadTableOp
                                // DefTimer
                                case 0x33: // TimerOp
                                // DefToBCD
                                case 0x29: // ToBCDOp
                                // DefWait
                                case 0x25: // WaitOp
                            // DataObject
                                // ComputationalData
                                    // RevisionOp
                                    case 0x30: // RevisionOp
                            
                            log("\n[AML] ERROR: Unimplemented extended opcode 0x%02x", extended_opcode);
                            return;
                        }
                        break;
                    }

                    // DataObject
                        // ComputationalData
                            // ByteConst
                            case 0x0A: { // BytePrefix
                                aml_ios_push(AML_ByteConst, NULL, end_of_this_section);
                                break;
                            }
                            // WordConst
                            case 0x0B: { // WordPrefix
                                aml_ios_push(AML_WordConst, NULL, end_of_this_section);
                                break;
                            }
                            // DWordConst
                            case 0x0C: // DWordPrefix
                            // QWordConst
                            case 0x0E: // QWordPrefix
                            // String
                            case 0x0D: // StringPrefix
                            // ConstObj
                                case 0x00: // ZeroOp
                                case 0x01: // OneOp
                                case 0xFF: { // OnesOp
                                    aml_ios_push(AML_ConstObj, NULL, end_of_this_section);
                                    break;
                                }
                            // DefBuffer
                            // case 0x11: // BufferOp Already defined above
                        // DefPackage
                        // case 0x12: // PackageOp Already defined above
                        // DefVarPackage
                        // case 0x13: // VarPackageOp Already defined above
                    
                    // ArgObj
                    case 0x68: // Arg0Op
                    case 0x69: // Arg1Op
                    case 0x6A: // Arg2Op
                    case 0x6B: // Arg3Op
                    case 0x6C: // Arg4Op
                    case 0x6D: // Arg5Op
                    case 0x6E: // Arg6Op
                    
                    // LocalObj
                    case 0x60: // Local0Op
                    case 0x61: // Local1Op
                    case 0x62: // Local2Op
                    case 0x63: // Local3Op
                    case 0x64: // Local4Op
                    case 0x65: // Local5Op
                    case 0x66: // Local6Op
                    case 0x67: // Local7Op

                    log("\n[AML] ERROR: Unimplemented opcode 0x%02x", opcode);
                    return;
                }
                break;
            }

            case AML_ByteConst: {
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 2) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                if(ptr[0] != 0x0A) {
                    log("\n[AML] ERROR: Expected ByteConst, got 0x%02x", ptr[0]);
                    return;
                }
                uint8_t *value = (uint8_t *) (ptr + 1);
                aml_return_value = *value;
                ptr += 2;
                break;
            }
            case AML_WordConst: {
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 3) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                if(ptr[0] != 0x0B) {
                    log("\n[AML] ERROR: Expected WordConst, got 0x%02x", ptr[0]);
                    return;
                }
                uint16_t *value = (uint16_t *) (ptr + 1);
                aml_return_value = *value;
                ptr += 3;
                break;
            }

            case AML_ConstObj: {
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                if((ptr + 1) > end_of_this_section) {
                    log("\n[AML] ERROR: Section overflow");
                    return;
                }
                switch(*ptr) {
                    case 0x00: {
                        aml_return_value = 0;
                        break;
                    }
                    case 0x01: {
                        aml_return_value = 1;
                        break;
                    }
                    case 0xFF: {
                        // TODO: return 32 bits if ACPI version is below 2.0
                        aml_return_value = 0xFFFFFFFFFFFFFFFF;
                        break;
                    }
                    default: {
                        log("\n[AML] ERROR: Expected ConstObj, got 0x%02x", ptr[0]);
                        return;
                    }
                }
                ptr++;
                break;
            }

            case AML_DefField: {
                uint8_t *end_of_this_section = aml_interpreter_operation_stack_first_entry->end_of_section;
                aml_ios_remove();
                aml_log_start(depth);
                log("[AML] DefField");
                aml_scope_stack_t *scopeop_stack = reserve_space_on_aml_stack(sizeof(aml_scope_stack_t));
                aml_ios_push(AML_Field_End, scopeop_stack, NULL);
                aml_ios_push(AML_FieldList_Start, &scopeop_stack->end_of_section, NULL);
                aml_ios_push(AML_FieldFlags, &scopeop_stack->end_of_section, NULL);
                aml_ios_push(AML_NameString, &scopeop_stack->namestring, end_of_this_section);
                aml_ios_push(AML_PkgLength, &scopeop_stack->end_of_section, end_of_this_section);
                aml_ios_push(AML_FieldOp, NULL, end_of_this_section); // <- implement this
                break;
            }

            default: {
                log("\n[AML] ERROR: Unknown operation %d", aml_interpreter_operation_stack_first_entry->operation);
                return;
            }
        }
    }
}

void *reserve_space_on_aml_stack(uint32_t size) {
    if((aml_stack_pointer + size) > aml_stack_end) {
        log("\n[AML] ERROR: Stack underflow");
        return NULL;
    }
    void *result = aml_stack_pointer;
    memset(result, 0, size);
    aml_stack_pointer += size;
    return result;
}

void remove_space_from_aml_stack(uint32_t size) {
    if((aml_stack_pointer - size) < aml_stack) {
        log("\n[AML] ERROR: Stack overflow");
        return;
    }
    aml_stack_pointer -= size;
}

aml_interpreter_operation_stack_entry_t *pop_aml_stack_free_entry(void) {
    if(aml_interpreter_operation_stack_number_of_free_entries == 0) {
        log("\n[AML] ERROR: Free entries stack underflow");
        return NULL;
    }
    return aml_interpreter_operation_stack_free_entries[--aml_interpreter_operation_stack_number_of_free_entries];
}

void push_aml_stack_free_entry(aml_interpreter_operation_stack_entry_t *entry) {
    if(aml_interpreter_operation_stack_number_of_free_entries == AML_INTERPRETER_STACK_SIZE) {
        log("\n[AML] ERROR: Free entries stack overflow");
        return;
    }
    aml_interpreter_operation_stack_free_entries[aml_interpreter_operation_stack_number_of_free_entries++] = entry;
}

void aml_ios_push(aml_interpreter_operation_t operation, void *stack, uint8_t *end_of_section) {
    aml_interpreter_operation_stack_entry_t *entry = pop_aml_stack_free_entry();
    if(entry == NULL) {
        return;
    }
    entry->operation = operation;
    entry->stack = stack;
    entry->end_of_section = end_of_section;
    entry->next = aml_interpreter_operation_stack_first_entry;
    entry->prev = NULL;
    if(aml_interpreter_operation_stack_first_entry != NULL) {
        aml_interpreter_operation_stack_first_entry->prev = entry;
    }
    aml_interpreter_operation_stack_first_entry = entry;
}

void aml_ios_remove(void) {
    if(aml_interpreter_operation_stack_first_entry == NULL) {
        return;
    }
    push_aml_stack_free_entry(aml_interpreter_operation_stack_first_entry);
    if(aml_interpreter_operation_stack_first_entry->next != NULL) {
        aml_interpreter_operation_stack_first_entry->next->prev = NULL;
    }
    aml_interpreter_operation_stack_first_entry = aml_interpreter_operation_stack_first_entry->next;
}
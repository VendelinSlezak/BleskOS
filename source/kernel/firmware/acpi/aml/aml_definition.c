/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* global variables */
extern uint32_t aml_PkgLength(void *aml_block);
extern uint32_t aml_RootChar(void *aml_block);
extern uint32_t aml_PrefixPath(void *aml_block);
extern uint32_t aml_NameSeg(void *aml_block);
extern uint32_t aml_MultiNamePath(void *aml_block);
extern uint32_t aml_RegionSpace(void *aml_block);
extern uint32_t aml_WordConst(void *aml_block);
extern uint32_t aml_OneOp(void *aml_block);
extern uint32_t aml_FieldFlags(void *aml_block);
extern uint32_t aml_MethodFlags(void *aml_block);

extern uint32_t aml_skip_opcode(void *aml_block);
extern uint32_t aml_skip_ext_opcode(void *aml_block);
extern uint32_t aml_skip_zone(void *aml_block);

extern uint32_t aml_debugpoint(void *aml_block);

const char* aml_state_names[] = {
    [0] = "UNKNOWN",

    [b_table_t] = "Table",

    [b_TermList_t] = "TermList",
    [b_TermListEntry_t] = "TermListEntry",
    [b_TermObj_t] = "TermObj",
    [b_TermArg_t] = "TermArg",

    [b_Object_t] = "Object",
    [b_StatementOpcode_t] = "StatementOpcode",
    [b_ExpressionOpcode_t] = "ExpressionOpcode",

    [b_NameSpaceModifierObj_t] = "NameSpaceModifierObj",
    [b_NamedObj_t] = "NamedObj",

    [b_DefAlias_t] = "DefAlias",
    [b_DefName_t] = "DefName",
    [b_DefScope_t] = "DefScope",

    [b_DefBankField_t] = "DefBankField",
    [b_DefCreateBitField_t] = "DefCreateBitField",
    [b_DefCreateByteField_t] = "DefCreateByteField",
    [b_DefCreateDWordField_t] = "DefCreateDWordField",
    [b_DefCreateField_t] = "DefCreateField",
    [b_DefCreateQWordField_t] = "DefCreateQWordField",
    [b_DefCreateWordField_t] = "DefCreateWordField",
    [b_DefDataRegion_t] = "DefDataRegion",
    [b_DefExternal_t] = "DefExternal",
    [b_DefOpRegion_t] = "DefOpRegion",
        [b_RegionSpace_t] = "RegionSpace",
        [b_RegionOffset_t] = "RegionOffset",
        [b_RegionLen_t] = "RegionLen",
    [b_DefProcessorOp_t] = "DefProcessorOp",
    [b_DefPowerRes_t] = "DefPowerRes",
    [b_DefThermalZone_t] = "DefThermalZone",
    [b_DefMethod_t] = "DefMethod",
        [b_MethodFlags_t] = "MethodFlags",

    [b_NameString_t] = "NameString",
        [b_NameString_RootPath_t] = "NameString_RootPath",
        [b_NameString_PrefixPath_t] = "NameString_PrefixPath",
        [b_NamePath_t] = "NamePath",
        [b_PrefixPath_t] = "PrefixPath",
        [b_NameSeg_t] = "NameSeg",
        [b_DualNamePath_t] = "DualNamePath",
        [b_MultiNamePath_t] = "MultiNamePath",
        [b_NullName_t] = "NullName",

    [b_DataRefObject_t] = "DataRefObject",
    [b_PkgLength_t] = "PkgLength",

    [b_DefBreak_t] = "DefBreak",
    [b_DefBreakPoint_t] = "DefBreakPoint",
    [b_DefContinue_t] = "DefContinue",
    [b_DefFatal_t] = "DefFatal",
    [b_DefIfElse_t] = "DefIfElse",
    [b_DefNoop_t] = "DefNoop",
    [b_DefNotify_t] = "DefNotify",
    [b_DefRelease_t] = "DefRelease",
    [b_DefReset_t] = "DefReset",
    [b_DefReturn_t] = "DefReturn",
    [b_DefSignal_t] = "DefSignal",
    [b_DefSleep_t] = "DefSleep",
    [b_DefStall_t] = "DefStall",
    [b_DefWhile_t] = "DefWhile",

    [b_DefAcquire_t] = "DefAcquire",
    [b_DefAdd_t] = "DefAdd",
    [b_DefAnd_t] = "DefAnd",
    [b_DefBuffer_t] = "DefBuffer",
    [b_DefConcat_t] = "DefConcat",
    [b_DefConcatRes_t] = "DefConcatRes",
    [b_DefCondRefOf_t] = "DefCondRefOf",
    [b_DefCopyObject_t] = "DefCopyObject",
    [b_DefDecrement_t] = "DefDecrement",
    [b_DefDerefOf_t] = "DefDerefOf",
    [b_DefDivide_t] = "DefDivide",
    [b_DefFindSetLeftBit_t] = "DefFindSetLeftBit",
    [b_DefFindSetRightBit_t] = "DefFindSetRightBit",
    [b_DefFromBCD_t] = "DefFromBCD",
    [b_DefIncrement_t] = "DefIncrement",
    [b_DefIndex_t] = "DefIndex",
    [b_DefLAnd_t] = "DefLAnd",
    [b_DefLEqual_t] = "DefLEqual",
    [b_DefLGreater_t] = "DefLGreater",
    [b_DefLGreaterEqual_t] = "DefLGreaterEqual",
    [b_DefLLess_t] = "DefLLess",
    [b_DefLLessEqual_t] = "DefLLessEqual",
    [b_DefMid_t] = "DefMid",
    [b_DefLNot_t] = "DefLNot",
    [b_DefLNotEqual_t] = "DefLNotEqual",
    [b_DefLoadTable_t] = "DefLoadTable",
    [b_DefLOr_t] = "DefLOr",
    [b_DefMatch_t] = "DefMatch",
    [b_DefMod_t] = "DefMod",
    [b_DefMultiply_t] = "DefMultiply",
    [b_DefNAnd_t] = "DefNAnd",
    [b_DefNOr_t] = "DefNOr",
    [b_DefNot_t] = "DefNot",
    [b_DefObjectType_t] = "DefObjectType",
    [b_DefOr_t] = "DefOr",
    [b_DefPackage_t] = "DefPackage",
    [b_DefVarPackage_t] = "DefVarPackage",
    [b_DefRefOf_t] = "DefRefOf",
    [b_DefShiftLeft_t] = "DefShiftLeft",
    [b_DefShiftRight_t] = "DefShiftRight",
    [b_DefSizeOf_t] = "DefSizeOf",
    [b_DefStore_t] = "DefStore",
    [b_DefSubtract_t] = "DefSubtract",
    [b_DefTimer_t] = "DefTimer",
    [b_DefToBCD_t] = "DefToBCD",
    [b_DefToBuffer_t] = "DefToBuffer",
    [b_DefToDecimalString_t] = "DefToDecimalString",
    [b_DefToHexString_t] = "DefToHexString",
    [b_DefToInteger_t] = "DefToInteger",
    [b_DefToString_t] = "DefToString",
    [b_DefWait_t] = "DefWait",
    [b_DefXOr_t] = "DefXOr",
    [b_MethodInvocation_t] = "MethodInvocation",

    [b_ByteConst_t] = "ByteConst",
    [b_WordConst_t] = "WordConst",
    [b_DWordConst_t] = "DWordConst",
    [b_QWordConst_t] = "QWordConst",
    [b_String_t] = "String",
    [b_ZeroOp_t] = "ZeroOp",
    [b_OneOp_t] = "OneOp",
    [b_OnesOp_t] = "OnesOp",
    [b_RevisionOp_t] = "RevisionOp",

    [b_Nothing_t] = "Nothing",
    [b_MoveLeftInNameSpace_t] = "MoveLeftInNameSpace",

    [b_DefField_t] = "DefField",
        [b_FieldFlags_t] = "FieldFlags",
        [b_FieldList_t] = "FieldList",
        [b_FieldElement_t] = "FieldElement",
        [b_FieldListElement_t] = "FieldListElement",
    
    [b_NamedField_t] = "NamedField",
    [b_ReservedField_t] = "ReservedField",
    [b_AccessField_t] = "AccessField",
    [b_ExtendedAccessField_t] = "ExtendedAccessField",
    [b_ConnectField_t] = "ConnectField",

    [b_DefDevice_t] = "DefDevice",
    [b_Operand_t] = "Operand",
    [b_Target_t] = "Target",

    [b_SkipZone_t] = "SkipZone",
    [b_Error_t] = "Error",

    [b_number_of_aml_states] = "number_of_aml_states"
};

aml_rule_t *convert_state_to_rule[b_number_of_aml_states] = {
    [b_MoveLeftInNameSpace_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = false,
        .process_state = NULL,
        .what_to_do_after = MOVE_LEFT
    },
    [b_SkipZone_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = false,
        .process_state = &aml_skip_zone,
        .what_to_do_after = MOVE_LEFT
    },

    [b_TermList_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = false,
        .process_state = NULL,
        .what_to_do_after = ADD_MULTIPLE_STATES,
        .states = (uint32_t []){ b_TermListEntry_t, b_Nothing_t, 0 }
    },
    [b_TermListEntry_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = false,
        .process_state = NULL,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_TermObj_t, b_TermList_t, 0 }
    },
    [b_TermObj_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = false,
        .process_state = NULL,
        .what_to_do_after = ADD_MULTIPLE_STATES,
        .states = (uint32_t []){ 
            // Object
                // NameSpaceModifierObj
                    // b_DefAlias_t,
                    // b_DefName_t,
                    b_DefScope_t,
                // NamedObj
                    // b_DefBankField_t,
                    // b_DefCreateBitField_t,
                    // b_DefCreateByteField_t,
                    // b_DefCreateDWordField_t,
                    // b_DefCreateField_t,
                    // b_DefCreateQWordField_t,
                    // b_DefCreateWordField_t,
                    // b_DefDataRegion_t,
                    // b_DefExternal_t,
                    b_DefOpRegion_t,
                    // b_DefProcessorOp_t,
                    // b_DefPowerRes_t,
                    // b_DefThermalZone_t,

                    b_DefMethod_t,
                    b_DefField_t,
                    b_DefDevice_t,
            // StatementOpcode
                // b_DefBreak_t,
                // b_DefBreakPoint_t,
                // b_DefContinue_t,
                // b_DefFatal_t,
                // b_DefIfElse_t,
                // b_DefNoop_t,
                // b_DefNotify_t,
                // b_DefRelease_t,
                // b_DefReset_t,
                // b_DefReturn_t,
                // b_DefSignal_t,
                // b_DefSleep_t,
                // b_DefStall_t,
                // b_DefWhile_t,
            // ExpressionOpcode
                // b_DefAcquire_t,
                // b_DefAdd_t,
                // b_DefAnd_t,
                // b_DefBuffer_t,
                // b_DefConcat_t,
                // b_DefConcatRes_t,
                // b_DefCondRefOf_t,
                // b_DefCopyObject_t,
                // b_DefDecrement_t,
                // b_DefDerefOf_t,
                // b_DefDivide_t,
                // b_DefFindSetLeftBit_t,
                // b_DefFindSetRightBit_t,
                // b_DefFromBCD_t,
                // b_DefIncrement_t,
                // b_DefIndex_t,
                // b_DefLAnd_t,
                // b_DefLEqual_t,
                // b_DefLGreater_t,
                // b_DefLGreaterEqual_t,
                // b_DefLLess_t,
                // b_DefLLessEqual_t,
                // b_DefMid_t,
                // b_DefLNot_t,
                // b_DefLNotEqual_t,
                // b_DefLoadTable_t,
                // b_DefLOr_t,
                // b_DefMatch_t,
                // b_DefMod_t,
                // b_DefMultiply_t,
                // b_DefNAnd_t,
                // b_DefNOr_t,
                // b_DefNot_t,
                // b_DefObjectType_t,
                // b_DefOr_t,
                // b_DefPackage_t,
                // b_DefVarPackage_t,
                // b_DefRefOf_t,
                // b_DefShiftLeft_t,
                // b_DefShiftRight_t,
                // b_DefSizeOf_t,
                // b_DefStore_t,
                // b_DefSubtract_t,
                // b_DefTimer_t,
                // b_DefToBCD_t,
                // b_DefToBuffer_t,
                // b_DefToDecimalString_t,
                b_DefToHexString_t,
                // b_DefToInteger_t,
                // b_DefToString_t,
                // b_DefWait_t,
                // b_DefXOr_t,
                // b_MethodInvocation_t,
            0
        }
    },

    [b_DefScope_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = 0x10,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_t),
        .process_state = &aml_skip_opcode,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_PkgLength_t, b_NameString_t, b_TermList_t, b_MoveLeftInNameSpace_t, 0 }
    },

    [b_DefMethod_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = 0x14,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_t),
        .process_state = &aml_skip_opcode,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_PkgLength_t, b_NameString_t, b_MethodFlags_t, b_TermList_t, b_MoveLeftInNameSpace_t, 0 }
    },
    [b_MethodFlags_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_MethodFlags_t),
        .process_state = &aml_MethodFlags,
        .new_block_state_after = STAY,
        .what_to_do_after = NOTHING
    },

    [b_PkgLength_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_PkgLength_t),
        .process_state = &aml_PkgLength,
        .new_block_state_after = STAY,
        .what_to_do_after = NOTHING
    },

    // parsing of names
    [b_NameString_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_NameString_t),
        .process_state = NULL,
        .new_block_state_after = STAY,
        .what_to_do_after = ADD_MULTIPLE_STATES,
        .states = (uint32_t []){ b_NameString_RootPath_t, b_NameString_PrefixPath_t, b_NameSeg_t, b_DualNamePath_t, b_MultiNamePath_t, b_NullName_t, 0 }
    },
    [b_NameString_RootPath_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = '\\',
        .create_new_block = false,
        .process_state = &aml_RootChar,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_NamePath_t, 0 }
    },
    [b_NameString_PrefixPath_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = '^',
        .create_new_block = false,
        .process_state = NULL,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_PrefixPath_t, b_NamePath_t, 0 }
    },
    [b_PrefixPath_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = '^',
        .create_new_block = false,
        .process_state = &aml_PrefixPath,
        .what_to_do_after = ADD_MULTIPLE_STATES,
        .states = (uint32_t []){ b_PrefixPath_t, b_Nothing_t, 0 }
    },
    [b_NamePath_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = false,
        .process_state = NULL,
        .what_to_do_after = ADD_MULTIPLE_STATES,
        .states = (uint32_t []){ b_NameSeg_t, b_DualNamePath_t, b_MultiNamePath_t, b_NullName_t, 0 }
    },
    [b_NameSeg_t] = &(aml_rule_t) {
        .condition = CONDITION_LEADNAMECHAR,
        .create_new_block = false,
        .process_state = &aml_NameSeg,
        .what_to_do_after = NOTHING
    },
    [b_DualNamePath_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = 0x2E,
        .create_new_block = false,
        .process_state = &aml_skip_opcode,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_NameSeg_t, b_NameSeg_t, 0 }
    },
    [b_MultiNamePath_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = 0x2F,
        .create_new_block = false,
        .process_state = &aml_MultiNamePath,
        .what_to_do_after = NOTHING
    },
    [b_NullName_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = 0x00,
        .create_new_block = false,
        .process_state = &aml_skip_opcode,
        .what_to_do_after = NOTHING
    },

    [b_DefOpRegion_t] = &(aml_rule_t) {
        .condition = CONDITION_EXT_BYTE,
        .condition_value = 0x80,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_t),
        .process_state = &aml_skip_ext_opcode,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_NameString_t, b_RegionSpace_t, b_RegionOffset_t, b_RegionLen_t, b_MoveLeftInNameSpace_t, 0 }
    },
    [b_RegionSpace_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_RegionSpace_t),
        .process_state = &aml_RegionSpace,
        .new_block_state_after = STAY,
        .what_to_do_after = NOTHING
    },
    [b_RegionOffset_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_t),
        .process_state = NULL,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_TermArg_t, b_MoveLeftInNameSpace_t, 0 }
    },
    [b_RegionLen_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_t),
        .process_state = NULL,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_TermArg_t, b_MoveLeftInNameSpace_t, 0 }
    },

    [b_TermArg_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = false,
        .process_state = NULL,
        .what_to_do_after = ADD_MULTIPLE_STATES,
        .states = (uint32_t []){ 
            // ExpressionOpcode
                // b_DefAcquire_t,
                // b_DefAdd_t,
                // b_DefAnd_t,
                // b_DefBuffer_t,
                // b_DefConcat_t,
                // b_DefConcatRes_t,
                // b_DefCondRefOf_t,
                // b_DefCopyObject_t,
                // b_DefDecrement_t,
                // b_DefDerefOf_t,
                // b_DefDivide_t,
                // b_DefFindSetLeftBit_t,
                // b_DefFindSetRightBit_t,
                // b_DefFromBCD_t,
                // b_DefIncrement_t,
                // b_DefIndex_t,
                // b_DefLAnd_t,
                // b_DefLEqual_t,
                // b_DefLGreater_t,
                // b_DefLGreaterEqual_t,
                // b_DefLLess_t,
                // b_DefLLessEqual_t,
                // b_DefMid_t,
                // b_DefLNot_t,
                // b_DefLNotEqual_t,
                // b_DefLoadTable_t,
                // b_DefLOr_t,
                // b_DefMatch_t,
                // b_DefMod_t,
                // b_DefMultiply_t,
                // b_DefNAnd_t,
                // b_DefNOr_t,
                // b_DefNot_t,
                // b_DefObjectType_t,
                // b_DefOr_t,
                // b_DefPackage_t,
                // b_DefVarPackage_t,
                // b_DefRefOf_t,
                // b_DefShiftLeft_t,
                // b_DefShiftRight_t,
                // b_DefSizeOf_t,
                // b_DefStore_t,
                // b_DefSubtract_t,
                // b_DefTimer_t,
                // b_DefToBCD_t,
                // b_DefToBuffer_t,
                // b_DefToDecimalString_t,
                b_DefToHexString_t,
                // b_DefToInteger_t,
                // b_DefToString_t,
                // b_DefWait_t,
                // b_DefXOr_t,
                // b_MethodInvocation_t,
            // DataObject
                // ComputationalData
                    // b_ByteConst_t,
                    b_WordConst_t,
                    // b_DWordConst_t,
                    // b_QWordConst_t,
                    // b_String_t,
                    // ConstObj
                        // b_ZeroOp_t,
                        b_OneOp_t,
                        // b_OnesOp_t,
                    // b_RevisionOp_t,
                    // b_DefBuffer_t,
                // b_DefPackage_t,
                // b_DefVarPackage_t,
            0
        }
    },

    [b_WordConst_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = 0x0B,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_NumConst_t),
        .process_state = &aml_WordConst,
        .new_block_state_after = STAY,
        .what_to_do_after = NOTHING
    },

    [b_OneOp_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = 0x01,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_NumConst_t),
        .process_state = &aml_OneOp,
        .new_block_state_after = STAY,
        .what_to_do_after = NOTHING
    },

    [b_DefField_t] = &(aml_rule_t) {
        .condition = CONDITION_EXT_BYTE,
        .condition_value = 0x81,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_t),
        .process_state = &aml_skip_ext_opcode,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_PkgLength_t, b_NameString_t, b_FieldFlags_t, b_FieldList_t, b_MoveLeftInNameSpace_t, 0 }
    },
    [b_FieldFlags_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_FieldFlags_t),
        .process_state = &aml_FieldFlags,
        .new_block_state_after = STAY,
        .what_to_do_after = NOTHING
    },
    [b_FieldList_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = false,
        .process_state = NULL,
        .what_to_do_after = ADD_MULTIPLE_STATES,
        .states = (uint32_t []){ b_FieldListElement_t, b_Nothing_t, 0 }
    },
    [b_FieldListElement_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = false,
        .process_state = NULL,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_FieldElement_t, b_FieldList_t, 0 }
    },
    [b_FieldElement_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = false,
        .process_state = NULL,
        .what_to_do_after = ADD_MULTIPLE_STATES,
        .states = (uint32_t []){
            b_NamedField_t,
            // b_ReservedField_t,
            // b_AccessField_t,
            // b_ExtendedAccessField_t,
            // b_ConnectField_t,
            0
        }
    },
    [b_NamedField_t] = &(aml_rule_t) {
        .condition = CONDITION_LEADNAMECHAR,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_NamedField_t),
        .process_state = NULL,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_NameSeg_t, b_PkgLength_t, b_MoveLeftInNameSpace_t, 0 }
    },

    [b_DefDevice_t] = &(aml_rule_t) {
        .condition = CONDITION_EXT_BYTE,
        .condition_value = 0x82,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_t),
        .process_state = &aml_skip_ext_opcode,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_PkgLength_t, b_NameString_t, b_TermList_t, b_MoveLeftInNameSpace_t, 0 }
    },

    [b_DefToHexString_t] = &(aml_rule_t) {
        .condition = CONDITION_BYTE,
        .condition_value = 0x98,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_t),
        .process_state = &aml_skip_opcode,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_Operand_t, b_Target_t, b_MoveLeftInNameSpace_t, 0 }
    },
    [b_Operand_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_t),
        .process_state = NULL,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_TermArg_t, b_MoveLeftInNameSpace_t, 0 }
    },
    [b_Target_t] = &(aml_rule_t) {
        .condition = CONDITION_NOTHING,
        .create_new_block = true,
        .size_of_new_block = sizeof(aml_block_t),
        .process_state = NULL,
        .new_block_state_after = MOVE_RIGHT,
        .what_to_do_after = ADD_STATES,
        .states = (uint32_t []){ b_TermArg_t, b_MoveLeftInNameSpace_t, 0 }
    },
};
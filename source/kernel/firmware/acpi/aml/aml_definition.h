/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// AML interpreter states / block types
enum {
    b_table_t = 1,

    b_TermList_t,
    b_TermListEntry_t,
    b_TermObj_t,
    b_TermArg_t,

    b_Object_t,
    b_StatementOpcode_t,
    b_ExpressionOpcode_t,

    b_NameSpaceModifierObj_t,
    b_NamedObj_t,

    b_DefAlias_t,
    b_DefName_t,
    b_DefScope_t,

    b_DefBankField_t,
    b_DefCreateBitField_t,
    b_DefCreateByteField_t,
    b_DefCreateDWordField_t,
    b_DefCreateField_t,
    b_DefCreateQWordField_t,
    b_DefCreateWordField_t,
    b_DefDataRegion_t,
    b_DefExternal_t,
    b_DefOpRegion_t,
        b_RegionSpace_t,
        b_RegionOffset_t,
        b_RegionLen_t,
    b_DefProcessorOp_t,
    b_DefPowerRes_t,
    b_DefThermalZone_t,
    b_DefMethod_t,
        b_MethodFlags_t,

    b_NameString_t,
        b_NameString_RootPath_t,
        b_NameString_PrefixPath_t,
        b_NamePath_t,
        b_PrefixPath_t,
        b_NameSeg_t,
        b_DualNamePath_t,
        b_MultiNamePath_t,
        b_NullName_t,

    b_DataRefObject_t,
    b_PkgLength_t,

    b_DefBreak_t,
    b_DefBreakPoint_t,
    b_DefContinue_t,
    b_DefFatal_t,
    b_DefIfElse_t,
    b_DefNoop_t,
    b_DefNotify_t,
    b_DefRelease_t,
    b_DefReset_t,
    b_DefReturn_t,
    b_DefSignal_t,
    b_DefSleep_t,
    b_DefStall_t,
    b_DefWhile_t,

    b_DefAcquire_t,
    b_DefAdd_t,
    b_DefAnd_t,
    b_DefBuffer_t,
    b_DefConcat_t,
    b_DefConcatRes_t,
    b_DefCondRefOf_t,
    b_DefCopyObject_t,
    b_DefDecrement_t,
    b_DefDerefOf_t,
    b_DefDivide_t,
    b_DefFindSetLeftBit_t,
    b_DefFindSetRightBit_t,
    b_DefFromBCD_t,
    b_DefIncrement_t,
    b_DefIndex_t,
    b_DefLAnd_t,
    b_DefLEqual_t,
    b_DefLGreater_t,
    b_DefLGreaterEqual_t,
    b_DefLLess_t,
    b_DefLLessEqual_t,
    b_DefMid_t,
    b_DefLNot_t,
    b_DefLNotEqual_t,
    b_DefLoadTable_t,
    b_DefLOr_t,
    b_DefMatch_t,
    b_DefMod_t,
    b_DefMultiply_t,
    b_DefNAnd_t,
    b_DefNOr_t,
    b_DefNot_t,
    b_DefObjectType_t,
    b_DefOr_t,
    b_DefPackage_t,
    b_DefVarPackage_t,
    b_DefRefOf_t,
    b_DefShiftLeft_t,
    b_DefShiftRight_t,
    b_DefSizeOf_t,
    b_DefStore_t,
    b_DefSubtract_t,
    b_DefTimer_t,
    b_DefToBCD_t,
    b_DefToBuffer_t,
    b_DefToDecimalString_t,
    b_DefToHexString_t,
    b_DefToInteger_t,
    b_DefToString_t,
    b_DefWait_t,
    b_DefXOr_t,
    b_MethodInvocation_t,

    b_ByteConst_t,
    b_WordConst_t,
    b_DWordConst_t,
    b_QWordConst_t,
    b_String_t,
    b_ZeroOp_t,
    b_OneOp_t,
    b_OnesOp_t,
    b_RevisionOp_t,

    b_Nothing_t,
    b_MoveLeftInNameSpace_t,

    b_DefField_t,
        b_FieldFlags_t,
        b_FieldList_t,
        b_FieldElement_t,
        b_FieldListElement_t,

    b_NamedField_t,
    b_ReservedField_t,
    b_AccessField_t,
    b_ExtendedAccessField_t,
    b_ConnectField_t,
    
    b_DefDevice_t,

    b_Operand_t,
    b_Target_t,

    b_SkipZone_t,
    b_Error_t,

    b_number_of_aml_states
};
extern const char* aml_state_names[];

// AML new block state
enum {
    MOVE_RIGHT = 1,
    STAY
};

typedef struct aml_block_t {
    struct aml_block_t *left;
    struct aml_block_t *down;
    struct aml_block_t *right;
    void *start; // first accessible byte of this block
    void *end; // last accessible byte of this block
    uint32_t type;
} aml_block_t;

typedef struct {
    aml_block_t header;
    uint32_t length;
} aml_block_PkgLength_t;

typedef struct {
    aml_block_t header;
    uint8_t *string;
    uint32_t length_of_string;
} aml_block_NameString_t;

typedef struct {
    aml_block_t header;
    uint8_t region_space;
} aml_block_RegionSpace_t;

typedef struct {
    aml_block_t header;
    uint64_t value;
} aml_block_NumConst_t;

typedef struct {
    aml_block_t header;
    uint8_t AccessType : 4;
    uint8_t LockRule : 1;
    uint8_t UpdateRule : 2;
    uint8_t : 1;
} aml_block_FieldFlags_t;

typedef struct {
    aml_block_t header;
    uint8_t *string;
    uint32_t length_of_string;
} aml_block_NamedField_t;

typedef struct {
    aml_block_t header;
    uint8_t ArgCount : 2;
    uint8_t SerializeFlag : 1;
    uint8_t SyncLevel : 5;
} aml_block_MethodFlags_t;

enum {
    CONDITION_NOTHING = 1,
    CONDITION_BYTE,
    CONDITION_EXT_BYTE,
    CONDITION_LEADNAMECHAR
};
enum {
    NOTHING = 1,
    ADD_STATES,
    ADD_MULTIPLE_STATES,
    MOVE_LEFT
};
typedef struct {
   uint32_t condition;
   uint32_t condition_value;
   uint32_t create_new_block;
   uint32_t size_of_new_block;
   uint32_t (*process_state)(void *aml_block);
   uint32_t new_block_state_after;
   uint32_t what_to_do_after;
   uint32_t *states;
} aml_rule_t;

extern aml_rule_t *convert_state_to_rule[b_number_of_aml_states];
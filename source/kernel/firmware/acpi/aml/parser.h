/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define AML_STACK_SIZE 4096
#define AML_INTERPRETER_STACK_SIZE 1024
typedef enum {
    AML_DefScope,
        AML_ScopeOp,
        AML_Scope_End,
    AML_DefOpRegion,
        AML_OpRegionOp,
        AML_RegionSpace,
        AML_RegionOffset,
            AML_RegionOffset_ProcessReturnedValue,
        AML_RegionLen,
            AML_RegionLen_ProcessReturnedValue,
        AML_OpRegion_End,

    AML_PkgLength,
    AML_NameString,
        AML_PrefixPath,
        AML_NamePath,
        AML_NameString_End,
        AML_DualNamePath,
        AML_MultiNamePath,
        AML_NameSeg,
    
    AML_TermList_Start,
    AML_TermList,
    AML_TermObj,

    AML_TermArgList_Start,
    AML_TermArgList,
    AML_TermArg,

    AML_MethodInvocation,

    AML_ByteConst,
    AML_WordConst,
    AML_ConstObj,

    AML_DefField,
        AML_FieldOp,
        AML_FieldFlags,
        AML_FieldList_Start,
        AML_Field_End,
} aml_interpreter_operation_t;
typedef struct aml_interpreter_operation_stack_entry_t{
    struct aml_interpreter_operation_stack_entry_t *next;
    struct aml_interpreter_operation_stack_entry_t *prev;
    aml_interpreter_operation_t operation;
    void *stack;
    uint8_t *end_of_section;
} aml_interpreter_operation_stack_entry_t;

typedef struct {
    uint32_t number_of_characters;
    uint8_t *name;
} aml_namestring_t;

typedef struct {
    uint8_t *end_of_section;
    aml_namestring_t namestring;
} aml_scope_stack_t;

typedef struct {
    uint8_t *end_of_section;
    aml_namestring_t namestring;
    uint8_t region_space;
    uint64_t region_offset;
    uint64_t region_len;
} aml_opregion_stack_t;
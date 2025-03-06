//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void parse_s5_shutdown_values(void) {
    // THIS IS OLD METHOD AND IS USED ONLY BECAUSE FULL AML PARSER IS NOT WRITTEN YET
    // THIS IS NOT PROPER WAY TO PARSE AML
    components->acpi.pm1a_control_reg_shutdown_s5_value = 0x2000;
    components->acpi.pm1b_control_reg_shutdown_s5_value = 0x2000;
    for(dword_t i = 0; i < (components->acpi.dsdt->length-4); i++) {
        if( components->acpi.dsdt_aml_data[i+0] == '_'
            && components->acpi.dsdt_aml_data[i+1] == 'S'
            && components->acpi.dsdt_aml_data[i+2] == '5'
            && components->acpi.dsdt_aml_data[i+3] == '_') {
            components->acpi.pm1a_control_reg_shutdown_s5_value |= (components->acpi.dsdt_aml_data[i+8] << 10);
            components->acpi.pm1b_control_reg_shutdown_s5_value |= (components->acpi.dsdt_aml_data[i+10] << 10);
            logf("\n  Shutdown value for pm1a: 0x%04x", components->acpi.pm1a_control_reg, components->acpi.pm1a_control_reg_shutdown_s5_value);
            logf("\n  Shutdown value for pm1b: 0x%04x", components->acpi.pm1b_control_reg, components->acpi.pm1b_control_reg_shutdown_s5_value);
            return;
        }
    }
}

void dump_aml_objects(byte_t *aml_code, dword_t length) {
    byte_t *end_of_code = (byte_t *) ((dword_t)aml_code + length);
    while(aml_code < end_of_code) {
        switch(*aml_code) {
            // ScopeOp
            case(0x10):
                byte_t *local_aml_code = aml_code;
                local_aml_code++;

                // PkgLength
                dword_t length = aml_parse_length_of_pkglength(local_aml_code);
                logf("\n  ScopeOp bytes %d length %d ", local_aml_code[0] >> 6, length);
                local_aml_code += ((local_aml_code[0] >> 6)+1);
                logf("Name: %s", &local_aml_code[0]);

                aml_code += length+1;
                break;
            default:
                logf("\n  Uknown AML opcode %02x", *aml_code);
                return;            
        }
    }
}

dword_t aml_parse_length_of_pkglength(byte_t *aml_code) {
    dword_t number_of_bytes = (*aml_code >> 6);
    dword_t length = *aml_code;

    logf("\n  PkgLength bytes %d length %d", number_of_bytes, length);

    if(length <= 63 || number_of_bytes == 0) {
        return length;
    }

    length = (*aml_code & 0xF);
    aml_code++;
    for(dword_t i = 0; i < number_of_bytes; i++) {
        length |= ((aml_code[i] << 4) << i*8);
    }

    return length;
}
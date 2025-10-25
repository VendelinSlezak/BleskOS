# Global rules

Intendation of one layer is 4 spaces.

Following types are used:

```
unsigned char = byte_t
unsigned short int = word_t
unsigned int = dword_t
unsigned long long = qword_t
```

Names of variables and functions are written in snake_case

# C files

Layout of every .c file is:

1. Start section
2. Includes section
3. Global variables
4. Local variables
5. Functions

## Start section

This section contains following comment:

```
/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
```

## Includes section

This section includes all .h files that are needed to compile this .c file.

Compiler automatically includes for you:

* `build/global_declarations.h`
* `.h file` of your .c file

Section starts with following comment:

```
/* includes */
```

Then follows including of all files. Default directory for <> includes is build folder. All .h files from folder sources are moved here during compilation and are extended for programming comfort.

### Example

```
/* includes */
#include <kernel/x86/kernel.h>
#include <kernel/x86/memory/vm_allocator.h>
```

## Global variables section

This section includes declarations / definitions of variables that are visible for files that includes .h file of this .c file. It means that every variable here has to have extern declaration in .h file.

Section starts with following comment:

```
/* global variables */
```

Then follows declarations / definitions of global variables.

### Example

```
/* global variables */
dword_t core_entity_number = 0;
```

## Local variables section

This section includes declarations / definitions of variables that are visible only for this .c file. It means that none of those variables are in .h file.

Section starts with following comment:

```
/* local variables */
```

Then follows declarations / definitions of local variables.

### Example

```
/* local variables */
byte_t *local_string = "abcd";
```

## Functions section

This section contains actual code of functions. It has at least one `/* functions */` comment, but for greater code visibility it is possible to include more detailed description about functions it is describing e.g. `/* functions for initializing controller */`

Opening curled bracket is always on same line as command it belongs to. Closing curled bracked is always on separate line on layer of opening command.

Example:

```
    if() {

    }
```

Every function *MUST* be defined in following style:

```
[return type] [* of return type][snake_case_name_of_function]([arguments]) {[new line]
Example: void *malloc(size_t size) {
Example: void initialize_device(void) {
```

It is because definitions or functions are generated automatically for programming comfort, and they are founded by this template.

Function can return globally defined constants TRUE / FALSE / SUCCESS / ERROR / INVALID. If it does, return type should be defined as `dword_t`.

Control commands are written by following template:

```
    if([arguments]) {

    }

    for([arguments]) {

    }

    while([arguments]) {

    }

    do {

    } while([arguments]);
```

In arguments of control commands, use spaces when possible.

Example:

```
    if(((i << 5) & 0xC0) == 0xC0)
```

If your argument has multiple statements and it is too long, move every statement to individual line and align them to intendation after ( bracket of control word.

Example:

```
    if(    buffer[0] == 'A'
        && buffer[1] == 'B'
        && buffer[2] == 'C'
        && buffer[3] == 'D') {
        // code
    }
```

# H files

Layout of H file is less strict than layout of C file. You have to start with license comment, and then include all files that are needed for compilation, exactly as with C file, but rest is freely ordered.

For programming comfort, compilation doew few things for you, which you have to take into consideration:

* Every H file has unique guard definition
* At the end of H file will be added all definitions of functions from C file to which H file belongs. If some of your function uses special type in declaration, you need to include file with its definition at start of H file, even through you do not use it in code you wrote.

This new expanded H file will be copied into `build` folder from which it will be included by C files.

Example of H file code:

```
/* license */

#include <kernel/x86/kernel.h>

extern dword_t var;
```

Example of C file code:

```
...

/* functions */
int initialize_func(dword_t a, dword_t *b) {
    ...
}
```

Compilation will transform this H file into:

```
#ifndef SOURCE_EXAMPLE_H
#define SOURCE_EXAMPLE_H
/* license */

#include <kernel/x86/kernel.h>

extern dword_t var;
int initialize_func(dword_t a, dword_t *b);
#endif
```

## Global variables

Every global variable that is defined in C file needs to be defined in H file with extern attribute:

Example:

```
extern dword_t global_var;
```

## Structures

Name of every structure must end with _t. It needs to be defined by following template:

```
typedef struct {
    ...
} my_structure_t;
```
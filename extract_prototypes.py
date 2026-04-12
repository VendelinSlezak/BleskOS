#!/usr/bin/env python3
import re
import sys
from typing import List

def remove_comments(code: str) -> str:
    code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
    code = re.sub(r'//.*', '', code)
    return code

def extract_function_prototypes_from_code(code: str) -> List[str]:
    pattern = re.compile(
        r'^'
        r'([A-Za-z_][A-Za-z0-9_]*(?:\s+[A-Za-z_][A-Za-z0-9_]*)*)'
        r'\s+'
        r'(\*+)?'
        r'([a-z_][a-z0-9_]*)'
        r'\('
        r'([^\n]*)'
        r'\)\s*\{\s*$'
        , flags=re.MULTILINE
    )

    prototypes: List[str] = []
    for m in pattern.finditer(code):
        full_line = m.group(0)
        cleaned_line = re.sub(r'\binline\b\s*', '', full_line)
        idx = cleaned_line.rfind(')')
        if idx != -1:
            proto = cleaned_line[:idx+1] + ';'
            prototypes.append(proto.rstrip())
    return prototypes

def read_input_from_filename_or_stdin() -> str:
    if len(sys.argv) >= 2 and sys.argv[1] not in ('', '-'):
        fname = sys.argv[1]
        with open(fname, 'r', encoding='utf-8') as f:
            return f.read()
    else:
        return sys.stdin.read()

def main():
    code = read_input_from_filename_or_stdin()
    code_no_comments = remove_comments(code)
    protos = extract_function_prototypes_from_code(code_no_comments)
    print("")
    for p in protos:
        print(p)

if __name__ == "__main__":
    main()

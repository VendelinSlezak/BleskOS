#ifndef BUILD_KERNEL_X86_LIBC_STDLIB_H
#define BUILD_KERNEL_X86_LIBC_STDLIB_H


void itoa(int value, char *str, size_t size);
int atoi(const char *str);
char *htoa(unsigned int value, char *str);
char *htoan(unsigned int value, char *str, size_t n);

#endif /* BUILD_KERNEL_X86_LIBC_STDLIB_H */

#ifndef BUILD_KERNEL_X86_LIBC_STRING_H
#define BUILD_KERNEL_X86_LIBC_STRING_H


size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
void *memset(void *dest, int value, size_t len);
void *memcpy(void *dest, const void *src, size_t len);
void *memmove(void *dest, const void *src, size_t len);
char *strcpy(char *dest, const char *src);

#endif /* BUILD_KERNEL_X86_LIBC_STRING_H */

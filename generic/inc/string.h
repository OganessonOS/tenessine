#pragma once

size_t strlen(const char* str);
int strncmp(const char* s1, const char* s2, size_t n);

void memcpy(void* dest, const void* src, size_t n);

// For C/C++ spec reasons, these are defined as extern "C"
#ifdef __cplusplus
extern "C" {
#endif

void* memset(void* s, int c, size_t n);

#ifdef __cplusplus
}
#endif

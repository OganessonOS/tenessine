#include <stddef.h>
#include <string.h>

size_t strlen(const char* str) {
	size_t ret = 0;
	for(; *str; ret++, str++);
	return ret;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        ++s1;
        ++s2;
        --n;
    }
    if (n == 0) {
        return 0;
    } else {
        return (*(unsigned char *)s1 - *(unsigned char *)s2);
    }
}


void memcpy(void* dest, const void* src, size_t n) {
	char* b = (char*)dest;
	char* a = (char*)src;
	for(; n; n--, a++, b++) {
		*b = *a;
	} 
}

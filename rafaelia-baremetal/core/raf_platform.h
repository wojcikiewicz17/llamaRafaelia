/**
 * Rafaelia Baremetal - Platform Utilities
 *
 * Minimal, dependency-free replacements for common libc routines.
 * Intended for low-level/baremetal usage where external dependencies
 * must be avoided.
 *
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_PLATFORM_H
#define RAFAELIA_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

void *raf_platform_memcpy(void *dst, const void *src, unsigned int size);
void *raf_platform_memset(void *dst, int value, unsigned int size);
int raf_platform_memcmp(const void *a, const void *b, unsigned int size);

unsigned int raf_platform_strlen(const char *str);
unsigned int raf_platform_strnlen(const char *str, unsigned int max_len);
char *raf_platform_strcpy(char *dst, const char *src);
char *raf_platform_strncpy(char *dst, const char *src, unsigned int max_len);
int raf_platform_strcmp(const char *a, const char *b);
int raf_platform_strncmp(const char *a, const char *b, unsigned int max_len);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_PLATFORM_H */

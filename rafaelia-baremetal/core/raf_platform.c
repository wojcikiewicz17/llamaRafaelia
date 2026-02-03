/**
 * Rafaelia Baremetal - Platform Utilities
 *
 * Dependency-free replacements for common libc routines.
 *
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_platform.h"

void *raf_platform_memcpy(void *dst, const void *src, unsigned int size) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    if (d == s || size == 0) {
        return dst;
    }

    for (unsigned int i = 0; i < size; i++) {
        d[i] = s[i];
    }

    return dst;
}

void *raf_platform_memset(void *dst, int value, unsigned int size) {
    unsigned char *d = (unsigned char *)dst;
    unsigned char v = (unsigned char)value;

    for (unsigned int i = 0; i < size; i++) {
        d[i] = v;
    }

    return dst;
}

int raf_platform_memcmp(const void *a, const void *b, unsigned int size) {
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;

    for (unsigned int i = 0; i < size; i++) {
        if (pa[i] != pb[i]) {
            return (int)pa[i] - (int)pb[i];
        }
    }

    return 0;
}

unsigned int raf_platform_strlen(const char *str) {
    unsigned int len = 0;

    if (str == NULL) {
        return 0;
    }

    while (str[len] != '\0') {
        len++;
    }

    return len;
}

unsigned int raf_platform_strnlen(const char *str, unsigned int max_len) {
    unsigned int len = 0;

    if (str == NULL) {
        return 0;
    }

    while (len < max_len && str[len] != '\0') {
        len++;
    }

    return len;
}

char *raf_platform_strcpy(char *dst, const char *src) {
    unsigned int i = 0;

    if (dst == NULL || src == NULL) {
        return dst;
    }

    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';

    return dst;
}

char *raf_platform_strncpy(char *dst, const char *src, unsigned int max_len) {
    unsigned int i = 0;

    if (dst == NULL || src == NULL) {
        return dst;
    }

    while (i < max_len && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }

    while (i < max_len) {
        dst[i] = '\0';
        i++;
    }

    return dst;
}

int raf_platform_strcmp(const char *a, const char *b) {
    unsigned int i = 0;

    if (a == NULL && b == NULL) {
        return 0;
    }
    if (a == NULL) {
        return -1;
    }
    if (b == NULL) {
        return 1;
    }

    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return (int)((unsigned char)a[i]) - (int)((unsigned char)b[i]);
        }
        i++;
    }

    return (int)((unsigned char)a[i]) - (int)((unsigned char)b[i]);
}

int raf_platform_strncmp(const char *a, const char *b, unsigned int max_len) {
    unsigned int i = 0;

    if (max_len == 0) {
        return 0;
    }
    if (a == NULL && b == NULL) {
        return 0;
    }
    if (a == NULL) {
        return -1;
    }
    if (b == NULL) {
        return 1;
    }

    while (i < max_len && a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return (int)((unsigned char)a[i]) - (int)((unsigned char)b[i]);
        }
        i++;
    }

    if (i == max_len) {
        return 0;
    }

    return (int)((unsigned char)a[i]) - (int)((unsigned char)b[i]);
}

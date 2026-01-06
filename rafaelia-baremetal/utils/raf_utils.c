/**
 * Rafaelia Baremetal - Utility Tools Implementation
 * 
 * Implementation of 42 specialized utility functions.
 * Pure C, no external dependencies.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_utils.h"
#include <string.h>

/* ===== Memory Operations (Tools 1-5) ===== */

int raf_memcmp_secure(const void *a, const void *b, size_t n) {
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    uint8_t diff = 0;
    
    for (size_t i = 0; i < n; i++) {
        diff |= pa[i] ^ pb[i];
    }
    
    return diff;
}

void raf_memzero_secure(void *ptr, size_t n) {
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    for (size_t i = 0; i < n; i++) {
        p[i] = 0;
    }
}

int raf_memcpy_safe(void *dst, const void *src, size_t n, size_t dst_size) {
    if (!dst || !src || n > dst_size) return -1;
    
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    
    /* Check for overlap */
    if ((d < s && d + n > s) || (s < d && s + n > d)) {
        return -2; /* Overlap detected */
    }
    
    memcpy(dst, src, n);
    return 0;
}

void raf_memswap(void *a, void *b, size_t n) {
    uint8_t *pa = (uint8_t *)a;
    uint8_t *pb = (uint8_t *)b;
    
    for (size_t i = 0; i < n; i++) {
        uint8_t temp = pa[i];
        pa[i] = pb[i];
        pb[i] = temp;
    }
}

void raf_memfill_pattern(void *ptr, size_t n, const uint8_t *pattern, size_t pattern_size) {
    if (!ptr || !pattern || pattern_size == 0) return;
    
    uint8_t *p = (uint8_t *)ptr;
    for (size_t i = 0; i < n; i++) {
        p[i] = pattern[i % pattern_size];
    }
}

/* ===== String Operations (Tools 6-10) ===== */

size_t raf_strnlen_safe(const char *str, size_t maxlen) {
    if (!str) return 0;
    
    size_t len = 0;
    while (len < maxlen && str[len] != '\0') {
        len++;
    }
    return len;
}

size_t raf_strcpy_safe(char *dst, const char *src, size_t dst_size) {
    if (!dst || !src || dst_size == 0) return 0;
    
    size_t i;
    for (i = 0; i < dst_size - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    
    return i;
}

size_t raf_strcat_safe(char *dst, const char *src, size_t dst_size) {
    if (!dst || !src || dst_size == 0) return 0;
    
    size_t dst_len = raf_strnlen_safe(dst, dst_size);
    if (dst_len >= dst_size - 1) return dst_len;
    
    return dst_len + raf_strcpy_safe(dst + dst_len, src, dst_size - dst_len);
}

int raf_strcasecmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return 0;
    
    while (*s1 && *s2) {
        char c1 = *s1;
        char c2 = *s2;
        
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        
        if (c1 != c2) return c1 - c2;
        
        s1++;
        s2++;
    }
    
    return *s1 - *s2;
}

char* raf_strtok_safe(char *str, const char *delim, char **saveptr) {
    if (!delim || !saveptr) return NULL;
    
    char *start = str ? str : *saveptr;
    if (!start) return NULL;
    
    /* Skip leading delimiters */
    while (*start && strchr(delim, *start)) start++;
    if (*start == '\0') return NULL;
    
    /* Find end of token */
    char *end = start;
    while (*end && !strchr(delim, *end)) end++;
    
    if (*end) {
        *end = '\0';
        *saveptr = end + 1;
    } else {
        *saveptr = NULL;
    }
    
    return start;
}

/* ===== Math Operations (Tools 11-15) ===== */

uint32_t raf_isqrt(uint32_t n) {
    if (n == 0) return 0;
    
    uint32_t x = n;
    uint32_t y = (x + 1) / 2;
    
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    
    return x;
}

uint64_t raf_ipow(uint64_t base, uint32_t exp) {
    uint64_t result = 1;
    
    while (exp) {
        if (exp & 1) result *= base;
        base *= base;
        exp >>= 1;
    }
    
    return result;
}

uint64_t raf_gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

uint64_t raf_lcm(uint64_t a, uint64_t b) {
    if (a == 0 || b == 0) return 0;
    return (a / raf_gcd(a, b)) * b;
}

uint64_t raf_modpow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base %= mod;
    
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    
    return result;
}

/* ===== Bit Operations (Tools 16-20) ===== */

uint32_t raf_popcount(uint64_t x) {
    uint32_t count = 0;
    while (x) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

uint32_t raf_ffs(uint64_t x) {
    if (x == 0) return 0;
    
    uint32_t pos = 1;
    while ((x & 1) == 0) {
        x >>= 1;
        pos++;
    }
    return pos;
}

uint32_t raf_fls(uint64_t x) {
    if (x == 0) return 0;
    
    uint32_t pos = 0;
    while (x) {
        x >>= 1;
        pos++;
    }
    return pos;
}

uint64_t raf_rotl(uint64_t x, uint32_t n) {
    n %= 64;
    return (x << n) | (x >> (64 - n));
}

uint64_t raf_rotr(uint64_t x, uint32_t n) {
    n %= 64;
    return (x >> n) | (x << (64 - n));
}

/* ===== Checksum and Hashing (Tools 21-25) ===== */

uint16_t raf_fletcher16(const uint8_t *data, size_t len) {
    uint16_t sum1 = 0, sum2 = 0;
    
    for (size_t i = 0; i < len; i++) {
        sum1 = (sum1 + data[i]) % 255;
        sum2 = (sum2 + sum1) % 255;
    }
    
    return (sum2 << 8) | sum1;
}

uint32_t raf_fletcher32(const uint16_t *data, size_t len) {
    uint32_t sum1 = 0, sum2 = 0;
    
    for (size_t i = 0; i < len; i++) {
        sum1 = (sum1 + data[i]) % 65535;
        sum2 = (sum2 + sum1) % 65535;
    }
    
    return (sum2 << 16) | sum1;
}

uint32_t raf_adler32(const uint8_t *data, size_t len) {
    uint32_t a = 1, b = 0;
    const uint32_t MOD = 65521;
    
    for (size_t i = 0; i < len; i++) {
        a = (a + data[i]) % MOD;
        b = (b + a) % MOD;
    }
    
    return (b << 16) | a;
}

uint32_t raf_murmur3_32(const void *key, size_t len, uint32_t seed) {
    const uint8_t *data = (const uint8_t *)key;
    uint32_t h = seed;
    
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    
    /* Process 4-byte chunks */
    size_t nblocks = len / 4;
    for (size_t i = 0; i < nblocks; i++) {
        uint32_t k = ((uint32_t)data[i*4]) | ((uint32_t)data[i*4+1] << 8) |
                     ((uint32_t)data[i*4+2] << 16) | ((uint32_t)data[i*4+3] << 24);
        
        k *= c1;
        k = (k << 15) | (k >> 17);
        k *= c2;
        
        h ^= k;
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    
    /* Process remaining bytes */
    uint32_t k = 0;
    switch (len & 3) {
        case 3: k ^= data[nblocks*4+2] << 16;
        case 2: k ^= data[nblocks*4+1] << 8;
        case 1: k ^= data[nblocks*4];
                k *= c1;
                k = (k << 15) | (k >> 17);
                k *= c2;
                h ^= k;
    }
    
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    
    return h;
}

uint64_t raf_hash64(const uint8_t *data, size_t len) {
    uint64_t hash = 14695981039346656037ULL;
    
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 1099511628211ULL;
    }
    
    return hash;
}

/* ===== Encoding/Decoding (Tools 26-30) ===== */

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t raf_base64_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_len) {
    if (!src || !dst) return 0;
    
    size_t needed = ((src_len + 2) / 3) * 4 + 1;
    if (dst_len < needed) return 0;
    
    size_t i = 0, j = 0;
    while (i < src_len) {
        uint32_t octet_a = i < src_len ? src[i++] : 0;
        uint32_t octet_b = i < src_len ? src[i++] : 0;
        uint32_t octet_c = i < src_len ? src[i++] : 0;
        
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        dst[j++] = base64_chars[(triple >> 18) & 0x3F];
        dst[j++] = base64_chars[(triple >> 12) & 0x3F];
        dst[j++] = base64_chars[(triple >> 6) & 0x3F];
        dst[j++] = base64_chars[triple & 0x3F];
    }
    
    /* Add padding */
    size_t pad = (3 - (src_len % 3)) % 3;
    for (size_t k = 0; k < pad; k++) {
        dst[j - 1 - k] = '=';
    }
    
    dst[j] = '\0';
    return j;
}

size_t raf_hex_encode(const uint8_t *src, size_t src_len, char *dst, size_t dst_len) {
    if (!src || !dst || dst_len < src_len * 2 + 1) return 0;
    
    const char hex_chars[] = "0123456789abcdef";
    
    for (size_t i = 0; i < src_len; i++) {
        dst[i * 2] = hex_chars[src[i] >> 4];
        dst[i * 2 + 1] = hex_chars[src[i] & 0x0F];
    }
    
    dst[src_len * 2] = '\0';
    return src_len * 2;
}

size_t raf_hex_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_len) {
    if (!src || !dst || src_len % 2 != 0 || dst_len < src_len / 2) return 0;
    
    for (size_t i = 0; i < src_len / 2; i++) {
        uint8_t high = src[i * 2];
        uint8_t low = src[i * 2 + 1];
        
        high = (high >= '0' && high <= '9') ? high - '0' :
               (high >= 'a' && high <= 'f') ? high - 'a' + 10 :
               (high >= 'A' && high <= 'F') ? high - 'A' + 10 : 0;
        
        low = (low >= '0' && low <= '9') ? low - '0' :
              (low >= 'a' && low <= 'f') ? low - 'a' + 10 :
              (low >= 'A' && low <= 'F') ? low - 'A' + 10 : 0;
        
        dst[i] = (high << 4) | low;
    }
    
    return src_len / 2;
}

size_t raf_url_encode(const char *src, char *dst, size_t dst_len) {
    if (!src || !dst) return 0;
    
    const char hex_chars[] = "0123456789ABCDEF";
    size_t j = 0;
    
    for (size_t i = 0; src[i] && j < dst_len - 1; i++) {
        char c = src[i];
        
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            dst[j++] = c;
        } else if (j < dst_len - 3) {
            dst[j++] = '%';
            dst[j++] = hex_chars[(c >> 4) & 0x0F];
            dst[j++] = hex_chars[c & 0x0F];
        }
    }
    
    dst[j] = '\0';
    return j;
}

/* ===== Random Number Generation (Tools 31-35) ===== */

void raf_prng_init(raf_prng *prng, uint64_t seed) {
    prng->state[0] = seed;
    prng->state[1] = seed ^ 0x123456789ABCDEF0ULL;
    prng->state[2] = seed + 0xFEDCBA9876543210ULL;
    prng->state[3] = ~seed;
}

uint64_t raf_prng_next(raf_prng *prng) {
    /* xoshiro256** algorithm */
    uint64_t result = raf_rotl(prng->state[1] * 5, 7) * 9;
    uint64_t t = prng->state[1] << 17;
    
    prng->state[2] ^= prng->state[0];
    prng->state[3] ^= prng->state[1];
    prng->state[1] ^= prng->state[2];
    prng->state[0] ^= prng->state[3];
    prng->state[2] ^= t;
    prng->state[3] = raf_rotl(prng->state[3], 45);
    
    return result;
}

uint64_t raf_prng_range(raf_prng *prng, uint64_t max) {
    if (max == 0) return 0;
    return raf_prng_next(prng) % max;
}

float raf_prng_float(raf_prng *prng) {
    return (float)(raf_prng_next(prng) >> 11) * (1.0f / 9007199254740992.0f);
}

void raf_prng_bytes(raf_prng *prng, uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i += 8) {
        uint64_t val = raf_prng_next(prng);
        for (size_t j = 0; j < 8 && i + j < len; j++) {
            buf[i + j] = (val >> (j * 8)) & 0xFF;
        }
    }
}

/* ===== Sorting and Searching (Tools 36-40) ===== */

static void qsort_partition(int32_t *arr, int low, int high) {
    if (low < high) {
        int32_t pivot = arr[high];
        int i = low - 1;
        
        for (int j = low; j < high; j++) {
            if (arr[j] < pivot) {
                i++;
                int32_t temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
        
        int32_t temp = arr[i + 1];
        arr[i + 1] = arr[high];
        arr[high] = temp;
        
        int pi = i + 1;
        qsort_partition(arr, low, pi - 1);
        qsort_partition(arr, pi + 1, high);
    }
}

void raf_qsort_int(int32_t *array, size_t len) {
    if (array && len > 1) {
        qsort_partition(array, 0, len - 1);
    }
}

int raf_bsearch_int(const int32_t *array, size_t len, int32_t key) {
    if (!array) return -1;
    
    int low = 0;
    int high = len - 1;
    
    while (low <= high) {
        int mid = low + (high - low) / 2;
        
        if (array[mid] == key) return mid;
        if (array[mid] < key) low = mid + 1;
        else high = mid - 1;
    }
    
    return -1;
}

void raf_isort_int(int32_t *array, size_t len) {
    if (!array || len <= 1) return;
    
    for (size_t i = 1; i < len; i++) {
        int32_t key = array[i];
        int j = i - 1;
        
        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];
            j--;
        }
        
        array[j + 1] = key;
    }
}

int32_t raf_min_int(const int32_t *array, size_t len) {
    if (!array || len == 0) return 0;
    
    int32_t min = array[0];
    for (size_t i = 1; i < len; i++) {
        if (array[i] < min) min = array[i];
    }
    
    return min;
}

int32_t raf_max_int(const int32_t *array, size_t len) {
    if (!array || len == 0) return 0;
    
    int32_t max = array[0];
    for (size_t i = 1; i < len; i++) {
        if (array[i] > max) max = array[i];
    }
    
    return max;
}

/* ===== Time and Profiling (Tools 41-42) ===== */

uint64_t raf_timestamp(void) {
    #if defined(__x86_64__) || defined(__i386__)
    /* Use RDTSC on x86 */
    uint32_t low, high;
    __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
    #elif defined(__aarch64__)
    /* Use virtual counter on ARM */
    uint64_t val;
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(val));
    return val;
    #else
    /* Fallback: use a simple counter */
    static uint64_t counter = 0;
    return counter++;
    #endif
}

void raf_delay_us(uint32_t microseconds) {
    /* Simple busy-wait delay - not accurate but baremetal */
    uint64_t start = raf_timestamp();
    uint64_t cycles = microseconds * 1000; /* Approximate */
    
    while (raf_timestamp() - start < cycles) {
        /* Busy wait */
    }
}

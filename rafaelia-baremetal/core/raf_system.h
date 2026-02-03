/**
 * Rafaelia Baremetal - System Architecture Registry
 *
 * Central registry that describes the module layout shown in the
 * Rafaelia Baremetal architecture diagram.
 *
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_SYSTEM_H
#define RAFAELIA_SYSTEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RAF_LAYER_HIGH = 0,
    RAF_LAYER_MID = 1,
    RAF_LAYER_CORE = 2,
    RAF_LAYER_HAL = 3
} raf_layer;

typedef struct {
    const char *name;
    const char *description;
    raf_layer layer;
} raf_module_info;

typedef struct {
    const char *rf_id;
    const char *kernel;
    const char *mode;
    const char *ethic;
    const char *hash_core;
    const char *vector_core;
    const char *cognition;
    const char *universe;
} raf_system_identity;

const raf_system_identity *raf_system_identity_get(void);
size_t raf_system_list_modules(const raf_module_info **modules);
const char *raf_layer_name(raf_layer layer);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_SYSTEM_H */

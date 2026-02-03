/**
 * Rafaelia Baremetal - System Architecture Registry
 *
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_system.h"

static const raf_system_identity k_identity = {
    .rf_id = "RF_ID",
    .kernel = "ΣΔΩ",
    .mode = "RAFAELIA",
    .ethic = "Amor",
    .hash_core = "AETHER",
    .vector_core = "RAF_VECTOR",
    .cognition = "TRINITY",
    .universe = "RAFAELIA_CORE"
};

static const raf_module_info k_modules[] = {
    {
        .name = "RAFAELIA",
        .description = "Cognitive module (vectors, cycles, ethics)",
        .layer = RAF_LAYER_HIGH
    },
    {
        .name = "BITSTACK",
        .description = "Non-linear logic operations",
        .layer = RAF_LAYER_HIGH
    },
    {
        .name = "TOROID",
        .description = "Toroidal topology",
        .layer = RAF_LAYER_HIGH
    },
    {
        .name = "RAFSTORE",
        .description = "Storage management",
        .layer = RAF_LAYER_HIGH
    },
    {
        .name = "BITRAF",
        .description = "Bit-level operations",
        .layer = RAF_LAYER_MID
    },
    {
        .name = "ZIPRAF",
        .description = "Compression and archive utilities",
        .layer = RAF_LAYER_MID
    },
    {
        .name = "42 TOOLS",
        .description = "Utility toolset",
        .layer = RAF_LAYER_MID
    },
    {
        .name = "CORE",
        .description = "Deterministic matrix operations",
        .layer = RAF_LAYER_CORE
    },
    {
        .name = "HARDWARE",
        .description = "CPU detection and features",
        .layer = RAF_LAYER_HAL
    }
};

const raf_system_identity *raf_system_identity_get(void) {
    return &k_identity;
}

size_t raf_system_list_modules(const raf_module_info **modules) {
    if (modules != NULL) {
        *modules = k_modules;
    }
    return sizeof(k_modules) / sizeof(k_modules[0]);
}

const char *raf_layer_name(raf_layer layer) {
    switch (layer) {
        case RAF_LAYER_HIGH:
            return "High-Level Components";
        case RAF_LAYER_MID:
            return "Mid-Level Components";
        case RAF_LAYER_CORE:
            return "Core Components";
        case RAF_LAYER_HAL:
            return "Hardware Abstraction Layer";
        default:
            return "Unknown";
    }
}

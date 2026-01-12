/**
 * Rafaelia Baremetal - Multi-Dimensional Vector Operations Implementation
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License
 */

#include "raf_rafaelia_vector.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Helper: Calculate total size from dimensions */
static uint32_t calc_total_size(const uint32_t *dim, uint32_t ndim) {
    uint32_t total = 1;
    for (uint32_t i = 0; i < ndim; i++) {
        total *= dim[i];
    }
    return total;
}

/* Helper: Calculate flat index from multi-dimensional indices 
 * Returns true if calculation succeeds, false if overflow would occur.
 * Result is stored in out_index parameter.
 */
static bool calc_flat_index(const uint32_t *indices, const uint32_t *dim, uint32_t ndim, uint32_t *out_index) {
    if (!indices || !dim || !out_index) return false;
    
    uint32_t flat_idx = 0;
    uint32_t multiplier = 1;
    for (int32_t i = (int32_t)ndim - 1; i >= 0; i--) {
        /* Check for multiplication overflow */
        if (indices[i] > 0 && multiplier > UINT32_MAX / indices[i]) {
            return false;  /* Overflow in multiplication */
        }
        
        uint32_t term = indices[i] * multiplier;
        
        /* Check for addition overflow */
        if (term > 0 && flat_idx > UINT32_MAX - term) {
            return false;  /* Overflow in addition */
        }
        
        flat_idx += term;
        
        /* Check for overflow before multiplying for next iteration */
        if (i > 0 && dim[i] > 0 && multiplier > UINT32_MAX / dim[i]) {
            return false;  /* Overflow in next multiplier */
        }
        multiplier *= dim[i];
    }
    *out_index = flat_idx;
    return true;
}

/* Toroidal wrapping for 1D */
uint32_t raf_toroid_wrap_1d(int32_t idx, uint32_t size) {
    if (size == 0) return 0;
    int32_t result = idx % (int32_t)size;
    if (result < 0) result += (int32_t)size;
    return (uint32_t)result;
}

/* Toroidal wrapping for ND */
void raf_toroid_wrap_nd(const int32_t *indices, const uint32_t *dimensions, 
                        uint32_t ndim, uint32_t *wrapped) {
    for (uint32_t i = 0; i < ndim; i++) {
        wrapped[i] = raf_toroid_wrap_1d(indices[i], dimensions[i]);
    }
}

/* 1D Vector operations */
raf_vec1d* raf_vec1d_create(uint32_t size) {
    raf_vec1d *vec = (raf_vec1d*)malloc(sizeof(raf_vec1d));
    if (!vec) return NULL;
    
    vec->size = size;
    vec->data = (raf_scalar_t*)calloc(size, sizeof(raf_scalar_t));
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

void raf_vec1d_destroy(raf_vec1d *vec) {
    if (vec) {
        if (vec->data) free(vec->data);
        free(vec);
    }
}

/* 2D Vector operations */
raf_vec2d* raf_vec2d_create(uint32_t width, uint32_t height) {
    raf_vec2d *vec = (raf_vec2d*)malloc(sizeof(raf_vec2d));
    if (!vec) return NULL;
    
    vec->dim[0] = width;
    vec->dim[1] = height;
    uint32_t total = width * height;
    vec->data = (raf_scalar_t*)calloc(total, sizeof(raf_scalar_t));
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

void raf_vec2d_destroy(raf_vec2d *vec) {
    if (vec) {
        if (vec->data) free(vec->data);
        free(vec);
    }
}

/* 3D Vector operations */
raf_vec3d* raf_vec3d_create(uint32_t width, uint32_t height, uint32_t depth) {
    raf_vec3d *vec = (raf_vec3d*)malloc(sizeof(raf_vec3d));
    if (!vec) return NULL;
    
    /* Check for potential overflow in size calculation */
    if (width > 0 && height > 0 && depth > 0) {
        if (width > UINT32_MAX / height || 
            (width * height) > UINT32_MAX / depth) {
            free(vec);
            return NULL;  /* Would overflow */
        }
    }
    
    vec->dim[0] = width;
    vec->dim[1] = height;
    vec->dim[2] = depth;
    uint32_t total = width * height * depth;
    vec->data = (raf_scalar_t*)calloc(total, sizeof(raf_scalar_t));
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

void raf_vec3d_destroy(raf_vec3d *vec) {
    if (vec) {
        if (vec->data) free(vec->data);
        free(vec);
    }
}

/* 4D Vector operations */
raf_vec4d* raf_vec4d_create(uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3) {
    raf_vec4d *vec = (raf_vec4d*)malloc(sizeof(raf_vec4d));
    if (!vec) return NULL;
    
    vec->dim[0] = d0;
    vec->dim[1] = d1;
    vec->dim[2] = d2;
    vec->dim[3] = d3;
    uint32_t total = d0 * d1 * d2 * d3;
    vec->data = (raf_scalar_t*)calloc(total, sizeof(raf_scalar_t));
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

void raf_vec4d_destroy(raf_vec4d *vec) {
    if (vec) {
        if (vec->data) free(vec->data);
        free(vec);
    }
}

/* 5D Vector operations */
raf_vec5d* raf_vec5d_create(uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4) {
    raf_vec5d *vec = (raf_vec5d*)malloc(sizeof(raf_vec5d));
    if (!vec) return NULL;
    
    vec->dim[0] = d0;
    vec->dim[1] = d1;
    vec->dim[2] = d2;
    vec->dim[3] = d3;
    vec->dim[4] = d4;
    uint32_t total = d0 * d1 * d2 * d3 * d4;
    vec->data = (raf_scalar_t*)calloc(total, sizeof(raf_scalar_t));
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

void raf_vec5d_destroy(raf_vec5d *vec) {
    if (vec) {
        if (vec->data) free(vec->data);
        free(vec);
    }
}

/* 6D Vector operations */
raf_vec6d* raf_vec6d_create(uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4, uint32_t d5) {
    raf_vec6d *vec = (raf_vec6d*)malloc(sizeof(raf_vec6d));
    if (!vec) return NULL;
    
    vec->dim[0] = d0;
    vec->dim[1] = d1;
    vec->dim[2] = d2;
    vec->dim[3] = d3;
    vec->dim[4] = d4;
    vec->dim[5] = d5;
    uint32_t total = d0 * d1 * d2 * d3 * d4 * d5;
    vec->data = (raf_scalar_t*)calloc(total, sizeof(raf_scalar_t));
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

void raf_vec6d_destroy(raf_vec6d *vec) {
    if (vec) {
        if (vec->data) free(vec->data);
        free(vec);
    }
}

/* 7D Vector operations */
raf_vec7d* raf_vec7d_create(uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3, 
                            uint32_t d4, uint32_t d5, uint32_t d6) {
    raf_vec7d *vec = (raf_vec7d*)malloc(sizeof(raf_vec7d));
    if (!vec) return NULL;
    
    vec->dim[0] = d0;
    vec->dim[1] = d1;
    vec->dim[2] = d2;
    vec->dim[3] = d3;
    vec->dim[4] = d4;
    vec->dim[5] = d5;
    vec->dim[6] = d6;
    uint32_t total = d0 * d1 * d2 * d3 * d4 * d5 * d6;
    vec->data = (raf_scalar_t*)calloc(total, sizeof(raf_scalar_t));
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

void raf_vec7d_destroy(raf_vec7d *vec) {
    if (vec) {
        if (vec->data) free(vec->data);
        free(vec);
    }
}

/* Generic ND vector operations */
raf_vecnd* raf_vecnd_create(uint32_t dimensions, const uint32_t *dim) {
    if (dimensions < 1 || dimensions > 7) return NULL;
    
    raf_vecnd *vec = (raf_vecnd*)malloc(sizeof(raf_vecnd));
    if (!vec) return NULL;
    
    vec->dimensions = dimensions;
    for (uint32_t i = 0; i < dimensions; i++) {
        vec->dim[i] = dim[i];
    }
    for (uint32_t i = dimensions; i < 7; i++) {
        vec->dim[i] = 0;
    }
    
    vec->total_size = calc_total_size(dim, dimensions);
    vec->data = (raf_scalar_t*)calloc(vec->total_size, sizeof(raf_scalar_t));
    if (!vec->data) {
        free(vec);
        return NULL;
    }
    return vec;
}

void raf_vecnd_destroy(raf_vecnd *vec) {
    if (vec) {
        if (vec->data) free(vec->data);
        free(vec);
    }
}

/* Get/Set operations */
raf_scalar_t raf_vecnd_get(const raf_vecnd *vec, const uint32_t *indices) {
    if (!vec || !vec->data || !indices) return 0.0f;
    uint32_t flat_idx;
    if (!calc_flat_index(indices, vec->dim, vec->dimensions, &flat_idx)) {
        return 0.0f;  /* Overflow in index calculation */
    }
    if (flat_idx >= vec->total_size) return 0.0f;
    return vec->data[flat_idx];
}

void raf_vecnd_set(raf_vecnd *vec, const uint32_t *indices, raf_scalar_t value) {
    if (!vec || !vec->data || !indices) return;
    uint32_t flat_idx;
    if (!calc_flat_index(indices, vec->dim, vec->dimensions, &flat_idx)) {
        return;  /* Overflow in index calculation */
    }
    if (flat_idx >= vec->total_size) return;
    vec->data[flat_idx] = value;
}

raf_scalar_t raf_vecnd_get_toroidal(const raf_vecnd *vec, const int32_t *indices) {
    if (!vec || !vec->data) return 0.0f;
    uint32_t wrapped[7];
    raf_toroid_wrap_nd(indices, vec->dim, vec->dimensions, wrapped);
    return raf_vecnd_get(vec, wrapped);
}

void raf_vecnd_set_toroidal(raf_vecnd *vec, const int32_t *indices, raf_scalar_t value) {
    if (!vec || !vec->data) return;
    uint32_t wrapped[7];
    raf_toroid_wrap_nd(indices, vec->dim, vec->dimensions, wrapped);
    raf_vecnd_set(vec, wrapped, value);
}

/* Arithmetic operations */
void raf_vecnd_add(const raf_vecnd *a, const raf_vecnd *b, raf_vecnd *result) {
    if (!a || !b || !result) return;
    if (a->total_size != b->total_size || a->total_size != result->total_size) return;
    
    for (uint32_t i = 0; i < a->total_size; i++) {
        result->data[i] = a->data[i] + b->data[i];
    }
}

void raf_vecnd_sub(const raf_vecnd *a, const raf_vecnd *b, raf_vecnd *result) {
    if (!a || !b || !result) return;
    if (a->total_size != b->total_size || a->total_size != result->total_size) return;
    
    for (uint32_t i = 0; i < a->total_size; i++) {
        result->data[i] = a->data[i] - b->data[i];
    }
}

void raf_vecnd_mul(const raf_vecnd *a, const raf_vecnd *b, raf_vecnd *result) {
    if (!a || !b || !result) return;
    if (a->total_size != b->total_size || a->total_size != result->total_size) return;
    
    for (uint32_t i = 0; i < a->total_size; i++) {
        result->data[i] = a->data[i] * b->data[i];
    }
}

void raf_vecnd_scale(const raf_vecnd *a, raf_scalar_t scalar, raf_vecnd *result) {
    if (!a || !result) return;
    if (a->total_size != result->total_size) return;
    
    for (uint32_t i = 0; i < a->total_size; i++) {
        result->data[i] = a->data[i] * scalar;
    }
}

/* Reduction operations */
raf_scalar_t raf_vecnd_sum(const raf_vecnd *vec) {
    if (!vec || !vec->data) return 0.0f;
    
    raf_scalar_t sum = 0.0f;
    for (uint32_t i = 0; i < vec->total_size; i++) {
        sum += vec->data[i];
    }
    return sum;
}

raf_scalar_t raf_vecnd_mean(const raf_vecnd *vec) {
    if (!vec || !vec->data || vec->total_size == 0) return 0.0f;
    return raf_vecnd_sum(vec) / (raf_scalar_t)vec->total_size;
}

raf_scalar_t raf_vecnd_min(const raf_vecnd *vec) {
    if (!vec || !vec->data || vec->total_size == 0) return 0.0f;
    
    raf_scalar_t min_val = vec->data[0];
    for (uint32_t i = 1; i < vec->total_size; i++) {
        if (vec->data[i] < min_val) {
            min_val = vec->data[i];
        }
    }
    return min_val;
}

raf_scalar_t raf_vecnd_max(const raf_vecnd *vec) {
    if (!vec || !vec->data || vec->total_size == 0) return 0.0f;
    
    raf_scalar_t max_val = vec->data[0];
    for (uint32_t i = 1; i < vec->total_size; i++) {
        if (vec->data[i] > max_val) {
            max_val = vec->data[i];
        }
    }
    return max_val;
}

/* Neighbor configuration */
raf_neighbor_config* raf_neighbor_config_create(uint32_t ndim, uint32_t connectivity) {
    if (ndim < 1 || ndim > 7) return NULL;
    
    /* Note: connectivity parameter reserved for future use (partial connectivity support) */
    /* Currently only full connectivity (3^ndim - 1) is implemented */
    (void)connectivity;  /* Mark as intentionally unused */
    
    raf_neighbor_config *config = (raf_neighbor_config*)malloc(sizeof(raf_neighbor_config));
    if (!config) return NULL;
    
    config->ndim = ndim;
    
    /* Calculate number of neighbors based on connectivity */
    /* For full connectivity: 3^ndim - 1 (all adjacent cells including diagonals) */
    uint32_t total_neighbors = 1;
    for (uint32_t i = 0; i < ndim; i++) {
        /* Check for overflow before multiplying */
        if (total_neighbors > UINT32_MAX / 3) {
            free(config);
            return NULL;  /* Would overflow */
        }
        total_neighbors *= 3;
    }
    total_neighbors -= 1;  /* Exclude center */
    
    config->count = total_neighbors;
    
    /* Check for overflow in offset allocation size */
    /* We need to allocate: total_neighbors * ndim * sizeof(int32_t) bytes */
    size_t total_elements = 0;
    if (total_neighbors > 0 && ndim > 0) {
        /* Check if total_neighbors * ndim would overflow size_t */
        if ((size_t)total_neighbors > SIZE_MAX / ndim) {
            free(config);
            return NULL;  /* Would overflow */
        }
        total_elements = (size_t)total_neighbors * ndim;
        
        /* Check if total_elements * sizeof(int32_t) would overflow size_t */
        if (total_elements > SIZE_MAX / sizeof(int32_t)) {
            free(config);
            return NULL;  /* Would overflow */
        }
    }
    
    /* Use validated size for allocation */
    config->offsets = (int32_t*)malloc(total_elements * sizeof(int32_t));
    if (!config->offsets) {
        free(config);
        return NULL;
    }
    
    /* Generate all offset combinations */
    uint32_t idx = 0;
    uint32_t total_combinations = total_neighbors + 1;
    for (uint32_t i = 0; i < total_combinations; i++) {
        bool is_center = true;
        uint32_t combo = i;
        
        for (uint32_t d = 0; d < ndim; d++) {
            int32_t offset = (int32_t)(combo % 3) - 1;  /* -1, 0, or 1 */
            combo /= 3;
            
            if (offset != 0) is_center = false;
            
            if (!is_center || i == total_combinations - 1) {
                config->offsets[idx * ndim + d] = offset;
            }
        }
        
        if (!is_center) idx++;
    }
    
    return config;
}

void raf_neighbor_config_destroy(raf_neighbor_config *config) {
    if (config) {
        if (config->offsets) free(config->offsets);
        free(config);
    }
}

/* Get neighbors in toroidal space */
void raf_vecnd_get_neighbors_toroidal(const raf_vecnd *vec, const uint32_t *indices,
                                      const raf_neighbor_config *config,
                                      raf_scalar_t *neighbors, uint32_t *count) {
    if (!vec || !config || !neighbors || !count) return;
    if (vec->dimensions != config->ndim) return;
    
    *count = 0;
    int32_t neighbor_idx[7];
    
    for (uint32_t i = 0; i < config->count; i++) {
        for (uint32_t d = 0; d < config->ndim; d++) {
            neighbor_idx[d] = (int32_t)indices[d] + config->offsets[i * config->ndim + d];
        }
        
        neighbors[i] = raf_vecnd_get_toroidal(vec, neighbor_idx);
        (*count)++;
    }
}

/* Distance in toroidal space */
raf_scalar_t raf_vecnd_distance_toroidal(const raf_vecnd *vec,
                                         const uint32_t *indices1,
                                         const uint32_t *indices2) {
    if (!vec) return 0.0f;
    
    raf_scalar_t dist_sq = 0.0f;
    for (uint32_t d = 0; d < vec->dimensions; d++) {
        int32_t diff = (int32_t)indices1[d] - (int32_t)indices2[d];
        int32_t dim_size = (int32_t)vec->dim[d];
        
        /* Wrap around difference */
        if (diff > dim_size / 2) diff -= dim_size;
        else if (diff < -dim_size / 2) diff += dim_size;
        
        dist_sq += (raf_scalar_t)(diff * diff);
    }
    
    return sqrtf(dist_sq);
}

/* Fill operations */
void raf_vecnd_fill(raf_vecnd *vec, raf_scalar_t value) {
    if (!vec || !vec->data) return;
    for (uint32_t i = 0; i < vec->total_size; i++) {
        vec->data[i] = value;
    }
}

void raf_vecnd_zero(raf_vecnd *vec) {
    raf_vecnd_fill(vec, 0.0f);
}

/* Copy operation */
void raf_vecnd_copy(const raf_vecnd *src, raf_vecnd *dst) {
    if (!src || !dst) return;
    if (src->total_size != dst->total_size) return;
    memcpy(dst->data, src->data, src->total_size * sizeof(raf_scalar_t));
}

/* Convolution (simplified - assumes same dimensions) */
void raf_vecnd_convolve_toroidal(const raf_vecnd *input, const raf_vecnd *kernel,
                                 raf_vecnd *output) {
    if (!input || !kernel || !output) return;
    if (input->dimensions != kernel->dimensions) return;
    if (input->total_size != output->total_size) return;
    
    /* Simplified convolution - iterate through all output points */
    uint32_t indices[7] = {0};
    int32_t kernel_idx[7];
    uint32_t kernel_center[7];
    
    /* Find kernel center */
    for (uint32_t d = 0; d < kernel->dimensions; d++) {
        kernel_center[d] = kernel->dim[d] / 2;
    }
    
    /* Iterate through all output positions */
    for (uint32_t flat = 0; flat < output->total_size; flat++) {
        /* Convert flat index to multi-dimensional indices */
        uint32_t temp = flat;
        for (int32_t d = (int32_t)input->dimensions - 1; d >= 0; d--) {
            indices[d] = temp % input->dim[d];
            temp /= input->dim[d];
        }
        
        raf_scalar_t sum = 0.0f;
        
        /* Convolve with kernel */
        for (uint32_t k_flat = 0; k_flat < kernel->total_size; k_flat++) {
            /* Convert kernel flat index to multi-dimensional */
            uint32_t k_temp = k_flat;
            uint32_t k_indices[7];
            for (int32_t d = (int32_t)kernel->dimensions - 1; d >= 0; d--) {
                k_indices[d] = k_temp % kernel->dim[d];
                k_temp /= kernel->dim[d];
            }
            
            /* Calculate input position (with toroidal wrapping) */
            for (uint32_t d = 0; d < input->dimensions; d++) {
                kernel_idx[d] = (int32_t)indices[d] + (int32_t)k_indices[d] - (int32_t)kernel_center[d];
            }
            
            raf_scalar_t input_val = raf_vecnd_get_toroidal(input, kernel_idx);
            raf_scalar_t kernel_val = kernel->data[k_flat];
            sum += input_val * kernel_val;
        }
        
        output->data[flat] = sum;
    }
}

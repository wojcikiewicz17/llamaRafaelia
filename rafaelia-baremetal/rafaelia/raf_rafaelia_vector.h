/**
 * Rafaelia Baremetal - Multi-Dimensional Vector Operations (1D-7D)
 * 
 * Pure C implementation of vector operations for 1D through 7D spaces
 * with toroidal topology support.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License
 */

#ifndef RAF_RAFAELIA_VECTOR_H
#define RAF_RAFAELIA_VECTOR_H

#include <stdint.h>
#include <stddef.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Basic scalar type for RAFAELIA */
typedef float raf_scalar_t;

/* 1D Vector */
typedef struct {
    raf_scalar_t *data;
    uint32_t size;
} raf_vec1d;

/* 2D Vector/Grid */
typedef struct {
    raf_scalar_t *data;
    uint32_t dim[2];  /* [width, height] */
} raf_vec2d;

/* 3D Vector/Grid */
typedef struct {
    raf_scalar_t *data;
    uint32_t dim[3];  /* [width, height, depth] */
} raf_vec3d;

/* 4D Vector/Grid (Tesseract) */
typedef struct {
    raf_scalar_t *data;
    uint32_t dim[4];  /* [x, y, z, w] */
} raf_vec4d;

/* 5D Vector/Grid */
typedef struct {
    raf_scalar_t *data;
    uint32_t dim[5];
} raf_vec5d;

/* 6D Vector/Grid */
typedef struct {
    raf_scalar_t *data;
    uint32_t dim[6];
} raf_vec6d;

/* 7D Vector/Grid */
typedef struct {
    raf_scalar_t *data;
    uint32_t dim[7];
} raf_vec7d;

/* Generic ND vector (up to 7D) */
typedef struct {
    raf_scalar_t *data;
    uint32_t dimensions;  /* 1-7 */
    uint32_t dim[7];
    uint32_t total_size;
} raf_vecnd;

/* Creation and destruction functions */
raf_vec1d* raf_vec1d_create(uint32_t size);
raf_vec2d* raf_vec2d_create(uint32_t width, uint32_t height);
raf_vec3d* raf_vec3d_create(uint32_t width, uint32_t height, uint32_t depth);
raf_vec4d* raf_vec4d_create(uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3);
raf_vec5d* raf_vec5d_create(uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4);
raf_vec6d* raf_vec6d_create(uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4, uint32_t d5);
raf_vec7d* raf_vec7d_create(uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4, uint32_t d5, uint32_t d6);
raf_vecnd* raf_vecnd_create(uint32_t dimensions, const uint32_t *dim);

void raf_vec1d_destroy(raf_vec1d *vec);
void raf_vec2d_destroy(raf_vec2d *vec);
void raf_vec3d_destroy(raf_vec3d *vec);
void raf_vec4d_destroy(raf_vec4d *vec);
void raf_vec5d_destroy(raf_vec5d *vec);
void raf_vec6d_destroy(raf_vec6d *vec);
void raf_vec7d_destroy(raf_vec7d *vec);
void raf_vecnd_destroy(raf_vecnd *vec);

/* Toroidal index wrapping for ND vectors */
uint32_t raf_toroid_wrap_1d(int32_t idx, uint32_t size);
void raf_toroid_wrap_nd(const int32_t *indices, const uint32_t *dimensions, 
                        uint32_t ndim, uint32_t *wrapped);

/* Generic ND vector operations */
raf_scalar_t raf_vecnd_get(const raf_vecnd *vec, const uint32_t *indices);
void raf_vecnd_set(raf_vecnd *vec, const uint32_t *indices, raf_scalar_t value);
raf_scalar_t raf_vecnd_get_toroidal(const raf_vecnd *vec, const int32_t *indices);
void raf_vecnd_set_toroidal(raf_vecnd *vec, const int32_t *indices, raf_scalar_t value);

/* Vector arithmetic operations */
void raf_vecnd_add(const raf_vecnd *a, const raf_vecnd *b, raf_vecnd *result);
void raf_vecnd_sub(const raf_vecnd *a, const raf_vecnd *b, raf_vecnd *result);
void raf_vecnd_mul(const raf_vecnd *a, const raf_vecnd *b, raf_vecnd *result);
void raf_vecnd_scale(const raf_vecnd *a, raf_scalar_t scalar, raf_vecnd *result);

/* Vector summation and reduction */
raf_scalar_t raf_vecnd_sum(const raf_vecnd *vec);
raf_scalar_t raf_vecnd_mean(const raf_vecnd *vec);
raf_scalar_t raf_vecnd_min(const raf_vecnd *vec);
raf_scalar_t raf_vecnd_max(const raf_vecnd *vec);

/* Toroidal neighbor operations for ND vectors */
typedef struct {
    int32_t *offsets;  /* Flattened array of neighbor offsets */
    uint32_t count;    /* Number of neighbors */
    uint32_t ndim;     /* Number of dimensions */
} raf_neighbor_config;

raf_neighbor_config* raf_neighbor_config_create(uint32_t ndim, uint32_t connectivity);
void raf_neighbor_config_destroy(raf_neighbor_config *config);
void raf_vecnd_get_neighbors_toroidal(const raf_vecnd *vec, const uint32_t *indices,
                                      const raf_neighbor_config *config,
                                      raf_scalar_t *neighbors, uint32_t *count);

/* Convolution for ND vectors (toroidal) */
void raf_vecnd_convolve_toroidal(const raf_vecnd *input, const raf_vecnd *kernel,
                                 raf_vecnd *output);

/* Distance calculations in toroidal space */
raf_scalar_t raf_vecnd_distance_toroidal(const raf_vecnd *vec,
                                         const uint32_t *indices1,
                                         const uint32_t *indices2);

/* Fill operations */
void raf_vecnd_fill(raf_vecnd *vec, raf_scalar_t value);
void raf_vecnd_zero(raf_vecnd *vec);

/* Copy operations */
void raf_vecnd_copy(const raf_vecnd *src, raf_vecnd *dst);

#ifdef __cplusplus
}
#endif

#endif /* RAF_RAFAELIA_VECTOR_H */

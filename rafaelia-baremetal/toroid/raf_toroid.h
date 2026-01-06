/**
 * Rafaelia Baremetal - TOROID Topology Structures
 * 
 * Pure C implementation for toroidal topology and graph structures.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_TOROID_H
#define RAFAELIA_TOROID_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 2D Toroidal Grid Structure */
typedef struct {
    float *data;
    uint32_t width;
    uint32_t height;
} raf_toroid_2d;

/* 3D Toroidal Grid Structure */
typedef struct {
    float *data;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} raf_toroid_3d;

/* Toroidal coordinate wrapping */
uint32_t raf_toroid_wrap(int32_t coord, uint32_t size);

/* 2D Toroid operations */
raf_toroid_2d* raf_toroid_2d_create(uint32_t width, uint32_t height);
void raf_toroid_2d_destroy(raf_toroid_2d *toroid);
float raf_toroid_2d_get(const raf_toroid_2d *toroid, int32_t x, int32_t y);
void raf_toroid_2d_set(raf_toroid_2d *toroid, int32_t x, int32_t y, float value);

/* 2D Toroid neighbors (8-connected) */
void raf_toroid_2d_get_neighbors(const raf_toroid_2d *toroid, int32_t x, int32_t y,
                                 float *neighbors, uint32_t *count);
float raf_toroid_2d_sum_neighbors(const raf_toroid_2d *toroid, int32_t x, int32_t y);
float raf_toroid_2d_avg_neighbors(const raf_toroid_2d *toroid, int32_t x, int32_t y);

/* 2D Toroid convolution */
void raf_toroid_2d_convolve(const raf_toroid_2d *input, const float *kernel,
                            uint32_t kernel_size, raf_toroid_2d *output);

/* 3D Toroid operations */
raf_toroid_3d* raf_toroid_3d_create(uint32_t width, uint32_t height, uint32_t depth);
void raf_toroid_3d_destroy(raf_toroid_3d *toroid);
float raf_toroid_3d_get(const raf_toroid_3d *toroid, int32_t x, int32_t y, int32_t z);
void raf_toroid_3d_set(raf_toroid_3d *toroid, int32_t x, int32_t y, int32_t z, float value);

/* 3D Toroid neighbors (26-connected) */
void raf_toroid_3d_get_neighbors(const raf_toroid_3d *toroid, 
                                 int32_t x, int32_t y, int32_t z,
                                 float *neighbors, uint32_t *count);

/* Toroidal distance calculations */
float raf_toroid_2d_distance(const raf_toroid_2d *toroid, 
                             int32_t x1, int32_t y1, 
                             int32_t x2, int32_t y2);
float raf_toroid_3d_distance(const raf_toroid_3d *toroid,
                             int32_t x1, int32_t y1, int32_t z1,
                             int32_t x2, int32_t y2, int32_t z2);

/* Toroidal path finding (shortest path) */
typedef struct {
    int32_t x;
    int32_t y;
} raf_toroid_2d_point;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} raf_toroid_3d_point;

int raf_toroid_2d_find_path(const raf_toroid_2d *toroid,
                            int32_t start_x, int32_t start_y,
                            int32_t end_x, int32_t end_y,
                            raf_toroid_2d_point *path, uint32_t *path_length);

/* Toroidal cellular automata */
void raf_toroid_2d_automata_step(const raf_toroid_2d *current, 
                                 raf_toroid_2d *next,
                                 float (*rule)(float cell, float neighbors[], uint32_t count));

/* Toroidal diffusion simulation */
void raf_toroid_2d_diffuse(const raf_toroid_2d *source, raf_toroid_2d *dest, float rate);
void raf_toroid_3d_diffuse(const raf_toroid_3d *source, raf_toroid_3d *dest, float rate);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_TOROID_H */

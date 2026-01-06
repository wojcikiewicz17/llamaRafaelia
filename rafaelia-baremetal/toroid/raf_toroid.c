/**
 * Rafaelia Baremetal - TOROID Implementation
 * 
 * Pure C implementation for toroidal topology and graph structures.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_toroid.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Coordinate wrapping for toroidal topology */
uint32_t raf_toroid_wrap(int32_t coord, uint32_t size) {
    if (size == 0) return 0;
    
    int32_t result = coord % (int32_t)size;
    if (result < 0) result += size;
    
    return (uint32_t)result;
}

/* 2D Toroid Creation */
raf_toroid_2d* raf_toroid_2d_create(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) return NULL;
    
    raf_toroid_2d *toroid = (raf_toroid_2d*)malloc(sizeof(raf_toroid_2d));
    if (!toroid) return NULL;
    
    toroid->width = width;
    toroid->height = height;
    toroid->data = (float*)calloc(width * height, sizeof(float));
    
    if (!toroid->data) {
        free(toroid);
        return NULL;
    }
    
    return toroid;
}

void raf_toroid_2d_destroy(raf_toroid_2d *toroid) {
    if (toroid) {
        if (toroid->data) free(toroid->data);
        free(toroid);
    }
}

/* 2D Toroid access with wrapping */
float raf_toroid_2d_get(const raf_toroid_2d *toroid, int32_t x, int32_t y) {
    if (!toroid || !toroid->data) return 0.0f;
    
    uint32_t wrapped_x = raf_toroid_wrap(x, toroid->width);
    uint32_t wrapped_y = raf_toroid_wrap(y, toroid->height);
    
    return toroid->data[wrapped_y * toroid->width + wrapped_x];
}

void raf_toroid_2d_set(raf_toroid_2d *toroid, int32_t x, int32_t y, float value) {
    if (!toroid || !toroid->data) return;
    
    uint32_t wrapped_x = raf_toroid_wrap(x, toroid->width);
    uint32_t wrapped_y = raf_toroid_wrap(y, toroid->height);
    
    toroid->data[wrapped_y * toroid->width + wrapped_x] = value;
}

/* Get 8-connected neighbors */
void raf_toroid_2d_get_neighbors(const raf_toroid_2d *toroid, int32_t x, int32_t y,
                                 float *neighbors, uint32_t *count) {
    if (!toroid || !neighbors || !count) return;
    
    *count = 0;
    
    /* 8 directions: N, NE, E, SE, S, SW, W, NW */
    int32_t dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int32_t dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    
    for (int i = 0; i < 8; i++) {
        neighbors[i] = raf_toroid_2d_get(toroid, x + dx[i], y + dy[i]);
        (*count)++;
    }
}

float raf_toroid_2d_sum_neighbors(const raf_toroid_2d *toroid, int32_t x, int32_t y) {
    float neighbors[8];
    uint32_t count;
    
    raf_toroid_2d_get_neighbors(toroid, x, y, neighbors, &count);
    
    float sum = 0.0f;
    for (uint32_t i = 0; i < count; i++) {
        sum += neighbors[i];
    }
    
    return sum;
}

float raf_toroid_2d_avg_neighbors(const raf_toroid_2d *toroid, int32_t x, int32_t y) {
    float sum = raf_toroid_2d_sum_neighbors(toroid, x, y);
    return sum / 8.0f;
}

/* 2D Convolution with toroidal wrapping */
void raf_toroid_2d_convolve(const raf_toroid_2d *input, const float *kernel,
                            uint32_t kernel_size, raf_toroid_2d *output) {
    if (!input || !kernel || !output) return;
    if (input->width != output->width || input->height != output->height) return;
    
    int32_t half_kernel = kernel_size / 2;
    
    for (uint32_t y = 0; y < input->height; y++) {
        for (uint32_t x = 0; x < input->width; x++) {
            float sum = 0.0f;
            
            for (uint32_t ky = 0; ky < kernel_size; ky++) {
                for (uint32_t kx = 0; kx < kernel_size; kx++) {
                    int32_t ix = x + kx - half_kernel;
                    int32_t iy = y + ky - half_kernel;
                    
                    float value = raf_toroid_2d_get(input, ix, iy);
                    float weight = kernel[ky * kernel_size + kx];
                    
                    sum += value * weight;
                }
            }
            
            raf_toroid_2d_set(output, x, y, sum);
        }
    }
}

/* 3D Toroid Creation */
raf_toroid_3d* raf_toroid_3d_create(uint32_t width, uint32_t height, uint32_t depth) {
    if (width == 0 || height == 0 || depth == 0) return NULL;
    
    raf_toroid_3d *toroid = (raf_toroid_3d*)malloc(sizeof(raf_toroid_3d));
    if (!toroid) return NULL;
    
    toroid->width = width;
    toroid->height = height;
    toroid->depth = depth;
    toroid->data = (float*)calloc(width * height * depth, sizeof(float));
    
    if (!toroid->data) {
        free(toroid);
        return NULL;
    }
    
    return toroid;
}

void raf_toroid_3d_destroy(raf_toroid_3d *toroid) {
    if (toroid) {
        if (toroid->data) free(toroid->data);
        free(toroid);
    }
}

/* 3D Toroid access with wrapping */
float raf_toroid_3d_get(const raf_toroid_3d *toroid, int32_t x, int32_t y, int32_t z) {
    if (!toroid || !toroid->data) return 0.0f;
    
    uint32_t wrapped_x = raf_toroid_wrap(x, toroid->width);
    uint32_t wrapped_y = raf_toroid_wrap(y, toroid->height);
    uint32_t wrapped_z = raf_toroid_wrap(z, toroid->depth);
    
    uint32_t index = wrapped_z * (toroid->width * toroid->height) + 
                     wrapped_y * toroid->width + wrapped_x;
    
    return toroid->data[index];
}

void raf_toroid_3d_set(raf_toroid_3d *toroid, int32_t x, int32_t y, int32_t z, float value) {
    if (!toroid || !toroid->data) return;
    
    uint32_t wrapped_x = raf_toroid_wrap(x, toroid->width);
    uint32_t wrapped_y = raf_toroid_wrap(y, toroid->height);
    uint32_t wrapped_z = raf_toroid_wrap(z, toroid->depth);
    
    uint32_t index = wrapped_z * (toroid->width * toroid->height) + 
                     wrapped_y * toroid->width + wrapped_x;
    
    toroid->data[index] = value;
}

/* Toroidal distance (minimum distance through wrapping) */
float raf_toroid_2d_distance(const raf_toroid_2d *toroid, 
                             int32_t x1, int32_t y1, 
                             int32_t x2, int32_t y2) {
    if (!toroid) return 0.0f;
    
    /* Calculate wrapped distance in each dimension */
    int32_t dx = abs(x2 - x1);
    int32_t dy = abs(y2 - y1);
    
    /* Consider wrapping around */
    if (dx > (int32_t)toroid->width / 2) {
        dx = toroid->width - dx;
    }
    if (dy > (int32_t)toroid->height / 2) {
        dy = toroid->height - dy;
    }
    
    return sqrtf((float)(dx * dx + dy * dy));
}

/* Diffusion simulation */
void raf_toroid_2d_diffuse(const raf_toroid_2d *source, raf_toroid_2d *dest, float rate) {
    if (!source || !dest) return;
    if (source->width != dest->width || source->height != dest->height) return;
    
    for (uint32_t y = 0; y < source->height; y++) {
        for (uint32_t x = 0; x < source->width; x++) {
            float center = raf_toroid_2d_get(source, x, y);
            float avg_neighbors = raf_toroid_2d_avg_neighbors(source, x, y);
            
            /* Simple diffusion: blend center with average of neighbors */
            float new_value = center * (1.0f - rate) + avg_neighbors * rate;
            
            raf_toroid_2d_set(dest, x, y, new_value);
        }
    }
}

void raf_toroid_3d_diffuse(const raf_toroid_3d *source, raf_toroid_3d *dest, float rate) {
    if (!source || !dest) return;
    if (source->width != dest->width || 
        source->height != dest->height || 
        source->depth != dest->depth) return;
    
    /* Simplified 3D diffusion */
    for (uint32_t z = 0; z < source->depth; z++) {
        for (uint32_t y = 0; y < source->height; y++) {
            for (uint32_t x = 0; x < source->width; x++) {
                float center = raf_toroid_3d_get(source, x, y, z);
                
                /* Average of 6 face neighbors */
                float sum = 0.0f;
                sum += raf_toroid_3d_get(source, x-1, y, z);
                sum += raf_toroid_3d_get(source, x+1, y, z);
                sum += raf_toroid_3d_get(source, x, y-1, z);
                sum += raf_toroid_3d_get(source, x, y+1, z);
                sum += raf_toroid_3d_get(source, x, y, z-1);
                sum += raf_toroid_3d_get(source, x, y, z+1);
                float avg_neighbors = sum / 6.0f;
                
                float new_value = center * (1.0f - rate) + avg_neighbors * rate;
                raf_toroid_3d_set(dest, x, y, z, new_value);
            }
        }
    }
}

/* Cellular automata step */
void raf_toroid_2d_automata_step(const raf_toroid_2d *current, 
                                 raf_toroid_2d *next,
                                 float (*rule)(float cell, float neighbors[], uint32_t count)) {
    if (!current || !next || !rule) return;
    if (current->width != next->width || current->height != next->height) return;
    
    for (uint32_t y = 0; y < current->height; y++) {
        for (uint32_t x = 0; x < current->width; x++) {
            float cell = raf_toroid_2d_get(current, x, y);
            float neighbors[8];
            uint32_t count;
            
            raf_toroid_2d_get_neighbors(current, x, y, neighbors, &count);
            
            float new_value = rule(cell, neighbors, count);
            raf_toroid_2d_set(next, x, y, new_value);
        }
    }
}

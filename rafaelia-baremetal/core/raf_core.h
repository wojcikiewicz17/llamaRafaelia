/**
 * Rafaelia Baremetal - Core Linear Algebra Operations
 * 
 * Pure C implementation of deterministic linear algebra operations.
 * Uses matrix arrays for coherent mathematical operations.
 * No external dependencies - baremetal implementation.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_CORE_H
#define RAFAELIA_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic types for deterministic operations */
typedef float raf_scalar;
typedef double raf_scalar_d;

/* Matrix structure - uses static arrays for determinism */
typedef struct {
    unsigned int rows;
    unsigned int cols;
    raf_scalar *data;  /* Flat array: data[i*cols + j] = element at (i,j) */
} raf_matrix;

typedef struct {
    unsigned int rows;
    unsigned int cols;
    raf_scalar_d *data;
} raf_matrix_d;

/* Vector structure - special case of matrix */
typedef struct {
    unsigned int size;
    raf_scalar *data;
} raf_vector;

/* Matrix creation and destruction */
raf_matrix* raf_matrix_create(unsigned int rows, unsigned int cols);
raf_matrix_d* raf_matrix_d_create(unsigned int rows, unsigned int cols);
void raf_matrix_destroy(raf_matrix *mat);
void raf_matrix_d_destroy(raf_matrix_d *mat);
void raf_matrix_zero(raf_matrix *mat);
void raf_matrix_identity(raf_matrix *mat);

/* Vector operations */
raf_vector* raf_vector_create(unsigned int size);
void raf_vector_destroy(raf_vector *vec);
void raf_vector_zero(raf_vector *vec);

/* Basic matrix operations - deterministic */
void raf_matrix_add(const raf_matrix *a, const raf_matrix *b, raf_matrix *result);
void raf_matrix_sub(const raf_matrix *a, const raf_matrix *b, raf_matrix *result);
void raf_matrix_mul(const raf_matrix *a, const raf_matrix *b, raf_matrix *result);
void raf_matrix_scale(const raf_matrix *a, raf_scalar scalar, raf_matrix *result);

/* Matrix-vector operations */
void raf_matrix_vec_mul(const raf_matrix *mat, const raf_vector *vec, raf_vector *result);

/* Linear algebra operations */
void raf_matrix_transpose(const raf_matrix *a, raf_matrix *result);
raf_scalar raf_matrix_determinant(const raf_matrix *mat);
int raf_matrix_inverse(const raf_matrix *mat, raf_matrix *result);

/* Dot products - deterministic order of operations */
raf_scalar raf_vector_dot(const raf_vector *a, const raf_vector *b);
raf_scalar raf_vector_norm(const raf_vector *vec);

/* Element-wise operations */
void raf_matrix_elem_mul(const raf_matrix *a, const raf_matrix *b, raf_matrix *result);
void raf_matrix_elem_div(const raf_matrix *a, const raf_matrix *b, raf_matrix *result);

/* Matrix utilities */
void raf_matrix_copy(const raf_matrix *src, raf_matrix *dst);
void raf_matrix_fill(raf_matrix *mat, raf_scalar value);
raf_scalar raf_matrix_get(const raf_matrix *mat, unsigned int row, unsigned int col);
void raf_matrix_set(raf_matrix *mat, unsigned int row, unsigned int col, raf_scalar value);

/* Deterministic sum - uses Kahan summation for precision */
raf_scalar raf_sum_deterministic(const raf_scalar *data, unsigned int size);
raf_scalar_d raf_sum_deterministic_d(const raf_scalar_d *data, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_CORE_H */

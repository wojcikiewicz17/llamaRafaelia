/**
 * Rafaelia Baremetal - BITSTACK Low-Level Logic Operations
 * 
 * Non-linear logic using bitstack operations with single-flip calculations.
 * Deterministic opportunity detection through state-based measurements.
 * Pure C implementation - baremetal, no external dependencies.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 * 
 * This module is part of the Rafaelia Baremetal extensions to llama.cpp
 * and maintains compatibility with the MIT License of the original project.
 */

#ifndef RAFAELIA_BITSTACK_H
#define RAFAELIA_BITSTACK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Core Bitstack Data Structures
 * ============================================================================ */

/**
 * Bitstack state structure
 * Tracks bit state and provides non-linear logic operations
 */
typedef struct {
    uint64_t *layers;          /* Bit layers for multi-dimensional state */
    size_t num_layers;         /* Number of layers in the stack */
    size_t bits_per_layer;     /* Bits per layer (typically 64) */
    uint32_t flip_count;       /* Track number of flips performed */
    uint64_t state_hash;       /* Current state hash for tracking */
} raf_bitstack_state;

/**
 * Opportunity measurement structure
 * Tracks multiple measurement points for opportunity detection
 */
typedef struct {
    uint64_t measurement_id;   /* Unique measurement identifier */
    double confidence;         /* Confidence score (0.0 - 1.0) */
    uint32_t bit_positions[8]; /* Up to 8 relevant bit positions */
    uint32_t num_positions;    /* Number of active positions */
    int32_t delta_score;       /* Change in score from previous state */
} raf_opportunity_measurement;

/**
 * Calculation result with multiple opportunities
 */
typedef struct {
    raf_opportunity_measurement *opportunities; /* Array of opportunities */
    size_t num_opportunities;                   /* Number of detected opportunities */
    size_t capacity;                            /* Allocated capacity */
    uint64_t total_evaluations;                 /* Total calculations performed */
} raf_calc_result;

/* ============================================================================
 * Bitstack Core Operations
 * ============================================================================ */

/**
 * Create a new bitstack state with specified layers and bits per layer
 * @param num_layers Number of bit layers
 * @param bits_per_layer Bits per layer (must be multiple of 64)
 * @return Newly allocated bitstack state or NULL on failure
 */
raf_bitstack_state* raf_bitstack_create(size_t num_layers, size_t bits_per_layer);

/**
 * Destroy bitstack state and free resources
 * @param state Bitstack state to destroy
 */
void raf_bitstack_destroy(raf_bitstack_state *state);

/**
 * Reset bitstack to initial state
 * @param state Bitstack state to reset
 */
void raf_bitstack_reset(raf_bitstack_state *state);

/**
 * Clone bitstack state for parallel evaluation
 * @param state Source state to clone
 * @return New cloned state or NULL on failure
 */
raf_bitstack_state* raf_bitstack_clone(const raf_bitstack_state *state);

/* ============================================================================
 * Single-Flip Operations (Non-Linear Logic)
 * ============================================================================ */

/**
 * Perform single bit flip at specified layer and position
 * This is the core operation for non-linear logic resolution
 * @param state Bitstack state
 * @param layer Layer index
 * @param bit_pos Bit position within layer
 * @return 0 on success, -1 on failure
 */
int raf_bitstack_flip_single(raf_bitstack_state *state, size_t layer, size_t bit_pos);

/**
 * Evaluate effect of potential flip without modifying state
 * @param state Current state
 * @param layer Layer to evaluate
 * @param bit_pos Bit position to evaluate
 * @param effect_score Output score indicating flip effect
 * @return 0 on success, -1 on failure
 */
int raf_bitstack_evaluate_flip(const raf_bitstack_state *state, 
                                size_t layer, size_t bit_pos,
                                double *effect_score);

/**
 * Apply flip cascade - single flip may trigger multiple related changes
 * Non-linear propagation through layers
 * @param state Bitstack state
 * @param initial_layer Starting layer
 * @param initial_pos Starting bit position
 * @param max_cascade Maximum cascade depth
 * @return Number of bits affected by cascade
 */
int raf_bitstack_flip_cascade(raf_bitstack_state *state,
                               size_t initial_layer, size_t initial_pos,
                               size_t max_cascade);

/* ============================================================================
 * Deterministic Calculation Functions
 * ============================================================================ */

/**
 * Calculate state hash for deterministic tracking
 * @param state Bitstack state
 * @return 64-bit hash of current state
 */
uint64_t raf_bitstack_calc_hash(const raf_bitstack_state *state);

/**
 * Calculate Hamming distance between two states
 * @param state1 First state
 * @param state2 Second state
 * @return Hamming distance (number of differing bits)
 */
size_t raf_bitstack_calc_distance(const raf_bitstack_state *state1,
                                   const raf_bitstack_state *state2);

/**
 * Calculate entropy of current state
 * Measures randomness/information content
 * @param state Bitstack state
 * @return Entropy value (0.0 = all same, 1.0 = maximum entropy)
 */
double raf_bitstack_calc_entropy(const raf_bitstack_state *state);

/**
 * Calculate correlation between layers
 * @param state Bitstack state
 * @param layer1 First layer index
 * @param layer2 Second layer index
 * @return Correlation coefficient (-1.0 to 1.0)
 */
double raf_bitstack_calc_correlation(const raf_bitstack_state *state,
                                      size_t layer1, size_t layer2);

/* ============================================================================
 * Multi-Point Measurement Functions
 * ============================================================================ */

/**
 * Create calculation result structure
 * @param initial_capacity Initial capacity for opportunities
 * @return New calc result or NULL on failure
 */
raf_calc_result* raf_calc_result_create(size_t initial_capacity);

/**
 * Destroy calculation result and free resources
 * @param result Result to destroy
 */
void raf_calc_result_destroy(raf_calc_result *result);

/**
 * Measure opportunities at current state
 * Evaluates multiple points to detect fitting results
 * @param state Current bitstack state
 * @param result Output result structure
 * @param measurement_points Array of bit positions to measure
 * @param num_points Number of measurement points
 * @return Number of opportunities detected
 */
int raf_bitstack_measure_opportunities(const raf_bitstack_state *state,
                                        raf_calc_result *result,
                                        const uint32_t *measurement_points,
                                        size_t num_points);

/**
 * Scan for optimal flip positions
 * Tests multiple positions to find best opportunities
 * @param state Current state
 * @param result Output result with ranked opportunities
 * @param scan_depth Number of positions to scan per layer
 * @return Number of opportunities found
 */
int raf_bitstack_scan_optimal(const raf_bitstack_state *state,
                               raf_calc_result *result,
                               size_t scan_depth);

/**
 * Apply best opportunity from result
 * Executes the highest-confidence opportunity
 * @param state Bitstack state to modify
 * @param result Result containing opportunities
 * @return 0 on success, -1 if no opportunities
 */
int raf_bitstack_apply_best_opportunity(raf_bitstack_state *state,
                                         const raf_calc_result *result);

/* ============================================================================
 * Advanced Non-Linear Logic Operations
 * ============================================================================ */

/**
 * Solve using iterative flip optimization
 * Applies single flips iteratively to reach target state
 * @param state Current state
 * @param target Target state to reach
 * @param max_iterations Maximum iterations allowed
 * @param result Output result tracking progress
 * @return Number of iterations to solution, or -1 if not reached
 */
int raf_bitstack_solve_iterative(raf_bitstack_state *state,
                                  const raf_bitstack_state *target,
                                  size_t max_iterations,
                                  raf_calc_result *result);

/**
 * Pattern matching using bitstack logic
 * Finds patterns in current state
 * @param state State to analyze
 * @param pattern Pattern bits to match
 * @param pattern_length Length of pattern
 * @param matches Output array of match positions
 * @param max_matches Maximum matches to find
 * @return Number of matches found
 */
int raf_bitstack_match_pattern(const raf_bitstack_state *state,
                                const uint64_t *pattern,
                                size_t pattern_length,
                                uint32_t *matches,
                                size_t max_matches);

/**
 * Optimize state towards goal using gradient descent on bit flips
 * @param state State to optimize
 * @param goal_function Function pointer to evaluate goal (higher = better)
 * @param max_steps Maximum optimization steps
 * @return Final goal function value
 */
double raf_bitstack_optimize_gradient(raf_bitstack_state *state,
                                       double (*goal_function)(const raf_bitstack_state*),
                                       size_t max_steps);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * Get bit value at specified layer and position
 * @param state Bitstack state
 * @param layer Layer index
 * @param bit_pos Bit position
 * @param value Output bit value
 * @return 0 on success, -1 on failure
 */
int raf_bitstack_get_bit(const raf_bitstack_state *state,
                          size_t layer, size_t bit_pos,
                          bool *value);

/**
 * Set bit value at specified layer and position
 * @param state Bitstack state
 * @param layer Layer index
 * @param bit_pos Bit position
 * @param value Bit value to set
 * @return 0 on success, -1 on failure
 */
int raf_bitstack_set_bit(raf_bitstack_state *state,
                          size_t layer, size_t bit_pos,
                          bool value);

/**
 * Count set bits in specified layer
 * @param state Bitstack state
 * @param layer Layer index
 * @return Number of set bits
 */
size_t raf_bitstack_count_bits(const raf_bitstack_state *state, size_t layer);

/**
 * Print state for debugging (prints to stdout)
 * @param state State to print
 * @param prefix Optional prefix string
 */
void raf_bitstack_print_state(const raf_bitstack_state *state, const char *prefix);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_BITSTACK_H */

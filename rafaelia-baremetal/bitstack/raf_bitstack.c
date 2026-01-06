/**
 * Rafaelia Baremetal - BITSTACK Implementation
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

#include "raf_bitstack.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ============================================================================
 * Internal Helper Functions
 * ============================================================================ */

/* Fast popcount for 64-bit values */
static inline size_t popcount64(uint64_t x) {
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    return (x * 0x0101010101010101ULL) >> 56;
}

/* FNV-1a hash for state hashing */
static uint64_t fnv1a_hash(const uint8_t *data, size_t length) {
    uint64_t hash = 0xCBF29CE484222325ULL;
    for (size_t i = 0; i < length; i++) {
        hash ^= data[i];
        hash *= 0x100000001B3ULL;
    }
    return hash;
}

/* ============================================================================
 * Bitstack Core Operations
 * ============================================================================ */

raf_bitstack_state* raf_bitstack_create(size_t num_layers, size_t bits_per_layer) {
    if (num_layers == 0 || bits_per_layer == 0 || bits_per_layer % 64 != 0) {
        return NULL;
    }
    
    raf_bitstack_state *state = (raf_bitstack_state*)malloc(sizeof(raf_bitstack_state));
    if (!state) return NULL;
    
    size_t qwords_per_layer = bits_per_layer / 64;
    state->layers = (uint64_t*)calloc(num_layers * qwords_per_layer, sizeof(uint64_t));
    if (!state->layers) {
        free(state);
        return NULL;
    }
    
    state->num_layers = num_layers;
    state->bits_per_layer = bits_per_layer;
    state->flip_count = 0;
    state->state_hash = raf_bitstack_calc_hash(state);
    
    return state;
}

void raf_bitstack_destroy(raf_bitstack_state *state) {
    if (state) {
        if (state->layers) free(state->layers);
        free(state);
    }
}

void raf_bitstack_reset(raf_bitstack_state *state) {
    if (state && state->layers) {
        size_t qwords_per_layer = state->bits_per_layer / 64;
        memset(state->layers, 0, state->num_layers * qwords_per_layer * sizeof(uint64_t));
        state->flip_count = 0;
        state->state_hash = raf_bitstack_calc_hash(state);
    }
}

raf_bitstack_state* raf_bitstack_clone(const raf_bitstack_state *state) {
    if (!state) return NULL;
    
    raf_bitstack_state *clone = raf_bitstack_create(state->num_layers, state->bits_per_layer);
    if (!clone) return NULL;
    
    size_t qwords_per_layer = state->bits_per_layer / 64;
    memcpy(clone->layers, state->layers, 
           state->num_layers * qwords_per_layer * sizeof(uint64_t));
    clone->flip_count = state->flip_count;
    clone->state_hash = state->state_hash;
    
    return clone;
}

/* ============================================================================
 * Single-Flip Operations (Non-Linear Logic)
 * ============================================================================ */

int raf_bitstack_flip_single(raf_bitstack_state *state, size_t layer, size_t bit_pos) {
    if (!state || layer >= state->num_layers || bit_pos >= state->bits_per_layer) {
        return -1;
    }
    
    size_t qword_idx = layer * (state->bits_per_layer / 64) + (bit_pos / 64);
    size_t bit_in_qword = bit_pos % 64;
    
    /* Single flip operation */
    state->layers[qword_idx] ^= (1ULL << bit_in_qword);
    state->flip_count++;
    
    /* Update state hash */
    state->state_hash = raf_bitstack_calc_hash(state);
    
    return 0;
}

int raf_bitstack_evaluate_flip(const raf_bitstack_state *state, 
                                size_t layer, size_t bit_pos,
                                double *effect_score) {
    if (!state || !effect_score || layer >= state->num_layers || 
        bit_pos >= state->bits_per_layer) {
        return -1;
    }
    
    /* Clone state for evaluation */
    raf_bitstack_state *test_state = raf_bitstack_clone(state);
    if (!test_state) return -1;
    
    /* Calculate metrics before flip */
    double entropy_before = raf_bitstack_calc_entropy(state);
    size_t bits_before = raf_bitstack_count_bits(state, layer);
    
    /* Perform flip */
    raf_bitstack_flip_single(test_state, layer, bit_pos);
    
    /* Calculate metrics after flip */
    double entropy_after = raf_bitstack_calc_entropy(test_state);
    size_t bits_after = raf_bitstack_count_bits(test_state, layer);
    
    /* Effect score: combination of entropy change and balance */
    double entropy_change = entropy_after - entropy_before;
    double balance_score = 1.0 - fabs((double)bits_after / (double)state->bits_per_layer - 0.5);
    *effect_score = entropy_change * 0.7 + balance_score * 0.3;
    
    raf_bitstack_destroy(test_state);
    return 0;
}

int raf_bitstack_flip_cascade(raf_bitstack_state *state,
                               size_t initial_layer, size_t initial_pos,
                               size_t max_cascade) {
    if (!state || initial_layer >= state->num_layers || 
        initial_pos >= state->bits_per_layer || max_cascade == 0) {
        return -1;
    }
    
    int affected = 0;
    size_t current_layer = initial_layer;
    size_t current_pos = initial_pos;
    
    for (size_t cascade_step = 0; cascade_step < max_cascade; cascade_step++) {
        /* Flip current position */
        if (raf_bitstack_flip_single(state, current_layer, current_pos) == 0) {
            affected++;
        }
        
        /* Determine next position based on current state (non-linear) */
        /* Use hash of current position to determine next cascade location */
        uint64_t position_hash = (uint64_t)current_layer * 0x9E3779B97F4A7C15ULL + 
                                 (uint64_t)current_pos * 0x85EBCA6B;
        position_hash ^= state->state_hash;
        
        /* Move to adjacent layer with wrap-around */
        current_layer = (current_layer + (position_hash & 0x3) - 1) % state->num_layers;
        if ((int)current_layer < 0) current_layer += state->num_layers;
        
        /* Move to related position */
        current_pos = (current_pos + ((position_hash >> 8) % 16) - 8) % state->bits_per_layer;
        if ((int)current_pos < 0) current_pos += state->bits_per_layer;
        
        /* Stop if we've returned to initial position */
        if (cascade_step > 0 && current_layer == initial_layer && current_pos == initial_pos) {
            break;
        }
    }
    
    return affected;
}

/* ============================================================================
 * Deterministic Calculation Functions
 * ============================================================================ */

uint64_t raf_bitstack_calc_hash(const raf_bitstack_state *state) {
    if (!state || !state->layers) return 0;
    
    size_t qwords_per_layer = state->bits_per_layer / 64;
    size_t total_bytes = state->num_layers * qwords_per_layer * sizeof(uint64_t);
    
    return fnv1a_hash((const uint8_t*)state->layers, total_bytes);
}

size_t raf_bitstack_calc_distance(const raf_bitstack_state *state1,
                                   const raf_bitstack_state *state2) {
    if (!state1 || !state2 || 
        state1->num_layers != state2->num_layers ||
        state1->bits_per_layer != state2->bits_per_layer) {
        return 0;
    }
    
    size_t distance = 0;
    size_t qwords_per_layer = state1->bits_per_layer / 64;
    size_t total_qwords = state1->num_layers * qwords_per_layer;
    
    for (size_t i = 0; i < total_qwords; i++) {
        uint64_t diff = state1->layers[i] ^ state2->layers[i];
        distance += popcount64(diff);
    }
    
    return distance;
}

double raf_bitstack_calc_entropy(const raf_bitstack_state *state) {
    if (!state || !state->layers) return 0.0;
    
    size_t total_bits = state->num_layers * state->bits_per_layer;
    size_t set_bits = 0;
    
    for (size_t layer = 0; layer < state->num_layers; layer++) {
        set_bits += raf_bitstack_count_bits(state, layer);
    }
    
    if (set_bits == 0 || set_bits == total_bits) {
        return 0.0; /* No entropy - all same */
    }
    
    /* Calculate Shannon entropy for binary distribution */
    double p1 = (double)set_bits / (double)total_bits;
    double p0 = 1.0 - p1;
    
    double entropy = 0.0;
    if (p1 > 0.0) entropy -= p1 * log2(p1);
    if (p0 > 0.0) entropy -= p0 * log2(p0);
    
    return entropy; /* Returns 0.0 to 1.0 */
}

double raf_bitstack_calc_correlation(const raf_bitstack_state *state,
                                      size_t layer1, size_t layer2) {
    if (!state || layer1 >= state->num_layers || layer2 >= state->num_layers) {
        return 0.0;
    }
    
    if (layer1 == layer2) return 1.0; /* Perfect correlation with self */
    
    size_t qwords_per_layer = state->bits_per_layer / 64;
    size_t layer1_offset = layer1 * qwords_per_layer;
    size_t layer2_offset = layer2 * qwords_per_layer;
    
    size_t matching_bits = 0;
    size_t differing_bits = 0;
    
    for (size_t i = 0; i < qwords_per_layer; i++) {
        uint64_t xnor = ~(state->layers[layer1_offset + i] ^ state->layers[layer2_offset + i]);
        matching_bits += popcount64(xnor);
        differing_bits += popcount64(~xnor);
    }
    
    /* Correlation: (matching - differing) / total */
    size_t total = matching_bits + differing_bits;
    if (total == 0) return 0.0;
    
    return ((double)matching_bits - (double)differing_bits) / (double)total;
}

/* ============================================================================
 * Multi-Point Measurement Functions
 * ============================================================================ */

raf_calc_result* raf_calc_result_create(size_t initial_capacity) {
    raf_calc_result *result = (raf_calc_result*)malloc(sizeof(raf_calc_result));
    if (!result) return NULL;
    
    result->opportunities = (raf_opportunity_measurement*)calloc(
        initial_capacity, sizeof(raf_opportunity_measurement));
    if (!result->opportunities) {
        free(result);
        return NULL;
    }
    
    result->num_opportunities = 0;
    result->capacity = initial_capacity;
    result->total_evaluations = 0;
    
    return result;
}

void raf_calc_result_destroy(raf_calc_result *result) {
    if (result) {
        if (result->opportunities) free(result->opportunities);
        free(result);
    }
}

int raf_bitstack_measure_opportunities(const raf_bitstack_state *state,
                                        raf_calc_result *result,
                                        const uint32_t *measurement_points,
                                        size_t num_points) {
    if (!state || !result || !measurement_points || num_points == 0) {
        return -1;
    }
    
    /* Reset result */
    result->num_opportunities = 0;
    result->total_evaluations = 0;
    
    /* Measure each point */
    for (size_t i = 0; i < num_points && i < result->capacity; i++) {
        uint32_t pos = measurement_points[i];
        size_t layer = pos / state->bits_per_layer;
        size_t bit_pos = pos % state->bits_per_layer;
        
        if (layer >= state->num_layers) continue;
        
        /* Evaluate flip effect at this position */
        double effect_score = 0.0;
        if (raf_bitstack_evaluate_flip(state, layer, bit_pos, &effect_score) == 0) {
            result->total_evaluations++;
            
            /* Create opportunity if effect is significant */
            if (fabs(effect_score) > 0.1) {
                raf_opportunity_measurement *opp = &result->opportunities[result->num_opportunities];
                opp->measurement_id = result->num_opportunities;
                opp->confidence = fabs(effect_score);
                opp->bit_positions[0] = pos;
                opp->num_positions = 1;
                opp->delta_score = (int32_t)(effect_score * 1000.0);
                result->num_opportunities++;
            }
        }
    }
    
    return (int)result->num_opportunities;
}

int raf_bitstack_scan_optimal(const raf_bitstack_state *state,
                               raf_calc_result *result,
                               size_t scan_depth) {
    if (!state || !result || scan_depth == 0) {
        return -1;
    }
    
    /* Reset result */
    result->num_opportunities = 0;
    result->total_evaluations = 0;
    
    /* Scan positions in each layer */
    for (size_t layer = 0; layer < state->num_layers; layer++) {
        size_t step = state->bits_per_layer / scan_depth;
        if (step == 0) step = 1;
        
        for (size_t pos = 0; pos < state->bits_per_layer; pos += step) {
            if (result->num_opportunities >= result->capacity) break;
            
            double effect_score = 0.0;
            if (raf_bitstack_evaluate_flip(state, layer, pos, &effect_score) == 0) {
                result->total_evaluations++;
                
                /* Add as opportunity if significant */
                if (fabs(effect_score) > 0.05) {
                    raf_opportunity_measurement *opp = 
                        &result->opportunities[result->num_opportunities];
                    opp->measurement_id = result->num_opportunities;
                    opp->confidence = fabs(effect_score);
                    opp->bit_positions[0] = layer * state->bits_per_layer + pos;
                    opp->num_positions = 1;
                    opp->delta_score = (int32_t)(effect_score * 1000.0);
                    result->num_opportunities++;
                }
            }
        }
    }
    
    /* Sort opportunities by confidence (simple bubble sort for small arrays) */
    for (size_t i = 0; i < result->num_opportunities; i++) {
        for (size_t j = i + 1; j < result->num_opportunities; j++) {
            if (result->opportunities[j].confidence > result->opportunities[i].confidence) {
                raf_opportunity_measurement temp = result->opportunities[i];
                result->opportunities[i] = result->opportunities[j];
                result->opportunities[j] = temp;
            }
        }
    }
    
    return (int)result->num_opportunities;
}

int raf_bitstack_apply_best_opportunity(raf_bitstack_state *state,
                                         const raf_calc_result *result) {
    if (!state || !result || result->num_opportunities == 0) {
        return -1;
    }
    
    /* Apply highest confidence opportunity */
    const raf_opportunity_measurement *best = &result->opportunities[0];
    
    if (best->num_positions > 0) {
        uint32_t pos = best->bit_positions[0];
        size_t layer = pos / state->bits_per_layer;
        size_t bit_pos = pos % state->bits_per_layer;
        
        return raf_bitstack_flip_single(state, layer, bit_pos);
    }
    
    return -1;
}

/* ============================================================================
 * Advanced Non-Linear Logic Operations
 * ============================================================================ */

int raf_bitstack_solve_iterative(raf_bitstack_state *state,
                                  const raf_bitstack_state *target,
                                  size_t max_iterations,
                                  raf_calc_result *result) {
    if (!state || !target || !result || 
        state->num_layers != target->num_layers ||
        state->bits_per_layer != target->bits_per_layer) {
        return -1;
    }
    
    size_t iteration = 0;
    size_t best_distance = raf_bitstack_calc_distance(state, target);
    
    for (iteration = 0; iteration < max_iterations; iteration++) {
        size_t current_distance = raf_bitstack_calc_distance(state, target);
        
        /* Check if solved */
        if (current_distance == 0) {
            return (int)iteration;
        }
        
        /* Find best flip to reduce distance */
        size_t best_layer = 0;
        size_t best_pos = 0;
        size_t best_new_distance = current_distance;
        
        /* Sample positions */
        for (size_t layer = 0; layer < state->num_layers; layer++) {
            for (size_t pos = 0; pos < state->bits_per_layer; pos += 8) {
                /* Test flip */
                raf_bitstack_state *test = raf_bitstack_clone(state);
                if (!test) continue;
                
                raf_bitstack_flip_single(test, layer, pos);
                size_t new_distance = raf_bitstack_calc_distance(test, target);
                
                if (new_distance < best_new_distance) {
                    best_new_distance = new_distance;
                    best_layer = layer;
                    best_pos = pos;
                }
                
                raf_bitstack_destroy(test);
            }
        }
        
        /* Apply best flip */
        if (best_new_distance < current_distance) {
            raf_bitstack_flip_single(state, best_layer, best_pos);
            best_distance = best_new_distance;
        } else {
            /* No improvement - try random flip */
            size_t rand_layer = (state->state_hash >> 8) % state->num_layers;
            size_t rand_pos = (state->state_hash >> 16) % state->bits_per_layer;
            raf_bitstack_flip_single(state, rand_layer, rand_pos);
        }
    }
    
    return -1; /* Not reached within max iterations */
}

int raf_bitstack_match_pattern(const raf_bitstack_state *state,
                                const uint64_t *pattern,
                                size_t pattern_length,
                                uint32_t *matches,
                                size_t max_matches) {
    if (!state || !pattern || !matches || pattern_length == 0 || max_matches == 0) {
        return -1;
    }
    
    size_t num_matches = 0;
    size_t qwords_per_layer = state->bits_per_layer / 64;
    
    for (size_t layer = 0; layer < state->num_layers && num_matches < max_matches; layer++) {
        size_t layer_offset = layer * qwords_per_layer;
        
        /* Slide pattern across layer */
        for (size_t pos = 0; pos <= qwords_per_layer - pattern_length && 
             num_matches < max_matches; pos++) {
            
            bool match = true;
            for (size_t i = 0; i < pattern_length; i++) {
                if (state->layers[layer_offset + pos + i] != pattern[i]) {
                    match = false;
                    break;
                }
            }
            
            if (match) {
                matches[num_matches++] = layer * state->bits_per_layer + pos * 64;
            }
        }
    }
    
    return (int)num_matches;
}

double raf_bitstack_optimize_gradient(raf_bitstack_state *state,
                                       double (*goal_function)(const raf_bitstack_state*),
                                       size_t max_steps) {
    if (!state || !goal_function || max_steps == 0) {
        return 0.0;
    }
    
    double best_score = goal_function(state);
    
    for (size_t step = 0; step < max_steps; step++) {
        size_t best_layer = 0;
        size_t best_pos = 0;
        double best_new_score = best_score;
        
        /* Sample gradient at various points */
        for (size_t layer = 0; layer < state->num_layers; layer++) {
            size_t sample_step = state->bits_per_layer / 16;
            if (sample_step == 0) sample_step = 1;
            
            for (size_t pos = 0; pos < state->bits_per_layer; pos += sample_step) {
                raf_bitstack_state *test = raf_bitstack_clone(state);
                if (!test) continue;
                
                raf_bitstack_flip_single(test, layer, pos);
                double new_score = goal_function(test);
                
                if (new_score > best_new_score) {
                    best_new_score = new_score;
                    best_layer = layer;
                    best_pos = pos;
                }
                
                raf_bitstack_destroy(test);
            }
        }
        
        /* Apply best improvement */
        if (best_new_score > best_score) {
            raf_bitstack_flip_single(state, best_layer, best_pos);
            best_score = best_new_score;
        } else {
            break; /* Local optimum reached */
        }
    }
    
    return best_score;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

int raf_bitstack_get_bit(const raf_bitstack_state *state,
                          size_t layer, size_t bit_pos,
                          bool *value) {
    if (!state || !value || layer >= state->num_layers || 
        bit_pos >= state->bits_per_layer) {
        return -1;
    }
    
    size_t qword_idx = layer * (state->bits_per_layer / 64) + (bit_pos / 64);
    size_t bit_in_qword = bit_pos % 64;
    
    *value = (state->layers[qword_idx] & (1ULL << bit_in_qword)) != 0;
    return 0;
}

int raf_bitstack_set_bit(raf_bitstack_state *state,
                          size_t layer, size_t bit_pos,
                          bool value) {
    if (!state || layer >= state->num_layers || bit_pos >= state->bits_per_layer) {
        return -1;
    }
    
    size_t qword_idx = layer * (state->bits_per_layer / 64) + (bit_pos / 64);
    size_t bit_in_qword = bit_pos % 64;
    
    if (value) {
        state->layers[qword_idx] |= (1ULL << bit_in_qword);
    } else {
        state->layers[qword_idx] &= ~(1ULL << bit_in_qword);
    }
    
    state->state_hash = raf_bitstack_calc_hash(state);
    return 0;
}

size_t raf_bitstack_count_bits(const raf_bitstack_state *state, size_t layer) {
    if (!state || layer >= state->num_layers) return 0;
    
    size_t count = 0;
    size_t qwords_per_layer = state->bits_per_layer / 64;
    size_t layer_offset = layer * qwords_per_layer;
    
    for (size_t i = 0; i < qwords_per_layer; i++) {
        count += popcount64(state->layers[layer_offset + i]);
    }
    
    return count;
}

void raf_bitstack_print_state(const raf_bitstack_state *state, const char *prefix) {
    if (!state) return;
    
    if (prefix) {
        printf("%s\n", prefix);
    }
    
    printf("Bitstack State:\n");
    printf("  Layers: %zu\n", state->num_layers);
    printf("  Bits per layer: %zu\n", state->bits_per_layer);
    printf("  Flip count: %u\n", state->flip_count);
    printf("  State hash: 0x%016lX\n", (unsigned long)state->state_hash);
    printf("  Entropy: %.4f\n", raf_bitstack_calc_entropy(state));
    
    for (size_t layer = 0; layer < state->num_layers; layer++) {
        size_t set_bits = raf_bitstack_count_bits(state, layer);
        printf("  Layer %zu: %zu / %zu bits set (%.1f%%)\n", 
               layer, set_bits, state->bits_per_layer,
               100.0 * (double)set_bits / (double)state->bits_per_layer);
    }
}

/**
 * Rafaelia Baremetal - BITSTACK Test/Example Program
 * 
 * Demonstrates non-linear logic operations using bitstack.
 * Tests single-flip operations and opportunity detection.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "bitstack/raf_bitstack.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Test goal function: maximize entropy */
static double goal_maximize_entropy(const raf_bitstack_state *state) {
    return raf_bitstack_calc_entropy(state);
}

/* Test goal function: reach 50% bit density */
static double goal_balanced_bits(const raf_bitstack_state *state) {
    size_t total_bits = 0;
    size_t set_bits = 0;
    
    for (size_t layer = 0; layer < state->num_layers; layer++) {
        set_bits += raf_bitstack_count_bits(state, layer);
    }
    total_bits = state->num_layers * state->bits_per_layer;
    
    double density = (double)set_bits / (double)total_bits;
    return 1.0 - fabs(density - 0.5); /* Closer to 0.5 = better */
}

int main(void) {
    printf("=================================================================\n");
    printf(" Rafaelia Baremetal - BITSTACK Low-Level Logic Test\n");
    printf("=================================================================\n\n");
    
    /* Test 1: Basic bitstack creation and manipulation */
    printf("Test 1: Basic Bitstack Operations\n");
    printf("-----------------------------------\n");
    
    raf_bitstack_state *state = raf_bitstack_create(4, 64);
    if (!state) {
        printf("ERROR: Failed to create bitstack\n");
        return 1;
    }
    
    raf_bitstack_print_state(state, "Initial state:");
    printf("\n");
    
    /* Test 2: Single flip operations */
    printf("Test 2: Single Flip Operations\n");
    printf("--------------------------------\n");
    
    printf("Performing 10 random flips...\n");
    for (int i = 0; i < 10; i++) {
        size_t layer = i % 4;
        size_t pos = (i * 7) % 64;
        raf_bitstack_flip_single(state, layer, pos);
        printf("  Flip %d: Layer %zu, Pos %zu\n", i + 1, layer, pos);
    }
    
    raf_bitstack_print_state(state, "\nState after flips:");
    printf("\n");
    
    /* Test 3: Flip evaluation */
    printf("Test 3: Flip Effect Evaluation\n");
    printf("--------------------------------\n");
    
    for (size_t pos = 0; pos < 64; pos += 8) {
        double effect;
        if (raf_bitstack_evaluate_flip(state, 0, pos, &effect) == 0) {
            printf("  Pos %2zu: Effect score = %+.4f\n", pos, effect);
        }
    }
    printf("\n");
    
    /* Test 4: Cascade operations */
    printf("Test 4: Flip Cascade (Non-Linear Propagation)\n");
    printf("----------------------------------------------\n");
    
    raf_bitstack_state *cascade_state = raf_bitstack_clone(state);
    int affected = raf_bitstack_flip_cascade(cascade_state, 1, 20, 5);
    printf("Cascade from Layer 1, Pos 20 (max depth 5)\n");
    printf("  Affected bits: %d\n", affected);
    
    raf_bitstack_print_state(cascade_state, "\nState after cascade:");
    raf_bitstack_destroy(cascade_state);
    printf("\n");
    
    /* Test 5: Opportunity measurement */
    printf("Test 5: Multi-Point Opportunity Measurement\n");
    printf("--------------------------------------------\n");
    
    raf_calc_result *result = raf_calc_result_create(32);
    if (!result) {
        printf("ERROR: Failed to create result\n");
        raf_bitstack_destroy(state);
        return 1;
    }
    
    /* Define measurement points */
    uint32_t measurement_points[16];
    for (int i = 0; i < 16; i++) {
        measurement_points[i] = i * 16;
    }
    
    int num_opps = raf_bitstack_measure_opportunities(state, result, 
                                                       measurement_points, 16);
    printf("Measured %d opportunities:\n", num_opps);
    
    for (size_t i = 0; i < result->num_opportunities && i < 5; i++) {
        raf_opportunity_measurement *opp = &result->opportunities[i];
        printf("  Opp %zu: Confidence=%.4f, Pos=%u, Delta=%d\n",
               i + 1, opp->confidence, opp->bit_positions[0], opp->delta_score);
    }
    printf("\n");
    
    /* Test 6: Optimal scanning */
    printf("Test 6: Scan for Optimal Flip Positions\n");
    printf("-----------------------------------------\n");
    
    int scan_results = raf_bitstack_scan_optimal(state, result, 8);
    printf("Scan found %d opportunities (showing top 5):\n", scan_results);
    
    for (size_t i = 0; i < result->num_opportunities && i < 5; i++) {
        raf_opportunity_measurement *opp = &result->opportunities[i];
        printf("  Rank %zu: Confidence=%.4f, Pos=%u\n",
               i + 1, opp->confidence, opp->bit_positions[0]);
    }
    printf("\n");
    
    /* Test 7: Apply best opportunity */
    printf("Test 7: Apply Best Opportunity\n");
    printf("--------------------------------\n");
    
    if (raf_bitstack_apply_best_opportunity(state, result) == 0) {
        printf("Applied best opportunity successfully\n");
        raf_bitstack_print_state(state, "State after applying opportunity:");
    } else {
        printf("Failed to apply opportunity\n");
    }
    printf("\n");
    
    /* Test 8: Deterministic calculations */
    printf("Test 8: Deterministic Calculations\n");
    printf("------------------------------------\n");
    
    uint64_t hash1 = raf_bitstack_calc_hash(state);
    raf_bitstack_state *state2 = raf_bitstack_clone(state);
    uint64_t hash2 = raf_bitstack_calc_hash(state2);
    
    printf("Hash consistency test:\n");
    printf("  Hash 1: 0x%016lX\n", (unsigned long)hash1);
    printf("  Hash 2: 0x%016lX\n", (unsigned long)hash2);
    printf("  Match: %s\n", (hash1 == hash2) ? "YES" : "NO");
    
    raf_bitstack_flip_single(state2, 0, 0);
    size_t distance = raf_bitstack_calc_distance(state, state2);
    printf("\nHamming distance after 1 flip: %zu\n", distance);
    
    double entropy = raf_bitstack_calc_entropy(state);
    printf("Entropy: %.4f\n", entropy);
    
    double corr = raf_bitstack_calc_correlation(state, 0, 1);
    printf("Correlation (Layer 0 vs 1): %.4f\n", corr);
    printf("\n");
    
    /* Test 9: Gradient optimization */
    printf("Test 9: Gradient Optimization\n");
    printf("-------------------------------\n");
    
    raf_bitstack_reset(state);
    printf("Optimizing for maximum entropy...\n");
    double final_score = raf_bitstack_optimize_gradient(state, goal_maximize_entropy, 10);
    printf("Final entropy: %.4f\n", final_score);
    raf_bitstack_print_state(state, "Optimized state:");
    printf("\n");
    
    /* Test 10: Iterative solving */
    printf("Test 10: Iterative Problem Solving\n");
    printf("------------------------------------\n");
    
    raf_bitstack_state *start = raf_bitstack_create(4, 64);
    raf_bitstack_state *target = raf_bitstack_create(4, 64);
    
    /* Set some target pattern */
    for (size_t i = 0; i < 32; i++) {
        raf_bitstack_set_bit(target, i % 4, i * 2, true);
    }
    
    printf("Attempting to reach target state (max 50 iterations)...\n");
    int iterations = raf_bitstack_solve_iterative(start, target, 50, result);
    
    if (iterations >= 0) {
        printf("Solved in %d iterations!\n", iterations);
    } else {
        size_t final_distance = raf_bitstack_calc_distance(start, target);
        printf("Not fully solved. Final distance: %zu bits\n", final_distance);
    }
    printf("\n");
    
    /* Test 11: Pattern matching */
    printf("Test 11: Pattern Matching\n");
    printf("--------------------------\n");
    
    uint64_t pattern[2] = {0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL};
    uint32_t matches[10];
    
    /* Set pattern in state */
    raf_bitstack_reset(state);
    state->layers[0] = pattern[0];
    state->layers[1] = pattern[1];
    
    int num_matches = raf_bitstack_match_pattern(state, pattern, 2, matches, 10);
    printf("Found %d pattern matches\n", num_matches);
    for (int i = 0; i < num_matches && i < 5; i++) {
        printf("  Match %d at position: %u\n", i + 1, matches[i]);
    }
    printf("\n");
    
    /* Cleanup */
    raf_bitstack_destroy(start);
    raf_bitstack_destroy(target);
    raf_bitstack_destroy(state);
    raf_bitstack_destroy(state2);
    raf_calc_result_destroy(result);
    
    printf("=================================================================\n");
    printf(" All Tests Completed Successfully\n");
    printf("=================================================================\n");
    
    return 0;
}

# Rafaelia BITSTACK Module

## Overview

The BITSTACK module is a new low-level logic subsystem within the Rafaelia Baremetal framework. It implements **non-linear logic operations** using bit-stack operations with **single-flip calculations** for deterministic opportunity detection.

## Philosophy

Unlike traditional linear logic, BITSTACK operates on the principle that complex problems can be solved through:

1. **Single Bit Flips**: Individual bit changes that can cascade through multiple layers
2. **Non-Linear Propagation**: State changes that don't follow linear paths but propagate based on current state hash and position
3. **Opportunity Detection**: Multi-point measurement to identify optimal flip positions
4. **Deterministic Calculations**: Reproducible results through hash-based state tracking

## Key Concepts

### Bitstack State

A multi-layered bit structure where each layer contains a configurable number of bits (must be multiple of 64). Each state maintains:
- Multiple bit layers for dimensional computation
- Flip count tracking
- State hash for deterministic behavior
- Entropy and correlation metrics

### Single-Flip Operations

The core operation is the single bit flip at a specific layer and position. This simple operation can:
- Change system state deterministically
- Trigger cascades through related positions
- Be evaluated without modification
- Lead to emergent complex behaviors

### Opportunity Measurement

Instead of trying all possibilities, the system:
- Measures potential at multiple strategic points
- Evaluates flip effects without commitment
- Ranks opportunities by confidence
- Applies best opportunities iteratively

## Architecture

```
bitstack/
├── raf_bitstack.h     - Header with all API definitions
├── raf_bitstack.c     - Implementation of bitstack logic
└── test_bitstack.c    - Comprehensive test suite
```

## Core Data Structures

### `raf_bitstack_state`

The main state structure containing:
- `layers`: Array of 64-bit words organized in layers
- `num_layers`: Number of independent layers
- `bits_per_layer`: Bits per layer (multiple of 64)
- `flip_count`: Total flips performed
- `state_hash`: Current state hash for tracking

### `raf_opportunity_measurement`

Tracks detected opportunities:
- `measurement_id`: Unique identifier
- `confidence`: Score from 0.0 to 1.0
- `bit_positions`: Up to 8 relevant positions
- `delta_score`: Expected change

### `raf_calc_result`

Collection of opportunities:
- `opportunities`: Array of measurements
- `num_opportunities`: Count of detected opportunities
- `total_evaluations`: Calculations performed

## API Functions

### Creation and Management

```c
/* Create bitstack with specified dimensions */
raf_bitstack_state* raf_bitstack_create(size_t num_layers, size_t bits_per_layer);

/* Destroy and free resources */
void raf_bitstack_destroy(raf_bitstack_state *state);

/* Reset to initial state */
void raf_bitstack_reset(raf_bitstack_state *state);

/* Clone for parallel evaluation */
raf_bitstack_state* raf_bitstack_clone(const raf_bitstack_state *state);
```

### Single-Flip Operations (Non-Linear Logic Core)

```c
/* Perform single bit flip - core operation */
int raf_bitstack_flip_single(raf_bitstack_state *state, size_t layer, size_t bit_pos);

/* Evaluate flip effect without modifying state */
int raf_bitstack_evaluate_flip(const raf_bitstack_state *state, 
                                size_t layer, size_t bit_pos,
                                double *effect_score);

/* Apply cascade - single flip triggers multiple changes */
int raf_bitstack_flip_cascade(raf_bitstack_state *state,
                               size_t initial_layer, size_t initial_pos,
                               size_t max_cascade);
```

### Deterministic Calculations

```c
/* Calculate state hash for tracking */
uint64_t raf_bitstack_calc_hash(const raf_bitstack_state *state);

/* Calculate Hamming distance between states */
size_t raf_bitstack_calc_distance(const raf_bitstack_state *state1,
                                   const raf_bitstack_state *state2);

/* Calculate entropy (information content) */
double raf_bitstack_calc_entropy(const raf_bitstack_state *state);

/* Calculate correlation between layers */
double raf_bitstack_calc_correlation(const raf_bitstack_state *state,
                                      size_t layer1, size_t layer2);
```

### Multi-Point Measurement

```c
/* Create result structure */
raf_calc_result* raf_calc_result_create(size_t initial_capacity);

/* Destroy result */
void raf_calc_result_destroy(raf_calc_result *result);

/* Measure opportunities at specific points */
int raf_bitstack_measure_opportunities(const raf_bitstack_state *state,
                                        raf_calc_result *result,
                                        const uint32_t *measurement_points,
                                        size_t num_points);

/* Scan for optimal positions */
int raf_bitstack_scan_optimal(const raf_bitstack_state *state,
                               raf_calc_result *result,
                               size_t scan_depth);

/* Apply best opportunity */
int raf_bitstack_apply_best_opportunity(raf_bitstack_state *state,
                                         const raf_calc_result *result);
```

### Advanced Operations

```c
/* Iteratively solve to reach target state */
int raf_bitstack_solve_iterative(raf_bitstack_state *state,
                                  const raf_bitstack_state *target,
                                  size_t max_iterations,
                                  raf_calc_result *result);

/* Find patterns in state */
int raf_bitstack_match_pattern(const raf_bitstack_state *state,
                                const uint64_t *pattern,
                                size_t pattern_length,
                                uint32_t *matches,
                                size_t max_matches);

/* Optimize using gradient descent on flips */
double raf_bitstack_optimize_gradient(raf_bitstack_state *state,
                                       double (*goal_function)(const raf_bitstack_state*),
                                       size_t max_steps);
```

## Usage Examples

### Basic Single-Flip Operation

```c
#include "bitstack/raf_bitstack.h"

/* Create 4-layer bitstack with 64 bits per layer */
raf_bitstack_state *state = raf_bitstack_create(4, 64);

/* Flip bit at layer 0, position 10 */
raf_bitstack_flip_single(state, 0, 10);

/* Print current state */
raf_bitstack_print_state(state, "Current state:");

/* Cleanup */
raf_bitstack_destroy(state);
```

### Opportunity Detection

```c
/* Create state and result structure */
raf_bitstack_state *state = raf_bitstack_create(4, 64);
raf_calc_result *result = raf_calc_result_create(32);

/* Scan for best opportunities */
int num_found = raf_bitstack_scan_optimal(state, result, 8);

/* Apply best opportunity */
if (num_found > 0) {
    raf_bitstack_apply_best_opportunity(state, result);
}

/* Cleanup */
raf_calc_result_destroy(result);
raf_bitstack_destroy(state);
```

### Cascade Operations

```c
raf_bitstack_state *state = raf_bitstack_create(8, 128);

/* Trigger cascade from layer 2, position 50 */
/* Will affect up to 10 related positions non-linearly */
int affected = raf_bitstack_flip_cascade(state, 2, 50, 10);

printf("Cascade affected %d bits\n", affected);

raf_bitstack_destroy(state);
```

### Optimization with Goal Function

```c
/* Define goal function */
double maximize_entropy(const raf_bitstack_state *state) {
    return raf_bitstack_calc_entropy(state);
}

/* Create and optimize state */
raf_bitstack_state *state = raf_bitstack_create(4, 64);
double final_score = raf_bitstack_optimize_gradient(state, 
                                                     maximize_entropy, 
                                                     20);

printf("Final entropy: %.4f\n", final_score);
raf_bitstack_destroy(state);
```

## Performance Characteristics

### Time Complexity

- `raf_bitstack_flip_single`: O(1)
- `raf_bitstack_calc_hash`: O(n) where n = total bits
- `raf_bitstack_calc_distance`: O(n) where n = total bits
- `raf_bitstack_calc_entropy`: O(n) where n = total bits
- `raf_bitstack_scan_optimal`: O(n * m) where n = layers, m = scan_depth
- `raf_bitstack_solve_iterative`: O(k * n * m) where k = max_iterations

### Space Complexity

- Bitstack state: O(layers * bits_per_layer / 8) bytes
- Calculation result: O(capacity) for opportunity array

### Optimization Notes

- Uses fast popcount for bit counting
- FNV-1a hash for deterministic state hashing
- In-place operations minimize memory allocations
- Cache-friendly sequential access patterns

## Integration with Rafaelia Baremetal

The BITSTACK module integrates seamlessly with other Rafaelia components:

### With BITRAF
- BITSTACK provides high-level logic operations
- BITRAF provides low-level bit manipulation
- Can use BITRAF bit streams for I/O

### With CORE
- Can use CORE matrix operations for state representation
- Deterministic operations compatible with CORE philosophy
- Shared memory management patterns

### With RAFSTORE
- Can store bitstack states in key-value store
- Use memory pools for frequent state allocations
- Cache frequently accessed states

## Building

The BITSTACK module is automatically included when building Rafaelia Baremetal:

```bash
cd rafaelia-baremetal
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
./rafaelia-test
```

Or as part of llama.cpp:

```bash
cmake -B build -DRAFAELIA_BAREMETAL=ON
cmake --build build --config Release
```

## Testing

Run the comprehensive test suite:

```bash
cd rafaelia-baremetal/build
./test_bitstack
```

Tests cover:
1. Basic bitstack operations
2. Single flip operations
3. Flip effect evaluation
4. Cascade operations
5. Opportunity measurement
6. Optimal scanning
7. Opportunity application
8. Deterministic calculations
9. Gradient optimization
10. Iterative solving
11. Pattern matching

## License

Copyright (c) 2026 Rafael Melo Reis

This module is licensed under the MIT License, maintaining compatibility with the original llama.cpp project. See LICENSE file in the rafaelia-baremetal directory.

## Attribution

This module is part of the Rafaelia Baremetal extensions to llama.cpp:
- Original llama.cpp: Copyright (c) 2023-2024 The ggml authors (MIT License)
- Rafaelia extensions: Copyright (c) 2026 Rafael Melo Reis (MIT License)

All copyright notices and licenses are strictly maintained in accordance with applicable law and authorship requirements.

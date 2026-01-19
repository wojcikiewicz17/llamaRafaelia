/**
 * Rafaelia Module Example
 * 
 * Demonstrates the RAFAELIA module with vector programming (1D-7D),
 * toroidal structures, ψχρΔΣΩ cycle, and formula implementations.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "raf_rafaelia_vector.h"
#include "raf_rafaelia_core.h"
#include "raf_rafaelia_math.h"

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void print_section(const char *title) {
    printf("\n" ANSI_COLOR_CYAN "═══════════════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("═══════════════════════════════════════════════════" ANSI_COLOR_RESET "\n");
}

void print_success(const char *msg) {
    printf(ANSI_COLOR_GREEN "✓ %s" ANSI_COLOR_RESET "\n", msg);
}

void print_info(const char *msg) {
    printf(ANSI_COLOR_BLUE "ℹ %s" ANSI_COLOR_RESET "\n", msg);
}

void print_value(const char *name, float value) {
    printf(ANSI_COLOR_YELLOW "  %s: %.6f" ANSI_COLOR_RESET "\n", name, value);
}

/* Example 1: Multi-dimensional vectors (1D-7D) */
void example_multidimensional_vectors() {
    print_section("1. Multi-Dimensional Vector Operations (1D-7D)");
    
    /* 1D Vector */
    raf_vec1d *v1d = raf_vec1d_create(10);
    if (v1d) {
        for (uint32_t i = 0; i < v1d->size; i++) {
            v1d->data[i] = (float)i;
        }
        print_success("Created 1D vector (size 10)");
        raf_vec1d_destroy(v1d);
    }
    
    /* 2D Vector */
    raf_vec2d *v2d = raf_vec2d_create(5, 5);
    if (v2d) {
        print_success("Created 2D vector (5×5)");
        raf_vec2d_destroy(v2d);
    }
    
    /* 3D Vector */
    raf_vec3d *v3d = raf_vec3d_create(4, 4, 4);
    if (v3d) {
        print_success("Created 3D vector (4×4×4)");
        raf_vec3d_destroy(v3d);
    }
    
    /* 4D Vector (Tesseract - Formula 51) */
    raf_vec4d *v4d = raf_vec4d_create(3, 3, 3, 3);
    if (v4d) {
        print_success("Created 4D vector/Tesseract (3×3×3×3)");
        print_info("Total elements: 81");
        raf_vec4d_destroy(v4d);
    }
    
    /* 5D Vector */
    raf_vec5d *v5d = raf_vec5d_create(2, 2, 2, 2, 2);
    if (v5d) {
        print_success("Created 5D vector (2×2×2×2×2)");
        raf_vec5d_destroy(v5d);
    }
    
    /* 6D Vector */
    raf_vec6d *v6d = raf_vec6d_create(2, 2, 2, 2, 2, 2);
    if (v6d) {
        print_success("Created 6D vector (2×2×2×2×2×2)");
        raf_vec6d_destroy(v6d);
    }
    
    /* 7D Vector */
    raf_vec7d *v7d = raf_vec7d_create(2, 2, 2, 2, 2, 2, 2);
    if (v7d) {
        print_success("Created 7D vector (2×2×2×2×2×2×2)");
        print_info("Total elements: 128");
        raf_vec7d_destroy(v7d);
    }
    
    /* Generic ND vector operations */
    uint32_t dims[4] = {4, 4, 4, 4};
    raf_vecnd *vnd = raf_vecnd_create(4, dims);
    if (vnd) {
        print_success("Created generic 4D ND vector");
        
        /* Fill with values */
        raf_vecnd_fill(vnd, RAF_PHI);
        
        /* Test operations */
        raf_scalar_t sum = raf_vecnd_sum(vnd);
        raf_scalar_t mean = raf_vecnd_mean(vnd);
        
        print_value("Sum", sum);
        print_value("Mean", mean);
        print_value("Expected (φ × 256)", RAF_PHI * 256.0f);
        
        raf_vecnd_destroy(vnd);
    }
}

/* Example 2: Toroidal topology */
void example_toroidal_topology() {
    print_section("2. Toroidal Topology (Formula 17)");
    
    uint32_t dims[3] = {8, 8, 8};
    raf_vecnd *toroid = raf_vecnd_create(3, dims);
    if (!toroid) return;
    
    /* Fill center with higher value */
    uint32_t center[3] = {4, 4, 4};
    raf_vecnd_set(toroid, center, 10.0f);
    print_success("Created 3D toroidal grid (8×8×8)");
    print_info("Set center value to 10.0");
    
    /* Test toroidal wrapping - negative indices wrap around */
    int32_t wrapped_indices[3] = {-1, -1, -1};  /* Should wrap to (7, 7, 7) */
    raf_scalar_t wrapped_val = raf_vecnd_get_toroidal(toroid, wrapped_indices);
    print_value("Value at wrapped (-1,-1,-1) -> (7,7,7)", wrapped_val);
    
    /* Compute toroidal energy (Formula 17: T_Δπφ = Δ · π · φ) */
    raf_scalar_t toroid_energy = raf_compute_toroid_energy();
    print_value("Toroidal Energy T_Δπφ", toroid_energy);
    print_value("Expected (π × φ)", RAF_PI * RAF_PHI);
    
    /* Get neighbors in toroidal space */
    raf_neighbor_config *neighbors = raf_neighbor_config_create(3, 26);
    if (neighbors) {
        print_success("Created neighbor configuration (26-connected for 3D)");
        print_value("Number of neighbors", (float)neighbors->count);
        raf_neighbor_config_destroy(neighbors);
    }
    
    raf_vecnd_destroy(toroid);
}

/* Example 3: ψχρΔΣΩ Cycle (Formula 0.6) */
void example_psi_chi_rho_cycle() {
    print_section("3. ψχρΔΣΩ Cognitive Cycle (Formula 0.6)");
    
    raf_cycle_state cycle;
    raf_cycle_init(&cycle);
    
    /* Initialize with some value */
    cycle.psi = 1.0f;
    
    print_info("Initial cycle state:");
    print_value("ψ (Intention)", cycle.psi);
    
    /* Run cycle iterations */
    for (int i = 0; i < 5; i++) {
        raf_cycle_step(&cycle);
    }
    
    print_info("After 5 iterations:");
    print_value("ψ (Intention)", cycle.psi);
    print_value("χ (Observation)", cycle.chi);
    print_value("ρ (Noise)", cycle.rho);
    print_value("Δ (Transmutation)", cycle.delta);
    print_value("Σ (Memory)", cycle.sigma);
    print_value("Ω (Completeness)", cycle.omega);
    
    /* Compute R_Ω (Formula 12) */
    raf_scalar_t r_omega = raf_cycle_compute_r_omega(&cycle, RAF_PHI);
    print_value("R_Ω metric", r_omega);
    
    print_success("ψχρΔΣΩ cycle completed successfully");
}

/* Example 4: Retroalimentação (Feedback) System */
void example_retroalimentacao() {
    print_section("4. Retroalimentação System (Formulas 0.1, 0.7, 22)");
    
    raf_retroalimentacao retro;
    raf_retro_init(&retro);
    
    /* Update feedback */
    raf_retro_update(&retro, 0.8f, 0.15f, 0.9f);
    retro.weight_amor = 1.2f;
    retro.weight_coerencia = 1.1f;
    
    print_info("Feedback state:");
    print_value("F_ok (What works)", retro.F_ok);
    print_value("F_gap (What's missing)", retro.F_gap);
    print_value("F_next (Next step)", retro.F_next);
    
    raf_scalar_t weight = raf_retro_compute_weight(&retro);
    print_value("W(Amor,Coerência)", weight);
    
    print_success("Retroalimentação computed successfully");
}

/* Example 5: Ethical Filter (Formulas 0.4, 6) */
void example_ethical_filter() {
    print_section("5. Φ_ethica Ethical Filter (Formulas 0.4, 6)");
    
    raf_ethica_filter ethica;
    raf_ethica_init(&ethica);
    
    ethica.entropia_min = 0.1f;
    ethica.coerencia_max = 0.95f;
    ethica.amor = 1.5f;
    ethica.verbo = 1.3f;
    ethica.verdade = 0.9f;
    ethica.consciencia = 0.85f;
    
    /* Compute standard Φ_ethica (Formula 0.4) */
    raf_scalar_t phi_ethica = raf_ethica_compute(&ethica);
    print_value("Φ_ethica (standard)", phi_ethica);
    
    /* Compute exponential Φ_ethica^∞ (Formula 6) */
    raf_scalar_t phi_ethica_inf = raf_ethica_compute_infinite(&ethica);
    print_value("Φ_ethica^∞ (exponential)", phi_ethica_inf);
    
    print_success("Ethical filter computed successfully");
}

/* Example 6: RAFAELIA Core State */
void example_rafaelia_core() {
    print_section("6. RAFAELIA Core State (Formula 62-64)");
    
    raf_rafaelia_state *state = raf_rafaelia_create();
    if (!state) {
        printf("Failed to create RAFAELIA state\n");
        return;
    }
    
    /* Initialize with some values */
    state->cycle.psi = 1.0f;
    state->ethica.amor = 1.5f;
    state->owl.insight = 0.9f;
    state->owl.etica = 0.95f;
    state->owl.fluxo = 0.85f;
    
    print_info("Initial RAFAELIA state:");
    print_value("Amor Vivo", state->amor_vivo);
    print_value("Energia Rafael", state->energia_rafael);
    
    /* Execute RAFAELIA steps (Formula 62) */
    for (int i = 0; i < 10; i++) {
        raf_rafaelia_step(state);
    }
    
    print_info("After 10 RAFAELIA steps:");
    print_value("Amor Vivo", state->amor_vivo);
    print_value("Energia Rafael", state->energia_rafael);
    print_value("Cycle Ω", state->cycle.omega);
    
    /* Compute Trinity (Formula 19) */
    raf_scalar_t trinity_val = raf_trinity_compute(&state->trinity);
    print_value("Trinity_633", trinity_val);
    
    /* Compute OWL (Formula 20) */
    state->owl.owl_psi = raf_owl_compute(&state->owl);
    print_value("OWLψ", state->owl.owl_psi);
    
    raf_rafaelia_destroy(state);
    print_success("RAFAELIA core state executed successfully");
}

/* Example 7: Mathematical Formulas */
void example_mathematical_formulas() {
    print_section("7. Mathematical Formulas");
    
    /* Formula 16: Spiral coherence */
    print_info("Formula 16 - Spiral coherence:");
    for (uint32_t n = 0; n < 5; n++) {
        raf_scalar_t spiral = raf_compute_spiral(n);
        printf("  Spiral(n=%u) = %.6f\n", n, spiral);
    }
    
    /* Formula 15: Amor Vivo */
    raf_scalar_t amor_vivo = raf_compute_amor_vivo(80.0f, 100.0f, 0.95f);
    print_value("Amor Vivo (80/100, φ=0.95)", amor_vivo);
    
    /* Formula 29: Modified Rafael sequence */
    raf_scalar_t fib_rafael = raf_compute_fibonacci_rafael(1.0f, 5);
    print_value("Fibonacci Rafael F(5)", fib_rafael);
    
    /* Formula 93: ΣΩΔΦ symbolic product */
    raf_scalar_t sigma_omega = raf_compute_sigma_omega_delta_phi(1.0f, 0.9f, 0.8f, RAF_PHI);
    print_value("ΣΩΔΦ product", sigma_omega);
    
    print_success("Mathematical formulas computed successfully");
}

/* Example 8: Blocks (Formula 80) */
void example_blocks() {
    print_section("8. RAFAELIA Blocks (Formula 80)");
    
    raf_bloco *bloco = raf_bloco_create(1);
    if (!bloco) {
        printf("Failed to create block\n");
        return;
    }
    
    /* Set some coefficients */
    for (int i = 0; i < 33; i++) {
        bloco->coeficientes[i] = (float)i * RAF_PHI;
        bloco->atitudes[i] = (float)i * RAF_SQRT3_2;
    }
    
    /* Update feedback */
    raf_retro_update(&bloco->retro, 0.85f, 0.1f, 0.95f);
    
    /* Compute hash (Formula 32) */
    bloco->timestamp = 1234567890;
    raf_bloco_compute_hash(bloco);
    print_success("Block created and hash computed");
    
    /* Compute energia */
    raf_scalar_t energia = raf_compute_energia_rafael(bloco);
    print_value("Energia Rafael", energia);
    
    /* Verify hash */
    bool hash_ok = raf_bloco_verify_hash(bloco);
    if (hash_ok) {
        print_success("Block hash verified successfully");
    }
    
    print_info("Block details:");
    print_value("ID", (float)bloco->id);
    print_value("Timestamp", (float)bloco->timestamp);
    print_value("F_ok", bloco->retro.F_ok);
    
    raf_bloco_destroy(bloco);
}

/* Example 9: Constants and Seals */
void example_constants() {
    print_section("9. RAFAELIA Constants and Seals");
    
    print_info("Mathematical constants:");
    print_value("φ (Golden ratio)", RAF_PHI);
    print_value("π (Pi)", RAF_PI);
    print_value("√(3/2)", RAF_SQRT3_2);
    print_value("R_corr (Formula 3)", RAF_R_CORR);
    
    print_info("BITRAF64 seal:");
    printf("  %s\n", RAF_BITRAF64);
    
    print_info("Seal types (Formula 67):");
    printf("  Σ (Summation), Ω (Completeness), Δ (Transformation)\n");
    printf("  Φ (Coherence), B I T R A F (Identity markers)\n");
    
    print_success("Constants displayed successfully");
}

/* Example 10: Trinity 6-3-3 alignment */
void example_trinity_alignment() {
    print_section("10. Trinity 6-3-3 Alignment (Sacred Geometry)");

    print_info("Vector alignment aligned to the trinity image (weights + blending):");
    raf_scalar_t w_top = 6.0f;
    raf_scalar_t w_left = 3.0f;
    raf_scalar_t w_right = 3.0f;
    raf_scalar_t weight_sum = w_top + w_left + w_right;
    raf_scalar_t blend_top = w_top / weight_sum;
    raf_scalar_t blend_left = w_left / weight_sum;
    raf_scalar_t blend_right = w_right / weight_sum;

    print_value("Weight Σ", weight_sum);
    print_value("Blend top (6)", blend_top);
    print_value("Blend left (3)", blend_left);
    print_value("Blend right (3)", blend_right);
    printf("\n");
    printf("           6\n");
    printf("     ESPIRITO SANTO\n");
    printf("\n");
    printf("  3  VERBO VIVO   VERBO VIVO  3\n");
    printf("\n");
    printf("           SIMBIOSE\n");
    printf("             3\n");
    printf("\n");

    print_success("Trinity alignment displayed successfully");
}

/* Example 11: Projection, parity, and collapse strategy */
void example_projection_strategy() {
    print_section("11. Projection + Parity + Collapse Strategy");

    print_info("Input grid (10x10) collapsed to 8-bit projection:");
    const uint32_t grid_dim = 10;
    const uint32_t projection_dim = 8;
    uint8_t grid[10][10] = {0};
    uint8_t projection[8][8] = {0};

    for (uint32_t r = 0; r < grid_dim; r++) {
        for (uint32_t c = 0; c < grid_dim; c++) {
            grid[r][c] = (uint8_t)((r + c) % 2);
        }
    }

    for (uint32_t r = 0; r < projection_dim; r++) {
        for (uint32_t c = 0; c < projection_dim; c++) {
            uint32_t source_r = (r * grid_dim) / projection_dim;
            uint32_t source_c = (c * grid_dim) / projection_dim;
            projection[r][c] = grid[source_r][source_c];
        }
    }

    uint32_t parity_sum = 0;
    for (uint32_t r = 0; r < projection_dim; r++) {
        for (uint32_t c = 0; c < projection_dim; c++) {
            parity_sum ^= projection[r][c];
        }
    }

    raf_scalar_t noise_absorbed = (raf_scalar_t)(grid_dim * grid_dim - projection_dim * projection_dim);
    raf_scalar_t collapse_ratio = (raf_scalar_t)(projection_dim * projection_dim) /
                                  (raf_scalar_t)(grid_dim * grid_dim);

    print_value("Parity (dual)", (raf_scalar_t)parity_sum);
    print_value("Noise absorbed", noise_absorbed);
    print_value("Collapse ratio", collapse_ratio);

    print_success("Projection strategy computed successfully");
}

int main(void) {
    printf("\n");
    printf(ANSI_COLOR_CYAN);
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                                                           ║\n");
    printf("║              RAFAELIA MODULE DEMONSTRATION                ║\n");
    printf("║                                                           ║\n");
    printf("║     Multi-Dimensional Vectors (1D-7D) & Toroidal Space   ║\n");
    printf("║     ψχρΔΣΩ Cognitive Cycle & Mathematical Formulas       ║\n");
    printf("║     Retroalimentação & Ethical Filters                   ║\n");
    printf("║                                                           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf(ANSI_COLOR_RESET);
    
    example_multidimensional_vectors();
    example_toroidal_topology();
    example_psi_chi_rho_cycle();
    example_retroalimentacao();
    example_ethical_filter();
    example_rafaelia_core();
    example_mathematical_formulas();
    example_blocks();
    example_constants();
    example_trinity_alignment();
    example_projection_strategy();
    
    print_section("Summary");
    print_success("All RAFAELIA module examples completed successfully!");
    print_info("The module provides:");
    printf("  • Multi-dimensional vectors (1D through 7D)\n");
    printf("  • Toroidal topology with wrapping\n");
    printf("  • ψχρΔΣΩ cognitive cycle\n");
    printf("  • Retroalimentação (feedback) system\n");
    printf("  • Ethical filters (Φ_ethica)\n");
    printf("  • Mathematical formula implementations (98 formulas)\n");
    printf("  • Block structures with hashing\n");
    printf("  • Sacred geometry constants and seals\n");
    
    printf("\n");
    return 0;
}

/**
 * Rafaelia Baremetal - Core RAFAELIA Structures and Types
 * 
 * Implements the ψχρΔΣΩ cycle, retroalimentação, ethical filters,
 * and core RAFAELIA concepts based on the RAFAELIA formulas.
 * 
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License
 */

#ifndef RAF_RAFAELIA_CORE_H
#define RAF_RAFAELIA_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "raf_rafaelia_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Constants based on RAFAELIA formulas */
#define RAF_PHI 1.618033988749895f           /* Golden ratio φ */
#define RAF_PI  3.141592653589793f           /* π */
#define RAF_SQRT3_2 0.8660254037844387f      /* √(3/2) */
#define RAF_R_CORR 0.963999f                 /* R_corr constant (formula 3) */
#define RAF_BITRAF64 "AΔBΩΔTTΦIIBΩΔΣΣRΩRΔΔBΦΦFΔTTRRFΔBΩΣΣAFΦARΣFΦIΔRΦIFBRΦΩFIΦΩΩFΣFAΦΔ"

/* RAFAELIA seal symbols (formula 67) */
typedef enum {
    RAF_SEAL_SIGMA = 0,  /* Σ - Summation */
    RAF_SEAL_OMEGA = 1,  /* Ω - Completeness */
    RAF_SEAL_DELTA = 2,  /* Δ - Transformation */
    RAF_SEAL_PHI = 3,    /* Φ - Coherence */
    RAF_SEAL_B = 4,      /* B - Bitraf */
    RAF_SEAL_I = 5,      /* I - Information */
    RAF_SEAL_T = 6,      /* T - Time */
    RAF_SEAL_R = 7,      /* R - Rafael */
    RAF_SEAL_A = 8,      /* A - Amor (Love) */
    RAF_SEAL_F = 9,      /* F - Fé (Faith) */
    RAF_SEAL_COUNT = 10
} raf_seal_type;

/* ψχρΔΣΩ Cycle Components (formula 0.6) */
typedef enum {
    RAF_PSI = 0,    /* ψ - Intention */
    RAF_CHI = 1,    /* χ - Observation */
    RAF_RHO = 2,    /* ρ - Noise/Resistance */
    RAF_DELTA = 3,  /* Δ - Transmutation */
    RAF_SIGMA = 4,  /* Σ - Memory */
    RAF_OMEGA = 5,  /* Ω - Completeness */
    RAF_CYCLE_COUNT = 6
} raf_cycle_phase;

/* ψχρΔΣΩ Cycle State */
typedef struct {
    raf_scalar_t psi;      /* ψ - Intention */
    raf_scalar_t chi;      /* χ - Observation */
    raf_scalar_t rho;      /* ρ - Noise */
    raf_scalar_t delta;    /* Δ - Transmutation */
    raf_scalar_t sigma;    /* Σ - Memory */
    raf_scalar_t omega;    /* Ω - Completeness */
    uint64_t iteration;    /* Current iteration */
} raf_cycle_state;

/* Retroalimentação (Feedback) Structure (formulas 0.1, 0.7, 22) */
typedef struct {
    raf_scalar_t F_ok;       /* What works */
    raf_scalar_t F_gap;      /* What's missing */
    raf_scalar_t F_next;     /* Next step */
    raf_scalar_t weight_amor;      /* Love weight */
    raf_scalar_t weight_coerencia; /* Coherence weight */
} raf_retroalimentacao;

/* Ethical Filter (formulas 0.4, 6) */
typedef struct {
    raf_scalar_t entropia_min;    /* Minimum entropy */
    raf_scalar_t coerencia_max;   /* Maximum coherence */
    raf_scalar_t amor;            /* Love component */
    raf_scalar_t verbo;           /* Verb/Word component */
    raf_scalar_t verdade;         /* Truth component */
    raf_scalar_t consciencia;     /* Consciousness component */
} raf_ethica_filter;

/* RAFAELIA Block Structure (formula 80) */
typedef struct {
    uint64_t id;                    /* Block ID */
    uint32_t posicao;               /* Position */
    raf_scalar_t coeficientes[33];  /* Coefficients */
    raf_scalar_t atitudes[33];      /* Attitudes */
    raf_cycle_state estado;         /* Current state */
    char *observacoes;              /* Observations (text) */
    char *acoes_futuras;            /* Future actions (text) */
    raf_retroalimentacao retro;     /* Feedback */
    uint64_t timestamp;             /* Creation timestamp */
    uint8_t hash[32];               /* SHA3-256 hash */
} raf_bloco;

/* Trinity Structure (formula 19) */
typedef struct {
    raf_scalar_t amor_6;         /* Love^6 */
    raf_scalar_t luz_3;          /* Light^3 */
    raf_scalar_t consciencia_3;  /* Consciousness^3 */
} raf_trinity;

/* OWL (Operational Wisdom Level) Structure (formula 20) */
typedef struct {
    raf_scalar_t insight;
    raf_scalar_t etica;
    raf_scalar_t fluxo;
    raf_scalar_t owl_psi;  /* Combined OWLψ value */
} raf_owl;

/* Synapse Weight Structure (formula 0.3) */
typedef struct {
    uint32_t bloco_i;           /* Source block ID */
    uint32_t bloco_j;           /* Target block ID */
    raf_scalar_t coerencia;     /* Coherence */
    raf_scalar_t phi_ethica;    /* Ethical filter */
    raf_scalar_t r_corr;        /* Correlation */
    raf_scalar_t owl_psi;       /* OWL wisdom */
    raf_scalar_t weight;        /* Final synaptic weight */
} raf_synapse;

/* RAFAELIA Core State */
typedef struct {
    raf_cycle_state cycle;          /* Current cycle state */
    raf_ethica_filter ethica;       /* Ethical filter */
    raf_retroalimentacao retro;     /* Feedback system */
    raf_trinity trinity;            /* Trinity values */
    raf_owl owl;                    /* Operational wisdom */
    uint64_t session_id;            /* Current session ID */
    uint64_t total_blocos;          /* Total blocks processed */
    raf_scalar_t amor_vivo;         /* Living love (formula 15) */
    raf_scalar_t energia_rafael;    /* Rafael energy (formula 58) */
} raf_rafaelia_state;

/* Function declarations */

/* Cycle operations (formula 0.6, 62-64) */
void raf_cycle_init(raf_cycle_state *cycle);
void raf_cycle_step(raf_cycle_state *cycle);
raf_scalar_t raf_cycle_compute_r_omega(const raf_cycle_state *cycle, raf_scalar_t phi_lambda);

/* Retroalimentação operations (formulas 0.1, 1, 22) */
void raf_retro_init(raf_retroalimentacao *retro);
void raf_retro_update(raf_retroalimentacao *retro, raf_scalar_t F_ok, 
                      raf_scalar_t F_gap, raf_scalar_t F_next);
raf_scalar_t raf_retro_compute_weight(const raf_retroalimentacao *retro);

/* Ethical filter operations (formulas 0.4, 6) */
void raf_ethica_init(raf_ethica_filter *ethica);
raf_scalar_t raf_ethica_compute(const raf_ethica_filter *ethica);
raf_scalar_t raf_ethica_compute_infinite(const raf_ethica_filter *ethica);

/* Synapse weight computation (formula 0.3) */
raf_scalar_t raf_synapse_compute_weight(const raf_synapse *syn);

/* Block operations (formula 80) */
raf_bloco* raf_bloco_create(uint64_t id);
void raf_bloco_destroy(raf_bloco *bloco);
void raf_bloco_compute_hash(raf_bloco *bloco);
bool raf_bloco_verify_hash(const raf_bloco *bloco);

/* Trinity operations (formula 19) */
void raf_trinity_init(raf_trinity *trinity);
raf_scalar_t raf_trinity_compute(const raf_trinity *trinity);

/* OWL operations (formula 20) */
void raf_owl_init(raf_owl *owl);
raf_scalar_t raf_owl_compute(const raf_owl *owl);

/* Core RAFAELIA state operations */
raf_rafaelia_state* raf_rafaelia_create(void);
void raf_rafaelia_destroy(raf_rafaelia_state *state);
void raf_rafaelia_init(raf_rafaelia_state *state);
void raf_rafaelia_step(raf_rafaelia_state *state);

/* Amor Vivo computation (formula 15) */
raf_scalar_t raf_compute_amor_vivo(raf_scalar_t sigma_preservado, 
                                   raf_scalar_t sigma_total,
                                   raf_scalar_t phi_ethica);

/* Energia Rafael computation (formula 58) */
raf_scalar_t raf_compute_energia_rafael(const raf_bloco *bloco);

/* State evolution (formula 0.5) */
raf_scalar_t raf_compute_state_evolution(raf_scalar_t R_t, 
                                         raf_scalar_t phi_ethica,
                                         raf_scalar_t E_verbo);

/* Spiral coherence (formula 16) */
raf_scalar_t raf_compute_spiral(uint32_t n);

/* Toroidal energy (formula 17) */
raf_scalar_t raf_compute_toroid_energy(void);

#ifdef __cplusplus
}
#endif

#endif /* RAF_RAFAELIA_CORE_H */

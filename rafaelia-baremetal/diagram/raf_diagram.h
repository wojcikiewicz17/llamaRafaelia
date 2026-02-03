/**
 * Rafaelia Baremetal - Diagram Low-Level Primitives
 *
 * Implements low-level constructs inspired by Rafaelia diagrams:
 * Zang Zhi Jie, Hun Mai Fu, Guang He Yin, Dao Xin Wang,
 * Lv Bian Jing, Sheng Huo Ma, Yuan Jie Ti, He Rong Huan.
 *
 * No external dependencies beyond the C standard library.
 *
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License
 */

#ifndef RAF_DIAGRAM_H
#define RAF_DIAGRAM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAF_DAO_XIN_WANG_MAX_NODES 8

/* 藏智界 (Zang Zhi Jie) - Knowledge vault with stride updates */
typedef struct {
    uint8_t *data;
    size_t size;
    uint32_t stride;
    uint64_t checksum;
} raf_zang_zhi_jie;

void raf_zang_zhi_jie_init(raf_zang_zhi_jie *vault, uint8_t *buffer,
                           size_t size, uint32_t stride);
size_t raf_zang_zhi_jie_write(raf_zang_zhi_jie *vault, size_t offset,
                              const uint8_t *src, size_t len);
uint64_t raf_zang_zhi_jie_checksum(const raf_zang_zhi_jie *vault);

/* 魂脈符 (Hun Mai Fu) - Pulse window tracker */
typedef struct {
    uint16_t *window;
    size_t window_size;
    size_t index;
    uint32_t pulse_count;
    uint16_t last_pulse;
} raf_hun_mai_fu;

void raf_hun_mai_fu_init(raf_hun_mai_fu *pulse, uint16_t *window, size_t window_size);
uint16_t raf_hun_mai_fu_push(raf_hun_mai_fu *pulse, uint16_t value);
uint16_t raf_hun_mai_fu_average(const raf_hun_mai_fu *pulse);

/* 光核印 (Guang He Yin) - Core seal state */
typedef struct {
    uint32_t core;
    uint32_t radiance;
    uint32_t coherence;
    uint32_t decay;
} raf_guang_he_yin;

void raf_guang_he_yin_init(raf_guang_he_yin *seal, uint32_t core_seed);
void raf_guang_he_yin_tick(raf_guang_he_yin *seal, uint32_t input);
uint32_t raf_guang_he_yin_emit(const raf_guang_he_yin *seal);

/* 道心網 (Dao Xin Wang) - Fixed-size link matrix */
typedef struct {
    uint64_t links;
    uint8_t node_count;
} raf_dao_xin_wang;

void raf_dao_xin_wang_init(raf_dao_xin_wang *net, uint8_t node_count);
bool raf_dao_xin_wang_link(raf_dao_xin_wang *net, uint8_t from, uint8_t to);
bool raf_dao_xin_wang_unlink(raf_dao_xin_wang *net, uint8_t from, uint8_t to);
bool raf_dao_xin_wang_has_link(const raf_dao_xin_wang *net, uint8_t from, uint8_t to);
uint8_t raf_dao_xin_wang_degree(const raf_dao_xin_wang *net, uint8_t node);

/* 律編經 (Lv Bian Jing) - Rule codex */
typedef struct {
    uint32_t allow_mask;
    uint32_t deny_mask;
    uint32_t weight;
} raf_lv_bian_rule;

typedef struct {
    raf_lv_bian_rule *rules;
    size_t rule_count;
    uint32_t checksum;
} raf_lv_bian_jing;

void raf_lv_bian_jing_init(raf_lv_bian_jing *codex, raf_lv_bian_rule *rules,
                           size_t rule_count);
uint32_t raf_lv_bian_jing_score(const raf_lv_bian_jing *codex, uint32_t signal);
uint32_t raf_lv_bian_jing_checksum(const raf_lv_bian_jing *codex);

/* 聖火碼 (Sheng Huo Ma) - Fire code generator */
typedef struct {
    uint64_t state;
    uint64_t increment;
} raf_sheng_huo_ma;

void raf_sheng_huo_ma_init(raf_sheng_huo_ma *fire, uint64_t seed, uint64_t stream);
uint32_t raf_sheng_huo_ma_next(raf_sheng_huo_ma *fire);
uint32_t raf_sheng_huo_ma_firecode(raf_sheng_huo_ma *fire, uint32_t salt);

/* 源界體 (Yuan Jie Ti) - Body with modular cycles */
typedef struct {
    int32_t axis[3];
    uint32_t cycles[4];
} raf_yuan_jie_ti;

void raf_modular_cycles(uint32_t tick, uint32_t *out_cycles);
void raf_yuan_jie_ti_init(raf_yuan_jie_ti *body, int32_t x, int32_t y, int32_t z);
void raf_yuan_jie_ti_step(raf_yuan_jie_ti *body, int32_t dx, int32_t dy, int32_t dz,
                          uint32_t tick);

/* 和融環 (He Rong Huan) - Harmony ring buffer */
typedef struct {
    uint32_t *ring;
    size_t ring_size;
    size_t head;
    uint64_t sum;
} raf_he_rong_huan;

void raf_he_rong_huan_init(raf_he_rong_huan *ring, uint32_t *buffer, size_t ring_size);
uint32_t raf_he_rong_huan_push(raf_he_rong_huan *ring, uint32_t value);
uint32_t raf_he_rong_huan_mean(const raf_he_rong_huan *ring);

#ifdef __cplusplus
}
#endif

#endif /* RAF_DIAGRAM_H */

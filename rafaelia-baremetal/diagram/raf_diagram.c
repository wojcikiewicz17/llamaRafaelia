/**
 * Rafaelia Baremetal - Diagram Low-Level Primitives
 *
 * Deterministic, dependency-free primitives for diagram-inspired constructs.
 *
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License
 */

#include "raf_diagram.h"

static uint64_t raf_diagram_hash64(const uint8_t *data, size_t len) {
    uint64_t hash = 1469598103934665603ULL;
    for (size_t i = 0; i < len; ++i) {
        hash ^= (uint64_t)data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

void raf_zang_zhi_jie_init(raf_zang_zhi_jie *vault, uint8_t *buffer,
                           size_t size, uint32_t stride) {
    if (!vault) {
        return;
    }
    vault->data = buffer;
    vault->size = size;
    vault->stride = (stride == 0) ? 1U : stride;
    vault->checksum = 0;
}

size_t raf_zang_zhi_jie_write(raf_zang_zhi_jie *vault, size_t offset,
                              const uint8_t *src, size_t len) {
    if (!vault || !vault->data || !src || vault->size == 0) {
        return 0;
    }
    size_t written = 0;
    size_t idx = offset % vault->size;
    for (size_t i = 0; i < len; ++i) {
        vault->data[idx] = src[i];
        idx = (idx + vault->stride) % vault->size;
        written++;
    }
    vault->checksum = raf_diagram_hash64(vault->data, vault->size);
    return written;
}

uint64_t raf_zang_zhi_jie_checksum(const raf_zang_zhi_jie *vault) {
    if (!vault || !vault->data || vault->size == 0) {
        return 0;
    }
    return raf_diagram_hash64(vault->data, vault->size);
}

void raf_hun_mai_fu_init(raf_hun_mai_fu *pulse, uint16_t *window, size_t window_size) {
    if (!pulse) {
        return;
    }
    pulse->window = window;
    pulse->window_size = window_size;
    pulse->index = 0;
    pulse->pulse_count = 0;
    pulse->last_pulse = 0;
    if (window) {
        for (size_t i = 0; i < window_size; ++i) {
            window[i] = 0;
        }
    }
}

uint16_t raf_hun_mai_fu_push(raf_hun_mai_fu *pulse, uint16_t value) {
    if (!pulse || !pulse->window || pulse->window_size == 0) {
        return value;
    }
    pulse->window[pulse->index] = value;
    pulse->last_pulse = value;
    pulse->index = (pulse->index + 1) % pulse->window_size;
    pulse->pulse_count++;
    return value;
}

uint16_t raf_hun_mai_fu_average(const raf_hun_mai_fu *pulse) {
    if (!pulse || !pulse->window || pulse->window_size == 0) {
        return 0;
    }
    uint32_t sum = 0;
    for (size_t i = 0; i < pulse->window_size; ++i) {
        sum += pulse->window[i];
    }
    return (uint16_t)(sum / pulse->window_size);
}

void raf_guang_he_yin_init(raf_guang_he_yin *seal, uint32_t core_seed) {
    if (!seal) {
        return;
    }
    seal->core = core_seed;
    seal->radiance = core_seed ^ 0x9e3779b9U;
    seal->coherence = core_seed + 0x6a09e667U;
    seal->decay = 1U;
}

void raf_guang_he_yin_tick(raf_guang_he_yin *seal, uint32_t input) {
    if (!seal) {
        return;
    }
    seal->core ^= input + 0x7f4a7c15U + (seal->core << 6) + (seal->core >> 2);
    seal->radiance = (seal->radiance << 5) | (seal->radiance >> 27);
    seal->radiance ^= seal->core + input;
    seal->coherence = seal->coherence + (seal->radiance ^ (input * 31U));
    seal->decay = (seal->decay << 1) | (seal->decay >> 31);
}

uint32_t raf_guang_he_yin_emit(const raf_guang_he_yin *seal) {
    if (!seal) {
        return 0;
    }
    return seal->core ^ seal->radiance ^ (seal->coherence + seal->decay);
}

static bool raf_dao_xin_wang_valid_node(const raf_dao_xin_wang *net, uint8_t node) {
    return net && node < net->node_count && node < RAF_DAO_XIN_WANG_MAX_NODES;
}

static uint8_t raf_dao_xin_wang_index(uint8_t from, uint8_t to) {
    return (uint8_t)(from * RAF_DAO_XIN_WANG_MAX_NODES + to);
}

void raf_dao_xin_wang_init(raf_dao_xin_wang *net, uint8_t node_count) {
    if (!net) {
        return;
    }
    if (node_count == 0) {
        node_count = 1;
    }
    if (node_count > RAF_DAO_XIN_WANG_MAX_NODES) {
        node_count = RAF_DAO_XIN_WANG_MAX_NODES;
    }
    net->links = 0;
    net->node_count = node_count;
}

bool raf_dao_xin_wang_link(raf_dao_xin_wang *net, uint8_t from, uint8_t to) {
    if (!raf_dao_xin_wang_valid_node(net, from) || !raf_dao_xin_wang_valid_node(net, to)) {
        return false;
    }
    uint64_t mask = 1ULL << raf_dao_xin_wang_index(from, to);
    net->links |= mask;
    return true;
}

bool raf_dao_xin_wang_unlink(raf_dao_xin_wang *net, uint8_t from, uint8_t to) {
    if (!raf_dao_xin_wang_valid_node(net, from) || !raf_dao_xin_wang_valid_node(net, to)) {
        return false;
    }
    uint64_t mask = 1ULL << raf_dao_xin_wang_index(from, to);
    net->links &= ~mask;
    return true;
}

bool raf_dao_xin_wang_has_link(const raf_dao_xin_wang *net, uint8_t from, uint8_t to) {
    if (!raf_dao_xin_wang_valid_node(net, from) || !raf_dao_xin_wang_valid_node(net, to)) {
        return false;
    }
    uint64_t mask = 1ULL << raf_dao_xin_wang_index(from, to);
    return (net->links & mask) != 0;
}

uint8_t raf_dao_xin_wang_degree(const raf_dao_xin_wang *net, uint8_t node) {
    if (!raf_dao_xin_wang_valid_node(net, node)) {
        return 0;
    }
    uint8_t degree = 0;
    for (uint8_t to = 0; to < net->node_count; ++to) {
        if (raf_dao_xin_wang_has_link(net, node, to)) {
            degree++;
        }
    }
    return degree;
}

void raf_lv_bian_jing_init(raf_lv_bian_jing *codex, raf_lv_bian_rule *rules,
                           size_t rule_count) {
    if (!codex) {
        return;
    }
    codex->rules = rules;
    codex->rule_count = rule_count;
    codex->checksum = 0;
    if (rules && rule_count > 0) {
        codex->checksum = raf_lv_bian_jing_checksum(codex);
    }
}

uint32_t raf_lv_bian_jing_score(const raf_lv_bian_jing *codex, uint32_t signal) {
    if (!codex || !codex->rules || codex->rule_count == 0) {
        return 0;
    }
    uint32_t score = 0;
    for (size_t i = 0; i < codex->rule_count; ++i) {
        const raf_lv_bian_rule *rule = &codex->rules[i];
        bool allow = (signal & rule->allow_mask) == rule->allow_mask;
        bool deny = (signal & rule->deny_mask) != 0;
        if (allow && !deny) {
            score += rule->weight;
        }
    }
    return score;
}

uint32_t raf_lv_bian_jing_checksum(const raf_lv_bian_jing *codex) {
    if (!codex || !codex->rules || codex->rule_count == 0) {
        return 0;
    }
    uint64_t hash = 1469598103934665603ULL;
    const uint8_t *bytes = (const uint8_t *)codex->rules;
    size_t len = codex->rule_count * sizeof(raf_lv_bian_rule);
    for (size_t i = 0; i < len; ++i) {
        hash ^= (uint64_t)bytes[i];
        hash *= 1099511628211ULL;
    }
    return (uint32_t)(hash ^ (hash >> 32));
}

void raf_sheng_huo_ma_init(raf_sheng_huo_ma *fire, uint64_t seed, uint64_t stream) {
    if (!fire) {
        return;
    }
    fire->state = seed + 0xda3e39cb94b95bdbULL;
    fire->increment = (stream << 1U) | 1U;
}

uint32_t raf_sheng_huo_ma_next(raf_sheng_huo_ma *fire) {
    if (!fire) {
        return 0;
    }
    uint64_t old_state = fire->state;
    fire->state = old_state * 6364136223846793005ULL + fire->increment;
    uint32_t xorshifted = (uint32_t)(((old_state >> 18U) ^ old_state) >> 27U);
    uint32_t rot = (uint32_t)(old_state >> 59U);
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31U));
}

uint32_t raf_sheng_huo_ma_firecode(raf_sheng_huo_ma *fire, uint32_t salt) {
    if (!fire) {
        return 0;
    }
    uint32_t value = raf_sheng_huo_ma_next(fire);
    value ^= salt + 0x9e3779b9U;
    value ^= (value << 13);
    value ^= (value >> 17);
    value ^= (value << 5);
    return value;
}

void raf_modular_cycles(uint32_t tick, uint32_t *out_cycles) {
    if (!out_cycles) {
        return;
    }
    out_cycles[0] = tick % 7U;
    out_cycles[1] = tick % 10U;
    out_cycles[2] = tick % 12U;
    out_cycles[3] = tick % 20U;
}

void raf_yuan_jie_ti_init(raf_yuan_jie_ti *body, int32_t x, int32_t y, int32_t z) {
    if (!body) {
        return;
    }
    body->axis[0] = x;
    body->axis[1] = y;
    body->axis[2] = z;
    raf_modular_cycles(0, body->cycles);
}

void raf_yuan_jie_ti_step(raf_yuan_jie_ti *body, int32_t dx, int32_t dy, int32_t dz,
                          uint32_t tick) {
    if (!body) {
        return;
    }
    body->axis[0] += dx;
    body->axis[1] += dy;
    body->axis[2] += dz;
    raf_modular_cycles(tick, body->cycles);
}

void raf_he_rong_huan_init(raf_he_rong_huan *ring, uint32_t *buffer, size_t ring_size) {
    if (!ring) {
        return;
    }
    ring->ring = buffer;
    ring->ring_size = ring_size;
    ring->head = 0;
    ring->sum = 0;
    if (buffer) {
        for (size_t i = 0; i < ring_size; ++i) {
            buffer[i] = 0;
        }
    }
}

uint32_t raf_he_rong_huan_push(raf_he_rong_huan *ring, uint32_t value) {
    if (!ring || !ring->ring || ring->ring_size == 0) {
        return value;
    }
    ring->sum -= ring->ring[ring->head];
    ring->ring[ring->head] = value;
    ring->sum += value;
    ring->head = (ring->head + 1) % ring->ring_size;
    return value;
}

uint32_t raf_he_rong_huan_mean(const raf_he_rong_huan *ring) {
    if (!ring || !ring->ring || ring->ring_size == 0) {
        return 0;
    }
    return (uint32_t)(ring->sum / ring->ring_size);
}

/*
 * Rafaelia Baremetal - Geometry Nucleus (Low-level)
 * Data-driven operations without external dependencies.
 */

#include "raf_geometry.h"

static uint32_t raf_geo_hash_cell(int32_t x, int32_t y, int32_t z) {
    uint32_t ux = (uint32_t)x;
    uint32_t uy = (uint32_t)y;
    uint32_t uz = (uint32_t)z;
    return (ux * 73856093u) ^ (uy * 19349663u) ^ (uz * 83492791u);
}

static uint32_t raf_geo_index_from_pos(raf_geo_vec3 pos, float cell_size) {
    float step = cell_size > 0.0f ? cell_size : 1.0f;
    int32_t cx = (int32_t)(pos.x / step);
    int32_t cy = (int32_t)(pos.y / step);
    int32_t cz = (int32_t)(pos.z / step);
    return raf_geo_hash_cell(cx, cy, cz);
}

void raf_geo_state_init(raf_geo_state *state) {
    if (!state) {
        return;
    }
    raf_geo_state_reset(state);
}

void raf_geo_state_reset(raf_geo_state *state) {
    uint16_t i;
    if (!state) {
        return;
    }
    state->count = 0;
    for (i = 0; i < RAF_GEO_MAX_PRIMITIVES; ++i) {
        state->primitives[i].type = RAF_GEO_PRIM_NONE;
        state->primitives[i].position.x = 0.0f;
        state->primitives[i].position.y = 0.0f;
        state->primitives[i].position.z = 0.0f;
        state->primitives[i].scale.x = 1.0f;
        state->primitives[i].scale.y = 1.0f;
        state->primitives[i].scale.z = 1.0f;
        state->primitives[i].params[0] = 0.0f;
        state->primitives[i].params[1] = 0.0f;
        state->primitives[i].params[2] = 0.0f;
        state->primitives[i].params[3] = 0.0f;
        state->primitives[i].index_key = 0;
    }
}

uint16_t raf_geo_add_primitive(raf_geo_state *state,
                               raf_geo_primitive_type type,
                               raf_geo_vec3 position,
                               raf_geo_vec3 scale,
                               const float *params,
                               size_t params_len) {
    uint16_t idx;
    size_t i;
    if (!state || state->count >= RAF_GEO_MAX_PRIMITIVES) {
        return UINT16_MAX;
    }
    idx = state->count++;
    state->primitives[idx].type = (uint8_t)type;
    state->primitives[idx].position = position;
    state->primitives[idx].scale = scale;
    for (i = 0; i < 4; ++i) {
        state->primitives[idx].params[i] = (params && i < params_len) ? params[i] : 0.0f;
    }
    state->primitives[idx].index_key = 0;
    return idx;
}

static void raf_geo_apply_translate(raf_geo_primitive *prim, raf_geo_vec3 delta) {
    prim->position.x += delta.x;
    prim->position.y += delta.y;
    prim->position.z += delta.z;
}

static void raf_geo_apply_scale(raf_geo_primitive *prim, raf_geo_vec3 scale) {
    prim->scale.x *= scale.x;
    prim->scale.y *= scale.y;
    prim->scale.z *= scale.z;
}

static void raf_geo_apply_blend(raf_geo_primitive *a, const raf_geo_primitive *b, float t) {
    float w = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
    a->position.x = a->position.x + (b->position.x - a->position.x) * w;
    a->position.y = a->position.y + (b->position.y - a->position.y) * w;
    a->position.z = a->position.z + (b->position.z - a->position.z) * w;
    a->scale.x = a->scale.x + (b->scale.x - a->scale.x) * w;
    a->scale.y = a->scale.y + (b->scale.y - a->scale.y) * w;
    a->scale.z = a->scale.z + (b->scale.z - a->scale.z) * w;
}

void raf_geo_exec(raf_geo_state *state, const raf_geo_op *ops, size_t op_count) {
    size_t i;
    if (!state || !ops) {
        return;
    }
    for (i = 0; i < op_count; ++i) {
        const raf_geo_op *op = &ops[i];
        if (op->opcode == RAF_GEO_OP_NOP) {
            continue;
        }
        if (op->opcode == RAF_GEO_OP_ADD_PRIM) {
            raf_geo_add_primitive(state,
                                  (raf_geo_primitive_type)op->prim_type,
                                  op->vec,
                                  (raf_geo_vec3){1.0f, 1.0f, 1.0f},
                                  op->params,
                                  4);
            continue;
        }
        if (op->target >= state->count) {
            continue;
        }
        if (op->opcode == RAF_GEO_OP_TRANSLATE) {
            raf_geo_apply_translate(&state->primitives[op->target], op->vec);
        } else if (op->opcode == RAF_GEO_OP_SCALE) {
            raf_geo_apply_scale(&state->primitives[op->target], op->vec);
        } else if (op->opcode == RAF_GEO_OP_BLEND) {
            if (op->other < state->count) {
                raf_geo_apply_blend(&state->primitives[op->target],
                                    &state->primitives[op->other],
                                    op->scalar);
            }
        } else if (op->opcode == RAF_GEO_OP_INDEX) {
            state->primitives[op->target].index_key =
                raf_geo_index_from_pos(state->primitives[op->target].position,
                                       op->scalar);
        }
    }
}

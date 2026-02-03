/*
 * Rafaelia Baremetal - Geometry Nucleus (Low-level)
 * Data-driven operations without external dependencies.
 */

#ifndef RAFAELIA_GEOMETRY_H
#define RAFAELIA_GEOMETRY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAF_GEO_MAX_PRIMITIVES 256u

typedef struct {
    float x;
    float y;
    float z;
} raf_geo_vec3;

typedef enum {
    RAF_GEO_PRIM_NONE = 0,
    RAF_GEO_PRIM_SPHERE = 1,
    RAF_GEO_PRIM_CUBE = 2,
    RAF_GEO_PRIM_CONE = 3,
    RAF_GEO_PRIM_CYLINDER = 4,
    RAF_GEO_PRIM_TORUS = 5
} raf_geo_primitive_type;

typedef struct {
    uint8_t type;
    raf_geo_vec3 position;
    raf_geo_vec3 scale;
    float params[4];
    uint32_t index_key;
} raf_geo_primitive;

typedef struct {
    raf_geo_primitive primitives[RAF_GEO_MAX_PRIMITIVES];
    uint16_t count;
} raf_geo_state;

typedef enum {
    RAF_GEO_OP_NOP = 0,
    RAF_GEO_OP_ADD_PRIM = 1,
    RAF_GEO_OP_TRANSLATE = 2,
    RAF_GEO_OP_SCALE = 3,
    RAF_GEO_OP_BLEND = 4,
    RAF_GEO_OP_INDEX = 5
} raf_geo_opcode;

typedef struct {
    uint8_t opcode;
    uint16_t target;
    uint16_t other;
    raf_geo_vec3 vec;
    float scalar;
    uint8_t prim_type;
    float params[4];
} raf_geo_op;

void raf_geo_state_init(raf_geo_state *state);
void raf_geo_state_reset(raf_geo_state *state);

uint16_t raf_geo_add_primitive(raf_geo_state *state,
                               raf_geo_primitive_type type,
                               raf_geo_vec3 position,
                               raf_geo_vec3 scale,
                               const float *params,
                               size_t params_len);

void raf_geo_exec(raf_geo_state *state, const raf_geo_op *ops, size_t op_count);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_GEOMETRY_H */

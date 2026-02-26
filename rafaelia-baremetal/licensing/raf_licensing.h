/**
 * Rafaelia Baremetal - Licensing Nucleus for Add-ons
 *
 * Deterministic fixed-size licensing structures for add-on integration
 * in low-level environments.
 *
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#ifndef RAFAELIA_LICENSING_H
#define RAFAELIA_LICENSING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAF_LICENSE_NAME_MAX 64u
#define RAF_LICENSE_AUTHOR_MAX 64u
#define RAF_LICENSE_ID_MAX 128u
#define RAF_LICENSE_JURISDICTION_MAX 48u
#define RAF_LICENSE_MAX_NORMS 16u

typedef enum {
    RAF_LICENSE_NORM_NONE = 0,
    RAF_LICENSE_NORM_BERNE_CONVENTION,
    RAF_LICENSE_NORM_MONTEVIDEO_CONVENTION,
    RAF_LICENSE_NORM_UDHR,
    RAF_LICENSE_NORM_UN_CHARTER,
    RAF_LICENSE_NORM_UNESCO_CULTURAL_DIVERSITY,
    RAF_LICENSE_NORM_UNICEF_CHILD_RIGHTS,
    RAF_LICENSE_NORM_TRADITIONAL_KNOWLEDGE,
    RAF_LICENSE_NORM_FAITH_SACRED_WORKS,
    RAF_LICENSE_NORM_CUSTOM_LOCAL_FRAMEWORK
} raf_license_normative;

typedef enum {
    RAF_LICENSE_RIGHT_NONE = 0u,
    RAF_LICENSE_RIGHT_COPYRIGHT = 1u << 0,
    RAF_LICENSE_RIGHT_AUTHORSHIP = 1u << 1,
    RAF_LICENSE_RIGHT_CULTURE = 1u << 2,
    RAF_LICENSE_RIGHT_BELIEF = 1u << 3,
    RAF_LICENSE_RIGHT_SPIRITUAL = 1u << 4,
    RAF_LICENSE_RIGHT_DERIVATIVE = 1u << 5,
    RAF_LICENSE_RIGHT_DISTRIBUTION = 1u << 6,
    RAF_LICENSE_RIGHT_ATTRIBUTION = 1u << 7
} raf_license_rights;

typedef struct {
    char addon_name[RAF_LICENSE_NAME_MAX];
    char addon_author[RAF_LICENSE_AUTHOR_MAX];
    char addon_license_id[RAF_LICENSE_ID_MAX];
    char jurisdiction[RAF_LICENSE_JURISDICTION_MAX];

    uint32_t rights_mask;
    uint8_t norm_count;
    raf_license_normative norms[RAF_LICENSE_MAX_NORMS];

    uint8_t requires_attribution;
    uint8_t allows_commercial_use;
    uint8_t allows_derivatives;
} raf_license_addon_manifest;

void raf_license_addon_init(raf_license_addon_manifest *manifest);
int raf_license_addon_set_identity(raf_license_addon_manifest *manifest,
                                   const char *addon_name,
                                   const char *addon_author,
                                   const char *addon_license_id,
                                   const char *jurisdiction);
int raf_license_addon_set_rights(raf_license_addon_manifest *manifest, uint32_t rights_mask);
int raf_license_addon_attach_norm(raf_license_addon_manifest *manifest, raf_license_normative norm);
int raf_license_addon_validate(const raf_license_addon_manifest *manifest);
const char *raf_license_norm_name(raf_license_normative norm);

#ifdef __cplusplus
}
#endif

#endif /* RAFAELIA_LICENSING_H */

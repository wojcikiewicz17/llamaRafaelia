/**
 * Rafaelia Baremetal - Licensing Nucleus for Add-ons
 *
 * Copyright (c) 2026 Rafael Melo Reis
 * Licensed under MIT License - See LICENSE file
 */

#include "raf_licensing.h"

#include <string.h>

static int raf_copy_cstr(char *dst, size_t dst_size, const char *src) {
    size_t i;

    if (!dst || !src || dst_size == 0u) {
        return -1;
    }

    for (i = 0u; i + 1u < dst_size && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    dst[i] = '\0';

    if (src[i] != '\0') {
        return -2;
    }

    return 0;
}

void raf_license_addon_init(raf_license_addon_manifest *manifest) {
    if (!manifest) {
        return;
    }

    memset(manifest, 0, sizeof(*manifest));
}

int raf_license_addon_set_identity(raf_license_addon_manifest *manifest,
                                   const char *addon_name,
                                   const char *addon_author,
                                   const char *addon_license_id,
                                   const char *jurisdiction) {
    int status;

    if (!manifest) {
        return -1;
    }

    status = raf_copy_cstr(manifest->addon_name, sizeof(manifest->addon_name), addon_name);
    if (status != 0) {
        return status;
    }

    status = raf_copy_cstr(manifest->addon_author, sizeof(manifest->addon_author), addon_author);
    if (status != 0) {
        return status;
    }

    status = raf_copy_cstr(manifest->addon_license_id, sizeof(manifest->addon_license_id), addon_license_id);
    if (status != 0) {
        return status;
    }

    status = raf_copy_cstr(manifest->jurisdiction, sizeof(manifest->jurisdiction), jurisdiction);
    if (status != 0) {
        return status;
    }

    return 0;
}

int raf_license_addon_set_rights(raf_license_addon_manifest *manifest, uint32_t rights_mask) {
    if (!manifest) {
        return -1;
    }

    manifest->rights_mask = rights_mask;
    manifest->requires_attribution = (rights_mask & RAF_LICENSE_RIGHT_ATTRIBUTION) ? 1u : 0u;
    manifest->allows_commercial_use = (rights_mask & RAF_LICENSE_RIGHT_DISTRIBUTION) ? 1u : 0u;
    manifest->allows_derivatives = (rights_mask & RAF_LICENSE_RIGHT_DERIVATIVE) ? 1u : 0u;

    return 0;
}

int raf_license_addon_attach_norm(raf_license_addon_manifest *manifest, raf_license_normative norm) {
    uint8_t i;

    if (!manifest) {
        return -1;
    }

    if (norm <= RAF_LICENSE_NORM_NONE || norm > RAF_LICENSE_NORM_CUSTOM_LOCAL_FRAMEWORK) {
        return -2;
    }

    for (i = 0u; i < manifest->norm_count; ++i) {
        if (manifest->norms[i] == norm) {
            return 0;
        }
    }

    if (manifest->norm_count >= RAF_LICENSE_MAX_NORMS) {
        return -3;
    }

    manifest->norms[manifest->norm_count] = norm;
    manifest->norm_count++;

    return 0;
}

int raf_license_addon_validate(const raf_license_addon_manifest *manifest) {
    if (!manifest) {
        return -1;
    }

    if (manifest->addon_name[0] == '\0' ||
        manifest->addon_author[0] == '\0' ||
        manifest->addon_license_id[0] == '\0' ||
        manifest->jurisdiction[0] == '\0') {
        return -2;
    }

    if (manifest->rights_mask == RAF_LICENSE_RIGHT_NONE) {
        return -3;
    }

    if (manifest->norm_count == 0u) {
        return -4;
    }

    return 0;
}

const char *raf_license_norm_name(raf_license_normative norm) {
    switch (norm) {
        case RAF_LICENSE_NORM_BERNE_CONVENTION:
            return "berne-convention";
        case RAF_LICENSE_NORM_MONTEVIDEO_CONVENTION:
            return "montevideo-convention";
        case RAF_LICENSE_NORM_UDHR:
            return "udhr";
        case RAF_LICENSE_NORM_UN_CHARTER:
            return "un-charter";
        case RAF_LICENSE_NORM_UNESCO_CULTURAL_DIVERSITY:
            return "unesco-cultural-diversity";
        case RAF_LICENSE_NORM_UNICEF_CHILD_RIGHTS:
            return "unicef-child-rights";
        case RAF_LICENSE_NORM_TRADITIONAL_KNOWLEDGE:
            return "traditional-knowledge";
        case RAF_LICENSE_NORM_FAITH_SACRED_WORKS:
            return "faith-sacred-works";
        case RAF_LICENSE_NORM_CUSTOM_LOCAL_FRAMEWORK:
            return "custom-local-framework";
        case RAF_LICENSE_NORM_NONE:
        default:
            return "none";
    }
}

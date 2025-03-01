/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/TextureInfo.h"

namespace skgpu::graphite {

TextureInfo& TextureInfo::operator=(const TextureInfo& that) {
    if (!that.isValid()) {
        fValid = false;
        return *this;
    }
    fBackend = that.fBackend;
    fSampleCount = that.fSampleCount;
    fMipmapped = that.fMipmapped;
    fProtected = that.fProtected;

    switch (that.backend()) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            fDawnSpec = that.fDawnSpec;
            break;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            fMtlSpec = that.fMtlSpec;
            break;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            fVkSpec = that.fVkSpec;
            break;
#endif
        default:
            SK_ABORT("Unsupport Backend");
    }

    fValid = true;
    return *this;
}

bool TextureInfo::operator==(const TextureInfo& that) const {
    if (!this->isValid() && !that.isValid()) {
        return true;
    }
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }

    if (fSampleCount != that.fSampleCount ||
        fMipmapped != that.fMipmapped ||
        fProtected != that.fProtected) {
        return false;
    }

    switch (fBackend) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            return fDawnSpec == that.fDawnSpec;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            return fMtlSpec == that.fMtlSpec;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            return fVkSpec == that.fVkSpec;
#endif
        default:
            return false;
    }
}

#ifdef SK_DAWN
bool TextureInfo::getDawnTextureInfo(DawnTextureInfo* info) const {
    if (!this->isValid() || fBackend != BackendApi::kDawn) {
        return false;
    }
    *info = DawnTextureSpecToTextureInfo(fDawnSpec, fSampleCount, fMipmapped);
    return true;
}
#endif

} // namespace skgpu::graphite


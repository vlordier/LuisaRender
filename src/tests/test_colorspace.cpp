//
// Unit tests for colorspace conversion functions (util/colorspace.h).
// All conversions are instantiated with plain luisa::float3 (non-DSL path),
// so no GPU backend or DSL context is required.
//

#include <cassert>
#include <cmath>
#include <cstdio>

#include <util/colorspace.h>

using luisa::float3;
using luisa::make_float3;
using namespace luisa::render;

namespace {

constexpr float eps = 2e-4f;

[[nodiscard]] bool near_f(float a, float b, float tol = eps) noexcept {
    return std::abs(a - b) <= tol;
}

[[nodiscard]] bool near3(float3 a, float3 b, float tol = eps) noexcept {
    return near_f(a.x, b.x, tol) && near_f(a.y, b.y, tol) && near_f(a.z, b.z, tol);
}

// The Y channel of the white-point in XYZ must be 1 (by definition of CIE XYZ).
void test_white_luminance() noexcept {
    auto xyz = linear_srgb_to_cie_xyz(make_float3(1.f, 1.f, 1.f));
    assert(near_f(xyz.y, 1.0f, 1e-3f));
}

// Verify the Rec.709 relative luminances of the three primaries.
void test_primary_luminances() noexcept {
    // Values are the matrix coefficients used in colorspace.h.
    assert(near_f(linear_srgb_to_cie_y(make_float3(1.f, 0.f, 0.f)), 0.212671f));
    assert(near_f(linear_srgb_to_cie_y(make_float3(0.f, 1.f, 0.f)), 0.715160f));
    assert(near_f(linear_srgb_to_cie_y(make_float3(0.f, 0.f, 1.f)), 0.072169f));
    // Luminance of white (all channels = 1) must be 1.
    assert(near_f(linear_srgb_to_cie_y(make_float3(1.f, 1.f, 1.f)), 1.0f));
}

// Linear sRGB → XYZ → linear sRGB must be identity.
void test_srgb_xyz_roundtrip() noexcept {
    const auto test_roundtrip = [](float3 rgb) noexcept {
        auto xyz = linear_srgb_to_cie_xyz(rgb);
        auto rgb2 = cie_xyz_to_linear_srgb(xyz);
        assert(near3(rgb, rgb2));
    };
    test_roundtrip(make_float3(0.3f, 0.5f, 0.9f));
    test_roundtrip(make_float3(0.0f, 0.0f, 0.0f));
    test_roundtrip(make_float3(1.0f, 1.0f, 1.0f));
    test_roundtrip(make_float3(1.0f, 0.0f, 0.0f));
    test_roundtrip(make_float3(0.0f, 1.0f, 0.0f));
    test_roundtrip(make_float3(0.0f, 0.0f, 1.0f));
}

// CIE XYZ → CIELAB → CIE XYZ must be identity.
void test_xyz_lab_roundtrip() noexcept {
    const auto test_roundtrip = [](float3 xyz) noexcept {
        auto lab = cie_xyz_to_lab(xyz);
        auto xyz2 = lab_to_cie_xyz(lab);
        assert(near3(xyz, xyz2, 1e-3f));
    };
    test_roundtrip(make_float3(0.2f, 0.3f, 0.4f));
    test_roundtrip(make_float3(0.0f, 0.0f, 0.0f));
    // White in XYZ (approximate D65).
    test_roundtrip(make_float3(0.95047f, 1.0f, 1.08883f));
}

}// namespace

int main() {
    test_white_luminance();
    test_primary_luminances();
    test_srgb_xyz_roundtrip();
    test_xyz_lab_roundtrip();
    std::puts("All colorspace unit tests passed.");
    return 0;
}

//
// Deterministic unit tests for create_alias_table() (util/sampling.h).
// Uses small, hand-verified inputs to check correctness of the construction
// without running the 1-billion-sample benchmark in test_alias_method.cpp.
//

#include <cassert>
#include <cmath>
#include <cstdio>
#include <numeric>

#include <util/sampling.h>

using namespace luisa;
using namespace luisa::render;

namespace {

// Helper: sample from the alias table on the CPU (mirrors the GPU template
// in sampling.h but uses plain C++ so it can be called in unit tests).
[[nodiscard]] uint sample_alias_table_cpu(
    const luisa::vector<AliasEntry> &table, float u_in) noexcept {
    auto n = static_cast<uint>(table.size());
    auto u = u_in * static_cast<float>(n);
    auto i = std::min(static_cast<uint>(u), n - 1u);
    float u_remapped = u - std::floor(u);
    return u_remapped < table[i].prob ? i : table[i].alias;
}

// 1. Single element: always produces index 0, pdf = 1.
void test_single_element() noexcept {
    float w = 5.f;
    auto [table, pdf] = create_alias_table(luisa::span<const float>{&w, 1u});
    assert(table.size() == 1u);
    assert(pdf.size() == 1u);
    assert(std::abs(pdf[0] - 1.0f) < 1e-6f);
    assert(table[0].alias == 0u);
    // Any sample should return 0.
    assert(sample_alias_table_cpu(table, 0.0f) == 0u);
    assert(sample_alias_table_cpu(table, 0.5f) == 0u);
    assert(sample_alias_table_cpu(table, 0.999f) == 0u);
}

// 2. Uniform weights: all pdf[i] == 1/n, all probs ≈ 1.
void test_uniform_weights() noexcept {
    constexpr uint n = 4u;
    float weights[n] = {1.f, 1.f, 1.f, 1.f};
    auto [table, pdf] = create_alias_table(luisa::span<const float>{weights, n});
    assert(table.size() == n && pdf.size() == n);
    for (uint i = 0u; i < n; i++) {
        assert(std::abs(pdf[i] - 0.25f) < 1e-6f);
        assert(table[i].prob >= 1.0f - 1e-5f);
    }
}

// 3. Degenerate: all weight goes to one element.
//    Element 2 of 3 has weight 1, others 0 → always samples index 2.
void test_single_nonzero() noexcept {
    float weights[3] = {0.f, 0.f, 1.f};
    auto [table, pdf] = create_alias_table(luisa::span<const float>{weights, 3u});
    assert(std::abs(pdf[0] - 0.0f) < 1e-6f);
    assert(std::abs(pdf[1] - 0.0f) < 1e-6f);
    assert(std::abs(pdf[2] - 1.0f) < 1e-6f);
    // All alias entries must redirect to index 2.
    constexpr int samples = 8;
    const float u_vals[samples] = {0.0f, 0.1f, 0.2f, 0.35f, 0.5f, 0.65f, 0.85f, 0.99f};
    for (float u : u_vals) {
        assert(sample_alias_table_cpu(table, u) == 2u);
    }
}

// 4. Two equal weights → each sampled ~50% of the time.
void test_two_equal() noexcept {
    float weights[2] = {1.f, 1.f};
    auto [table, pdf] = create_alias_table(luisa::span<const float>{weights, 2u});
    assert(std::abs(pdf[0] - 0.5f) < 1e-6f);
    assert(std::abs(pdf[1] - 0.5f) < 1e-6f);
}

// 5. PDF must sum to 1.0 for arbitrary weights.
void test_pdf_sums_to_one() noexcept {
    float weights[5] = {1.f, 2.f, 3.f, 4.f, 5.f};
    auto [table, pdf] = create_alias_table(luisa::span<const float>{weights, 5u});
    float sum = 0.f;
    for (float p : pdf) { sum += p; }
    assert(std::abs(sum - 1.0f) < 1e-5f);
}

// 6. All alias indices must be in range [0, n).
void test_alias_indices_in_range() noexcept {
    float weights[6] = {0.5f, 3.f, 1.f, 0.1f, 2.f, 4.f};
    auto [table, pdf] = create_alias_table(luisa::span<const float>{weights, 6u});
    for (uint i = 0u; i < 6u; i++) {
        assert(table[i].alias < 6u);
        assert(table[i].prob >= 0.0f && table[i].prob <= 1.0f + 1e-5f);
    }
}

// 7. PDF correctness: pdf[i] == weight[i] / sum(weights).
void test_pdf_values() noexcept {
    float weights[3] = {1.f, 2.f, 3.f};
    auto [table, pdf] = create_alias_table(luisa::span<const float>{weights, 3u});
    assert(std::abs(pdf[0] - 1.f / 6.f) < 1e-6f);
    assert(std::abs(pdf[1] - 2.f / 6.f) < 1e-6f);
    assert(std::abs(pdf[2] - 3.f / 6.f) < 1e-6f);
}

}// namespace

int main() {
    test_single_element();
    test_uniform_weights();
    test_single_nonzero();
    test_two_equal();
    test_pdf_sums_to_one();
    test_alias_indices_in_range();
    test_pdf_values();
    std::puts("All alias table unit tests passed.");
    return 0;
}

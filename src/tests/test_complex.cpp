//
// Unit tests for Complex<T> (util/complex.h).
// Pure CPU arithmetic – no GPU backend, no DSL context required.
//

#include <cassert>
#include <cmath>
#include <cstdio>

#include <util/complex.h>

using luisa::render::Complex;

namespace {

constexpr float eps = 1e-5f;

[[nodiscard]] bool near_f(float a, float b) noexcept {
    return std::abs(a - b) <= eps;
}

void test_construction() noexcept {
    // Two-argument constructor stores re and im.
    Complex<float> z{3.f, 4.f};
    assert(z.re == 3.f && z.im == 4.f);

    // One-argument (real-only) constructor sets im = 0.
    Complex<float> r{2.f};
    assert(r.re == 2.f && r.im == 0.f);
}

void test_negate() noexcept {
    Complex<float> z{1.f, -2.f};
    auto neg = -z;
    assert(neg.re == -1.f && neg.im == 2.f);
}

void test_add() noexcept {
    auto c = Complex<float>{1.f, 2.f} + Complex<float>{3.f, 4.f};
    assert(c.re == 4.f && c.im == 6.f);
}

void test_subtract() noexcept {
    auto c = Complex<float>{5.f, 7.f} - Complex<float>{2.f, 3.f};
    assert(c.re == 3.f && c.im == 4.f);
}

void test_multiply() noexcept {
    // (1+2i)(3+4i) = (3-8) + (4+6)i = -5+10i
    auto c = Complex<float>{1.f, 2.f} * Complex<float>{3.f, 4.f};
    assert(c.re == -5.f && c.im == 10.f);

    // multiplication by real: (1+2i)*3 = 3+6i
    auto d = Complex<float>{1.f, 2.f} * Complex<float>{3.f};
    assert(d.re == 3.f && d.im == 6.f);
}

void test_divide() noexcept {
    // (2+4i)/(1+2i): multiply by conj(1+2i)/(1+2i)
    //   num  = (2+4i)(1-2i) = 2-4i+4i-8i^2 = 2+8 = 10
    //   denom = 1+4 = 5
    //   result = 10/5 = 2+0i
    auto c = Complex<float>{2.f, 4.f} / Complex<float>{1.f, 2.f};
    assert(near_f(c.re, 2.f) && near_f(c.im, 0.f));

    // division by real: (3+6i)/3 = 1+2i
    auto d = Complex<float>{3.f, 6.f} / Complex<float>{3.f};
    assert(near_f(d.re, 1.f) && near_f(d.im, 2.f));
}

void test_scalar_left_ops() noexcept {
    Complex<float> z{1.f, 2.f};

    auto add = 3.f + z;
    assert(add.re == 4.f && add.im == 2.f);

    auto sub = 3.f - z;
    assert(sub.re == 2.f && sub.im == -2.f);

    auto mul = 3.f * z;
    assert(mul.re == 3.f && mul.im == 6.f);

    auto div = 4.f / Complex<float>{2.f};
    assert(near_f(div.re, 2.f) && near_f(div.im, 0.f));
}

void test_identity_elements() noexcept {
    Complex<float> z{3.f, 4.f};
    Complex<float> zero{0.f};
    Complex<float> one{1.f};

    // additive identity
    auto sum = z + zero;
    assert(sum.re == z.re && sum.im == z.im);

    // multiplicative identity
    auto prod = z * one;
    assert(near_f(prod.re, z.re) && near_f(prod.im, z.im));
}

}// namespace

int main() {
    test_construction();
    test_negate();
    test_add();
    test_subtract();
    test_multiply();
    test_divide();
    test_scalar_left_ops();
    test_identity_elements();
    std::puts("All Complex<float> unit tests passed.");
    return 0;
}

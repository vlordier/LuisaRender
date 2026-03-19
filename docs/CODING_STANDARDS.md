# LuisaRender Coding Standards

This document is the authoritative reference for coding conventions used
throughout LuisaRender's first-party source code (`src/`).

---

## Table of Contents

1. [Naming conventions](#1-naming-conventions)
2. [File organisation](#2-file-organisation)
3. [Memory and ownership](#3-memory-and-ownership)
4. [`const`-correctness](#4-const-correctness)
5. [Error handling and logging](#5-error-handling-and-logging)
6. [C++ language guidelines](#6-c-language-guidelines)
7. [LuisaCompute DSL guidelines](#7-luisacompute-dsl-guidelines)
8. [Comments and documentation](#8-comments-and-documentation)
9. [Formatting](#9-formatting)
10. [Build system conventions](#10-build-system-conventions)

---

## 1. Naming conventions

| Entity | Convention | Example |
|--------|-----------|---------|
| Class / struct | `PascalCase` | `SampledSpectrum`, `Pipeline` |
| Type alias / `using` | `PascalCase` | `using NodePtr = std::unique_ptr<Node>` |
| Free function | `snake_case` | `offset_ray_origin` |
| Member function | `snake_case` | `evaluate_opacity` |
| Private member variable | `_snake_case` | `_texture_id`, `_pipeline` |
| Public member variable | `snake_case` | rarely used; prefer getters |
| Local variable | `snake_case` | `path_length`, `swl` |
| Function parameter | `snake_case` | `const Transform &local_to_world` |
| Constant | `snake_case` (prefer `constexpr`) | `constexpr auto max_depth = 64u` |
| Enumerator | `snake_case` | `enum class Tag { diffuse, specular }` |
| Macro | `UPPER_SNAKE_CASE` | `LUISA_ASSERT`, `LUISA_RENDER_PLUGIN` |
| Namespace | `snake_case` | `luisa::render` |
| Template parameter (type) | `PascalCase` | `template<typename Spectrum>` |
| Template parameter (value) | `snake_case` | `template<size_t dim>` |
| File name | `snake_case` | `sampled_spectrum.h`, `path_tracer.cpp` |

### Rationale

Using `_` prefix for private members instantly distinguishes fields from locals
without needing `this->` disambiguation. `PascalCase` for types follows the
C++ Standard Library style and makes types immediately recognisable in dense
template code.

---

## 2. File organisation

### Header files (`.h`)

- One class or tightly related group of classes per header.
- Always use `#pragma once` (no include guards).
- Keep `#include` lists minimal; prefer forward declarations for pointer/reference
  parameters.
- Never place `using namespace` at file scope in a header.
  `using namespace std::literals` inside a function body or a local scope is fine.

### Source files (`.cpp`)

- Named to match the primary header: `sampled_spectrum.h` → `sampled_spectrum.cpp`.
- Start with the corresponding header include, then system headers, then
  LuisaCompute headers, then other first-party headers.

### Directory layout

```
src/
  base/          # abstract plugin interfaces (Camera, Integrator, Surface, …)
  apps/          # top-level executables (CLI, exporter)
  films/         # Film implementations
  integrators/   # path tracers, photon mappers, …
  lights/        # area lights, environment maps, …
  media/         # participating media
  samplers/      # Sobol, PMJ02, …
  sdl/           # Scene Description Language parser
  shapes/        # meshes, spheres, subdivision surfaces
  spectra/       # spectrum representations
  surfaces/      # BSDFs
  textures/      # image, procedural textures
  util/          # shared utilities (math, I/O, data structures)
  tests/         # unit and integration tests
```

---

## 3. Memory and ownership

### Heap allocation

- **Prefer `std::unique_ptr<T>`** for single-owner heap objects.
  Use `luisa::make_unique<T>(…)` (which wraps `std::make_unique`) where
  LuisaCompute allocators are required.
- **Use `std::shared_ptr<T>`** only when shared ownership is genuinely needed.
  Shared ownership adds overhead and makes lifetime reasoning harder; prefer
  to pass raw (non-owning) pointers or references when ownership is already
  clear.
- **Never use raw `new` / `delete`** in new code. For third-party C APIs that
  return owning raw pointers, wrap them immediately:

  ```cpp
  // Bad
  auto *p = SomeAPI::Create();
  // … many lines later …
  delete p;   // easily forgotten or skipped on error paths

  // Good
  auto p = std::unique_ptr<SomeType>(SomeAPI::Create());
  // p is automatically cleaned up on all exit paths
  ```

  When the deleter is not `delete` (e.g., `SomeAPI::Release(p)`), supply a
  custom deleter:

  ```cpp
  struct SubdividerDeleter {
      void operator()(Assimp::Subdivider *s) const noexcept { delete s; }
  };
  auto subdiv = std::unique_ptr<Assimp::Subdivider, SubdividerDeleter>(
      Assimp::Subdivider::Create(Assimp::Subdivider::CATMULL_CLARKE));
  ```

### RAII

- All resources (files, GPU buffers, locks) must be acquired in constructors
  and released in destructors.
- Never acquire a resource and store it in a raw member variable without a
  matching destructor.

### Stack vs. heap

- Prefer stack allocation for small, short-lived objects.
- Never allocate large arrays (> ~4 KB) on the stack.

---

## 4. `const`-correctness

- Mark every member function that does not mutate `this` as `const`.
- Declare every local variable that is not re-assigned as `const` (or
  `constexpr`).
- Prefer `const T &` parameters over `T` when `T` is non-trivial and the
  parameter is not modified.
- Prefer `std::string_view` over `const std::string &` for read-only string
  parameters (avoids an implicit copy when the caller has a `const char*`).
- Prefer `std::span<const T>` over `const std::vector<T> &` for read-only
  array parameters.

---

## 5. Error handling and logging

### Assertion macros

Use the LuisaCompute logging macros consistently:

| Situation | Macro |
|-----------|-------|
| Programmer error / violated invariant | `LUISA_ASSERT(cond, fmt, …)` |
| Unrecoverable runtime error | `LUISA_ERROR_WITH_LOCATION(fmt, …)` |
| Recoverable but problematic condition | `LUISA_WARNING_WITH_LOCATION(fmt, …)` |
| Informational progress message | `LUISA_INFO(fmt, …)` |
| Verbose debugging (stripped in Release) | `LUISA_VERBOSE(fmt, …)` |

### Rules

- Never use bare `assert()` from `<cassert>` in library code; use `LUISA_ASSERT`.
- Never call `exit()` or `abort()` for graceful teardown. Signal the
  `Film`'s abort mechanism or propagate the error up the call stack instead.
- Never use `printf` / `fprintf` in library code; use the `LUISA_*` macros
  which integrate with the centralised logger.
- Error messages must include enough context to diagnose the problem without
  a debugger: file path, parameter value, expected range, etc.

---

## 6. C++ language guidelines

### General

- Use C++20 features where they improve clarity: ranges, concepts, designated
  initialisers, `[[likely]]` / `[[unlikely]]`, `std::span`, coroutines (where
  appropriate).
- Mark all functions `noexcept` that cannot throw.  This is especially
  important for GPU DSL lambdas and small utility functions.
- Mark all factory functions and functions whose return value must not be
  silently discarded with `[[nodiscard]]`.
- Prefer `auto` for complex type deductions but spell out simple types for
  readability: `auto v = std::vector<float>{}` is fine; `auto x = 42;` is
  less clear than `int x = 42;`.
- Do not use C-style casts `(T)expr`; use `static_cast<T>`, `reinterpret_cast<T>`,
  or `bit_cast<T>`.

### Avoid

- `goto` — always replaceable with structured control flow or RAII.
- Implicit narrowing conversions — enable `-Wconversion` warnings
  (provided by `cmake/CompilerWarnings.cmake`) to catch these.
- Signed / unsigned comparison mismatches — use `static_cast` or `std::ssize`.
- `using namespace std;` at file scope in any header or in `.cpp` files with
  a broad scope.
- Magic numbers — declare `constexpr` named constants instead.

### Output streams

- Prefer `'\n'` over `std::endl`. The `std::endl` manipulator flushes the
  underlying stream buffer on every call, which has measurable overhead in
  tight loops or log-heavy code.

---

## 7. LuisaCompute DSL guidelines

LuisaRender uses the LuisaCompute embedded DSL (domain-specific language) to
write GPU kernels in C++. DSL code runs on both the CPU (for compilation) and
the GPU (after JIT compilation). The following rules apply to DSL code.

- All kernel lambdas must be marked `noexcept`.
- Use `Expr<T>` / `Var<T>` for DSL types; never mix host and DSL types
  without an explicit conversion.
- Prefer `ite(cond, a, b)` (DSL ternary) over branching where the branch
  predicate is a DSL boolean; branches force warp divergence.
- Use `[[nodiscard]]` on DSL helper functions that return `Expr<T>`.
- Add `LUISA_DISABLE_DSL_ADDRESS_OF_OPERATOR(T)` for types that must not have
  their address taken in the DSL (see `spec.h` for examples).

---

## 8. Comments and documentation

### Doc comments

Public APIs (classes, methods, free functions in `include/` or `base/`) must
have a brief doc comment:

```cpp
/// Evaluates the BSDF for the given pair of directions.
/// @param wo  Outgoing direction in local (shading) frame.
/// @param wi  Incident direction in local (shading) frame.
/// @returns   Spectral BSDF value (sr^-1).
[[nodiscard]] SampledSpectrum evaluate(
    Expr<float3> wo, Expr<float3> wi) const noexcept;
```

### Inline comments

- Prefer comments that explain *why*, not *what*:

  ```cpp
  // Bad: increments i by 1
  i++;

  // Good: skip the degenerate first vertex produced by subdivision
  i++;
  ```

- Keep inline comments to a single line where possible. For multi-line
  explanations, use a block comment above the code.

### TODO / FIXME

Every `TODO` or `FIXME` must include:
1. **What** needs to be done or fixed.
2. **Why** it is not done yet (blocker, known prerequisite, or research needed).
3. **How** it should be fixed (algorithm reference, data-structure suggestion,
   or API change needed).

```cpp
// Bad
// TODO: fix this

// Good
// TODO: replace the O(n log n) binary-search interpolation with a single
// O(n) scan over the sorted wavelength table.  The table is always sorted
// at construction time so a linear scan suffices; see Dutre §A.2 for the
// algorithm.
```

---

## 9. Formatting

All formatting is enforced by `clang-format-18` using the `.clang-format` file
in the repository root. **Do not manually adjust whitespace** that
`clang-format` would change; let the tool do it.

Key rules (from `.clang-format`):

- Indent width: 4 spaces (no tabs).
- Column limit: 120 characters.
- Brace style: follows LLVM defaults.
- Include sort order: own header first, then system, then LuisaCompute,
  then first-party.

Run before committing:

```bash
clang-format-18 -i $(git diff --name-only --cached | grep -E '\.(cpp|h|cuh)$')
```

Or install the pre-commit hook (see [CONTRIBUTING.md](../CONTRIBUTING.md)) to
have it run automatically.

---

## 10. Build system conventions

### Target-scoped options

All compiler flags must be attached to specific targets, not set globally:

```cmake
# Bad – pollutes every target, including vendored code
add_compile_options(-Wall -Wextra)

# Good – only this target gets the flags
target_compile_options(my_target PRIVATE -Wall -Wextra)
```

Use the helper from `cmake/CompilerWarnings.cmake`:

```cmake
luisa_render_set_compiler_warnings(my_target)
```

### Sanitizers

Use the helper from `cmake/Sanitizers.cmake` to attach ASan + UBSan to a
target in Debug builds:

```cmake
luisa_render_enable_sanitizers(my_target)
```

The sanitizers are only active when `-DLUISA_RENDER_ENABLE_SANITIZERS=ON` is
passed to CMake and the compiler is Clang or GCC.

### New targets

When adding a new library or executable:

1. Add it to the appropriate `CMakeLists.txt` with explicit `PRIVATE` /
   `PUBLIC` / `INTERFACE` visibility on all `target_*` calls.
2. Call `luisa_render_set_compiler_warnings(target)` on first-party code.
3. If the target has tests, call `luisa_render_enable_sanitizers(test_target)`
   and register the test with `add_test` + `set_tests_properties` (LABELS +
   TIMEOUT).
4. Add an `install(TARGETS …)` rule if the target produces a distributable
   artefact.

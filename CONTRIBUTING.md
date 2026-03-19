# Contributing to LuisaRender

Thank you for your interest in contributing. This document explains how to
get the project building locally, what the coding standards are, and what the
pull-request workflow looks like.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Getting the source](#2-getting-the-source)
3. [Building](#3-building)
4. [Running the tests](#4-running-the-tests)
5. [Pre-commit hooks](#5-pre-commit-hooks)
6. [Coding standards](#6-coding-standards)
7. [Pull-request checklist](#7-pull-request-checklist)
8. [Reporting bugs](#8-reporting-bugs)

---

## 1. Prerequisites

| Tool | Minimum version | Notes |
|------|----------------|-------|
| CMake | 3.20 | CMake 3.25+ recommended for `cmake --preset` support |
| C++ compiler | C++20 | Clang 13+, GCC 11+, or MSVC 2019+ |
| clang-format | 18 | Enforced by CI; install with `apt install clang-format-18` |
| clang-tidy | 18 | Used by CI; install with `apt install clang-tidy-18` |
| cppcheck | 2.x | Used by CI; install with `apt install cppcheck` |
| pre-commit | ≥ 3 | Optional but strongly recommended; `pip install pre-commit` |
| Python | ≥ 3.9 | Required by pre-commit hooks |

GPU back-end requirements (only needed for full rendering, not for tests):

- **CUDA**: CUDA 11.2+ with an NVIDIA RTX 20/30/40-series card
- **DirectX**: Windows with DX12-capable card (RTX-enabled)
- **Metal**: macOS 12+ with Apple Silicon or AMD dGPU
- **ISPC / LLVM**: any platform (software renderer, slower)

---

## 2. Getting the source

```bash
git clone --recursive https://github.com/LuisaGroup/LuisaRender.git
cd LuisaRender
```

The `--recursive` flag is required because LuisaRender uses Git submodules for
vendored dependencies (`LuisaCompute`, `assimp`, `tinyexr`, …).

---

## 3. Building

### CPU-only / test build (no GPU required)

```bash
cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_COMPILER=clang-18 \
      -DCMAKE_CXX_COMPILER=clang++-18 \
      -DLUISA_COMPUTE_ENABLE_CUDA=OFF \
      -DLUISA_COMPUTE_ENABLE_DX=OFF \
      -DLUISA_COMPUTE_ENABLE_METAL=OFF \
      -DLUISA_COMPUTE_ENABLE_VULKAN=OFF \
      -DLUISA_RENDER_BUILD_TESTS=ON

cmake --build build --target test_u64 test_complex test_colorspace test_alias_table_unit
```

### Full release build

```bash
cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DLUISA_COMPUTE_ENABLE_CUDA=ON

cmake --build build --parallel $(nproc)
```

### With sanitizers (Debug + ASan + UBSan)

```bash
cmake -S . -B build-asan \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_COMPILER=clang++-18 \
      -DLUISA_RENDER_ENABLE_SANITIZERS=ON \
      -DLUISA_RENDER_BUILD_TESTS=ON \
      -DLUISA_COMPUTE_ENABLE_CUDA=OFF \
      -DLUISA_COMPUTE_ENABLE_DX=OFF \
      -DLUISA_COMPUTE_ENABLE_METAL=OFF \
      -DLUISA_COMPUTE_ENABLE_VULKAN=OFF

cmake --build build-asan --target test_u64 test_complex test_colorspace test_alias_table_unit
ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  ctest --test-dir build-asan -L unit --output-on-failure
```

---

## 4. Running the tests

```bash
# All unit tests (fast, no GPU)
ctest --test-dir build -L unit --output-on-failure

# All integration tests (CPU only, no GPU)
ctest --test-dir build -L integration --output-on-failure

# A single test by name
ctest --test-dir build -R test_colorspace -V

# Everything
ctest --test-dir build --output-on-failure
```

The `test_alias_method` binary is intentionally **not registered** with CTest
because it runs 1 billion sampling iterations (useful as a precision benchmark
but too slow for CI). Run it manually: `./build/bin/test_alias_method`.

---

## 5. Pre-commit hooks

Install the hooks once after cloning:

```bash
pip install pre-commit
pre-commit install
```

After installation every `git commit` automatically runs:

- `clang-format-18` — reformats changed C++ and C files
- `cmake-lint` — lint CMakeLists.txt files
- `ruff` — lint Python files in `tools/`
- General hygiene (trailing whitespace, final newline, YAML/JSON validation, …)

To run the hooks manually on all files:

```bash
pre-commit run --all-files
```

---

## 6. Coding standards

See [docs/CODING_STANDARDS.md](docs/CODING_STANDARDS.md) for the full
reference. The key rules are summarised below.

### Naming

| Entity | Convention | Example |
|--------|-----------|---------|
| Types / classes | `PascalCase` | `SampledSpectrum` |
| Functions / methods | `snake_case` | `evaluate_opacity` |
| Member variables | `_snake_case` (leading `_`) | `_texture_id` |
| Constants / enumerators | `snake_case` | `max_depth` |
| Template parameters | `PascalCase` | `typename T` |
| Macros | `UPPER_SNAKE_CASE` | `LUISA_ASSERT` |
| Namespaces | `snake_case` | `luisa::render` |

### Ownership & memory

- Prefer `std::unique_ptr<T>` for single-owner heap objects.
- Never use raw `new` / `delete` in new code.
- Use RAII wrappers for all system resources (files, sockets, handles).
- Third-party C APIs that return owning raw pointers must be wrapped in a
  custom deleter: `std::unique_ptr<T, CustomDeleter>`.

### `const`-correctness

- Mark every member function that does not modify `this` as `const`.
- Mark every local variable that is never re-assigned as `const` (or `constexpr`
  if the value is known at compile time).
- Prefer `std::string_view` / `std::span` over `const std::string&` /
  `const std::vector<T>&` in function parameters.

### Error handling

- Use `LUISA_ASSERT(cond, fmt, …)` for programmer errors (violated invariants).
- Use `LUISA_ERROR_WITH_LOCATION(fmt, …)` and throw for recoverable errors.
- Use `LUISA_WARNING_WITH_LOCATION(fmt, …)` for non-fatal conditions.
- Never use `exit()` for orderly teardown — propagate errors up or signal the
  film's abort mechanism instead.

### Style / formatting

- `clang-format` (`.clang-format` in the root) is the single source of truth
  for formatting. Do not manually adjust whitespace that `clang-format` would
  change.
- Prefer `'\n'` over `std::endl` — the latter flushes the buffer on every
  call and causes unnecessary I/O overhead.
- Use `[[nodiscard]]` on all factory functions and functions whose return value
  must not be silently discarded.
- Use `noexcept` on all GPU DSL kernel lambdas and on any function that
  demonstrably cannot throw.

### Comments & documentation

- All public APIs must have a brief doc comment explaining purpose, parameters,
  and return value.
- `TODO`/`FIXME` comments must carry a root-cause note and a concrete fix hint
  (not a bare `// TODO`).
- Comments must describe *why*, not *what* the code does.

---

## 7. Pull-request checklist

Before opening a PR:

- [ ] `clang-format-18 --dry-run --Werror` passes on all changed files
- [ ] `cppcheck --enable=all --error-exitcode=1` passes
- [ ] All CTest unit tests pass locally (`ctest -L unit`)
- [ ] No new bare `TODO` / `FIXME` stubs introduced
- [ ] New logic is covered by at least one test or the PR description explains
      why a test is not feasible
- [ ] Any new public API has `[[nodiscard]]` and `noexcept` where appropriate

---

## 8. Reporting bugs

Please open a [GitHub issue](https://github.com/LuisaGroup/LuisaRender/issues)
with the following information:

1. **Platform**: OS, GPU model, driver version
2. **Build type**: Release / Debug; back-end (CUDA / DX / Metal / ISPC)
3. **Reproduction steps**: minimal scene file or command line invocation
4. **Expected vs. actual behaviour**
5. **Crash output**: stack trace or `LUISA_ERROR` message if available

For rendering correctness issues (wrong colours, fireflies, …), attach a
reference image (e.g., from Mitsuba 3 or PBRT) alongside the LuisaRender
output.

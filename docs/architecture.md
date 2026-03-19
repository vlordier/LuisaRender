# Architecture Overview

LuisaRender is a high-performance, cross-platform physically-based renderer built on
top of the [LuisaCompute](https://github.com/LuisaGroup/LuisaCompute) GPU DSL framework.
It implements a plugin-based scene graph that lets integrators, materials, lights, and
cameras be mixed and matched at runtime through a JSON/DSL scene description file.

---

## High-Level Pipeline

```
Scene file (.json / .luisa)
        │
        ▼
  ┌─────────────────────┐
  │    SDL / Parser      │  Scene Description Language + JSON front-end
  │  (src/sdl/)         │
  └──────────┬──────────┘
             │  SceneDesc / SceneNodeDesc
             ▼
  ┌─────────────────────┐
  │    Base layer        │  Abstract interfaces: Integrator, Camera, Film,
  │  (src/base/)        │  Surface, Light, Sampler, Shape, Medium, …
  └──────────┬──────────┘
             │  Concrete plugin instances (loaded as shared modules)
             ▼
  ┌─────────────────────┐
  │    Plugin modules    │  cameras/, films/, integrators/, lights/,
  │  (src/*/*)          │  lightsamplers/, materials/, media/, samplers/,
  └──────────┬──────────┘  shapes/, spectra/, surfaces/, textures/, …
             │
             ▼
  ┌─────────────────────┐
  │    Pipeline / Geo    │  Acceleration structure (BVH), texture cache,
  │  (src/base/)        │  binding table, resource management
  └──────────┬──────────┘
             │  LuisaCompute kernel dispatch
             ▼
  ┌─────────────────────┐
  │  LuisaCompute DSL    │  GPU JIT kernel compilation and dispatch
  │  (src/compute/)     │  Backends: CUDA, Vulkan, DX12, Metal, CPU
  └─────────────────────┘
```

---

## Key Design Decisions

### 1. Plugin architecture
Every renderer element (integrator, surface model, light source, …) is a **shared
library plugin** discovered at runtime.  The `base/` layer defines abstract C++
interfaces (`Surface`, `Light`, `Camera`, …); `src/*/` subdirectories implement
them.  Adding a new material means writing one `.cpp` + registering it in the
corresponding `CMakeLists.txt` — no changes to the core.

### 2. LuisaCompute DSL
All GPU code is written in a **C++-embedded DSL** (LuisaCompute), not CUDA or HLSL.
The DSL compiles to backend-specific shaders at application startup, giving the same
source code on CUDA, Vulkan, DX12, Metal, and CPU backends.

Consequence: GPU kernel code looks like C++ but uses `$if`, `$while`, `$for`,
`Var<T>`, `Expr<T>` constructs instead of plain C++ control flow.

### 3. Spectral rendering
Colours are represented as `SampledSpectrum` (wavelength samples, default 4)
everywhere in the rendering pipeline.  Helper math functions (`clamp`, `max`, `log`,
`pow`, …) are defined in `src/util/spec.h` / `spec.cpp`.

### 4. Plugin loading
`Pipeline` (defined in `src/base/pipeline.h`) owns the plugin registry and the
`Geometry` acceleration structure.  It is the central resource manager that
integrators query to evaluate materials, sample lights, and trace rays.

---

## Thread and Execution Model

| Layer | Concurrency model |
|---|---|
| Scene parsing | Single-threaded on the host |
| Plugin loading | Single-threaded on the host |
| Kernel dispatch | Asynchronous `CommandBuffer` per frame, executed on the GPU device thread |
| Multi-sample rendering | SPP outer loop; each sample dispatched as a separate command buffer |
| Wavefront integrators | Persistent kernel loop with compaction queues |

---

## Build Profiles

| Profile | Flags | Use for |
|---|---|---|
| Debug | `-O0 -g` | Stepping with gdb/lldb |
| RelWithDebInfo | `-O2 -g` | Profiling with symbols |
| Release | `-O3 -DNDEBUG` | Production renders |

Enable sanitizers in Debug builds:

```bash
cmake -DLUISA_RENDER_ENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug …
```

---

## Source Map

```
cmake/         CMake helper modules (warnings, sanitizers, deps)
docs/          This documentation
src/
  sdl/         Scene Description Language parser (JSON + custom DSL)
  base/        Abstract interfaces and the Pipeline resource manager
  util/        Math helpers, spectrum types, image I/O primitives
  compute/     LuisaCompute submodule (GPU DSL + backends)
  ext/         Vendored third-party code (assimp, cxxopts, json, tinyexr)
  apps/        Entry-point executables (cli renderer, scene exporter)
  tests/       CTest unit and integration tests
  cameras/     Camera model plugins
  films/       Film/display plugins (HDR, LDR, interactive preview)
  integrators/ Path-tracer, photon-mapping, wavefront integrators
  lights/      Light source plugins (area, distant, point, …)
  lightsamplers/ Light sampling strategy plugins
  media/       Participating media plugins (homogeneous, …)
  phasefunctions/ Phase function plugins (HG, …)
  samplers/    Sampler plugins (PMJ02, sobol, …)
  shapes/      Shape plugins (mesh, sphere, loop subdivision)
  spectra/     Spectral upsampling plugins
  surfaces/    BSDF plugins (Disney, layered, rough conductor/dielectric, …)
  textures/    Texture plugins (constant, image, checkerboard)
  texturemappings/ UV mapping plugins
  transforms/  Transform plugins (rigid, animated)
  environments/ Environment light plugins (sky, HDRI)
  filters/     Reconstruction filter plugins (Gaussian, box, Mitchell)
  tools/       Python helper scripts (converter, tone-mapper, …)
```

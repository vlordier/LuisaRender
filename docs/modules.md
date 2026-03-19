# Module Inventory

This file lists every first-party module in `src/`, its responsibility,
key public types, and the main headers to read first.

---

## Core modules

### `src/sdl/` — Scene Description Language
Parses `.json` and `.luisa` scene files into an in-memory `SceneDesc` graph.

| File | Responsibility |
|---|---|
| `scene_desc.h/.cpp` | Top-level scene graph container |
| `scene_node_desc.h/.cpp` | Single scene graph node (tag + properties) |
| `scene_node_tag.h/.cpp` | Enum of all recognised node types (Camera, Film, …) |
| `scene_parser.h/.cpp` | Entry-point: parse file → `SceneDesc` |
| `scene_parser_json.cpp` | JSON front-end for the parser |

**Key type:** `SceneNodeDesc` — a node in the scene description graph; holds a tag
and a map of named property values (strings, numbers, nested nodes).

---

### `src/base/` — Abstract interfaces + Pipeline
Defines the abstract base classes that every plugin must implement, and the
`Pipeline` resource manager that ties everything together at render time.

| Header | Abstract type |
|---|---|
| `camera.h` | `Camera` / `CameraInstance` |
| `film.h` | `Film` / `FilmInstance` |
| `filter.h` | `Filter` / `FilterInstance` |
| `geometry.h` | `Geometry` (BVH, intersection) |
| `integrator.h` | `Integrator` |
| `light.h` | `Light` / `LightInstance` |
| `lightsampler.h` | `LightSampler` / `LightSamplerInstance` |
| `medium.h` | `Medium` / `MediumInstance` |
| `pipeline.h` | `Pipeline` (resource manager) |
| `sampler.h` | `Sampler` / `SamplerInstance` |
| `shape.h` | `Shape` / `ShapeInstance` |
| `surface.h` | `Surface` / `SurfaceInstance` / `SurfaceClosure` |
| `texture.h` | `Texture` / `TextureInstance` |
| `transform.h` | `Transform` / `TransformInstance` |

**Key type:** `Pipeline` — owns the scene geometry (BVH), all plugin instances,
the render loop, and the `CommandBuffer` pool.

---

### `src/util/` — Utilities
Math helpers, spectrum types, and image I/O primitives shared across modules.

| File | Responsibility |
|---|---|
| `spec.h/.cpp` | `SampledSpectrum` type + math functions |
| `colorspace.h` | XYZ ↔ sRGB ↔ Lab colour space conversions |
| `complex.h` | `Complex<T>` template for Fresnel calculations |
| `imageio.h/.cpp` | `LoadedImage` (EXR, HDR, PNG, … loading/saving) |
| `loop_subdiv.h/.cpp` | Loop subdivision mesh utility |
| `bluenoise.h/.cpp` | Blue-noise sample tables |
| `counter_buffer.h/.cpp` | GPU atomic counter helper |
| `command_buffer.h/.cpp` | Deferred GPU command recording helper |

---

## Plugin modules

Each plugin module is a shared library loaded at runtime.  All plugins follow
the same pattern: implement the abstract base class from `src/base/`, register
with a `LUISA_REGISTER_…` macro.

### `src/cameras/` — Camera models
| Plugin | Description |
|---|---|
| `pinhole.cpp` | Standard pinhole (perspective) camera |
| `orthographic.cpp` | Orthographic projection |
| `thinlens.cpp` | Thin-lens depth-of-field camera |
| `environment.cpp` | Panoramic / equirectangular camera |
| `fisheye.cpp` | Fisheye projection |

### `src/films/` — Film / display outputs
| Plugin | Description |
|---|---|
| `hdr.cpp` | High-dynamic-range EXR/HDR output |
| `color.cpp` | 8-bit sRGB PNG/PPM output |
| `display.cpp` | Interactive OpenGL preview window |

### `src/integrators/` — Light transport integrators
| Plugin | Description |
|---|---|
| `mega_path.cpp` | Megakernel unidirectional path tracer |
| `mega_vpt.cpp` | Megakernel volumetric path tracer |
| `mega_volume_path.cpp` | Megakernel volume path (delta tracking) |
| `mega_vpt_naive.cpp` | Simple VPT (for debugging/comparison) |
| `megapm.cpp` | Stochastic progressive photon mapping |
| `megawave.cpp` | Wavefront path tracer |
| `wave_path.cpp` | Wavefront unidirectional path tracer |
| `wave_path_v2.cpp` | Wavefront path tracer v2 (block compaction) |
| `gpt.cpp` | Gradient-domain path tracing |
| `direct.cpp` | Direct illumination integrator |
| `normal.cpp` | Normal-buffer visualiser |
| `aov.cpp` | Arbitrary output variable renderer |
| `nfor.cpp` | Non-local feature regression denoiser |
| `group.cpp` | Integrator group (multi-pass) |
| `pssmlt.cpp` | Primary Sample Space MLT |
| `wave_path_readback.cpp` | Wavefront path tracer with CPU readback |

### `src/surfaces/` — BSDF / surface models
| Plugin | Description |
|---|---|
| `diffuse.cpp` | Lambertian diffuse |
| `metal.cpp` | Rough conductor (GGX) |
| `glass.cpp` | Rough dielectric |
| `plastic.cpp` | Plastic (diffuse + specular layer) |
| `substrate.cpp` | Substrate model |
| `layered.cpp` | General multi-layer BSDF |
| `disney.cpp` | Disney Principled BSDF |
| `mirror.cpp` | Perfect mirror |
| `null.cpp` | Null (pass-through) surface |

### `src/lights/` — Light sources
| Plugin | Description |
|---|---|
| `area.cpp` | Area light (emissive surface) |
| `point.cpp` | Point light |
| `spot.cpp` | Spot light |
| `directional.cpp` | Directional (sun-like) light |
| `hdr.cpp` | HDRI environment light (used via lightsamplers) |

### `src/environments/` — Environment / sky models
| Plugin | Description |
|---|---|
| `spherical.cpp` | Spherical HDRI environment map |
| `sky.cpp` | Nishita physically-based sky model |

### `src/media/` — Participating media
| Plugin | Description |
|---|---|
| `homogeneous.cpp` | Homogeneous participating medium |

### `src/shapes/` — Geometry shapes
| Plugin | Description |
|---|---|
| `mesh.cpp` | Triangle mesh (via Assimp import) |
| `sphere.cpp` | Analytic sphere |
| `loop_subdiv.cpp` | Loop-subdivision mesh |

### `src/samplers/` — Sample generators
| Plugin | Description |
|---|---|
| `independent.cpp` | Independent uniform sampler |
| `pmj02bn.cpp` | PMJ02bn low-discrepancy sampler |
| `sobol.cpp` | Sobol QMC sampler |

### `src/spectra/` — Spectral upsampling
| Plugin | Description |
|---|---|
| `srgb.cpp` | sRGB → spectrum upsampling |
| `aces.cpp` | ACES colour space |
| `hdr.cpp` | HDR spectral representation |
| `simple.cpp` | Flat single-wavelength spectrum |

### `src/textures/` — Texture sources
| Plugin | Description |
|---|---|
| `constant.cpp` | Constant-value texture |
| `image.cpp` | 2-D image texture (with mipmapping) |
| `checkerboard.cpp` | Procedural checkerboard |

### `src/filters/` — Reconstruction filters
| Plugin | Description |
|---|---|
| `box.cpp` | Box filter |
| `gaussian.cpp` | Gaussian filter |
| `mitchell.cpp` | Mitchell-Netravali filter |
| `triangle.cpp` | Triangle (tent) filter |

### `src/transforms/` — Transforms
| Plugin | Description |
|---|---|
| `rigid.cpp` | Static rigid transform (translate/rotate/scale) |
| `animated.cpp` | Key-framed animated transform |
| `matrix.cpp` | Raw 4×4 matrix transform |

### `src/phasefunctions/` — Phase functions
| Plugin | Description |
|---|---|
| `henyey_greenstein.cpp` | Henyey-Greenstein scattering |
| `isotropic.cpp` | Isotropic (uniform) scattering |

### `src/lightsamplers/` — Light sampling strategies
| Plugin | Description |
|---|---|
| `uniform.cpp` | Uniform light selection |
| `power.cpp` | Power-proportional selection |

### `src/texturemappings/` — UV Mappings
| Plugin | Description |
|---|---|
| `uv.cpp` | UV coordinate from mesh attribute |
| `spherical.cpp` | Spherical UV projection |
| `planar.cpp` | Planar projection |

---

## Support tooling

| Location | Responsibility |
|---|---|
| `cmake/CompilerWarnings.cmake` | Reusable strict warning flags function |
| `cmake/Sanitizers.cmake` | ASan + UBSan opt-in for debug builds |
| `cmake/Dependencies.cmake` | Dependency inventory and future vcpkg hook |
| `.clang-format` | Canonical code formatting rules |
| `.clang-tidy` | Static analysis checks (all enabled as errors) |
| `.editorconfig` | Per-file whitespace/charset rules for editors |
| `.pre-commit-config.yaml` | Pre-commit hooks (clang-format, hygiene checks) |
| `tools/` | Python helper scripts (scene converters, tone-mapping) |

---

## Testing

Tests live in `src/tests/` and are registered with CTest:

| Test binary | Label | What it tests |
|---|---|---|
| `test_u64` | unit | 64-bit arithmetic DSL |
| `test_complex` | unit | `Complex<T>` arithmetic |
| `test_colorspace` | unit | XYZ ↔ sRGB ↔ Lab conversions |
| `test_alias_table_unit` | unit | `create_alias_table()` correctness |
| `test_sky` | integration | Nishita sky model precomputation |
| `test_sphere` | integration | Loop-subdivision icosphere |
| `test_scene_node_tag` | integration | SDL tag parsing + `SceneDesc` API |

Run all unit tests:
```bash
ctest --test-dir build -L unit --output-on-failure
```

Run all integration tests:
```bash
ctest --test-dir build -L integration --output-on-failure
```

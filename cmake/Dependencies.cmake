# cmake/Dependencies.cmake
# ─────────────────────────────────────────────────────────────────────────────
# Centralises all third-party dependency discovery for LuisaRender.
#
# All dependencies are vendored under src/ext/ or pulled transitively through
# the LuisaCompute submodule (src/compute/).  No external package manager is
# required for a standard build.
#
# This file documents the logical dependency graph and provides a single place
# to update versions, policies, or switch to vcpkg/conan if needed in future.
# ─────────────────────────────────────────────────────────────────────────────

# ── Vendored under src/ext/ ──────────────────────────────────────────────────

# assimp  — 3-D model import (OBJ, FBX, GLTF …)
#   Configured in src/ext/CMakeLists.txt; exposed as target luisa-render-ext-assimp
#   Version: bundled (see src/ext/assimp/)

# cxxopts — CLI argument parsing used by src/apps/cli.cpp
#   Header-only; exposed as target luisa-render-ext-cxxopts
#   Version: bundled (see src/ext/cxxopts/)

# fast_float — fast string-to-float parsing used by the SDL parser
#   Header-only; exposed as target luisa-render-ext-fast-float
#   Version: bundled (see src/ext/fast_float/)

# nlohmann/json — JSON serialisation used by the scene description layer
#   Header-only; exposed as target luisa-render-ext-json
#   Version: bundled (see src/ext/json/)

# tinyexr — OpenEXR image I/O
#   Compiled as a single-file library (src/ext/tinyexr.cpp)
#   Version: bundled (see src/ext/tinyexr/)

# ── Via LuisaCompute submodule (src/compute/) ────────────────────────────────

# LuisaCompute — GPU DSL, shader compilation, device abstraction
#   Submodule pinned in .gitmodules; configures as add_subdirectory(src/compute)

# ── System dependencies (required on Linux) ──────────────────────────────────

# These are installed as OS packages and do NOT need explicit find_package()
# calls in CMakeLists.txt because they are pulled in transitively by
# LuisaCompute's own CMake files:
#
#   libxinerama-dev   — X11 multi-monitor (GLFW dependency)
#   libxcursor-dev    — X11 cursor theming (GLFW dependency)
#   libxi-dev         — X11 input extension (GLFW dependency)
#   libxrandr-dev     — X11 RandR extension (GLFW dependency)
#   libvulkan-dev     — Vulkan headers (optional GPU backend)
#   libxkbcommon-dev  — Wayland keyboard (optional, GLFW)
#   uuid-dev          — libuuid used by LuisaCompute asset system

# ── Future: vcpkg / conan integration ────────────────────────────────────────

# If the project migrates to a package manager, this file is the right place
# to add the toolchain file inclusion or conan_cmake_run() call, keeping the
# root CMakeLists.txt clean:
#
#   if (LUISA_RENDER_USE_VCPKG)
#       include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
#   endif ()

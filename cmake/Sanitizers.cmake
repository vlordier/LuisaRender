# cmake/Sanitizers.cmake
# ─────────────────────────────────────────────────────────────────────────────
# Provides a reusable function that enables runtime sanitizers on a CMake
# target when the LUISA_RENDER_ENABLE_SANITIZERS option is ON.
#
# Only Clang and GCC support -fsanitize.  MSVC is silently skipped.
# Sanitizers are only meaningful in Debug / RelWithDebInfo builds.
#
# Usage:
#   luisa_render_enable_sanitizers(<target>)
#
# Sanitizers enabled:
#   AddressSanitizer  (ASan) — heap/stack/global buffer overflows, use-after-free
#   UndefinedBehaviorSanitizer (UBSan) — signed integer overflow, misaligned load, etc.
#
# ThreadSanitizer (TSan) is NOT combined with ASan (they are mutually exclusive).
# Enable TSan separately with:
#   target_compile_options(<target> PRIVATE -fsanitize=thread)
#   target_link_options   (<target> PRIVATE -fsanitize=thread)
# ─────────────────────────────────────────────────────────────────────────────

option(LUISA_RENDER_ENABLE_SANITIZERS
    "Enable AddressSanitizer + UndefinedBehaviorSanitizer in debug builds"
    OFF)

function(luisa_render_enable_sanitizers target)
    if (NOT LUISA_RENDER_ENABLE_SANITIZERS)
        return()
    endif ()

    if (MSVC)
        message(WARNING
            "luisa_render_enable_sanitizers: MSVC does not support -fsanitize; "
            "skipping for target '${target}'.")
        return()
    endif ()

    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        message(WARNING
            "luisa_render_enable_sanitizers: unknown compiler "
            "'${CMAKE_CXX_COMPILER_ID}'; skipping sanitizers for '${target}'.")
        return()
    endif ()

    message(STATUS "Enabling ASan + UBSan on target '${target}'")

    target_compile_options(${target} PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer   # Better stack traces with ASan
        -fno-sanitize-recover=all # Abort on first UBSan violation
    )

    target_link_options(${target} PRIVATE
        -fsanitize=address,undefined
    )
endfunction()

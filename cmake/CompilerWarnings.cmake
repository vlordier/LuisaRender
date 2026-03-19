# cmake/CompilerWarnings.cmake
# ─────────────────────────────────────────────────────────────────────────────
# Provides a reusable function that attaches a consistent, strict set of
# compiler warnings to a CMake target.  Warnings are applied as PRIVATE so
# they do not propagate to consumers of the target.
#
# Usage:
#   luisa_render_set_compiler_warnings(<target>)
#
# Rationale:
#   Global add_compile_options() pollutes every target including vendored
#   third-party code.  Target-scoped options keep warnings surgical.
# ─────────────────────────────────────────────────────────────────────────────

function(luisa_render_set_compiler_warnings target)
    if (MSVC)
        target_compile_options(${target} PRIVATE
            /W4          # Highest reasonable warning level
            /WX          # Treat warnings as errors
            /w14242      # 'identifier': conversion from 'type1' to 'type2' (possible data loss)
            /w14254      # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits'
            /w14263      # Member function does not override any base class virtual member function
            /w14265      # Class has virtual functions but non-virtual destructor
            /w14287      # Unsigned / negative constant mismatch
            /we4289      # Loop-control variable used outside for-loop scope
            /w14296      # Expression is always true/false
            /w14311      # Pointer truncation from 'type1' to 'type2'
            /w14545      # Expression before comma evaluates to a function missing argument list
            /w14546      # Function call before comma missing argument list
            /w14547      # Operator before comma has no effect; expected operator with side-effect
            /w14549      # Operator before comma has no effect; did you intend 'operator'?
            /w14619      # Pragma warning: there is no warning number 'number'
            /w14640      # Thread-unsafe static member initialization
            /w14826      # Conversion from 'type1' to 'type_2' is sign-extended
            /w14905      # Wide string literal cast to 'LPSTR'
            /w14906      # String literal cast to 'LPWSTR'
            /w14928      # Illegal copy-initialization; more than one user-defined conversion applied
        )
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wcast-align          # Potential performance problem from pointer casts
            -Wcast-qual           # Cast discards const/volatile qualifiers
            -Wconversion          # Implicit conversions that may alter a value
            -Wdouble-promotion    # float implicitly promoted to double
            -Wformat=2            # printf/scanf format string mismatches
            -Wimplicit-fallthrough # Switch fall-through without annotation
            -Wmissing-noreturn    # Function should be marked [[noreturn]]
            -Wnon-virtual-dtor    # Virtual function but non-virtual destructor
            -Wnull-dereference    # Detects likely null dereferences
            -Wold-style-cast      # C-style casts in C++ code
            -Woverloaded-virtual  # Hiding base-class virtual function
            -Wshadow              # Local variable shadows another
            -Wsign-conversion     # Signed/unsigned conversion
            -Wunused              # Unused variables, parameters, etc.
        )
        # GCC-only flags
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_compile_options(${target} PRIVATE
                -Wduplicated-branches   # if/else branches with identical code
                -Wduplicated-cond       # Duplicated condition in if/else-if chain
                -Wlogical-op            # Suspicious logical operator use
                -Wmisleading-indentation # Indentation that does not match control flow
                -Wuseless-cast          # Cast to the same type
            )
        endif ()
        # Clang-only flags
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
            target_compile_options(${target} PRIVATE
                -Wno-gnu-zero-variadic-macro-arguments # LuisaCompute DSL uses GNU extension
            )
        endif ()
    endif ()
endfunction()

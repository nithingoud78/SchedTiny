# ARM GNU Toolchain CMake toolchain file for cross-compilation to Cortex-M targets.
# Referenced by: cmake -DCMAKE_TOOLCHAIN_FILE=tools/cmake/arm-none-eabi.cmake
#
# This file sets up cross-compilation without requiring any CMake
# try_compile tests (which would fail without target hardware).

# ---------------------------------------------------------------------------
# Toolchain identification
# ---------------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_PROCESSOR  arm)

# Require ARM GNU toolchain
set(TOOLCHAIN_PREFIX arm-none-eabi)

find_program(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++ REQUIRED)
find_program(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}-gcc REQUIRED)
find_program(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}-objcopy REQUIRED)
find_program(CMAKE_OBJDUMP      ${TOOLCHAIN_PREFIX}-objdump REQUIRED)
find_program(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}-size REQUIRED)

# ---------------------------------------------------------------------------
# Skip compiler validation (cross-compiler cannot run on host)
# ---------------------------------------------------------------------------
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ---------------------------------------------------------------------------
# Sysroot and search paths
# ---------------------------------------------------------------------------
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ---------------------------------------------------------------------------
# Reproducible builds: strip build timestamps from object files
# ---------------------------------------------------------------------------
if(DEFINED ENV{SOURCE_DATE_EPOCH})
    add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)
endif()

message(STATUS "ARM Toolchain: ${CMAKE_C_COMPILER}")

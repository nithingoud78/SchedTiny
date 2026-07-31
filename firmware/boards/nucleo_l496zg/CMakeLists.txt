# SchedTiny — Board: STM32 NUCLEO-L496ZG

# Board-specific CMake configuration for STM32 NUCLEO-L496ZG.
# Included by firmware/CMakeLists.txt when BOARD=nucleo_l496zg.
# Note: Cortex-M4 (no double-precision FPU) — TFLM must use single precision.

set(BOARD_CPU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

set(BOARD_DEFS
    STM32L496xx
    NUCLEO_L496ZG
    HSE_VALUE=8000000U
    SCHEDTINY_BOARD_L496
)

set(BOARD_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/STM32L496ZG_FLASH.ld")
set(BOARD_STARTUP_FILE  "${CMAKE_CURRENT_LIST_DIR}/startup_stm32l496xx.s")

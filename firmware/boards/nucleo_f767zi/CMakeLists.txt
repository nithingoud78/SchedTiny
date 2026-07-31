# SchedTiny — Board: STM32 NUCLEO-F767ZI

# Board-specific CMake configuration for STM32 NUCLEO-F767ZI.
# Included by firmware/CMakeLists.txt when BOARD=nucleo_f767zi.

set(BOARD_CPU_FLAGS
    -mcpu=cortex-m7
    -mthumb
    -mfpu=fpv5-d16
    -mfloat-abi=hard
)

set(BOARD_DEFS
    STM32F767xx
    NUCLEO_F767ZI
    HSE_VALUE=8000000U
    SCHEDTINY_BOARD_F767
)

set(BOARD_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/STM32F767ZI_FLASH.ld")
set(BOARD_STARTUP_FILE  "${CMAKE_CURRENT_LIST_DIR}/startup_stm32f767xx.s")

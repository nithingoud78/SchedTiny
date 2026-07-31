# SchedTiny — Board: STM32 NUCLEO-H743ZI2

# Board-specific CMake configuration for STM32 NUCLEO-H743ZI2.
# Included by firmware/CMakeLists.txt when BOARD=nucleo_h743zi2.

# ---------------------------------------------------------------------------
# CPU Architecture Flags
# ---------------------------------------------------------------------------
set(BOARD_CPU_FLAGS
    -mcpu=cortex-m7
    -mthumb
    -mfpu=fpv5-d16
    -mfloat-abi=hard
)

# ---------------------------------------------------------------------------
# Preprocessor Definitions
# ---------------------------------------------------------------------------
set(BOARD_DEFS
    STM32H743xx
    NUCLEO_H743ZI2
    HSE_VALUE=8000000U   # 8 MHz crystal on Nucleo board
    SCHEDTINY_BOARD_H743
)

# ---------------------------------------------------------------------------
# Linker Script
# ---------------------------------------------------------------------------
set(BOARD_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/STM32H743ZI_FLASH.ld")

# ---------------------------------------------------------------------------
# Startup File
# ---------------------------------------------------------------------------
set(BOARD_STARTUP_FILE "${CMAKE_CURRENT_LIST_DIR}/startup_stm32h743xx.s")

# ---------------------------------------------------------------------------
# Clock Configuration
# ---------------------------------------------------------------------------
# HSE = 8 MHz -> PLL1 -> SYSCLK = 480 MHz
# These values are used in board_config.h to verify at runtime.
# See: SystemCoreClock should equal 480000000U after clock init.

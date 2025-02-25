# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.10)

project(ch32v307_template)

enable_language(CXX)
enable_language(ASM)

# Different linker scripts
set(TARGET_LDSCRIPT_FLASH "${CMAKE_SOURCE_DIR}/Ld/Link.ld")
set(TARGET_LDSCRIPT_RAM   "${CMAKE_SOURCE_DIR}/Ld/Link.ld")

# Copy them from Makefile
set(TARGET_SOURCES
    "SDK/Core/core_riscv.c"
    "SDK/Debug/debug.c"
    "SDK/Peripheral/src/ch32v30x_adc.c"
    "SDK/Peripheral/src/ch32v30x_bkp.c"
    "SDK/Peripheral/src/ch32v30x_can.c"
    "SDK/Peripheral/src/ch32v30x_crc.c"
    "SDK/Peripheral/src/ch32v30x_dac.c"
    "SDK/Peripheral/src/ch32v30x_dbgmcu.c"
    "SDK/Peripheral/src/ch32v30x_dma.c"
    "SDK/Peripheral/src/ch32v30x_dvp.c"
    "SDK/Peripheral/src/ch32v30x_eth.c"
    "SDK/Peripheral/src/ch32v30x_exti.c"
    "SDK/Peripheral/src/ch32v30x_flash.c"
    "SDK/Peripheral/src/ch32v30x_fsmc.c"
    "SDK/Peripheral/src/ch32v30x_gpio.c"
    "SDK/Peripheral/src/ch32v30x_i2c.c"
    "SDK/Peripheral/src/ch32v30x_iwdg.c"
    "SDK/Peripheral/src/ch32v30x_misc.c"
    "SDK/Peripheral/src/ch32v30x_opa.c"
    "SDK/Peripheral/src/ch32v30x_pwr.c"
    "SDK/Peripheral/src/ch32v30x_rcc.c"
    "SDK/Peripheral/src/ch32v30x_rng.c"
    "SDK/Peripheral/src/ch32v30x_rtc.c"
    "SDK/Peripheral/src/ch32v30x_sdio.c"
    "SDK/Peripheral/src/ch32v30x_spi.c"
    "SDK/Peripheral/src/ch32v30x_tim.c"
    "SDK/Peripheral/src/ch32v30x_usart.c"
    "SDK/Peripheral/src/ch32v30x_wwdg.c"
    "SDK/Startup/startup_ch32v30x_D8C.S"
    "src/ch32v30x_it.c"
    "src/freertos_helper.c"
    "src/main.c"
    "src/syscalls.c"
    "src/system_ch32v30x.c"
)

# Copy them from Makefile
set(TARGET_C_DEFINES
    "CH32V30x_D8C"
)

# Copy them from Makefile
set(TARGET_C_INCLUDES
    "SDK/Core"
    "SDK/Debug"
    "SDK/Peripheral/inc"
    "include"
)

# Shared libraries linked with application
set(TARGET_LIBS
    "freertos_kernel"
    "c"
    "m"
    "nosys"
)

# Shared library and linker script search paths
set(TARGET_LIB_DIRECTORIES
)

# Conditional flags
# DEBUG
set(CMAKE_C_FLAGS_DEBUG     "-DDEBUG -O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG   "-DDEBUG -O0 -g")
set(CMAKE_ASM_FLAGS_DEBUG   "-DDEBUG -O0 -g")

# RELEASE
set(CMAKE_C_FLAGS_RELEASE        "-DNDEBUG -O2 -flto")
set(CMAKE_CXX_FLAGS_RELEASE      "-DNDEBUG -O2 -flto")
set(CMAKE_ASM_FLAGS_RELEASE      "-DNDEBUG -O2 -flto")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")

# Final compiler flags
set(CMAKE_C_FLAGS          "${CMAKE_C_FLAGS} -Wall -fno-common -fno-builtin -ffreestanding -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS} -Wall -fno-common -fno-builtin -ffreestanding -fdata-sections -ffunction-sections")
set(CMAKE_ASM_FLAGS        "${CMAKE_C_FLAGS} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

# Include sub directories here
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(FREERTOS_PORT GCC_RISC_V_GENERIC CACHE STRING "")
set(FREERTOS_RISCV_EXTENSION WCH_RISC_V4A_FPU CACHE STRING "")
set(FREERTOS_HEAP "4" CACHE STRING "")
add_subdirectory(lib/FreeRTOS-Kernel)


# Shared sources, includes and definitions
add_compile_definitions(${TARGET_C_DEFINES})
include_directories(${TARGET_C_INCLUDES})
link_directories(${TARGET_LIB_DIRECTORIES})
link_libraries(${TARGET_LIBS})

# Main targets are added here

# Create ELF
add_executable("${CMAKE_PROJECT_NAME}_FLASH.elf" ${TARGET_SOURCES})
target_compile_definitions("${CMAKE_PROJECT_NAME}_FLASH.elf"
        PRIVATE ${TARGET_C_DEFINES_XIP}
        )
target_link_options("${CMAKE_PROJECT_NAME}_FLASH.elf"
        PRIVATE "-T${TARGET_LDSCRIPT_FLASH}"
        PRIVATE "-Wl,--Map=${CMAKE_PROJECT_NAME}_FLASH.map"
        )
set_property(TARGET "${CMAKE_PROJECT_NAME}_FLASH.elf" APPEND
        PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_PROJECT_NAME}_FLASH.map"
        )
add_custom_command(OUTPUT "${CMAKE_PROJECT_NAME}_FLASH.hex"
        COMMAND ${CMAKE_OBJCOPY} "-O" "ihex" "${CMAKE_PROJECT_NAME}_FLASH.elf" "${CMAKE_PROJECT_NAME}_FLASH.hex"
        DEPENDS "${CMAKE_PROJECT_NAME}_FLASH.elf"
        )
add_custom_target("${CMAKE_PROJECT_NAME}_FLASH_HEX" DEPENDS "${CMAKE_PROJECT_NAME}_FLASH.hex")
if(DEFINED TARGET_TOOLCHAIN_SIZE)
    add_custom_command(TARGET "${CMAKE_PROJECT_NAME}_FLASH.elf" POST_BUILD
            COMMAND ${TARGET_TOOLCHAIN_SIZE} "${CMAKE_PROJECT_NAME}_FLASH.elf"
            )
endif()

# Create ELF
add_executable("${CMAKE_PROJECT_NAME}_RAM.elf" ${TARGET_SOURCES})
target_link_options("${CMAKE_PROJECT_NAME}_RAM.elf"
        PRIVATE "-T${TARGET_LDSCRIPT_RAM}"
        PRIVATE "-Wl,--Map=${CMAKE_PROJECT_NAME}_RAM.map"
        )
set_property(TARGET "${CMAKE_PROJECT_NAME}_RAM.elf" APPEND
        PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_PROJECT_NAME}_RAM.map"
        )
add_custom_command(OUTPUT "${CMAKE_PROJECT_NAME}_RAM.hex"
        COMMAND ${CMAKE_OBJCOPY} "-O" "ihex" "${CMAKE_PROJECT_NAME}_RAM.elf" "${CMAKE_PROJECT_NAME}_RAM.hex"
        DEPENDS "${CMAKE_PROJECT_NAME}_RAM.elf"
        )
add_custom_target("${CMAKE_PROJECT_NAME}_RAM_HEX" DEPENDS "${CMAKE_PROJECT_NAME}_RAM.hex")
if(DEFINED TARGET_TOOLCHAIN_SIZE)
    add_custom_command(TARGET "${CMAKE_PROJECT_NAME}_RAM.elf" POST_BUILD
            COMMAND ${TARGET_TOOLCHAIN_SIZE} "${CMAKE_PROJECT_NAME}_RAM.elf"
            )
endif()

# Convert to BIN

add_custom_target(
    "${CMAKE_PROJECT_NAME}_FLASH.bin"
    ALL
    DEPENDS "${CMAKE_PROJECT_NAME}_FLASH.elf"
    COMMAND ${CMAKE_OBJCOPY} "-O" "binary" "${CMAKE_PROJECT_NAME}_FLASH.elf" "${CMAKE_PROJECT_NAME}_FLASH.bin"
)

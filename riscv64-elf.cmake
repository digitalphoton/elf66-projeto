# Poor old Windows...
if(WIN32)
    set(CMAKE_SYSTEM_NAME "Generic")
endif()

set(CMAKE_C_COMPILER   riscv64-elf-gcc)
set(CMAKE_CXX_COMPILER riscv64-elf-g++)

# Optionally set size binary name, for elf section size reporting.
set(TARGET_TOOLCHAIN_SIZE arm-none-eabi-size)

set(CMAKE_C_FLAGS_INIT          "-march=rv32imafc -mabi=ilp32f")
set(CMAKE_CXX_FLAGS_INIT        "-march=rv32imafc -mabi=ilp32f")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-specs=nano.specs -specs=nosys.specs -nostartfiles -nostdlib -Wl,--print-memory-usage -Wl,--no-warn-rwx-segments")

# Make CMake happy about those compilers
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
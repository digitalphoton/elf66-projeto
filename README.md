<!-- SPDX-License-Identifier: MIT -->

# FreeRTOS demo for WCH CH32V30x series MCU

## How to use
* Write your own CMake toolchain file (see `riscv64-elf.cmake` for example)
* Configure project and build as usual. (see below for a simple test)

## Sample usage
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=riscv64-elf.cmake ..
make -j${nprocs}
```

## Notes

### Startup files
There are two versions of startup assembly files, located at
* `BSP/Startup/startup_ch32v30x_D8.S`
* `BSP/Startup/startup_ch32v30x_D8C.S`

The first file is used for CH32V303 devices, which lacks of the following peripherals:
* Ethernet
* CAN2
* USBHS
* DVP

*** Change the startup file to the correct version for the hardware in use. ***

### Compilers
These MCUs uses something called "RISC-V4F" core with Chinese documentation provided:
* Implemented RV32IMAFC ABI

Here is the list of features proprietary or incompatible with upstream toolchain:
* Fast IRQ handlers: uses a special attribute of `interrupt`, which value is `WCH-Interrupt-fast`
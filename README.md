# Domoriks Firmware

STM32G0-based firmware for Domoriks devices.

## Target hardware
- MCU: STM32G030F6Px (FLASH ~28KB, RAM ~8KB)

## Quick start (STM32CubeIDE / Windows)
Prerequisites:
- STM32CubeIDE (recommended)
- ST-Link or compatible programmer

Steps:
1. Open `domoriks_firmware.ioc` in STM32CubeIDE.
2. Configure your debug probe (ST-Link) and build configuration (Release/Debug).
3. Build the project via **Project → Build** or press `Ctrl+B`.
4. Flash using **Run → Debug** or use STM32CubeProgrammer to load the generated ELF/bin from the `Release/` folder.

## CLI build (arm-none-eabi)
A release makefile exists in the `Release/` folder. To build from CLI (GNU Arm toolchain required):

```sh
make -C Release
```

The produced binary/ELF will be in `Release/` (see `makefile` for exact artifact names).

## Notes & Known issues
- Some features are TODO (PWM stubs, extended Modbus tests). See `Core/Src/IO/outputs.c` for TODO markers.
- The firmware expects to be flashed to a device with sufficient erased flash sectors for user data. The project includes helper functions in `Core/Src/Flash/`.

## Development
- Static analysis: add `cppcheck` and `clang-tidy` in CI.
- Tests: consider adding host-side tests using `Core/Platform_linux/` transport.

## License
This repository is available under the MIT License (see `LICENSE`).

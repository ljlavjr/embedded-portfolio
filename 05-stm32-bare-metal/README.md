# STM32 Bare-Metal Blink

**Description:** This project is an introduction to ARM development on a STM32 board. We learn how to set up the board using our own linker and startup files and how to address registers in the Cortex MCU using manual memory addressing. Then with everything set up we blink one of the on-board LEDs.

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4)
- **Clock Speed**: 16MHz (default HSI, no PLL configured)
- **Flash**: 1MB
- **RAM**: 192KB
- **LED Used**: PD12 (Green)

## STM32F407 Architecture

### Memory Map
| Region | Start Address | Size | Purpose |
|--------|---------------|------|---------|
| Flash | 0x08000000 | 1024KB | Program code, constants, vector table |
| RAM | 0x20000000 | 128KB | Variables, stack |

The Cortex-M4 boots from 0x08000000. The first two words at that address are:
1. Initial stack pointer (top of RAM)
2. Reset handler address

### Boot Process
1. **Power on** — Cortex-M4 reads first two words from 0x08000000
2. **Load SP** — First word loaded into Stack Pointer (we provide _estack, top of RAM)
3. **Load PC** — Second word loaded into Program Counter (Reset_Handler address)
4. **Reset_Handler runs:**
   - Copies .data section from flash to RAM (initialized globals)
   - Zeros .bss section in RAM (uninitialized globals)
   - Calls main()
5. **main() runs** — Your code takes over
6. **If main returns** — Infinite loop (hang) to prevent undefined behavior

## Build

### Prerequisites
This is what should be installed (using *sudo apt install ...*):
- **gcc-arm-none-eabi**
- **binutils-arm-none-eabi**
- **libnewlib-arm-none-eabi**
- **stlink-tools**
- **make**
### Compile
```bash
make
```
### Flash
```bash
st-flash write main.bin 0x08000000
```
**NOTE**: You may run into permission issues. To fix this, you have two options:

*Quick Fix*:
```bash
sudo st-flash write main.bin 0x08000000
```
*Proper Fix (udev rules so you do not need sudo)*:
```bash
sudo nano /etc/udev/rules.d/99-stlink.rules
```
Add:
```bash
SUBSYSTEM=="usb", ATTR{idVendor}=="0483", ATTR{idProduct}=="3748", MODE="0666"
SUBSYSTEM=="usb", ATTR{idVendor}=="0483", ATTR{idProduct}=="374b", MODE="0666"
```
Then reload:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```
## Files Explained

### linker.ld
This file defines the memory layout, entry point, and sections of our Cortex-M4 chip. Unlike AVR, we need to define all of this manually which gives us full control but more responsibility. 

In the MEMORY section we define the ORIGIN address of our FLASH and RAM storage as well as the size of each of them. In ENTRY we define the assembly code method in startup.s that we call on system turn on. 

In SECTIONS we define where to place code and data on startup:
- **.isr_vector** — Vector table, must be first in FLASH (address 0x08000000)
- **.text** — Program code, stored in FLASH
- **.rodata** — Read-only data (const variables), stored in FLASH
- **.data** — Initialized globals, stored in FLASH but copied to RAM at startup
- **.bss** — Uninitialized globals, located in RAM, zeroed at startup

The linker script also exports symbols (_sdata, _edata, _sidata, _sbss, _ebss, _estack) that startup.s uses to know where to copy and zero memory.

### startup.s
This is assembly code that runs before main(). It handles the low-level setup that C can't do itself.

**Vector Table:**
- Placed at 0x08000000 (start of flash)
- First entry: initial stack pointer (_estack from linker script)
- Second entry: address of Reset_Handler

**Reset_Handler:**
1. Copies .data section from flash to RAM using _sidata (source), _sdata (dest start), _edata (dest end)
2. Zeros .bss section using _sbss (start) and _ebss (end)
3. Calls main() with `bl main`
4. If main ever returns, enters infinite loop to prevent undefined behavior

This is code you normally never see—on AVR or when using vendor libraries, it's provided for you. Writing it yourself teaches you exactly how the chip boots.

### main.c
This is the C code that runs our program for blinking the green LED. It defines our registers as macros to be accessed by casting the memory address as a pointer and then dereferencing that same pointer.

**Registers used:**
- **RCC_AHB1ENR** (0x40023830) — Enables peripheral clocks. We set bit 3 to turn on GPIOD clock.
- **GPIOD_MODER** (0x40020C00) — Configures pin modes. Each pin uses 2 bits (00=input, 01=output). We set bits 25-24 to 01 for PD12 output.
- **GPIOD_ODR** (0x40020C14) — Output data register. Writing to bit 12 controls PD12 state.

**Key difference from AVR:** On STM32, peripherals are disabled by default to save power. You must enable the GPIO clock in RCC before the port will respond. Forgetting this is a common mistake.

**Program flow:**
1. Enable GPIOD clock
2. Configure PD12 as output
3. Loop forever, toggling PD12 and delaying

## What I Learned
**The complex difference between AVR and ARM**: During this project, I learned first how to program a bare-metal function on the STM32 board. Unlike ARM, a lot needs to be done manual at first before you can write you main.c program. There is no library you can include at the top of main.c that does this for you like in AVR. You need to provide your own linker.ld and startup.s files as well as defining your own macros for the register's memory addresses. ARM makes you dig deep into the reference manual to find starting memory location of the main registers and calculate the offsets of each specific bit to get the right address to modify. 

**Peripherals need clocks enabled**: On STM32, GPIO ports are off by default. You must enable the clock in RCC_AHB1ENR before the port responds. This tripped me up at first—on AVR, ports are always available.

**The vector table**: The Cortex-M4 boot process relies on a vector table at 0x08000000. The first word is the stack pointer, the second is the reset handler address. Understanding this explains why startup.s and linker.ld must work together.

**Toolchain differences**: ARM uses separate tools (arm-none-eabi-gcc, arm-none-eabi-as, arm-none-eabi-ld) and requires -nostdlib since we have no standard library. The output is .bin instead of .hex.

## Resources
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F407G-DISC1 User Manual](https://www.st.com/resource/en/user_manual/um1472-discovery-kit-with-stm32f407vg-mcu-stmicroelectronics.pdf)
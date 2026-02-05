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
SUBSYSTEM=="usb", ATTR{idVendor}=="0483", ATTR{idProdcut}=="374b", MODE="0666"
```
Then reload:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```
## Files Explained

### linker.ld

### startup.s

### main.c

## What I Learned

## Resources
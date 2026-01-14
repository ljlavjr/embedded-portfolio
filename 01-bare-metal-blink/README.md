# Bare Metal Blink
**Description:** This project writes a blink program using only AVR-GCC and direct register manipulation. Beginning project to relearn the fundamentals of embedded development.

## Hardware
The hardware used for this project:
- **Chip**: *ATmega2560*
- **Clock Speed**: *16Mhz*
- **Pins**: *Port B pin 7 (PB7)*

## ATmega2560 Architecture
### Memory Map
- **Registers**: 0x000 - 0x001F (32 general purpose registers)
- **I/O Registers**: 0x0020 - 0x005F (64 registers)
- **External I/O Registers**: 0x0060 - 0x01FF (416 registers)
- **Internal SRAM**: 0x0200 - 0x21FF (8192 x 8)
- **External SRAM**: 0x2200 - 0xFFFF (0 - 64k x 8)

### Clock System
The ATmega2560 defaults to an internal 8MHz RC oscillator divided down to 1MHz. The Arduino Mega overrides this using an external 16MHz crystal. F_CPU must be defined so functions like `_delay_ms()` can calculate accurate timing and without it, delays would be wrong by a factor of 16.

## Build
How to build this project:
### Prerequisites
This is what should be installed (using *sudo apt install ...*):
- **gcc-avr**
- **avr-libc**
- **avrdude**
- **make**
### Compile
```bash
make
```
### Flash
```bash
make flash
```

## What I Learned
### GPIO registers (DDRx, PORTx)
GPIO registers are configured and accessed differently. The DDRx register sets whether a pin at a specific GPIO port is an output pin or an input pin. If it is an output pin, DDRx is set to logical high (1) like this: 
```c
// Setting Pin 7 on port B as output
DDRB |= (1 << PB7);
```
If it is an input pin , DDRx is set to logical low (0) like this:
```c
// Setting Pin 7 on port B as input
DDRB &= ~(1 << PB7);
```
The PORTx register sets the value of the port to either logical high (1) or logical low (0) like this:
```c
// Setting Pin 7 on port B as logical high if PB7 is an output
PORTB |= (1 << PB7);
```

## Resources
- [ATmega2560 Datasheet] (https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)
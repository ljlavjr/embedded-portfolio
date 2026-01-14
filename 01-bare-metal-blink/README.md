# Bare Metal Blink
**Description:** This project writes a blink program using only AVR-GCC and direct register manipulation. Beginning project to relearn the fundamentals of embedded development.

## Hardware
The hardware used for this project:
- **Chip**: *ATmega328p*
- **Clock Speed**: *16Mhz*
- **Pins**: *Port B pin 5 (PB5)*

## ATmega328P Architecture
### Memory Map
### Clock System

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
// Setting Pin 5 on port B as output
DDRB |= (1 << PB5);
```
If it is an input pin , DDRx is set to logical low (0) like this:
```c
// Setting Pin 5 on port B as input
DDRB &= ~(1 << PB5);
```
The PORTx register sets the value of the port to either logical high (1) or logical low (0) like this:
```c
// Setting Pin 5 on port B as logical high if PB5 is an output
PORTB |= (1 << PB5);
```

## Resources
- [ATmega328p Datasheet] (https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)